#pragma once
#include <rip/binary/containers/binary-file/BinaryFile.h>
#include <config.h>
#include "InputFile.h"
#include <fstream>

template<typename T>
class BinaryInputFile : public InputFile<T> {
	std::unique_ptr<uint8_t[]> fileData;
	std::unique_ptr<rip::binary::containers::binary_file::v2::BinaryFileResolver> resolver;

public:
	BinaryInputFile(const Config& config) {
		std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
		size_t fileSize = ifs.tellg();

		fileData = std::make_unique<uint8_t[]>(fileSize);

		ifs.seekg(std::ios::beg);
		ifs.read((char*)&fileData[0], fileSize);

		resolver = std::make_unique<rip::binary::containers::binary_file::v2::BinaryFileResolver>(&fileData[0]);
	}

	virtual T* getData() override {
		return (T*)resolver->getData(0);
	}
};
