#pragma once
#include <ucsl/resources/swif/v6.h>
#include <rip/binary/containers/swif/SWIF.h>
#include <config.h>
#include "InputFile.h"
#include <fstream>

class SWIFInputFile : public InputFile<ucsl::resources::swif::v6::SRS_PROJECT> {
	std::unique_ptr<uint8_t[]> fileData;
	std::unique_ptr<rip::binary::containers::swif::v6::SWIFResolver> resolver;

public:
	SWIFInputFile(const Config& config) {
		std::ifstream ifs{ config.inputFile, std::ios::binary | std::ios::ate };
		size_t fileSize = ifs.tellg();

		fileData = std::make_unique<uint8_t[]>(fileSize);

		ifs.seekg(std::ios::beg);
		ifs.read((char*)&fileData[0], fileSize);

		resolver = std::make_unique<rip::binary::containers::swif::v6::SWIFResolver>(&fileData[0]);
	}

	virtual ucsl::resources::swif::v6::SRS_PROJECT* getData() override {
		return resolver->getProject();
	}
};
