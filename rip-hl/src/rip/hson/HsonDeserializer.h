#pragma once
#include <rfl.hpp>
#include <rfl/json.hpp>
#include <ctime>
#include <ucsl/resources/object-world/v2.h>
#include <ucsl/resources/object-world/v3.h>
#include <ucsl/resources/sobj/v1.h>
#include <ucsl-reflection/providers/rflclass.h>
#include <rip/util/math.h>
#include <rip/util/object-id-guids.h>
#include <rip/binary/serialization/BlobWorker.h>
#include <rip/binary/serialization/JsonDeserializer.h>
#include <random>
#include "./JsonReflections.h"

namespace rip::hson {
	inline std::vector<json_reflections::Object> readHSON(const std::string& filename) {
		std::time_t now = std::time(nullptr);
		std::string date = std::asctime(std::localtime(&now));

		auto hson = rfl::json::load<json_reflections::File>(filename);

		return std::move(hson.value().objects);
	}

	inline bool has_parent(const json_reflections::Object& object) {
		return object.parentId.has_value() && object.parentId.value() != "{00000000-0000-0000-0000-000000000000}";
	}

	inline const json_reflections::Object& find_object(const std::string& id, const std::vector<json_reflections::Object>& objects) {
		return *std::find_if(objects.begin(), objects.end(), [&id](const auto& o) { return o.id == id; });
	}

	inline Eigen::Affine3f get_absolute_transform(const json_reflections::Object& object, const std::vector<json_reflections::Object>& objects) {
		auto pos = object.position.value();
		auto rot = object.rotation.value();

		Eigen::Affine3f localTransform = Eigen::Translation3f{ pos[0], pos[1], pos[2] } * Eigen::Quaternionf{ rot[3], rot[0], rot[1], rot[2] };

		if (!has_parent(object))
			return localTransform;

		return get_absolute_transform(find_object(object.parentId.value(), objects), objects) * localTransform;
	}

	template<typename T, typename F>
	inline const T* get_object_property(const json_reflections::Object& object, const std::vector<json_reflections::Object>& objects, F f) {
		const std::optional<T>& res{ f(object) };

		return res.has_value()
			? &res.value()
			: object.instanceOf.has_value()
			? get_object_property<T>(find_object(object.instanceOf.value(), objects), objects, std::forward<decltype(f)>(f))
			: nullptr;
	}

	template<typename GameInterface>
	inline gedit_reflections::ObjectWorldData deserializeGedit(const std::string& filename) {
		auto objs = readHSON(filename);

		std::vector<gedit_reflections::ObjectData> resultObjs{};

		for (auto& obj : objs) {
			auto pos = obj.position.value();
			auto rot = obj.rotation.value();
			auto rotEuler = util::quatToEuler(Eigen::Quaternionf{ rot[3], rot[0], rot[1], rot[2] });

			gedit_reflections::ObjectTransformData localTransform{ std::array<float, 3>{ pos[0], pos[1], pos[2] }, std::array<float, 3>{ rotEuler[0], rotEuler[1], rotEuler[2] } };
			gedit_reflections::ObjectTransformData transform{ localTransform };
			
			if (has_parent(obj)) {
				auto tf = get_absolute_transform(obj, objs);
				auto absPos = tf.translation();
				auto absRot = util::matrixToEuler(tf.rotation());

				transform.position = { absPos.x(), absPos.y(), absPos.z() };
				transform.rotation = { absRot.x(), absRot.y(), absRot.z() };
			}

			std::vector<gedit_reflections::ComponentData> resultComponents{};

			const auto& params = *get_object_property<json_reflections::Parameters>(obj, objs, [](const json_reflections::Object& o) -> const std::optional<json_reflections::Parameters>& { return o.parameters; });
			if (auto& components = params.tags) {
				for (auto& [type, component] : components.value()) {
					auto* componentRflClass = GameInterface::GameObjectSystem::GetInstance()->goComponentRegistry->GetComponentInformationByName(type.c_str())->GetSpawnerDataClass();

					resultComponents.emplace_back(gedit_reflections::ComponentData{
						.type = type,
						.size = componentRflClass->GetSize(),
						.data = component,
					});
				}
			}

			const auto* parentId = get_object_property<std::string>(obj, objs, [](const json_reflections::Object& o) { return o.parentId; });
			
			resultObjs.emplace_back(gedit_reflections::ObjectData{
				.gameObjectClass = *get_object_property<std::string>(obj, objs, [](const json_reflections::Object& o) { return o.type; }),
				.name = obj.name.value(),
				.id = obj.id.value(),
				.parentID = parentId == nullptr ? "{00000000-0000-0000-0000-000000000000}" : *parentId,
				.transform = transform,
				.localTransform = localTransform,
				.componentData = resultComponents,
				.spawnerData = params.parameters,
			});
		}

		return { resultObjs };
	}

	template<typename GameInterface>
	inline ucsl::resources::object_world::v2::ObjectWorldData<typename GameInterface::AllocatorSystem>* deserializeGeditV2(const std::string& filename) {
		auto json = rfl::json::write(deserializeGedit<GameInterface>(filename));
		rip::binary::JsonDeserializer<GameInterface, true> deserializer{ json.c_str() };
		return deserializer.deserializeString<ucsl::resources::object_world::v2::ObjectWorldData<typename GameInterface::AllocatorSystem>>(
			ucsl::reflection::providers::simplerfl<GI>::template reflect<ucsl::resources::object_world::v2::ObjectWorldData<typename GameInterface::AllocatorSystem>>()
		);
	}

