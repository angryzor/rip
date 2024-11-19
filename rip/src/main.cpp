#include <rip/binary/containers/binary-file/BinaryFile.h>
#include <rip/binary/serialization/JsonSerializer.h>
#include <rip/binary/serialization/JsonDeserializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <rip/schemas/hedgeset.h>
#include <ucsl-reflection/traversals/simplerfl.h>
#include <ucsl-reflection/traversals/rflclass.h>
#include <ucsl-reflection/game-interfaces/standalone/game-interface.h>
#include <ucsl/resources/asm/v103.h>
#include <ucsl/resources/object-world/v2.h>
#include <ucsl/resources/object-world/v3.h>
#include <ucsl/resources/vertex-animation-texture/v1-rangers.h>
#include <ucsl/resources/vertex-animation-texture/v1-miller.h>
#include <ucsl-reflection/reflections/resources/asm/v103.h>
#include <ucsl-reflection/reflections/resources/object-world/v2.h>
#include <ucsl-reflection/reflections/resources/object-world/v3.h>
#include <ucsl-reflection/reflections/resources/vertex-animation-texture/v1-rangers.h>
#include <ucsl-reflection/reflections/resources/vertex-animation-texture/v1-miller.h>
#include <CLI/CLI.hpp>
#include <fstream>
#include <iostream>
#include <map>
#include <filesystem>

template<typename TS>
using GI = ucsl::reflection::game_interfaces::standalone::StandaloneGameInterface<TS>;

enum class Game {
	WARS,
	RANGERS,
	MILLER,
};

enum class Format {
	BINARY,
	JSON,
};

enum class ResourceType {
	ASM,
	GEDIT,
	RFL,
	VAT,
};

std::map<std::string, Game> gameMap{
	{ "wars", Game::WARS },
	{ "rangers", Game::RANGERS },
	{ "miller", Game::MILLER },
};

std::map<std::string, Format> formatMap{
	{ "binary", Format::BINARY },
	{ "json", Format::JSON },
};

std::map<std::string, ResourceType> resourceTypeMap{
	{ "asm", ResourceType::ASM },
	{ "gedit", ResourceType::GEDIT },
	{ "vat", ResourceType::VAT },
	//{ "rfl", ResourceType::RFL },
};

std::map<Game, std::string> gameMapReverse{
	{ Game::WARS, "wars" },
	{ Game::RANGERS, "rangers" },
	{ Game::MILLER, "miller" },
};

std::map<Format, std::string> formatMapReverse{
	{ Format::BINARY, "binary" },
	{ Format::JSON, "json" },
};

std::map<ResourceType, std::string> resourceTypeMapReverse{
	{ ResourceType::ASM, "asm" },
	{ ResourceType::GEDIT, "gedit" },
	//{ ResourceType::RFL, "rfl" },
	{ ResourceType::VAT, "vat" },
};

std::map<std::string, ResourceType> resourceTypeByExt{
	{ ".asm", ResourceType::ASM },
	{ ".gedit", ResourceType::GEDIT },
	//{ ".rfl", ResourceType::RFL },
	{ ".vat", ResourceType::VAT },
};

std::map<ResourceType, std::string> extByResourceType{
	{ ResourceType::ASM, ".asm" },
	{ ResourceType::GEDIT, ".gedit" },
	//{ ResourceType::RFL, ".rfl" },
	{ ResourceType::VAT, ".vat" },
};

struct Config {
	std::filesystem::path inputFile{ "input.rfl" };
	std::filesystem::path outputFile{};
	Game game{ Game::MILLER };
	std::optional<ResourceType> resourceType{};
	std::optional<std::string> version{};
	std::optional<Format> inputFormat{};
	std::optional<Format> outputFormat{};
	std::filesystem::path schema{};
	std::filesystem::path hedgesetTemplate{};

