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
}
