#pragma once
#include <ucsl/rfl/rflclass.h>
#include <config.h>
#include "BinaryInputFile.h"
#include "JsonInputFile.h"

template<typename TS, typename T>
InputFile<T>* loadInputFileWithTypes(const Config& config) {
	switch (config.getInputFormat()) {
	case Format::BINARY: return new BinaryInputFile<T>{ config };
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
