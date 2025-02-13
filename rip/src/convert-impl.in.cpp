/*
 * We're using this to split up the converters,
 * because the compiler can't handle the amount of types otherwise.
 */

#include "convert-impl.h"
#include "convert-common.h"

namespace rip::cli::convert {
	template<typename Resource>
	void convertResource(const Config& config, Resource resource) { 
		convertVersions(config, resource);
	}

	template void convertResource<resources::${RIP_RESOURCE}>(const Config& config, resources::${RIP_RESOURCE} resource);
}
