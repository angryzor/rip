#pragma once
#include <ucsl/rfl/rflclass.h>
#include <ucsl/resources/sobj/v1.h>
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
		else if constexpr (std::is_same_v<T, ucsl::resources::sobj::v1::SetObjectData<GI::AllocatorSystem>>)
			return new BinaryInputFileV1<T, size_t>{ config };
		else
			return new BinaryInputFileV2<T, size_t>{ config };
	case Format::JSON: return new JsonInputFile<T>{ config };
	default: assert("unknown input format"); return nullptr;
	}
}
