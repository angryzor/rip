#include "config.h"
#include <map>
#include <util.h>

std::map<std::string, ResourceType> resourceTypeByExt{
	{ ".asm", ResourceType::ASM },
	{ ".gedit", ResourceType::GEDIT },
	{ ".map.bin", ResourceType::MAP },
	{ ".path.bin", ResourceType::PATH },
	{ ".material", ResourceType::MATERIAL },
	{ ".rfl", ResourceType::RFL },
	{ ".vat", ResourceType::VAT },
	{ ".fxcol", ResourceType::FXCOL },
	{ ".svcol", ResourceType::SVCOL },
	{ ".swif", ResourceType::SWIF },
	{ ".orc", ResourceType::SOBJ },
	{ ".pcmodel", ResourceType::PCMODEL },
	{ ".mlevel", ResourceType::MASTER_LEVEL },
	{ ".densitysetting", ResourceType::DENSITY_SETTING },
	{ ".aism", ResourceType::AISM },
};

auto extByResourceType = reverse_map(resourceTypeByExt);

std::string Config::rflClass{};

std::optional<ResourceType> getResourceTypeByExtension(const std::filesystem::path& file) {
	std::filesystem::path filename{ file };
	std::string ext{};

	while (filename.has_extension()) {
		ext = filename.extension().generic_string() + ext;
		filename = filename.stem();

		if (resourceTypeByExt.contains(ext))
			return resourceTypeByExt[ext];
	}

	return std::nullopt;
}

ResourceType Config::getResourceType() const {
	if (resourceType.has_value())
		return resourceType.value();

	std::string inputExt = inputFile.extension().generic_string();
	if ((inputExt == ".json" || inputExt == ".hson"))
		if (auto resType = getResourceTypeByExtension(inputFile.stem()))
			return resType.value();

	if (auto resType = getResourceTypeByExtension(inputFile))
		return resType.value();

	if (!outputFile.empty())
		if (auto resType = getResourceTypeByExtension(outputFile))
			return resType.value();

	throw std::runtime_error{ "The resource type was not specified and it cannot be deduced from the other selected options." };
}

Format Config::getInputFormat() const {
	if (inputFormat.has_value())
		return inputFormat.value();

	if (inputFile.extension() == ".json")
		return Format::JSON;

	if (inputFile.extension() == ".hson")
		return Format::HSON;

	if (getResourceTypeByExtension(inputFile) == getResourceType())
		return Format::BINARY;

	throw std::runtime_error{ "The input format was not specified and it cannot be deduced from the other selected options." };
}

Format Config::getOutputFormat() const {
	if (outputFormat.has_value())
		return outputFormat.value();

	if (outputFile.empty()) {
		if (getInputFormat() != Format::BINARY)
			return Format::BINARY;

		auto resType = getResourceType();

		return resType == ResourceType::GEDIT || resType == ResourceType::SOBJ ? Format::HSON : Format::JSON;
	}

	if (outputFile.extension() == ".json")
		return Format::JSON;

	if (outputFile.extension() == ".hson")
		return Format::HSON;

	if (outputFile.extension().generic_string() == extByResourceType[getResourceType()])
		return Format::BINARY;

	throw std::runtime_error{ "The output format was not specified and it cannot be deduced from the other selected options." };
}

std::filesystem::path Config::getOutputFile() const {
	if (!outputFile.empty())
		return outputFile;

	std::filesystem::path replacedFile{ inputFile };

	if ((replacedFile.extension() == ".json" || replacedFile.extension() == ".hson") && replacedFile.stem().has_extension() && resourceTypeByExt.contains(replacedFile.stem().extension().generic_string()))
		replacedFile.replace_extension();

	switch (getOutputFormat()) {
	case Format::JSON: return std::move(replacedFile.replace_extension(extByResourceType[getResourceType()] + ".json"));
	case Format::HSON: return std::move(replacedFile.replace_extension(extByResourceType[getResourceType()] + ".hson"));
	case Format::BINARY: return std::move(replacedFile.replace_extension(extByResourceType[getResourceType()]));
	}

	throw std::runtime_error{ "The output file was not specified and it cannot be deduced from the other selected options." };
}

void Config::validate() const {
	getResourceType();
	getInputFormat();
	getOutputFormat();
	getOutputFile();
}
