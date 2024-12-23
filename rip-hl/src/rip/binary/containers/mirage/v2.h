#pragma once
#include <ucsl/bitset.h>
#include <ucsl/magic.h>
#include <rip/binary/stream.h>
#include <rip/util/byteswap.h>

namespace rip::binary::containers::mirage::v2 {
    struct NodeHeader {
        enum Flag : unsigned int {
            LEAF = 0x20000000,
            LAST_CHILD = 0x40000000,
        };

        unsigned int nodeSizeAndFlags;
        unsigned int version;
		ucsl::magic_t<8> magic;
    };

    /*
     * File structure is as follows:
     * ---
     * FileHeader
     * ---
     * NodeHeader <- base address of offsets in offset table
     * node data (can be more nodes if it is not a leaf)
     * ---
     * offset table
     */
    struct FileHeader {
        unsigned int fileSize; // MSB is set for a V2 mirage container
        unsigned int magic;
        uint32_t offsetTableOffset;
        uint32_t offsetCount;
    };
}

namespace rip::util {
    template<> inline void byteswap_deep(rip::binary::containers::mirage::v2::NodeHeader& value) noexcept {
        byteswap_deep(value.nodeSizeAndFlags);
        byteswap_deep(value.version);
    }

    template<> inline void byteswap_deep(rip::binary::containers::mirage::v2::FileHeader& value) noexcept {
        byteswap_deep(value.fileSize);
        byteswap_deep(value.magic);
        byteswap_deep(value.offsetTableOffset);
        byteswap_deep(value.offsetCount);
    }
}

namespace rip::binary::containers::mirage::v2 {
	template<typename AddressType>
    class MirageResourceImageReader {
        class node_istream : public binary_istream<AddressType> {
            size_t nodeOffset{};

        public:
            NodeHeader header;

            node_istream(fast_istream& raw_stream, std::endian endianness) : binary_istream<AddressType>{ raw_stream, endianness, 0x10 }, nodeOffset{ this->tellg() } {
                this->read(header);
            }

            bool isLastChild() const {
                return header.nodeSizeAndFlags & NodeHeader::LAST_CHILD;
            }

            template<typename F>
            void forEachChild(F f) {
                assert((header.nodeSizeAndFlags & NodeHeader::LEAF) == 0 && "not a branch node");

                bool isLast{};
                do {
                    node_istream child{ this->stream, this->endianness };

                    f(child);

                    this->seekg(nodeOffset + (header.nodeSizeAndFlags & 0x1FFFFFFF));
                    this->skip_padding(16);
                    isLast = child.isLastChild();
                } while (!isLast);
            }
        };

    private:
        fast_istream raw_stream;
        binary_istream<AddressType> stream;
        FileHeader header;
        std::endian endianness{ std::endian::big };

    public:
        MirageResourceImageReader(std::istream& stream_) : raw_stream{ stream_ }, stream{ raw_stream } {
            stream.read(header);
        }

        node_istream get_root_node() {
            return { raw_stream, endianness };
        }
    };

    template<typename AddressType, std::endian endianness = std::endian::big>
    class MirageResourceImageWriter {
    private:
        fast_ostream raw_stream;
        binary_ostream<AddressType, endianness> stream;
        std::vector<unsigned int> addressLocations{};

    public:
        class node_ostream : public binary_ostream<AddressType, endianness> {
        protected:
            MirageResourceImageWriter& writer;
            const ucsl::magic_t<8> magic{};
            unsigned int version{};
            size_t nodeOffset{};
            unsigned int flags{};

        public:
            node_ostream(const ucsl::magic_t<8>& magic, unsigned int version, MirageResourceImageWriter& writer, unsigned int flags, bool is_last = false) : magic{ magic }, version{ version }, writer{ writer }, binary_ostream<AddressType, endianness>{ writer.raw_stream, 0x10 }, nodeOffset{ this->tellp() }, flags{ flags | (is_last ? NodeHeader::LAST_CHILD : 0) } {
                this->write(NodeHeader{});
            }

            ~node_ostream() {
                finish();
            }

            static constexpr bool hasNativeStrings = false;

            template<typename T>
            void write(const T& obj) {
                binary_ostream<AddressType, endianness>::write(obj);
            }

            template<typename T>
            void write(const offset_t<T>& obj) {
                if (obj.has_value())
                    writer.addressLocations.push_back(static_cast<unsigned int>(this->tellp()));

                binary_ostream<AddressType, endianness>::write(obj);
            }

            void finish() {
                this->write_padding(4);
                size_t nodeEnd = this->tellp();

                NodeHeader nodeHeader{};
                nodeHeader.nodeSizeAndFlags = static_cast<unsigned int>(nodeEnd - nodeOffset) | flags;
                nodeHeader.version = version;
                nodeHeader.magic = magic;

                this->seekp(nodeOffset);
                this->write(nodeHeader);
                this->seekp(nodeEnd);

                this->write_padding(16);
            }
        };

        class leaf_node_ostream : public node_ostream {
        public:
            leaf_node_ostream(const ucsl::magic_t<8>& magic, unsigned int version, MirageResourceImageWriter& writer, unsigned int flags) : node_ostream{ magic, version, writer, NodeHeader::LEAF | flags } {}
        };

        class branch_node_ostream : public node_ostream {
        public:
            branch_node_ostream(const ucsl::magic_t<8>& magic, unsigned int version, MirageResourceImageWriter& writer, unsigned int flags) : node_ostream{ magic, version, writer, flags } {}

            branch_node_ostream add_branch_node(const ucsl::magic_t<8>& magic, unsigned int version) {
                return { magic, version, this->writer, 0 };
            }

            branch_node_ostream add_last_branch_node(const ucsl::magic_t<8>& magic, unsigned int version) {
                return { magic, version, this->writer, NodeHeader::LAST_CHILD };
            }

            leaf_node_ostream add_leaf_node(const ucsl::magic_t<8>& magic, unsigned int version) {
                return { magic, version, this->writer, 0 };
            }

            leaf_node_ostream add_last_leaf_node(const ucsl::magic_t<8>& magic, unsigned int version) {
                return { magic, version, this->writer, NodeHeader::LAST_CHILD };
            }
        };

    public:
        MirageResourceImageWriter(std::ostream& stream_) : raw_stream{ stream_ }, stream{ raw_stream } {
            stream.write(FileHeader{});
        }

        ~MirageResourceImageWriter() {
            finish();
        }

        branch_node_ostream add_root_node(const ucsl::magic_t<8>& magic, unsigned int version) {
            return { magic, version, *this, NodeHeader::LAST_CHILD };
        }

        void finish() {
            size_t offsetTableOffset = stream.tellp();
            writeAddressResolutionChunk();
            size_t pos = stream.tellp();

            FileHeader fileHeader{};
            fileHeader.fileSize = static_cast<unsigned int>(pos) | 0x80000000;
            fileHeader.magic = 0x0133054A;
            fileHeader.offsetCount = addressLocations.size();
            fileHeader.offsetTableOffset = offsetTableOffset;

            stream.seekp(0);
            stream.write(fileHeader);
            stream.seekp(pos);
        }

        void writeAddressResolutionChunk() {
            for (unsigned int addressLocation : addressLocations)
                stream.write(addressLocation);
        }
    };
}