	ResourceType getResourceType() const {
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

	Format getInputFormat() const {
		if (inputFormat.has_value())
			return inputFormat.value();

		if (inputFile.extension() == ".json")
			return Format::JSON;

		if (inputFile.extension().generic_string() == extByResourceType[getResourceType()])
			return Format::BINARY;

		throw std::runtime_error{ "The input format was not specified and it cannot be deduced from the other selected options." };
	}

	Format getOutputFormat() const {
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

	std::filesystem::path getOutputFile() const {
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

	void validate() const {
		getResourceType();
		getInputFormat();
		getOutputFormat();
		getOutputFile();
	}
};

template<typename T>
class InputFile {
public:
	virtual ~InputFile() = default;
	virtual T* getData() = 0;
};

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

template<typename TS, typename T>
class JsonInputFile : public InputFile<T> {
	T* data{};

public:
	JsonInputFile(const Config& config) {
		std::string inputFile = config.inputFile.generic_string();
		data = rip::binary::JsonDeserializer<GI<TS>>{ inputFile.c_str() }.deserialize<ucsl::reflection::traversals::simplerfl<GI<TS>>, T>();
	}

	virtual ~JsonInputFile() {
		GI<TS>::get_fallback_allocator()->Free(data);
	}

	virtual T* getData() override {
		return data;
	}
};

template<typename TS, typename T>
InputFile<T>* loadInputFile(const Config& config) {
	switch (config.getInputFormat()) {
	case Format::BINARY: return new BinaryInputFile<T>{ config };
	case Format::JSON: return new JsonInputFile<TS, T>{ config };
	}
}

template<typename T>
InputFile<T>* loadInputFileForGame(const Config& config) {
	switch (config.game) {
	case Game::WARS: return loadInputFile<ucsl::rfl::type_sets::wars, T>(config);
	default: return loadInputFile<ucsl::rfl::type_sets::rangers, T>(config);
	}
}

template<typename TS, typename T>
void writeOutputFile(const Config& config, T* data) {
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
void writeOutputFileForGame(const Config& config, T* data) {
	switch (config.game) {
	case Game::WARS: writeOutputFile<ucsl::rfl::type_sets::wars, T>(config, data); break;
	default: writeOutputFile<ucsl::rfl::type_sets::rangers, T>(config, data); break;
	}
}

template<typename TS>
void loadHedgesetTemplate(const Config& config) {
	auto templ = config.hedgesetTemplate.generic_string();
	rip::schemas::hedgeset::Template<typename GI<TS>::RflSystem> t{ templ.c_str() };
	t.load(GI<TS>::reflectionDB);
}

void loadHedgesetTemplateForGame(const Config& config) {
	switch (config.game) {
	case Game::WARS: loadHedgesetTemplate<ucsl::rfl::type_sets::wars>(config); break;
	default: loadHedgesetTemplate<ucsl::rfl::type_sets::rangers>(config); break;
	}
}

void convertAsm(const Config& config) {
	std::string version = config.version.value_or("103");

	if (version == "103") {
		std::unique_ptr<InputFile<ucsl::resources::animation_state_machine::v103::AsmData>> ifl{ loadInputFileForGame<ucsl::resources::animation_state_machine::v103::AsmData>(config) };
		writeOutputFileForGame<ucsl::resources::animation_state_machine::v103::AsmData>(config, ifl->getData());
		return;
	}

	throw std::runtime_error{ "Unknown ASM version." };
}

void convertGedit(const Config& config) {
	std::string version = config.version.value_or("3");

	if (version == "2") {
		std::unique_ptr<InputFile<ucsl::resources::object_world::v2::ObjectWorldData>> ifl{ loadInputFileForGame<ucsl::resources::object_world::v2::ObjectWorldData>(config) };
		writeOutputFileForGame<ucsl::resources::object_world::v2::ObjectWorldData>(config, ifl->getData());
		return;
	}

	if (version == "3") {
		std::unique_ptr<InputFile<ucsl::resources::object_world::v3::ObjectWorldData>> ifl{ loadInputFileForGame<ucsl::resources::object_world::v3::ObjectWorldData>(config) };
		writeOutputFileForGame<ucsl::resources::object_world::v3::ObjectWorldData>(config, ifl->getData());
		return;
	}

	throw std::runtime_error{ "Unknown gedit version." };
}

void convertVAT(const Config& config) {
	std::string version = config.version.value_or("1-miller");

	if (version == "1-rangers") {
		std::unique_ptr<InputFile<ucsl::resources::vertex_animation_texture::v1_rangers::VertexAnimationTextureData>> ifl{ loadInputFileForGame<ucsl::resources::vertex_animation_texture::v1_rangers::VertexAnimationTextureData>(config) };
		writeOutputFileForGame<ucsl::resources::vertex_animation_texture::v1_rangers::VertexAnimationTextureData>(config, ifl->getData());
		return;
	}

	if (version == "1-miller") {
		std::unique_ptr<InputFile<ucsl::resources::vertex_animation_texture::v1_miller::VertexAnimationTextureData>> ifl{ loadInputFileForGame<ucsl::resources::vertex_animation_texture::v1_miller::VertexAnimationTextureData>(config) };
		writeOutputFileForGame<ucsl::resources::vertex_animation_texture::v1_miller::VertexAnimationTextureData>(config, ifl->getData());
		return;
	}

	throw std::runtime_error{ "Unknown VAT version." };
}

int main(int argc, char** argv) {
	CLI::App app{ "Restoration Issue Pocketknife" };
	argv = app.ensure_utf8(argv);

	Config config{};

	app.add_option("input", config.inputFile, "The input file.")
		->required()
		->check(CLI::ExistingFile);
	app.add_option("output", config.outputFile, "The output file.");
	app.add_option("-g,--game", config.game, "The target game.")
		->transform(CLI::CheckedTransformer(gameMap, CLI::ignore_case));
	app.add_option("-r,--resource-type", config.resourceType, "The resource type.")
		->transform(CLI::CheckedTransformer(resourceTypeMap, CLI::ignore_case));
	auto* version = app.add_option("-v,--version", config.version, "The resource version. Available options are: asm -> 103; gedit -> 2, 3; vat -> 1-rangers, 1-miller");
	app.add_option("-i,--input-format", config.inputFormat, "The input format.")
		->transform(CLI::CheckedTransformer(formatMap, CLI::ignore_case));
	app.add_option("-o,--output-format", config.outputFormat, "The output format.")
		->transform(CLI::CheckedTransformer(formatMap, CLI::ignore_case));
	auto* schemaOpt = app.add_option("-s,--schema", config.schema, "The RFL Schema file to use. (doesn't work yet)");
	app.add_option("-t,--hedgeset-template", config.hedgesetTemplate, "The HedgeSet template file to use.")
		->excludes(schemaOpt);
	app.validate_positionals();

	CLI11_PARSE(app, argc, argv);

	try {
		config.validate();

		std::cerr << "Converting " << resourceTypeMapReverse[config.getResourceType()] << " from " << formatMapReverse[config.getInputFormat()] << " to " << formatMapReverse[config.getOutputFormat()] << " using types for game " << gameMapReverse[config.game] << "..." << std::endl;
		std::cerr << "Input file: " << config.inputFile.generic_string() << std::endl;
		std::cerr << "Output file: " << config.getOutputFile().generic_string() << std::endl;

		if (!config.hedgesetTemplate.empty())
			loadHedgesetTemplateForGame(config);

		switch (config.getResourceType()) {
		case ResourceType::ASM: convertAsm(config); break;
		case ResourceType::GEDIT: convertGedit(config); break;
		//case ResourceType::RFL: convertRfl(config); break;
		case ResourceType::VAT: convertVAT(config); break;
		}

		std::cerr << "Conversion successful." << std::endl;
	}
	catch (std::runtime_error& e) {
		std::cerr << e.what();
		return 1;
	}

	return 0;
}
