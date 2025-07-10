#pragma once
#include <rfl.hpp>
#include <rfl/json.hpp>

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
			std::optional<std::string> name{};
			std::optional<std::string> author{};
			std::optional<std::string> date{};
			std::optional<std::string> version{};
			std::optional<std::string> description{};
		};

		struct File {
			rfl::Rename<"$schema", std::string> schema{};
			unsigned int version{};
			std::optional<Metadata> metadata{};
			std::vector<Object> objects{};
		};
	}

	namespace gedit_reflections {
		struct ObjectTransformData {
			std::optional<std::array<float, 3>> position;
			std::optional<std::array<float, 3>> rotation;
		};

		struct ComponentData {
			std::string type;
			unsigned long long size;
			rfl::Generic data;
		};

		struct ObjectData {
			std::string gameObjectClass;
			std::string name;
			std::string id;
			std::string parentID;
			ObjectTransformData transform;
			ObjectTransformData localTransform;
			std::vector<ComponentData> componentData;
			rfl::Generic spawnerData;
		};

		struct ObjectWorldData {
			std::vector<ObjectData> objects;
		};
	}

	namespace sobj_reflections {
		struct ObjectTransformData {
			std::optional<std::array<float, 3>> position;
			std::optional<std::array<float, 3>> rotation;
		};

		struct ObjectData {
			std::string id;
			unsigned int objectClassId;
			unsigned int bvhNode;
			float replicationInterval;
			float m_distance;
			float m_range;
			std::vector<ObjectTransformData> instances;
			rfl::Generic spawnerData;
		};

		struct ObjectTypeData {
			std::string name;
			unsigned int objectIndexCount;
			std::vector<unsigned short> objectIndices;
		};

		struct SetObjectData {
			unsigned int magic;
			unsigned int version;
			unsigned int objectTypeCount;
			std::vector<ObjectTypeData> objectTypes;
			int bvh;
			std::vector<ObjectData> objects;
			unsigned int objectCount;
			unsigned int bvhNodeCount;
			unsigned int objectInstanceCount;
		};
	}
}
