#include <simple-reflection/simple-reflection.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <rip/binary/containers/binary-file/BinaryFile.h>
#include <rip/reflection/traversals/simplerfl.h>
#include <fstream>

using namespace simplerfl;

using Experiment = structure<"Experiment", void,
	field<int, "foo">,
	field<float, "bar">,
	field<const char*, "name">
>;

//using FFF = canonical_t<const char*>;

struct Exp {
	int foo;
	float bar;
	const char* name;
};

int main() {
	{
		std::ofstream of{ "test.bin", std::ios::binary };
		rip::binary::binary_ostream bof{ of };
		rip::binary::containers::binary_file::v2::BinaryFileWriter writer{ bof };

		auto chunk = writer.addDataChunk();

		rip::binary::ReflectionSerializer rfls{chunk};

		rfls.serialize<rip::reflection::simplerfl_traversal>([](auto& t) {
			Exp exp{ 4, 6, "hello there!" };
			t.operator()<Experiment>(&exp);
		});
	}

	{
		std::ifstream ifs{ "test.bin", std::ios::binary | std::ios::ate };
		size_t fileSize = ifs.tellg();

		char* data = new char[fileSize];

		ifs.seekg(std::ios::beg);
		ifs.read(data, fileSize);

		rip::binary::containers::binary_file::v2::BinaryFileResolver resolver{ data };

		Exp exp = *(Exp*)resolver.getData(0);

		std::cout << "foo:" << exp.foo << ", bar:" << exp.bar << ", name: " << exp.name << std::endl;

		delete[] data;
	}
}
