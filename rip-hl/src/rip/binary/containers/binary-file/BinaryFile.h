#pragma once
#include <bit>
#include <map>
#include <ucsl/magic.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/stream.h>
#include <rip/util/byteswap.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <iostream>
#include <vector>

namespace rip::binary::containers::binary_file::v2 {
	struct FileHeader {
		ucsl::magic_t<4> magic;
		ucsl::magic_t<3> version;
		char endianness;
		unsigned int fileSize;
		unsigned short chunkCount;
		unsigned char flags;

		inline void byteswap_deep() noexcept {
			rip::byteswap_deep(fileSize);
			rip::byteswap_deep(chunkCount);
		}
	};

	struct ChunkHeader {
		ucsl::magic_t<4> magic;
		unsigned int size;
		unsigned int dataSize;
		unsigned int stringTableSize;
		unsigned int offsetTableSize;
		unsigned short additionalHeaderSize;

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
		const ucsl::magic_t<4> magic{};
		size_t chunkOffset{};
		unsigned short additionalHeaderSize{};
		std::vector<std::string> strings{}; // This seems superfluous but it is here to keep the discovery order, to generate a file that is closer to official files.
		std::map<std::string, std::vector<size_t>> stringOffsets{};
		std::vector<size_t> offsets{};

		void writeStringTable();
		void writeOffsetTable();

	public:
		static constexpr bool hasNativeStrings = true;

		chunk_ostream(const ucsl::magic_t<4>& magic, binary_ostream& stream, unsigned short additionalHeaderSize = 0x18);
		~chunk_ostream();

		template<typename T> void write(const T& obj) {
			stream.write(obj);
		}

		template<> void write(const char* const& obj) {
			if (obj != nullptr) {
				auto i = stringOffsets.find(obj);

				if (i == stringOffsets.end()) {
					strings.emplace_back(obj);
					stringOffsets[obj] = { tellp() };
				}
				else
					i->second.emplace_back(tellp());

				offsets.emplace_back(tellp());
			}

			stream.write(0ull);
		}

		template<typename T> void write(const serialized_types::o64_t<T>& obj) {
			offsets.emplace_back(tellp());
			stream.write(obj.has_value() ? obj.value() : 0ull);
		}

		template<typename T> void write(const serialized_types::o32_t<T>& obj) {
			offsets.emplace_back(tellp());
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

	class BinaryFileSerializer {
		BinaryFileWriter container;

	public:
		BinaryFileSerializer(binary_ostream& stream, std::endian endianness = std::endian::native);

		template<typename T, typename R>
		void serialize(T& data, R refl) {
			auto chunk = container.addDataChunk();

			rip::binary::ReflectionSerializer serializer{ chunk };

			serializer.serialize(data, refl);
		}

		template<typename GameInterface, typename T>
		void serialize(T& data) {
			serialize(data, ucsl::reflection::providers::simplerfl<GameInterface>::template reflect<T>());
		}
	};
}
