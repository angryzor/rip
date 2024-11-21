#include <config.h>
#include <io/load_hedgeset_template.h>
#include <io/load_input.h>
#include <io/write_output.h>
#include <convert.h>
#include <util.h>
#include <CLI/CLI.hpp>
#include <iostream>
#include <map>

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
	{ "rfl", ResourceType::RFL },
	{ "vat", ResourceType::VAT },
	{ "fxcol", ResourceType::FXCOL },
};

auto gameMapReverse = reverse_map(gameMap);
auto formatMapReverse = reverse_map(formatMap);
auto resourceTypeMapReverse = reverse_map(resourceTypeMap);

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

		std::cerr << "Converting " << resourceTypeMapReverse[config.getResourceType()] << " from " << formatMapReverse[config.getInputFormat()] << " to " << formatMapReverse[config.getOutputFormat()] << " using types for game " << gameMapReverse[config.game] << "..." << std::endl;
		std::cerr << "Input file: " << config.inputFile.generic_string() << std::endl;
		std::cerr << "Output file: " << config.getOutputFile().generic_string() << std::endl;

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
