#pragma once
#include <ucsl/rfl/rflclass.h>
#include <config.h>
#include "BinaryInputFile.h"
#include "SWIFInputFile.h"
#include "JsonInputFile.h"

template<typename T>
InputFile<T>* loadInputFile(const Config& config) {
	switch (config.getInputFormat()) {
	case Format::BINARY:
		if constexpr (std::is_same_v<T, ucsl::resources::swif::v6::SRS_PROJECT>)
			return new SWIFInputFile{ config };
		else
			return new BinaryInputFile<T>{ config };
	case Format::JSON: return new JsonInputFile<T>{ config };
	default: assert("unknown input format"); return nullptr;
	}
}
