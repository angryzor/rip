#pragma once
#include <ucsl-reflection/providers/simplerfl.h>
#include <rip/binary/containers/mirage/v1.h>
#include <rip/binary/containers/mirage/v2.h>
#include <rip/binary/serialization/ReflectionDeserializer.h>
#include <config.h>
#include "InputFile.h"
#include "mem_stream.h"
#include <fstream>

template<typename T, typename AddrType>
class MirageInputFileV1 : public InputFile<T> {
    std::unique_ptr<uint8_t[]> fileData;
    T* data{};

public:
    MirageInputFileV1(const Config& config) {
        std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
        size_t fileSize = ifs.tellg();

        fileData = std::make_unique<uint8_t[]>(fileSize);

        ifs.seekg(std::ios::beg);
        ifs.read((char*)&fileData[0], fileSize);

        imemstream ims{ (char*)&fileData[0], fileSize };
        rip::binary::containers::mirage::v1::MirageResourceImageReader<AddrType> reader{ ims };

        auto stream = reader.get_data();

        rip::binary::ReflectionDeserializer<GI, decltype(stream)> deserializer{ stream };
        data = deserializer.deserialize<T>(ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());
    }

    virtual ~MirageInputFileV1() {
        GI::AllocatorSystem::get_allocator()->Free(data);
    }

    virtual T* getData() override {
        return data;
    }
};

template<typename T, typename AddrType>
class MirageInputFileV2 : public InputFile<T> {
    std::unique_ptr<uint8_t[]> fileData;
    T* data{};

public:
    MirageInputFileV2(const Config& config) {
        std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
        size_t fileSize = ifs.tellg();

        fileData = std::make_unique<uint8_t[]>(fileSize);

        ifs.seekg(std::ios::beg);
        ifs.read((char*)&fileData[0], fileSize);

        imemstream ims{ (char*)&fileData[0], fileSize };
        rip::binary::containers::mirage::v2::MirageResourceImageReader<AddrType> reader{ ims };

        auto root_node = reader.get_root_node();

        if (root_node.header.magic == "Material") {
            root_node.forEachChild([&](auto& child) {
                if (child.header.magic == "Contexts") {
                    rip::binary::ReflectionDeserializer<GI, decltype(child)> deserializer{ child };

                    data = deserializer.deserialize<T>(ucsl::reflection::providers::simplerfl<GI>::template reflect<T>());
                }
            });
        }

        //std::ifstream ifs{ config.inputFile, std::ios::binary };
        //rip::binary::containers::binary_file::v2::BinaryFileDeserializer<size_t> deserializer{ ifs };
    }

    virtual ~MirageInputFileV2() {
        GI::AllocatorSystem::get_allocator()->Free(data);
    }

    virtual T* getData() override {
        return data;
    }
};
