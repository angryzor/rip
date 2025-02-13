#pragma once
#include <config.h>
#include <io/load_input.h>
#include <io/write_output.h>
#include "resource-table.h"

namespace rip::cli::convert {
	template<typename T>
	void convertVersion(const Config& config) {
		std::unique_ptr<InputFile<T>> ifl{ loadInputFile<T>(config) };
		writeOutputFile<T>(config, ifl->getData());
	}

	template<ResourceType type, strlit defaultVersion, typename... Versions>
	void convertVersions(const Config& config, resources::resource<type, defaultVersion, Versions...>) {
		std::string defVer = defaultVersion;
		std::string version = config.version.value_or(defVer);

		if (!((version == Versions::name.operator std::string() && (convertVersion<typename Versions::resourceDef>(config), true)) || ...))
			throw std::runtime_error{ std::string{ "Version " } + version + " is invalid for selected resource type." };
	}
}
