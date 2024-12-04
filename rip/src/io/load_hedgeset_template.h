#pragma once
#include <ucsl/rfl/rflclass.h>
#include <rip/schemas/hedgeset.h>
#include <config.h>

void loadHedgesetTemplate(const Config& config) {
	auto templ = rip::schemas::hedgeset::load(config.hedgesetTemplate.generic_string());
	rip::schemas::hedgeset::schema_builder s{ templ };
	GI::reflectionDB->load_schema(s.get_schema());
}
