#pragma once
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <ctime>
#include <ucsl/resources/object-world/v2.h>
#include <ucsl/resources/object-world/v3.h>
#include <ucsl/resources/sobj/v1.h>
#include <ucsl-reflection/providers/rflclass.h>
#include <rip/util/object-id-guids.h>
#include <random>

namespace rip::hson {
	namespace json_reflections {
		struct Parameters {
			std::optional<rfl::Object<rfl::Generic>> tags{};
			rfl::ExtraFields<rfl::Generic> parameters{};
		};

		struct Object {
			std::optional<std::string> id{};
			std::optional<std::string> name{};
			std::optional<std::string> parentId{};
			std::optional<std::string> instanceOf{};
			std::optional<std::string> type{};
			std::optional<std::array<float, 3>> position{};
			std::optional<std::array<float, 4>> rotation{};
			std::optional<std::array<float, 3>> scale{};
			std::optional<bool> isEditorVisible{};
			std::optional<bool> isExcluded{};
			std::optional<Parameters> parameters{};
		};
		struct Metadata {
			std::string name{};
			std::string author{};
			std::string date{};
			std::string version{};
			std::string description{};
		};

		struct File {
			rfl::Rename<"$schema", std::string> schema{};
			unsigned int version{};
			Metadata metadata{};
			std::vector<Object> objects{};
		};
	}

	inline void writeHSON(const std::string& filename, auto getObjects) {
		std::time_t now = std::time(nullptr);
		std::string date = std::asctime(std::localtime(&now));

		rfl::json::save(filename, json_reflections::File{
			.schema = "https://raw.githubusercontent.com/hedge-dev/hson-schema/main/hson.schema.json",
			.version = 1,
			.metadata = json_reflections::Metadata{
				.name = filename,
				.author = "Restoration Issue Pocketknife",
				.date = date.substr(0, date.size() - 1),
				.version = "1.0.0",
				.description = "Converted by Restoration Issue Pocketknife.",
			},
			.objects = getObjects(),
		}, YYJSON_WRITE_PRETTY_TWO_SPACES);
	}

	inline Eigen::Quaternionf eulerToQuat(const Eigen::Vector3f& vec) {
		return Eigen::AngleAxisf(vec[1], Eigen::Vector3f::UnitY()) * Eigen::AngleAxisf(vec[0], Eigen::Vector3f::UnitX()) * Eigen::AngleAxisf(vec[2], Eigen::Vector3f::UnitZ());
	}

	inline Eigen::Vector3f matrixToEuler(const Eigen::Matrix3f& mat) {
		auto absoluteEuler = mat.eulerAngles(1, 0, 2);

		return { absoluteEuler[1], absoluteEuler[0], absoluteEuler[2] };
	}

	inline Eigen::Vector3f quatToEuler(const Eigen::Quaternionf& quat) {
		return matrixToEuler(quat.toRotationMatrix());
	}

	// This is temporary. Cleaner would be to instead make a ReflectCppSerializer that generates an rfl::Generic, so that we can export to many formats.
	template<typename GameInterface>
	inline rfl::Object<rfl::Generic> getRflClassSerialization(void* obj, const typename GameInterface::RflSystem::RflClass* rflClass) {
		yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);

		rip::binary::JsonSerializer serializer{ doc };
		yyjson_mut_val* json = serializer.serialize(obj, ucsl::reflection::providers::rflclass<GameInterface>::reflect(rflClass));

		yyjson_mut_doc_set_root(doc, json);

		yyjson_doc* idoc = yyjson_mut_doc_imut_copy(doc, nullptr);
		yyjson_val* iroot = yyjson_doc_get_root(idoc);

		auto result = rfl::json::read<rfl::Object<rfl::Generic>>(iroot);

		yyjson_doc_free(idoc);
		yyjson_mut_doc_free(doc);

