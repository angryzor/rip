#include <vector>
#include <string>
#include <ucsl/magic.h>
#include <rip/binary/stream.h>
#include <rip/binary/types.h>
#include <rip/util/byteswap.h>
#include <rip/util/memory.h>
#include <iostream>
#include "v1.h"

namespace rip::binary::containers::binary_file::v1 {
	//void chunk_ostream::writeStringTable() {
	//	for (auto& string : strings) {
	//		auto offsets = stringOffsets[string];
	//		auto pos = tellp();

	//		for (auto offset : offsets) {
	//			seekp(offset);
	//			stream.write(pos);
	//		}

	//		seekp(pos);
	//		stream.write(*string.c_str(), string.size() + 1);
	//	}

	//	stream.write_padding(4);
	//}

	//void chunk_ostream::writeOffsetTable() {
	//	size_t last_offset = 0;

	//	for (size_t offset : offsets) {
	//		size_t diff = offset - last_offset;

	//		if (diff >= (1 << 16))
	//			stream.write(byteswap_to_native(std::endian::big, static_cast<unsigned int>((diff >> 2u) | (3u << 30u))));
	//		else if (diff >= (1 << 8))
	//			stream.write(byteswap_to_native(std::endian::big, static_cast<unsigned short>((diff >> 2u) | (2u << 14u))));
	//		else
	//			stream.write(static_cast<unsigned char>((diff >> 2u) | (1u << 6u)));

	//		last_offset = offset;
	//	}

	//	stream.write_padding(4);
	//}

	//chunk_ostream::chunk_ostream(const ucsl::magic_t<4>& magic, binary_ostream& stream, unsigned short additionalHeaderSize) : magic{ magic }, stream{ stream }, chunkOffset{ stream.tellp() }, offset_binary_ostream { stream, stream.tellp() + sizeof(ChunkHeader) + additionalHeaderSize } {
	//	stream.write(ChunkHeader{});

	//	for (unsigned short i = 0; i < additionalHeaderSize; i++)
	//		stream.write(static_cast<unsigned char>(0));
	//}

	//chunk_ostream::~chunk_ostream() {
	//	finish();
	//}

	//void chunk_ostream::finish() {
	//	size_t stringTableStart = stream.tellp();
	//	writeStringTable();
	//	size_t offsetTableStart = stream.tellp();
	//	writeOffsetTable();
	//	size_t chunkEnd = stream.tellp();
	//	
	//	ChunkHeader chunkHeader{};
	//	chunkHeader.magic = magic;
	//	chunkHeader.size = static_cast<unsigned int>(chunkEnd - chunkOffset);
	//	chunkHeader.dataSize = static_cast<unsigned int>(stringTableStart - sizeof(ChunkHeader) - additionalHeaderSize - chunkOffset);
	//	chunkHeader.stringTableSize = static_cast<unsigned int>(offsetTableStart - stringTableStart);
	//	chunkHeader.offsetTableSize = static_cast<unsigned int>(chunkEnd - offsetTableStart);
	//	chunkHeader.additionalHeaderSize = additionalHeaderSize;
	//	
	//	stream.seekp(chunkOffset);
	//	stream.write(chunkHeader);
	//	stream.seekp(chunkEnd);
	//}

	//BinaryFileWriter::BinaryFileWriter(binary_ostream& stream, std::endian endianness) : stream{stream}, endianness{endianness} {
	//	stream.write(FileHeader{});
	//}

	//BinaryFileWriter::~BinaryFileWriter()
	//{
	//	finish();
	//}

	//chunk_ostream BinaryFileWriter::addDataChunk() {
	//	chunkCount++;
	//	return { "DATA", stream };
	//}

	//void BinaryFileWriter::finish()
	//{
	//	size_t pos = stream.tellp();

	//	FileHeader fileHeader{};
	//	fileHeader.magic = "BINA";
	//	fileHeader.version = "210";
	//	fileHeader.endianness = endianness == std::endian::big ? 'B' : 'L';
	//	fileHeader.fileSize = static_cast<unsigned int>(pos);
	//	fileHeader.chunkCount = chunkCount;

	//	stream.seekp(0);
	//	stream.write(fileHeader);
	//	stream.seekp(pos);
	//}

	BinaryFileResolver::BinaryFileResolver(void* file_) : file{ static_cast<FileHeader*>(file_) } {
		if (file->flags & 1)
			return;

		if (!(file->flags & 2) && std::endian::native != (file->endianness == 'B' ? std::endian::big : std::endian::little))
			doByteswaps();

		resolveAddresses();
	}

	//template<typename F>
	//void BinaryFileResolver::forEachChunk(F f) {
	//	size_t offset{ sizeof(FileHeader) };
	//	for (unsigned int i = 0; i < file->chunkCount; i++) {
	//		auto* chunk = reinterpret_cast<ChunkHeader*>(addptr(file, offset));

	//		f(chunk);

	//		offset += chunk->size;
	//	}
	//}

	void BinaryFileResolver::doByteswaps() {
		byteswap_deep(*file);
	}

	void BinaryFileResolver::resolveAddresses() {
		void* dataStart = addptr(file, sizeof(FileHeader));
		void* offsetLoc = dataStart;
		void* offsetsStart = addptr(dataStart, file->dataSize);
		void* offsets = offsetsStart;

		while (offsets != addptr(offsetsStart, file->offsetTableSize) && (*static_cast<unsigned char*>(offsets) & 0xC0) != 0) {
			switch ((*static_cast<unsigned char*>(offsets) & 0xC0) >> 6) {
			case 1: offsetLoc = addptr(offsetLoc, (*static_cast<unsigned char*>(offsets) & 0x3F) << 2); offsets = addptr(offsets, 1); break;
			case 2: offsetLoc = addptr(offsetLoc, (byteswap_to_native(std::endian::big, *static_cast<unsigned short*>(offsets)) & 0x3FFF) << 2); offsets = addptr(offsets, 2); break;
			case 3: offsetLoc = addptr(offsetLoc, (byteswap_to_native(std::endian::big, *static_cast<unsigned int*>(offsets)) & 0x3FFFFFFF) << 2); offsets = addptr(offsets, 4); break;
			}

			size_t* offset = static_cast<size_t*>(offsetLoc);

			byteswap_deep_to_native(file->endianness == 'B' ? std::endian::big : std::endian::little, *offset);

			if (*offset != 0)
				*offset += reinterpret_cast<size_t>(dataStart);
		}
	}

	void* BinaryFileResolver::getData() {
		return addptr(file, sizeof(FileHeader));
	}

	//BinaryFileSerializer::BinaryFileSerializer(binary_ostream& stream, std::endian endianness) : container{ stream, endianness } {}
}
