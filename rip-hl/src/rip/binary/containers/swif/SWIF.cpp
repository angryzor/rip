#include <vector>
#include <ucsl/resources/swif/v6.h>
#include <rip/binary/stream.h>
#include "SWIF.h"

namespace rip::binary::containers::swif::v6 {
	SWIFSerializer::swif_ostream::swif_ostream(SWIFSerializer& serializer) : offset_binary_ostream{ serializer.stream, 0 }, serializer { serializer }
	{
	}

	SWIFSerializer::SWIFSerializer(binary_ostream& stream) : stream{ stream } {
		writeBinaryFileHeaderChunk();

		chunksStart = stream.tellp();
	}

	SWIFSerializer::~SWIFSerializer() {
		size_t chunksEnd = stream.tellp();

		writeAddressResolutionChunk();
		writeEndChunk();

		ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER header{};
		header.chunkCount = chunkCount;
		header.chunksStart = static_cast<unsigned int>(chunksStart);
		header.chunksSize = static_cast<unsigned int>(chunksEnd - chunksStart);
		header.addressResolutionHeaderOffset = static_cast<unsigned int>(addressResolutionChunkOffset);
		header.revision = 20120705;

		stream.seekp(8);
		stream.write(header);
	}

	void SWIFSerializer::writeBinaryFileHeaderChunk() {
		writeChunk(SWIF, [&]() {
			stream.write(ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER{});
		});
	}

	void SWIFSerializer::writeAddressResolutionChunk() {
		addressResolutionChunkOffset = stream.tellp();

		writeChunk(SOF0, [&]() {
			ucsl::resources::swif::v6::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER header{};
			header.addressToResolveCount = static_cast<unsigned int>(addressLocations.size());
			header.isResolved = 0;

			stream.write(header);
			for (unsigned int addressLocation : addressLocations)
				stream.write(addressLocation);
		});
	}

	void SWIFSerializer::writeEndChunk() {
		writeChunk(SEND, [&]() {});
	}


	SWIFResolver::SWIFResolver(void* file_) : file{ static_cast<ucsl::resources::swif::v6::SRS_CHUNK_HEADER*>(file_) } {
		resolveAddresses();
	}

	template<typename F>
	inline void SWIFResolver::forEachChunk(F f)
	{
		// Note: SRS_BINARY_FILE_HEADER_CHUNK_HEADER is always the first chunk.
		ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER* binaryFileHeader = (ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER*)&file[1];

		auto* chunk = addptr(&file[1], file->chunkSize);

		for (unsigned int i = 0; i < binaryFileHeader->chunkCount; i++, chunk = addptr(&chunk[1], chunk->chunkSize))
			f(chunk);
	}

	void SWIFResolver::resolveAddresses()
	{
		// Note: SRS_BINARY_FILE_HEADER_CHUNK_HEADER is always the first chunk.
		ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER* binaryFileHeader = (ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER*)&file[1];
		ucsl::resources::swif::v6::SRS_CHUNK_HEADER* addressResolutionChunk = (ucsl::resources::swif::v6::SRS_CHUNK_HEADER*)addptr(file, binaryFileHeader->addressResolutionHeaderOffset);
		ucsl::resources::swif::v6::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER* addressResolutionChunkHeader = (ucsl::resources::swif::v6::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER*)&addressResolutionChunk[1];

		if (addressResolutionChunkHeader->isResolved != 0)
			return;

		unsigned int* offsetLocations = (unsigned int*)&addressResolutionChunkHeader[1];

		for (auto offsetLocation : std::views::counted(offsetLocations, addressResolutionChunkHeader->addressToResolveCount)) {
			auto* offset = (size_t*)addptr(file, offsetLocation);
			
			if (*offset != 0)
				*offset += (size_t)file;
		}

		addressResolutionChunkHeader->isResolved = 1;
	}

	ucsl::resources::swif::v6::SRS_PROJECT* SWIFResolver::getProject()
	{
		ucsl::resources::swif::v6::SRS_PROJECT* result{};

		forEachChunk([&result](ucsl::resources::swif::v6::SRS_CHUNK_HEADER* chunk) {
			if (chunk->magic == SWPR) {
				auto projHeader = (ucsl::resources::swif::v6::SRS_PROJECT_CHUNK_HEADER*)&chunk[1];

				result = (ucsl::resources::swif::v6::SRS_PROJECT*)addptr(chunk, projHeader->startOffset);
			}
		});

		return result;
	}
}
