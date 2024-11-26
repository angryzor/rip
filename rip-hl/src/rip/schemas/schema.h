#pragma once
#include <ucsl/rfl/rflclass.h>
#include <ucsl-reflection/game-interfaces/standalone/rfl-system.h>
#include <map>

namespace rip::schemas {
	class Schema {
		using RflSystem = ucsl::rfl::miller;

		struct ObjectInfo {
			std::optional<std::string> category{};
			std::optional<std::string> rfl_class{};
		};

		struct ComponentInfo {
			std::optional<std::string> rfl_class{};
		};

		std::map<std::string, std::shared_ptr<typename RflSystem::RflClass>> classes{};
		std::map<std::string, ObjectInfo> objects{};
		std::map<std::string, ComponentInfo> tags{};

		//void apply_to_standalone_db(ucsl::reflection::game_interfaces::standalone::ReflectionDB<typename RflSystem>& db) {
		//	for (auto& [name, rfl_class] : classes)
		//		db.classes[name] = rfl_class;

		//	for (auto& [name, rfl_class] : objects)
		//		db.spawnerDataRflClasses[name] = rfl_class;
		//}
	public:
		template<typename GameInterface>
		static Schema generate_gedit_schema() {

		}
	};

}