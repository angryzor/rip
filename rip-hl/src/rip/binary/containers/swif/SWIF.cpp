#include <vector>
#include <rip/binary/stream.h>
#include "SWIF.h"

namespace rip::binary::containers::swif::v1 {
	SWIFSerializer::swif_ostream::swif_ostream(SWIFSerializer& serializer) : binary_ostream<size_t>{ serializer.rawStream, 0 }, serializer { serializer }
	{
	}

	SWIFSerializer::SWIFSerializer(std::ostream& stream) : rawStream{ stream } {
		writeBinaryFileHeaderChunk();

		chunksStart = stream.tellp();
	}

	SWIFSerializer::~SWIFSerializer() {
		size_t chunksEnd = stream.tellp();

		writeAddressResolutionChunk();
		writeEndChunk();

		rip::binary::containers::swif::v1::SRS_BINARY_FILE_HEADER_CHUNK_HEADER header{};
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
			stream.write(rip::binary::containers::swif::v1::SRS_BINARY_FILE_HEADER_CHUNK_HEADER{});
		});
	}

	void SWIFSerializer::writeAddressResolutionChunk() {
		addressResolutionChunkOffset = stream.tellp();

		writeChunk(SOF0, [&]() {
			rip::binary::containers::swif::v1::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER header{};
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

	SWIFResolver::SWIFResolver(void* file_) : file{ static_cast<rip::binary::containers::swif::v1::SRS_CHUNK_HEADER*>(file_) } {
		resolveAddresses();
	}

	void SWIFResolver::resolveAddresses()
	{
		// Note: SRS_BINARY_FILE_HEADER_CHUNK_HEADER is always the first chunk.
		rip::binary::containers::swif::v1::SRS_BINARY_FILE_HEADER_CHUNK_HEADER* binaryFileHeader = (rip::binary::containers::swif::v1::SRS_BINARY_FILE_HEADER_CHUNK_HEADER*)&file[1];
		rip::binary::containers::swif::v1::SRS_CHUNK_HEADER* addressResolutionChunk = (rip::binary::containers::swif::v1::SRS_CHUNK_HEADER*)addptr(file, binaryFileHeader->addressResolutionHeaderOffset);
		rip::binary::containers::swif::v1::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER* addressResolutionChunkHeader = (rip::binary::containers::swif::v1::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER*)&addressResolutionChunk[1];

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
}
