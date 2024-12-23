#pragma once
#include <rip/binary/containers/binary-file/v1.h>
#include <rip/binary/containers/binary-file/v2.h>
#include <config.h>
#include "InputFile.h"
#include "mem_stream.h"
#include <fstream>

template<typename T, typename AddrType>
class BinaryInputFileV1 : public InputFile<T> {
	std::unique_ptr<uint8_t[]> fileData;
	T* data{};

public:
	BinaryInputFileV1(const Config& config) {
		std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
		size_t fileSize = ifs.tellg();
		
		fileData = std::make_unique<uint8_t[]>(fileSize);
		
		ifs.seekg(std::ios::beg);
		ifs.read((char*)&fileData[0], fileSize);

        imemstream ims{ (char*)&fileData[0], fileSize };
        rip::binary::containers::binary_file::v1::BinaryFileDeserializer<AddrType> deserializer{ ims };

		//std::ifstream ifs{ config.inputFile, std::ios::binary };
		//rip::binary::containers::binary_file::v1::BinaryFileDeserializer<size_t> deserializer{ ifs };

		data = deserializer.deserialize<GI, T>();
	}

	virtual ~BinaryInputFileV1() {
		GI::AllocatorSystem::get_allocator()->Free(data);
	}

	virtual T* getData() override {
		return data;
	}
};

template<typename T, typename AddrType>
class BinaryInputFileV2 : public InputFile<T> {
    std::unique_ptr<uint8_t[]> fileData;
    T* data{};

public:
    BinaryInputFileV2(const Config& config) {
        std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
        size_t fileSize = ifs.tellg();

        fileData = std::make_unique<uint8_t[]>(fileSize);

        ifs.seekg(std::ios::beg);
        ifs.read((char*)&fileData[0], fileSize);

        imemstream ims{ (char*)&fileData[0], fileSize };
        rip::binary::containers::binary_file::v2::BinaryFileDeserializer<AddrType> deserializer{ ims };

        //std::ifstream ifs{ config.inputFile, std::ios::binary };
        //rip::binary::containers::binary_file::v2::BinaryFileDeserializer<size_t> deserializer{ ifs };

        data = deserializer.deserialize<GI, T>();
    }

    virtual ~BinaryInputFileV2() {
        GI::AllocatorSystem::get_allocator()->Free(data);
    }

    virtual T* getData() override {
        return data;
    }
};
