#pragma once
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/hson/HsonDeserializer.h>
#include <config.h>
#include "InputFile.h"

template<typename T>
class HsonInputFile : public InputFile<T> {
	T* data{};

public:
	HsonInputFile(const Config& config) {
		std::string inputFile = config.inputFile.generic_string();
		if constexpr (std::is_same_v<T, ucsl::resources::object_world::v2::ObjectWorldData<GI::AllocatorSystem>>)
			data = rip::hson::deserializeGeditV2<GI>(inputFile);
		else if constexpr (std::is_same_v<T, ucsl::resources::object_world::v3::ObjectWorldData<GI::AllocatorSystem>>)
			data = rip::hson::deserializeGeditV3<GI>(inputFile);
		else if constexpr (std::is_same_v<T, ucsl::resources::sobj::v1::SetObjectData<GI::AllocatorSystem>>)
			data = rip::hson::deserializeSOBJV1<GI>(inputFile);
		else
			throw std::runtime_error{ "Invalid input resource for HSON input. Use one of GEDIT,SOBJ." };
	}

	virtual ~HsonInputFile() {
		GI::AllocatorSystem::get_allocator()->Free(data);
	}

	virtual T* getData() override {
		return data;
	}
};
