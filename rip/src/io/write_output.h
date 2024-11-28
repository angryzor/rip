#pragma once
#include <ucsl/rfl/rflclass.h>
#include <ucsl/resources/swif/v6.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/containers/binary-file/BinaryFile.h>
#include <rip/binary/containers/swif/SWIF.h>
#include <rip/binary/serialization/JsonSerializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <config.h>

template<typename T>
void writeOutputFile(const Config& config, T* data) {
	switch (config.getOutputFormat()) {
	case Format::BINARY: {
		std::ofstream ofs{ config.getOutputFile(), std::ios::binary };
		rip::binary::binary_ostream bofs{ ofs };

		if constexpr (std::is_same_v<T, ucsl::resources::swif::v6::SRS_PROJECT>) {
			rip::binary::containers::swif::v6::SWIFSerializer serializer{ bofs };
			serializer.serialize<GI>(*data);
		}
		else {
			rip::binary::containers::binary_file::v2::BinaryFileSerializer serializer{ bofs };
			serializer.serialize<GI>(*data);
		}
		break;
	}
	case Format::JSON: {
		std::string outputFile = config.getOutputFile().generic_string();
		rip::binary::JsonSerializer serializer{ outputFile.c_str() };
		serializer.serialize(*data, ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());
		break;
	}
	}
}
