#pragma once
#include <bit>
#include <map>
#include <ucsl/magic.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/stream.h>
#include <rip/util/byteswap.h>
#include <rip/binary/serialization/ReflectionDeserializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <iostream>
#include <vector>
#include "common.h"

namespace rip::binary::containers::binary_file::v1 {
	struct FileHeader {
		unsigned int size;
		unsigned int dataSize;
		unsigned int offsetTableSize;
		unsigned int padding1;
		unsigned short flags;
		unsigned short footerCount;
		ucsl::magic_t<3> version;
		char endianness;
		ucsl::magic_t<4> magic;
		unsigned int padding2;

		inline void byteswap_deep() noexcept {
			util::byteswap_deep(size);
			util::byteswap_deep(dataSize);
			util::byteswap_deep(offsetTableSize);
			util::byteswap_deep(footerCount);
		}
	};

	//class chunk_ostream : public offset_binary_ostream {
	//	binary_ostream& stream;
	//	const ucsl::magic_t<4> magic{};
	//	size_t chunkOffset{};
	//	unsigned short additionalHeaderSize{};
	//	std::vector<std::string> strings{}; // This seems superfluous but it is here to keep the discovery order, to generate a file that is closer to official files.
	//	std::map<std::string, std::vector<size_t>> stringOffsets{};
	//	std::vector<size_t> offsets{};

	//	void writeStringTable();
	//	void writeOffsetTable();

	//public:
	//	static constexpr bool hasNativeStrings = true;

	//	chunk_ostream(binary_ostream& stream);
	//	~chunk_ostream();

	//	template<typename T> void write(const T& obj) {
	//		stream.write(obj);
	//	}

	//	template<> void write(const char* const& obj) {
	//		if (obj != nullptr) {
	//			auto i = stringOffsets.find(obj);

	//			if (i == stringOffsets.end()) {
	//				strings.emplace_back(obj);
	//				stringOffsets[obj] = { tellp() };
	//			}
	//			else
	//				i->second.emplace_back(tellp());

	//			offsets.emplace_back(tellp());
	//		}

	//		stream.write(0ull);
	//	}

	//	template<typename T> void write(const serialized_types::o64_t<T>& obj) {
	//		offsets.emplace_back(tellp());
	//		stream.write(obj.has_value() ? obj.value() : 0ull);
	//	}

	//	template<typename T> void write(const serialized_types::o32_t<T>& obj) {
	//		offsets.emplace_back(tellp());
	//		stream.write(obj.has_value() ? obj.value() : 0u);
	//	}

	//	void finish();
	//};

	template<typename AddrType>
	class BinaryFileReader {
		fast_istream raw_stream;
		binary_istream<AddrType> stream;
		FileHeader header;

	public:
		BinaryFileReader(std::istream& stream_) : raw_stream{ stream_ }, stream{ raw_stream } {
			stream.read(header);
			stream.seekg(0);
			stream.endianness = header.endianness == 'B' ? std::endian::big : std::endian::little;
			stream.read(header);
		}

		data_istream<AddrType> getData() {
			return { raw_stream, stream, header.endianness == 'B' ? std::endian::big : std::endian::little, 0 };
		}
	};

	//class BinaryFileWriter {
	//	binary_ostream& stream;
	//	std::endian endianness;
	//	unsigned short chunkCount{};

	//public:
	//	BinaryFileWriter(binary_ostream& stream, std::endian endianness = std::endian::native);
	//	~BinaryFileWriter();
	//	chunk_ostream addData();
	//	void finish();
	//};

	//class BinaryFileResolver {
	//	FileHeader* file;

	//	void doByteswaps();
	//	void resolveAddresses();

	//public:
	//	BinaryFileResolver(void* file);
	//	void* getData();
	//};

	template<typename AddrType>
	class BinaryFileDeserializer {
		BinaryFileReader<AddrType> container;

	public:
		BinaryFileDeserializer(std::istream& stream) : container{ stream } {}

		template<typename GameInterface, typename T, typename R>
		T* deserialize(R refl) {
			auto chunk = container.getData();

			rip::binary::ReflectionDeserializer<GameInterface, decltype(chunk)> deserializer{ chunk };

			return deserializer.deserialize<T, R>(refl);
		}

		template<typename GameInterface, typename T>
		T* deserialize() {
			return deserialize<GameInterface, T>(ucsl::reflection::providers::simplerfl<GameInterface>::template reflect<T>());
		}
	};

	//template<typename AddrType, std::endian endianness = std::endian::native>
	//class BinaryFileSerializer {
	//	BinaryFileWriter<AddrType, endianness> container;

	//public:
	//	BinaryFileSerializer(std::ostream& stream) : container{ stream } {}

	//	template<typename T, typename R>
	//	void serialize(T& data, R refl) {
	//		auto chunk = container.addDataChunk();

	//		rip::binary::ReflectionSerializer serializer{ chunk };

	//		serializer.serialize(data, refl);
	//	}

	//	template<typename GameInterface, typename T>
	//	void serialize(T& data) {
	//		serialize(data, ucsl::reflection::providers::simplerfl<GameInterface>::template reflect<T>());
	//	}
	//};
}
