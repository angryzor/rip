#include "config.h"
#include <map>
#include <util.h>

std::map<std::string, ResourceType> resourceTypeByExt{
	{ ".asm", ResourceType::ASM },
	{ ".gedit", ResourceType::GEDIT },
	{ ".rfl", ResourceType::RFL },
	{ ".vat", ResourceType::VAT },
	{ ".fxcol", ResourceType::FXCOL },
	{ ".swif", ResourceType::SWIF },
	{ ".orc", ResourceType::SOBJ },
};

auto extByResourceType = reverse_map(resourceTypeByExt);

std::string Config::rflClass{};

ResourceType Config::getResourceType() const {
	if (resourceType.has_value())
		return resourceType.value();

	std::string inputExt = inputFile.extension().generic_string();
	if (inputExt == ".json" && inputFile.stem().has_extension()) {
		std::string binExt = inputFile.stem().extension().generic_string();

		if (resourceTypeByExt.contains(binExt))
			return resourceTypeByExt[binExt];
	}

	if (resourceTypeByExt.contains(inputExt))
		return resourceTypeByExt[inputExt];

	if (!outputFile.empty()) {
		std::string outputExt = outputFile.extension().generic_string();

		if (resourceTypeByExt.contains(outputExt))
			return resourceTypeByExt[outputExt];
	}

	throw std::runtime_error{ "The resource type was not specified and it cannot be deduced from the other selected options." };
}

Format Config::getInputFormat() const {
	if (inputFormat.has_value())
		return inputFormat.value();

	if (inputFile.extension() == ".json")
		return Format::JSON;

	if (inputFile.extension().generic_string() == extByResourceType[getResourceType()])
		return Format::BINARY;

	throw std::runtime_error{ "The input format was not specified and it cannot be deduced from the other selected options." };
}

Format Config::getOutputFormat() const {
	if (outputFormat.has_value())
		return outputFormat.value();

	if (outputFile.empty())
		return getInputFormat() == Format::BINARY ? Format::JSON : Format::BINARY;

	if (outputFile.extension() == ".json")
		return Format::JSON;

	if (outputFile.extension().generic_string() == extByResourceType[getResourceType()])
		return Format::BINARY;

	throw std::runtime_error{ "The output format was not specified and it cannot be deduced from the other selected options." };
}

std::filesystem::path Config::getOutputFile() const {
	if (!outputFile.empty())
		return outputFile;

	std::filesystem::path replacedFile{ inputFile };

	if (replacedFile.extension() == ".json" && replacedFile.stem().has_extension() && resourceTypeByExt.contains(replacedFile.stem().extension().generic_string()))
		replacedFile.replace_extension();

	switch (getOutputFormat()) {
	case Format::JSON: return std::move(replacedFile.replace_extension(extByResourceType[getResourceType()] + ".json"));
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
