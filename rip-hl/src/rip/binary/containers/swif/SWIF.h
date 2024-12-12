#pragma once
#include <vector>
#include <simple-reflection/simple-reflection.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/serialization/ReflectionSerializer.h>

namespace rip::binary::containers::swif::v6 {
	static thread_local unsigned int gTextureListCount;
	struct TextureListArray {};

	using TextureListArrayRefl = ::simplerfl::dynamic_carray<ucsl::resources::swif::v6::SRS_TEXTURELIST, int, [](const int& parent) -> size_t { return gTextureListCount; }>;
}

namespace simplerfl {
	template<> struct canonical<rip::binary::containers::swif::v6::TextureListArray> { using type = rip::binary::containers::swif::v6::TextureListArrayRefl; };
}

namespace rip::util {
	template<> inline void byteswap_deep(ucsl::resources::swif::v6::SRS_BINARY_FILE_HEADER_CHUNK_HEADER& value) noexcept {
		byteswap_deep(value.chunkCount);
		byteswap_deep(value.chunksStart);
		byteswap_deep(value.chunksSize);
		byteswap_deep(value.addressResolutionHeaderOffset);
		byteswap_deep(value.revision);
		byteswap_deep(value.addressResolutionHeaderOffset);
	}

	template<> inline void byteswap_deep(ucsl::resources::swif::v6::SRS_ADDRESS_RESOLUTION_CHUNK_HEADER& value) noexcept {
		byteswap_deep(value.addressToResolveCount);
		byteswap_deep(value.isResolved);
	}

	template<> inline void byteswap_deep(ucsl::resources::swif::v6::SRS_CHUNK_HEADER& value) noexcept {
		byteswap_deep(value.magic);
		byteswap_deep(value.chunkSize);
	}

	template<> inline void byteswap_deep(ucsl::resources::swif::v6::SRS_TEXTURELIST_CHUNK_HEADER& value) noexcept {
		byteswap_deep(value.startOffset);
		byteswap_deep(value.textureListCount);
	}

	template<> inline void byteswap_deep(ucsl::resources::swif::v6::SRS_PROJECT_CHUNK_HEADER& value) noexcept {
		byteswap_deep(value.startOffset);
	}
}

namespace rip::binary::containers::swif::v6 {
	static constexpr const unsigned int SWIF = 0x46495753;
	static constexpr const unsigned int SWTL = 0x4C545753;
	static constexpr const unsigned int SWPR = 0x52505753;
	static constexpr const unsigned int SEND = 0x444E4553;
	static constexpr const unsigned int SOF0 = 0x30464F53;

	class SWIFSerializer {
		class swif_ostream : public binary_ostream<size_t> {
			SWIFSerializer& serializer;

		public:
			swif_ostream(SWIFSerializer& serializer);

			static constexpr bool hasNativeStrings = false;

			template<typename T>
			void write(const T& obj) {
				binary_ostream<size_t>::write(obj);
			}

			template<typename T>
			void write(const offset_t<T>& obj) {
				if (obj.has_value())
					serializer.addressLocations.push_back(static_cast<unsigned int>(tellp()));
				
				binary_ostream<size_t>::write(obj);
			}
		};

		fast_ostream rawStream;
		swif_ostream stream{ *this };
		ReflectionSerializer<swif_ostream> reflectionSerializer{ stream };
		std::vector<unsigned int> addressLocations{};
		size_t addressResolutionChunkOffset{};
		size_t chunksStart{};
		unsigned int chunkCount{};

		template<typename F>
		void writeChunk(unsigned int magic, F&& writeFunc) {
			size_t start = stream.tellp();

			ucsl::resources::swif::v6::SRS_CHUNK_HEADER chunkHeader{};
			chunkHeader.magic = magic;
			chunkHeader.chunkSize = 0;

			stream.write(chunkHeader);

			size_t dataStart = stream.tellp();

			writeFunc();
			stream.write_padding(16);

			size_t end = stream.tellp();
			chunkHeader.chunkSize = static_cast<unsigned int>(end - dataStart);

			stream.seekp(start);
			stream.write(chunkHeader);
			stream.seekp(end);
		}

		template<typename F>
		void writeMainChunk(unsigned int magic, F&& writeFunc) {
			writeChunk(magic, writeFunc);
			chunkCount++;
		}

		void writeBinaryFileHeaderChunk();

		template<typename GameInterface>
		void writeTextureListChunk(ucsl::resources::swif::v6::SRS_TEXTURELIST* textureLists, size_t textureListCount) {
			writeMainChunk(SWTL, [&]() {
				TextureListArray& textureListArr{ *reinterpret_cast<TextureListArray*>(textureLists) };

				ucsl::resources::swif::v6::SRS_TEXTURELIST_CHUNK_HEADER header{};
				header.startOffset = 16;
				header.textureListCount = static_cast<unsigned int>(textureListCount);

				stream.write(header);
				stream.write_padding(16);
				gTextureListCount = static_cast<unsigned int>(textureListCount);
				reflectionSerializer.serialize(textureListArr, ucsl::reflection::providers::simplerfl<GameInterface>::reflect(textureListArr));
			});
		}

		template<typename GameInterface>
		void writeProjectChunk(ucsl::resources::swif::v6::SRS_PROJECT& project) {
			writeMainChunk(SWPR, [&]() {
				ucsl::resources::swif::v6::SRS_PROJECT_CHUNK_HEADER header{};
				header.startOffset = 16;

				stream.write(header);
				stream.write_padding(16);
				reflectionSerializer.serialize(project, ucsl::reflection::providers::simplerfl<GameInterface>::reflect(project));
			});
		}

		void writeAddressResolutionChunk();
		void writeEndChunk();
	public:
		SWIFSerializer(std::ostream& stream);
		~SWIFSerializer();

		template<typename GameInterface>
		void serialize(ucsl::resources::swif::v6::SRS_PROJECT& project) {
			writeTextureListChunk<GameInterface>(project.textureLists, project.textureListCount);
			writeProjectChunk<GameInterface>(project);
		}
	};

	class SWIFResolver {
		ucsl::resources::swif::v6::SRS_CHUNK_HEADER* file;

		template<typename F>
		void forEachChunk(F f);
		void resolveAddresses();

	public:
		SWIFResolver(void* file);
		ucsl::resources::swif::v6::SRS_PROJECT* getProject();
	};
}
