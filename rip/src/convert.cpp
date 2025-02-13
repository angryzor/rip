#include "convert.h"
#include "convert-impl.h"
#include <tuple>

namespace rip::cli::convert {
	template<typename... Resources>
	void convertResources(const Config& config, std::tuple<Resources...>) {
		auto resourceType = config.getResourceType();

		if (!((resourceType == Resources::type && (convertResource(config, Resources{}), true)) || ...))
			throw std::runtime_error{ "Resource type does not exist." };
	}

	void convert(const Config& config) {
		convertResources(config, resources::all{});
	}
}
