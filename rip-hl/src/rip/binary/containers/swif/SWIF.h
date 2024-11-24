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

namespace rip::binary::containers::swif::v6 {
	static constexpr const unsigned int SWIF = 0x46495753;
	static constexpr const unsigned int SWTL = 0x4C545753;
	static constexpr const unsigned int SWPR = 0x52505753;
	static constexpr const unsigned int SEND = 0x444E4553;
	static constexpr const unsigned int SOF0 = 0x30464F53;

	class SWIFSerializer {
		class swif_ostream : public offset_binary_ostream {
			SWIFSerializer& serializer;

		public:
			swif_ostream(SWIFSerializer& serializer);

			static constexpr bool hasNativeStrings = false;

			template<typename T>
			void write(const T& obj, size_t count = 1) {
				serializer.stream.write(obj, count);
			}

			template<typename T>
			void write(const serialized_types::o64_t<T>& obj) {
				if (obj.has_value()) {
					serializer.addressLocations.push_back(static_cast<unsigned int>(serializer.stream.tellp()));
					serializer.stream.write(obj.value());
				}
				else
					serializer.stream.write(0ull);
			}

			template<typename T>
			void write(const serialized_types::o32_t<T>& obj) {
				assert(!"This container only supports 64-bit offsets");
			}
		};

		binary_ostream& stream;
		swif_ostream swifStream{ *this };
		ReflectionSerializer<swif_ostream> reflectionSerializer{ swifStream };
		std::vector<unsigned int> addressLocations{};
		size_t addressResolutionChunkOffset{};
		size_t chunksStart{};
		unsigned int chunkCount{};

		template<typename F>
		void writeChunk(unsigned int magic, F&& writeFunc) {
			size_t start = stream.tellp();

			ucsl::resources::swif::v6::SRS_CHUNK_HEADER chunkHeader{ magic, 0 };
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

				stream.write(ucsl::resources::swif::v6::SRS_TEXTURELIST_CHUNK_HEADER{ 16, static_cast<unsigned int>(textureListCount) });
				stream.write_padding(16);
				reflectionSerializer.serialize(textureListArr, ucsl::reflection::providers::simplerfl<GameInterface>::reflect(textureListArr));
			});
		}

		template<typename GameInterface>
		void writeProjectChunk(ucsl::resources::swif::v6::SRS_PROJECT& project) {
			writeMainChunk(SWPR, [&]() {
				stream.write(ucsl::resources::swif::v6::SRS_PROJECT_CHUNK_HEADER{ 16 });
				stream.write_padding(16);
				reflectionSerializer.serialize(project, ucsl::reflection::providers::simplerfl<GameInterface>::reflect(project));
			});
		}

		void writeAddressResolutionChunk();
		void writeEndChunk();
	public:
		SWIFSerializer(binary_ostream& stream);
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
