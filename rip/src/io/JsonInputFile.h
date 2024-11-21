#pragma once
#include <ucsl-reflection/traversals/simplerfl.h>
#include <rip/binary/serialization/JsonDeserializer.h>
#include <config.h>
#include "InputFile.h"

template<typename TS, typename T>
class JsonInputFile : public InputFile<T> {
	T* data{};

public:
	JsonInputFile(const Config& config) {
		std::string inputFile = config.inputFile.generic_string();
		data = rip::binary::JsonDeserializer<GI<TS>>{ inputFile.c_str() }.deserialize<ucsl::reflection::traversals::simplerfl<GI<TS>>, T>();
	}

	virtual ~JsonInputFile() {
		GI<TS>::get_fallback_allocator()->Free(data);
	}

	virtual T* getData() override {
		return data;
	}
};