	template<typename GameInterface>
	inline ucsl::resources::object_world::v3::ObjectWorldData<typename GameInterface::AllocatorSystem>* deserializeGeditV3(const std::string& filename) {
		auto json = rfl::json::write(deserializeGedit<GameInterface>(filename));
		rip::binary::JsonDeserializer<GameInterface, true> deserializer{ json.c_str() };
		return deserializer.deserializeString<ucsl::resources::object_world::v3::ObjectWorldData<typename GameInterface::AllocatorSystem>>(
			ucsl::reflection::providers::simplerfl<GI>::template reflect<ucsl::resources::object_world::v3::ObjectWorldData<typename GameInterface::AllocatorSystem>>()
		);
	}

	inline bool is_sobj_instance_object(const json_reflections::Object& object) {
		return object.instanceOf.has_value()
			&& !object.name.has_value()
			&& !object.parameters.has_value()
			&& !object.parentId.has_value()
			&& !object.type.has_value();
	}

	struct SOBJObjectContext {
		sobj_reflections::ObjectData object{};
		std::string objType{};
	};

	inline sobj_reflections::ObjectData convert_hson_to_sobj_object(const json_reflections::Object& object, const std::vector<json_reflections::Object>& objects) {
		const auto& params = *get_object_property<json_reflections::Parameters>(object, objects, [](const json_reflections::Object& o) -> const std::optional<json_reflections::Parameters>& { return o.parameters; });
		const auto rangeSpawning = params.tags.value().get("RangeSpawning").value().to_object().value();
		
		return {
			.id = object.id.value(),
			.objectClassId = 0,
			.bvhNode = 0,
			.replicationInterval = 0.0f,
			.m_distance = static_cast<float>(rangeSpawning.get("rangeIn").value().to_double().value()),
			.m_range = static_cast<float>(rangeSpawning.get("rangeOut").value().to_double().value()),
			.instances = {},
			.spawnerData = params.parameters,
		};
	}

	template<typename GameInterface>
	inline sobj_reflections::SetObjectData deserializeSOBJ(const std::string& filename) {
		auto objs = readHSON(filename);
		
		std::map<std::string, SOBJObjectContext> resultObjectsById{};
		std::map<std::string, sobj_reflections::ObjectTypeData> resultObjectTypesByName{};

		unsigned int objInstanceCount{};

		for (auto& obj : objs) {
			if (obj.isExcluded.value_or(false))
				continue;
			
			auto pos = obj.position.value();
			auto rot = obj.rotation.value();
			auto rotEuler = util::quatToEuler(Eigen::Quaternionf{ rot[3], rot[0], rot[1], rot[2] });

			sobj_reflections::ObjectTransformData transform{ std::array<float, 3>{ pos[0], pos[1], pos[2] }, std::array<float, 3>{ rotEuler[0], rotEuler[1], rotEuler[2] } };

			auto& objType = *get_object_property<std::string>(obj, objs, [](const json_reflections::Object& o) -> const std::optional<std::string>& { return o.type; });

			if (is_sobj_instance_object(obj)) {
				auto classId = obj.instanceOf.value();
				auto it = resultObjectsById.find(classId);
				auto& objCtx = it != resultObjectsById.end() ? it->second : (resultObjectsById[classId] = { .object = convert_hson_to_sobj_object(find_object(classId, objs), objs), .objType = objType });

				objCtx.object.instances.emplace_back(transform);
			}
			else {
				auto& objCtx = resultObjectsById[obj.id.value()] = { .object = convert_hson_to_sobj_object(obj, objs), .objType = objType };

				objCtx.object.instances.emplace_back(transform);
			}

			objInstanceCount++;
		}

		size_t idx{};
		for (auto& obj : resultObjectsById) {
			auto& objType = resultObjectTypesByName[obj.second.objType];

			objType.objectIndices.emplace_back(idx++);
		}

		for (auto& objType : resultObjectTypesByName) {
			objType.second.name = objType.first;
			objType.second.objectIndexCount = objType.second.objectIndices.size();
		}

		std::vector<sobj_reflections::ObjectData> resultObjects{};
		std::ranges::copy(std::views::values(resultObjectsById) | std::views::transform([](SOBJObjectContext& ctx) { return ctx.object; }), std::back_inserter(resultObjects));
		auto objectCount = static_cast<unsigned int>(resultObjects.size());

		std::vector<sobj_reflections::ObjectTypeData> resultObjectTypes{};
		std::ranges::copy(std::views::values(resultObjectTypesByName), std::back_inserter(resultObjectTypes));
		auto objectTypeCount = static_cast<unsigned int>(resultObjectTypes.size());

		return {
			.magic = 0x534F424A,
			.version = 1,
			.objectTypeCount = objectTypeCount,
			.objectTypes = std::move(resultObjectTypes),
			.bvh = -1,
			.objects = std::move(resultObjects),
			.objectCount = objectCount,
			.bvhNodeCount = 0,
			.objectInstanceCount = objInstanceCount,
		};
	}

	template<typename GameInterface>
	inline ucsl::resources::sobj::v1::SetObjectData<typename GameInterface::AllocatorSystem>* deserializeSOBJV1(const std::string& filename) {
		auto json = rfl::json::write(deserializeSOBJ<GameInterface>(filename));
		rip::binary::JsonDeserializer<GameInterface, true> deserializer{ json.c_str() };
		return deserializer.deserializeString<ucsl::resources::sobj::v1::SetObjectData<typename GameInterface::AllocatorSystem>>(
			ucsl::reflection::providers::simplerfl<GI>::template reflect<ucsl::resources::sobj::v1::SetObjectData<typename GameInterface::AllocatorSystem>>()
		);
	}
}