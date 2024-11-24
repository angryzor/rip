#include "convert.h"
#include <ucsl/resources/asm/v103.h>
#include <ucsl/resources/fxcol/v1.h>
#include <ucsl/resources/object-world/v2.h>
#include <ucsl/resources/object-world/v3.h>
#include <ucsl/resources/rfl/v1.h>
#include <ucsl/resources/rfl/v2.h>
#include <ucsl/resources/vertex-animation-texture/v1-rangers.h>
#include <ucsl/resources/vertex-animation-texture/v1-miller.h>
#include <ucsl/resources/swif/v6.h>
#include <ucsl-reflection/reflections/resources/asm/v103.h>
#include <ucsl-reflection/reflections/resources/fxcol/v1.h>
#include <ucsl-reflection/reflections/resources/object-world/v2.h>
#include <ucsl-reflection/reflections/resources/object-world/v3.h>
#include <ucsl-reflection/reflections/resources/rfl/v1.h>
#include <ucsl-reflection/reflections/resources/rfl/v2.h>
#include <ucsl-reflection/reflections/resources/vertex-animation-texture/v1-rangers.h>
#include <ucsl-reflection/reflections/resources/vertex-animation-texture/v1-miller.h>
#include <ucsl-reflection/reflections/resources/swif/v6.h>

static_assert(std::is_same_v<simplerfl::canonical_t<ucsl::resources::animation_state_machine::v103::AsmData>, ucsl::resources::animation_state_machine::v103::reflections::AsmData>);

#include <config.h>
#include <io/load_input.h>
#include <io/write_output.h>
#include <tuple>
#include <algorithm>
#include <array>
#include <string>
#include <string_view>

const char* get_rfl1_class(const ucsl::resources::rfl::v1::Ref1Data<>& parent) { return Config::rflClass.c_str(); }
const char* get_rfl2_class(const ucsl::resources::rfl::v2::Ref2Data<>& parent) { return Config::rflClass.c_str(); }

namespace simplerfl {
	template<> struct canonical<ucsl::resources::rfl::v1::Ref1Data<>> { using type = ucsl::resources::rfl::v1::reflections::Ref1Data<ucsl::resources::rfl::v1::Ref1RflData, get_rfl1_class>; };
	template<> struct canonical<ucsl::resources::rfl::v2::Ref2Data<>> { using type = ucsl::resources::rfl::v2::reflections::Ref2Data<ucsl::resources::rfl::v2::Ref2RflData, get_rfl2_class>; };
}

namespace rip::cli::convert {
	template <size_t N>
	struct strlit {
		std::array<char, N> buffer{};

		constexpr strlit(const auto... characters) : buffer{ characters..., '\0' } {}
		constexpr strlit(const std::array<char, N> buffer) : buffer{ buffer } {}
		constexpr strlit(const char(&str)[N]) { std::copy_n(str, N, std::data(buffer)); }

		operator const char* () const { return std::data(buffer); }
		operator std::string() const { return std::string(std::data(buffer), N - 1); }
		constexpr operator std::string_view() const { return std::string_view(std::data(buffer), N - 1); }
	};

	template<ResourceType type, strlit defaultVersion, typename... Versions> struct resource {
		static constexpr ResourceType type = type;
		static constexpr strlit defaultVersion = defaultVersion;
		using versions = std::tuple<Versions...>;
	};
	template<strlit name, typename ResourceDef> struct version {
		static constexpr strlit name = name;
		using resourceDef = ResourceDef;
	};


	using ResourceMap = std::tuple<
		resource<ResourceType::ASM, "v1.03",
			version<"1.03", ucsl::resources::animation_state_machine::v103::AsmData>
		>,
		resource<ResourceType::GEDIT, "3",
			version<"2", ucsl::resources::object_world::v2::ObjectWorldData>,
			version<"3", ucsl::resources::object_world::v3::ObjectWorldData>
		>,
		resource<ResourceType::RFL, "2-1.00",
			version<"1", ucsl::resources::rfl::v1::Ref1Data<>>,
			version<"2-1.00", ucsl::resources::rfl::v2::Ref2Data<>>
		>,
		resource<ResourceType::VAT, "3",
			version<"1-rangers", ucsl::resources::vertex_animation_texture::v1_rangers::VertexAnimationTextureData>,
			version<"1-miller", ucsl::resources::vertex_animation_texture::v1_miller::VertexAnimationTextureData>
		>,
		resource<ResourceType::FXCOL, "1",
			version<"1", ucsl::resources::fxcol::v1::FxColData>
		>,
		resource<ResourceType::SWIF, "6",
			version<"6", ucsl::resources::swif::v6::SRS_PROJECT>
		>
	>;


	template<typename T>
	void convertResource(const Config& config) {
		std::unique_ptr<InputFile<T>> ifl{ loadInputFile<T>(config) };
		writeOutputFile<T>(config, ifl->getData());
	}

	template<ResourceType type, strlit defaultVersion, typename... Versions>
	void convertVersions(const Config& config, resource<type, defaultVersion, Versions...>) {
		std::string defVer = defaultVersion;
		std::string version = config.version.value_or(defVer);

		if (!((version == Versions::name.operator std::string() && (convertResource<typename Versions::resourceDef>(config), true)) || ...))
			throw std::runtime_error{ std::string{ "Version " } + version + " is invalid for selected resource type." };
	}

	template<typename... Resources>
	void convertResources(const Config& config, std::tuple<Resources...>) {
		auto resourceType = config.getResourceType();

		if (!((resourceType == Resources::type && (convertVersions(config, Resources{}), true)) || ...))
			throw std::runtime_error{ "Resource type does not exist." };
	}

	void convert(const Config& config) {
		convertResources(config, ResourceMap{});
	}
}