		return result.value();
	}

	template<typename GameInterface>
	inline void serialize(const std::string& filename, ucsl::resources::object_world::v2::ObjectWorldData<typename GameInterface::AllocatorSystem>& data) {
		writeHSON(filename, [&data]() {
			auto transformedObjects = std::views::all(data.objects) | std::views::transform([](auto* obj) {
				auto* rflClass = GameInterface::GameObjectSystem::GetInstance()->gameObjectRegistry->GetGameObjectClassByName(obj->gameObjectClass)->GetSpawnerDataClass();

				rfl::Object<rfl::Generic> tags{};
				
				for (auto* componentData : obj->componentData) {
					auto* componentRflClass = GameInterface::GameObjectSystem::GetInstance()->goComponentRegistry->GetComponentInformationByName(obj->gameObjectClass)->GetSpawnerDataClass();

					tags[componentData->type] = getRflClassSerialization<GameInterface>(componentData->data, componentRflClass);
				}

				auto rotation = eulerToQuat(obj->localTransform.rotation);

				return json_reflections::Object{
					.id = util::toGUID(obj->id),
					.name = std::string{ obj->name },
					.parentId = obj->id.id != 0 ? std::make_optional(util::toGUID(obj->parentID)) : std::nullopt,
					.type = std::string{ obj->gameObjectClass },
					.position = std::array<float, 3>{ obj->localTransform.position.x(), obj->localTransform.position.y(), obj->localTransform.position.z() },
					.rotation = std::array<float, 4>{ rotation.x(), rotation.y(), rotation.z(), rotation.w() },
					.parameters = json_reflections::Parameters{
						.tags = tags,
						.parameters = getRflClassSerialization<GameInterface>(obj->spawnerData, rflClass),
					},
				};
			});

			return std::vector<json_reflections::Object>{ transformedObjects.begin(), transformedObjects.end() };
		});
	}

	template<typename GameInterface>
	inline void serialize(const std::string& filename, ucsl::resources::object_world::v3::ObjectWorldData<typename GameInterface::AllocatorSystem>& data) {
		writeHSON(filename, [&data]() {
			auto transformedObjects = std::views::all(data.objects) | std::views::transform([](auto* obj) {
				auto* rflClass = GameInterface::GameObjectSystem::GetInstance()->gameObjectRegistry->GetGameObjectClassByName(obj->gameObjectClass)->GetSpawnerDataClass();

				rfl::Object<rfl::Generic> tags{};

				for (auto* componentData : obj->componentData) {
					auto* componentRflClass = GameInterface::GameObjectSystem::GetInstance()->goComponentRegistry->GetComponentInformationByName(obj->gameObjectClass)->GetSpawnerDataClass();

					tags[componentData->type] = getRflClassSerialization<GameInterface>(componentData->data, componentRflClass);
				}

				auto rotation = eulerToQuat(obj->localTransform.rotation);

				return json_reflections::Object{
					.id = util::toGUID(obj->id),
					.name = std::string{ obj->name },
					.parentId = obj->parentID.groupId != 0 || obj->parentID.objectId != 0 ? std::make_optional(util::toGUID(obj->parentID)) : std::nullopt,
					.type = std::string{ obj->gameObjectClass },
					.position = std::array<float, 3>{ obj->localTransform.position.x(), obj->localTransform.position.y(), obj->localTransform.position.z() },
					.rotation = std::array<float, 4>{ rotation.x(), rotation.y(), rotation.z(), rotation.w() },
					.parameters = json_reflections::Parameters{
						.tags = tags,
						.parameters = getRflClassSerialization<GameInterface>(obj->spawnerData, rflClass),
					},
				};
			});

			return std::vector<json_reflections::Object>{ transformedObjects.begin(), transformedObjects.end() };
		});
	}

	template<typename GameInterface>
	inline void serialize(const std::string& filename, ucsl::resources::sobj::v1::SetObjectData<typename GameInterface::AllocatorSystem>& data) {
		writeHSON(filename, [&data]() {
			std::vector<json_reflections::Object> result{};
			std::mt19937 mt{ std::random_device{}() };

			for (auto& type : std::span{ data.objectTypes, data.objectTypeCount }) {
				auto* rflClass = GameInterface::GameObjectSystem::GetInstance()->gameObjectRegistry->GetGameObjectClassByName(type.name)->GetSpawnerDataClass();

				for (auto idx : std::span{ type.objectIndices, type.objectIndexCount }) {
					auto* obj = data.objects[idx];
					auto id = util::toGUID(ucsl::objectids::ObjectIdV1{ obj->id.id & 0x0000FFFF });

					rfl::Object<rfl::Generic> rangeSpawning{};
					rangeSpawning["rangeIn"] = obj->m_distance;
					rangeSpawning["rangeOut"] = obj->m_range;

					rfl::Object<rfl::Generic> tags{};
					tags["RangeSpawning"] = rangeSpawning;

					if (obj->instances.size() > 1) {
						result.push_back({
							.id = id,
							.type = std::string{ type.name },
							.isExcluded = true,
							.parameters = json_reflections::Parameters{
								.tags = tags,
								.parameters = getRflClassSerialization<GameInterface>(&obj[1], rflClass),
							},
						});

						for (auto& instance : obj->instances) {
							auto rotation = eulerToQuat(instance.rotation);

							result.push_back({
								.id = util::toGUID(ucsl::objectids::ObjectIdV1{ mt() }),
								.instanceOf = id,
								.position = std::array<float, 3>{ instance.position.x(), instance.position.y(), instance.position.z() },
								.rotation = std::array<float, 4>{ rotation.x(), rotation.y(), rotation.z(), rotation.w() },
							});
						}
					}
					else {
						auto& instance = obj->instances[0];
						auto rotation = eulerToQuat(instance.rotation);

						result.push_back({
							.id = id,
							.type = std::string{ type.name },
							.position = std::array<float, 3>{ instance.position.x(), instance.position.y(), instance.position.z() },
							.rotation = std::array<float, 4>{ rotation.x(), rotation.y(), rotation.z(), rotation.w() },
							.parameters = json_reflections::Parameters{
								.tags = tags,
								.parameters = getRflClassSerialization<GameInterface>(&obj[1], rflClass),
							},
						});
					}
				}
			}

			return result;
		});
	}
}