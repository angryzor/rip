#pragma once
#include <ucsl/rfl/rflclass.h>
#include <config.h>
#include "BinaryInputFile.h"
#include "SWIFInputFile.h"
#include "JsonInputFile.h"

template<typename TS, typename T>
InputFile<T>* loadInputFileWithTypes(const Config& config) {
	switch (config.getInputFormat()) {
	case Format::BINARY:
		if constexpr (std::is_same_v<T, ucsl::resources::swif::v6::SRS_PROJECT>)
			return new SWIFInputFile{ config };
		else
			return new BinaryInputFile<T>{ config };
	case Format::JSON: return new JsonInputFile<TS, T>{ config };
	}
}

template<typename T>
InputFile<T>* loadInputFile(const Config& config) {
	switch (config.game) {
	case Game::WARS: return loadInputFileWithTypes<ucsl::rfl::type_sets::wars, T>(config);
	default: return loadInputFileWithTypes<ucsl::rfl::type_sets::rangers, T>(config);
	}
}
