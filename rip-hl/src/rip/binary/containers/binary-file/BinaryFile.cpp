#include <vector>
#include <string>
#include <rip/binary/stream.h>
#include <rip/binary/types.h>
#include <rip/util/magic.h>
#include <rip/types.h>
#include <rip/byteswap.h>
#include <iostream>
#include "BinaryFile.h"

namespace rip::binary::containers::binary_file::v2 {
	void chunk_ostream::writeStringTable() {
		for (auto& [offset, string] : strings) {
			auto pos = tellp();

			seekp(offset);
			stream.write(pos);
			seekp(pos);
			stream.write(*string.c_str(), string.size() + 1);
		}
	}

	void chunk_ostream::writeOffsetTable() {
		size_t last_offset = 0;

		for (size_t offset : offsets) {
			size_t diff = offset - last_offset;

			std::cout << "Writing offset " << std::hex << offset << ", last offset was " << last_offset << " diff is " << diff << std::endl;

			if (diff >= (1 << 16))
				stream.write(static_cast<unsigned int>(byteswap_to_native(std::endian::big, static_cast<unsigned int>(diff >> 2)) | (3 << 30)));
			else if (diff >= (1 << 8))
				stream.write(static_cast<unsigned short>(byteswap_to_native(std::endian::big, static_cast<unsigned short>(diff >> 2)) | (2 << 14)));
			else
				stream.write(static_cast<unsigned char>(diff >> 2 | (1 << 6)));

			last_offset = offset;
		}

		stream.write(static_cast<unsigned char>(0));
	}

	chunk_ostream::chunk_ostream(const magic_t<4>& magic, binary_ostream& stream, unsigned short additionalHeaderSize) : magic{ magic }, stream{ stream }, additionalHeaderSize{ additionalHeaderSize }, chunkOffset{ stream.tellp() }, offset_binary_ostream { stream, stream.tellp() + sizeof(ChunkHeader) + additionalHeaderSize } {
		std::cout << "Chunk starting at " << std::hex << chunkOffset << ", data will start at " << chunkOffset + sizeof(ChunkHeader) + additionalHeaderSize << std::endl;
		stream.write(ChunkHeader{ magic, 0, 0, 0, 0, additionalHeaderSize });

		for (unsigned short i = 0; i < additionalHeaderSize; i++)
			stream.write(static_cast<unsigned char>(0));
	}

	chunk_ostream::~chunk_ostream() {
		finish();
	}

	void chunk_ostream::finish() {
		size_t stringTableStart = stream.tellp();
		std::cout << "String table at " << std::hex << stringTableStart << std::endl;
		writeStringTable();
		size_t offsetTableStart = stream.tellp();
		std::cout << "Offset table at " << std::hex << offsetTableStart << std::endl;
		writeOffsetTable();
		size_t chunkEnd = stream.tellp();
		std::cout << "eof at " << std::hex << chunkEnd << std::endl;
		
		stream.seekp(chunkOffset);
		stream.write(ChunkHeader{
			.magic = magic,
			.size = static_cast<unsigned int>(chunkEnd - chunkOffset),
			.dataSize = static_cast<unsigned int>(stringTableStart - sizeof(ChunkHeader) - additionalHeaderSize - chunkOffset),
			.stringTableSize = static_cast<unsigned int>(offsetTableStart - stringTableStart),
			.offsetTableSize = static_cast<unsigned int>(chunkEnd - offsetTableStart),
			.additionalHeaderSize = additionalHeaderSize,
		});
		stream.seekp(chunkEnd);
	}

	BinaryFileWriter::BinaryFileWriter(binary_ostream& stream, std::endian endianness) : stream{stream}, endianness{endianness} {
		stream.write(FileHeader{ "BINA", "210", endianness == std::endian::big ? 'B' : 'L', 0, 0 });
	}

	BinaryFileWriter::~BinaryFileWriter()
	{
		finish();
	}

	chunk_ostream BinaryFileWriter::addDataChunk() {
		chunkCount++;
		return { "DATA", stream };
	}

	void BinaryFileWriter::finish()
	{
		size_t pos = stream.tellp();
		stream.seekp(0);
		stream.write(FileHeader{
			.magic = "BINA",
			.version = "210",
			.endianness = endianness == std::endian::big ? 'B' : 'L',
			.fileSize = static_cast<unsigned int>(pos),
			.chunkCount = chunkCount,
		});
		stream.seekp(pos);
	}

	BinaryFileResolver::BinaryFileResolver(void* file_) : file{ static_cast<FileHeader*>(file_) } {
		if (file->flags & 1)
			return;

		if (!(file->flags & 2) && std::endian::native != (file->endianness == 'B' ? std::endian::big : std::endian::little))
			doByteswaps();

		resolveAddresses();
	}

	template<typename F>
	void BinaryFileResolver::forEachChunk(F f) {
		size_t offset{ sizeof(FileHeader) };
		for (unsigned int i = 0; i < file->chunkCount; i++) {
			auto* chunk = reinterpret_cast<ChunkHeader*>(addptr(file, offset));

			f(chunk);

			offset += chunk->size;
		}
	}

	void BinaryFileResolver::doByteswaps() {
		byteswap_deep(*file);

		forEachChunk([](ChunkHeader* chunk) { byteswap_deep(*chunk); });
	}

	void BinaryFileResolver::resolveAddresses() {
		forEachChunk([=](ChunkHeader* chunk) {
			void* dataStart = addptr(chunk, sizeof(ChunkHeader) + chunk->additionalHeaderSize);
			void* offsetLoc = dataStart;
			void* offsets = addptr(chunk, sizeof(ChunkHeader) + chunk->additionalHeaderSize + chunk->dataSize + chunk->stringTableSize);

			while ((*static_cast<unsigned char*>(offsets) & 0xC0) != 0) {
				switch ((*static_cast<unsigned char*>(offsets) & 0xC0) >> 6) {
				case 1: offsetLoc = addptr(offsetLoc, (*static_cast<unsigned char*>(offsets) & 0x3F) << 2); offsets = addptr(offsets, 1); break;
				case 2: offsetLoc = addptr(offsetLoc, (byteswap_to_native(std::endian::big, *static_cast<unsigned short*>(offsets)) & 0x3FFF) << 2); offsets = addptr(offsets, 2); break;
				case 3: offsetLoc = addptr(offsetLoc, (byteswap_to_native(std::endian::big, *static_cast<unsigned int*>(offsets)) & 0x3FFFFFFF) << 2); offsets = addptr(offsets, 4); break;
				}

				size_t* offset = static_cast<size_t*>(offsetLoc);

				byteswap_deep_to_native(file->endianness == 'B' ? std::endian::big : std::endian::little, *offset);

				*offset += reinterpret_cast<size_t>(dataStart);
			}
		});
	}

	void* BinaryFileResolver::getData(unsigned short chunkId) {
		if (chunkId >= file->chunkCount)
			return nullptr;

		size_t offset{ sizeof(FileHeader) };
		for (unsigned short i = 0; i < chunkId; i++)
			offset += reinterpret_cast<ChunkHeader*>(addptr(file, offset))->size;

		auto* chunk = reinterpret_cast<ChunkHeader*>(addptr(file, offset));

		return addptr(chunk, sizeof(ChunkHeader) + chunk->additionalHeaderSize);
	}
}
