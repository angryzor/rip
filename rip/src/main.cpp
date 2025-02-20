#include <config.h>
#include <io/load_hedgeset_template.h>
#include <io/load_input.h>
#include <io/write_output.h>
#include <convert.h>
#include <util.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <map>
#include <ucsl-reflection/reflections/resources/fxcol/v1.h>

std::map<std::string, Format> formatMap{
	{ "binary", Format::BINARY },
	{ "json", Format::JSON },
	{ "hson", Format::HSON },
};

std::map<std::string, ResourceType> resourceTypeMap{
	{ "asm", ResourceType::ASM },
	{ "gedit", ResourceType::GEDIT },
	{ "map", ResourceType::MAP },
	{ "path", ResourceType::PATH },
	{ "material", ResourceType::MATERIAL },
	{ "rfl", ResourceType::RFL },
	{ "vat", ResourceType::VAT },
	{ "fxcol", ResourceType::FXCOL },
	{ "swif", ResourceType::SWIF },
	{ "sobj", ResourceType::SOBJ },
	{ "nxs", ResourceType::NXS },
	{ "pcmodel", ResourceType::PCMODEL },
	{ "masterlevel", ResourceType::MASTER_LEVEL },
	{ "densitysetting", ResourceType::DENSITY_SETTING },
	{ "aism", ResourceType::AISM },
};

std::map<std::string, AddressingMode> addressingModeMap{
	{ "32", AddressingMode::_32 },
	{ "64", AddressingMode::_64 },
};

auto formatMapReverse = reverse_map(formatMap);
auto resourceTypeMapReverse = reverse_map(resourceTypeMap);
auto addressingModeMapReverse = reverse_map(addressingModeMap);

int main(int argc, char** argv) {
	CLI::App app{ "Restoration Issue Pocketknife" };
	argv = app.ensure_utf8(argv);
	
	Config config{};

	app.add_option("input", config.inputFile, "The input file.")
		->required()
		->check(CLI::ExistingFile);
	app.add_option("output", config.outputFile, "The output file.");
	app.add_option("-r,--resource-type", config.resourceType, "The resource type.")
		->transform(CLI::CheckedTransformer(resourceTypeMap, CLI::ignore_case));
	auto* version = app.add_option("-v,--version", config.version, "The resource version. Available options are: asm -> 103; gedit -> 2, 3; vat -> 1-rangers, 1-miller; fxcol -> 1");
	app.add_option("-i,--input-format", config.inputFormat, "The input format.")
		->transform(CLI::CheckedTransformer(formatMap, CLI::ignore_case));
	app.add_option("-o,--output-format", config.outputFormat, "The output format.")
		->transform(CLI::CheckedTransformer(formatMap, CLI::ignore_case));
	auto* schemaOpt = app.add_option("-s,--schema", config.schema, "The RFL Schema file to use. (doesn't work yet)");
	app.add_option("-t,--hedgeset-template", config.hedgesetTemplate, "The HedgeSet template file to use.")
		->excludes(schemaOpt);
	app.add_option("-c,--rfl-class", Config::rflClass, "When converting RFL files: the name of the RflClass to use.");
	app.validate_positionals();

	CLI11_PARSE(app, argc, argv);

	try {
		config.validate();

		std::cerr << "Converting " << resourceTypeMapReverse[config.getResourceType()] << " from " << formatMapReverse[config.getInputFormat()] << " to " << formatMapReverse[config.getOutputFormat()] << std::endl;
		std::cerr << "Input file: " << config.inputFile.generic_string() << std::endl;
		std::cerr << "Output file: " << config.getOutputFile().generic_string() << std::endl;

		if (config.getInputFormat() == Format::HSON)
			throw new std::runtime_error{ "HSON input currently not yet supported." };

		ucsl::reflection::game_interfaces::standalone::StandaloneGameInterface::boot();

		if (!config.hedgesetTemplate.empty())
			loadHedgesetTemplate(config);

		rip::cli::convert::convert(config);

		std::cerr << "Conversion successful." << std::endl;
	}
	catch (std::runtime_error& e) {
		std::cerr << e.what();
		return 1;
	}

	return 0;
}
