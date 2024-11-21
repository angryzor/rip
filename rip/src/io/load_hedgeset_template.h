#pragma once
#include <ucsl/rfl/rflclass.h>
#include <rip/schemas/hedgeset.h>
#include <config.h>

template<typename TS>
void loadHedgesetTemplateWithTypes(const Config& config) {
	auto templ = config.hedgesetTemplate.generic_string();
	rip::schemas::hedgeset::Template<typename GI<TS>::RflSystem> t{ templ.c_str() };
	t.load(GI<TS>::reflectionDB);
}

void loadHedgesetTemplate(const Config& config) {
	switch (config.game) {
	case Game::WARS: loadHedgesetTemplateWithTypes<ucsl::rfl::type_sets::wars>(config); break;
	default: loadHedgesetTemplateWithTypes<ucsl::rfl::type_sets::rangers>(config); break;
	}
}
