#pragma once
#include <bit>
#include <rip/byteswap.h>
#include <rip/binary/stream.h>
#include <rip/util/magic.h>
#include <iostream>

namespace rip::binary::containers::binary_file::v2 {
	struct FileHeader {
		magic_t<4> magic{};
		magic_t<3> version{};
		char endianness{};
		unsigned int fileSize{};
		unsigned short chunkCount{};
		unsigned char flags{};

		inline void byteswap_deep() noexcept {
			rip::byteswap_deep(fileSize);
			rip::byteswap_deep(chunkCount);
		}
	};

	struct ChunkHeader {
		magic_t<4> magic{};
		unsigned int size{};
		unsigned int dataSize{};
		unsigned int stringTableSize{};
		unsigned int offsetTableSize{};
		unsigned short additionalHeaderSize{};

		inline void byteswap_deep() noexcept {
			rip::byteswap_deep(size);
			rip::byteswap_deep(dataSize);
			rip::byteswap_deep(stringTableSize);
			rip::byteswap_deep(offsetTableSize);
			rip::byteswap_deep(additionalHeaderSize);
		}
	};

	class chunk_ostream : public offset_binary_ostream {
		binary_ostream& stream;
		const magic_t<4> magic{};
		size_t chunkOffset{};
		unsigned short additionalHeaderSize{};
		std::vector<std::pair<size_t, std::string>> strings{};
		std::vector<size_t> offsets{};

		void writeStringTable();
		void writeOffsetTable();

	public:
		static constexpr bool hasNativeStrings = true;

		chunk_ostream(const magic_t<4>& magic, binary_ostream& stream, unsigned short additionalHeaderSize = 0x18);
		~chunk_ostream();

		template<typename T> void write(const T& obj) {
			std::cout << "w DEFAULT " << std::hex << &obj << " as " << obj << std::endl;

			stream.write(obj);
		}

		template<> void write(const char* const& obj) {
			std::cout << "w STR " << std::hex << &obj << " as " << obj << std::endl;

			if (obj != nullptr) {
				strings.push_back({ tellp(), obj });
				offsets.push_back(tellp());
			}

			stream.write(0ull);
		}

		template<typename T> void write(const serialized_types::o64_t<T>& obj) {
			std::cout << "w OFF64 " << std::hex << &obj << " as " << obj << std::endl;

			if (obj.has_value())
				offsets.push_back(tellp());

			stream.write(obj.has_value() ? obj.value() : 0ull);
		}

		template<typename T> void write(const serialized_types::o32_t<T>& obj) {
			std::cout << "w OFF32 " << std::hex << &obj << " as " << obj << std::endl;

			if (obj.has_value())
				offsets.push_back(tellp());

			stream.write(obj.has_value() ? obj.value() : 0u);
		}

		void finish();
	};

	class BinaryFileWriter {
		binary_ostream& stream;
		std::endian endianness;
		unsigned short chunkCount{};

	public:
		BinaryFileWriter(binary_ostream& stream, std::endian endianness = std::endian::native);
		~BinaryFileWriter();
		chunk_ostream addDataChunk();
		void finish();
	};

	class BinaryFileResolver {
		FileHeader* file;

		template<typename F>
		void forEachChunk(F f);

		void doByteswaps();
		void resolveAddresses();

	public:
		BinaryFileResolver(void* file);
		void* getData(unsigned short chunkId);
	};
}
