#pragma once
#include <ucsl/bitset.h>
#include <ucsl/magic.h>
#include <rip/binary/stream.h>
#include <rip/util/byteswap.h>

namespace rip::binary::containers::mirage::v1 {
    /*
     * Data layout is as follows:
     * 
     * ----
     * FileHeader
     * ---- (size of this whole chunk = dataSize)
     * Data <- base address of offset locations in offset table
     * ----
     * unsigned int offsetTableSize
     * unsigned int offsetLocations[offsetTableSize]
     * ---
     */
    struct FileHeader {
        unsigned int fileSize; // if MSB is not set, this is a V1 mirage container
        unsigned int version;
        unsigned int dataSize;
        unsigned int headerSize;
        unsigned int offsetTableOffset;
    };
}

namespace rip::util {
    template<> inline void byteswap_deep(rip::binary::containers::mirage::v1::FileHeader& value) noexcept {
        byteswap_deep(value.fileSize);
        byteswap_deep(value.version);
        byteswap_deep(value.dataSize);
        byteswap_deep(value.headerSize);
        byteswap_deep(value.offsetTableOffset);
    }
}

namespace rip::binary::containers::mirage::v1 {
    template<typename AddressType>
    class MirageResourceImageReader {
    private:
        fast_istream raw_stream;
        binary_istream<AddressType> stream;
        FileHeader header;

    public:
        class data_istream : public binary_istream<AddressType> {
        public:
            data_istream(fast_istream& raw_stream, std::endian endianness, size_t offset) : binary_istream<AddressType>{ raw_stream, endianness, offset } { }
        };

        MirageResourceImageReader(std::istream& stream_) : raw_stream{ stream_ }, stream{ raw_stream, std::endian::big } {
            stream.read(header);
            stream.skip_padding_bytes(header.headerSize - sizeof(FileHeader));
        }

        data_istream get_data() {
            return { raw_stream, std::endian::big, stream.tellg() };
        }
    };

    template<typename AddressType, std::endian endianness = std::endian::big>
    class MirageResourceImageWriter {
    private:
        fast_ostream raw_stream;
        binary_ostream<AddressType, endianness> stream;
        std::vector<unsigned int> addressLocations{};

    public:
        class data_ostream : public binary_ostream<AddressType, endianness> {
        protected:
            MirageResourceImageWriter& writer;

        public:
            static constexpr bool hasNativeStrings = false;

            data_ostream(MirageResourceImageWriter& writer, size_t offset) : writer{ writer }, binary_ostream<AddressType, endianness>{ writer.raw_stream, offset } { }

            ~data_ostream() {
                finish();
            }

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
            }
        };

        MirageResourceImageWriter(std::ostream& stream_) : raw_stream{ stream_ }, stream{ raw_stream } {
            stream.write(FileHeader{});
            stream.write_padding_bytes(0x18 - sizeof(FileHeader));
        }

        ~MirageResourceImageWriter() {
            finish();
        }

        data_ostream add_data(unsigned int version) {
            return { *this, stream.tellp() };
        }

        void finish() {
            size_t offsetTableOffset = stream.tellp();
            writeAddressResolutionChunk();
            size_t pos = stream.tellp();

            FileHeader fileHeader{};
            fileHeader.fileSize = static_cast<unsigned int>(pos);
            fileHeader.version = 3;
            fileHeader.dataSize = offsetTableOffset - 0x18;
            fileHeader.headerSize = 0x18;
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
