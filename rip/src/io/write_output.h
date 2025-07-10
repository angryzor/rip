#pragma once
#include <ucsl/rfl/rflclass.h>
#include <ucsl/resources/object-world/v2.h>
#include <ucsl/resources/object-world/v3.h>
#include <ucsl/resources/sobj/v1.h>
#include <ucsl/resources/swif/v6.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/containers/binary-file/v2.h>
#include <rip/binary/containers/mirage/v1.h>
#include <rip/binary/containers/mirage/v2.h>
#include <rip/binary/containers/swif/SWIF.h>
#include <rip/binary/serialization/JsonSerializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <rip/hson/HsonSerializer.h>
#include <config.h>
#include <ctime>

template<typename T>
void writeOutputFileHSON(const Config& config, T* data) {
	throw std::runtime_error{ "Invalid input resource for HSON output. Use one of GEDIT,SOBJ." };
}

template<typename AllocatorSystem>
void writeOutputFileHSON(const Config& config, ucsl::resources::object_world::v2::ObjectWorldData<AllocatorSystem>* data) {
	rip::hson::serialize<GI>(config.getOutputFile().generic_string(), *data);
}

template<typename AllocatorSystem>
void writeOutputFileHSON(const Config& config, ucsl::resources::object_world::v3::ObjectWorldData<AllocatorSystem>* data) {
	rip::hson::serialize<GI>(config.getOutputFile().generic_string(), *data);
}

template<typename AllocatorSystem>
void writeOutputFileHSON(const Config& config, ucsl::resources::sobj::v1::SetObjectData<AllocatorSystem>* data) {
	rip::hson::serialize<GI>(config.getOutputFile().generic_string(), *data);
}

template<typename T>
void writeOutputFileOther(const Config& config, T* data) {
	switch (config.getOutputFormat()) {
	case Format::BINARY: {
		std::ofstream ofs{ config.getOutputFile(), std::ios::binary };

		if constexpr (std::is_same_v<T, ucsl::resources::swif::v5::SRS_PROJECT> || std::is_same_v<T, ucsl::resources::swif::v6::SRS_PROJECT>) {
			rip::binary::containers::swif::v1::SWIFSerializer serializer{ ofs };
			serializer.serialize<GI>(*data);
		}
		else if constexpr (std::is_same_v<T, ucsl::resources::material::contexts::ContextsData>) {
			if (config.version == "1") {
				rip::binary::containers::mirage::v1::MirageResourceImageWriter<size_t> writer{ ofs };
				auto stream = writer.add_data(3);
				rip::binary::ReflectionSerializer serializer{ stream };
				serializer.serialize<T>(*data, ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());
			}
			else {
				rip::binary::containers::mirage::v2::MirageResourceImageWriter<size_t> writer{ ofs };
				auto root = writer.add_root_node("Material", 1);
				auto contexts = root.add_last_leaf_node("Contexts", 3);
				rip::binary::ReflectionSerializer serializer{ contexts };
				serializer.serialize<T>(*data, ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());
			}
		}
		else if constexpr (std::is_same_v<T, ucsl::resources::sobj::v1::SetObjectData<GI::AllocatorSystem>>) {
			if (config.version == "1-colors") {
				rip::binary::containers::binary_file::v1::BinaryFileSerializer<size_t, std::endian::big, true> serializer{ ofs };
				serializer.serialize<GI>(*data);
			}
			else {
				rip::binary::containers::binary_file::v1::BinaryFileSerializer<size_t, std::endian::little, true> serializer{ ofs };
				serializer.serialize<GI>(*data);
			}
		}
		else if constexpr (std::is_same_v<T, ucsl::resources::nxs::v1::NXSData>) {
			if (config.version == "1-colors") {
				rip::binary::containers::binary_file::v1::BinaryFileSerializer<size_t, std::endian::big> serializer{ ofs };
				serializer.serialize<GI>(*data);
			}
			else {
				rip::binary::containers::binary_file::v1::BinaryFileSerializer<size_t, std::endian::little> serializer{ ofs };
				serializer.serialize<GI>(*data);
			}
		}
		else {
			rip::binary::containers::binary_file::v2::BinaryFileSerializer<size_t> serializer{ ofs };
			serializer.serialize<GI>(*data);
		}
		break;
	}
	case Format::JSON: {
		yyjson_mut_doc* doc = yyjson_mut_doc_new(nullptr);

		rip::binary::JsonSerializer serializer{ doc };
		yyjson_mut_val* result = serializer.serialize(*data, ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());

		yyjson_mut_doc_set_root(doc, result);

		yyjson_write_err err;
		std::string filename = config.getOutputFile().generic_string();
		yyjson_mut_write_file(filename.c_str(), doc, YYJSON_WRITE_PRETTY_TWO_SPACES | YYJSON_WRITE_ALLOW_INF_AND_NAN | YYJSON_WRITE_ALLOW_INVALID_UNICODE, nullptr, &err);

		if (err.code != YYJSON_WRITE_SUCCESS) {
			std::cerr << "Error writing json: " << err.msg << std::endl;
		}

		yyjson_mut_doc_free(doc);
		break;
	}
	}
}

template<typename T>
void writeOutputFile(const Config& config, T* data) {
	if (config.getOutputFormat() == Format::HSON)
		writeOutputFileHSON(config, data);
	else
		writeOutputFileOther(config, data);
}