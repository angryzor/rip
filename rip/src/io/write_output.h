#pragma once
#include <ucsl/rfl/rflclass.h>
#include <ucsl-reflection/traversals/simplerfl.h>
#include <rip/binary/containers/binary-file/BinaryFile.h>
#include <rip/binary/serialization/JsonSerializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <config.h>

template<typename TS, typename T>
void writeOutputFileWithTypes(const Config& config, T* data) {
	switch (config.getOutputFormat()) {
	case Format::BINARY: {
		std::ofstream ofs{ config.getOutputFile(), std::ios::binary };
		rip::binary::binary_ostream bofs{ ofs };
		rip::binary::containers::binary_file::v2::BinaryFileWriter container{ bofs };

		auto chunk = container.addDataChunk();

		rip::binary::ReflectionSerializer serializer{ chunk };
		serializer.serialize<ucsl::reflection::traversals::simplerfl<GI<TS>>>(*data);
		break;
	}
	case Format::JSON: {
		std::string outputFile = config.getOutputFile().generic_string();
		rip::binary::JsonSerializer serializer{ outputFile.c_str() };
		serializer.serialize<ucsl::reflection::traversals::simplerfl<GI<TS>>>(*data);
		break;
	}
	}
}

template<typename T>
void writeOutputFile(const Config& config, T* data) {
	switch (config.game) {
	case Game::WARS: writeOutputFileWithTypes<ucsl::rfl::type_sets::wars, T>(config, data); break;
	default: writeOutputFileWithTypes<ucsl::rfl::type_sets::rangers, T>(config, data); break;
	}
}
