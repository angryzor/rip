#pragma once
#include <config.h>
#include "resource-table.h"

namespace rip::cli::convert {
	template<typename Resource>
	void convertResource(const Config& config, Resource resource);
}
