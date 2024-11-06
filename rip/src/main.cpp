#include <rip/binary/containers/binary-file/BinaryFile.h>
//#include <rip/binary/serialization/JsonSerializer.h>
#include <rip/binary/serialization/ReflectionSerializer.h>
#include <rip/reflection/traversals/simplerfl.h>
#include <stf/anim/asm/v2.h>
#include <fstream>


int main() {
	std::ifstream ifs{ "chr_sonic.asm", std::ios::binary | std::ios::ate };
	size_t fileSize = ifs.tellg();

	char* data = new char[fileSize];

	ifs.seekg(std::ios::beg);
	ifs.read(data, fileSize);

	rip::binary::containers::binary_file::v2::BinaryFileResolver resolver{ data };
	auto* asmData = (stf::anim::animation_state_machine::v2::AsmData*)resolver.getData(0);

	std::ofstream of{ "chr_sonic_2.asm", std::ios::binary };
	rip::binary::binary_ostream bof{ of };
	rip::binary::containers::binary_file::v2::BinaryFileWriter writer{ bof };
	auto chunk = writer.addDataChunk();
	rip::binary::ReflectionSerializer rfls{ chunk };
	rfls.serialize<rip::reflection::simplerfl_traversal>([&](auto& t) { t(asmData); });
}





//#include <ucsl/containers/arrays/array.h>
//#include <ucsl/strings/variable-string.h>

//struct Exp {
//	unsigned char u8;
//	char i8;
//	unsigned short u16;
//	short i16;
//	unsigned int u32;
//	int i32;
//	unsigned long long u64;
//	long long i64;
//	float f32;
//	double f64;
//	const char* cstr;
//	ucsl::strings::VariableString vstr;
//	ucsl::containers::arrays::Array<int> intarr;
//};
//
//using Experiment = structure<Exp, "Experiment", void,
//	field<unsigned char, "u8">,
//	field<char, "i8">,
//	field<unsigned short, "u16">,
//	field<short, "i16">,
//	field<unsigned int, "u32">,
//	field<int, "i32">,
//	field<unsigned long long, "u64">,
//	field<long long, "i64">,
//	field<float, "f32">,
//	field<double, "f64">,
//	field<const char*, "cstr">,
//	field<ucsl::strings::VariableString, "string">,
//	field<ucsl::containers::arrays::Array<int>, "intarr">
//>;
//
//namespace simplerfl {
//	template<> struct canonical<Exp> { using type = Experiment; };
//}
//using namespace simplerfl;
//
//class FakeAllocator : public ucsl::memory::IAllocator {
//public:
//	void* Alloc(size_t size, size_t alignment) override
//	{
//		return _aligned_malloc(size, alignment);
//	}
//
//	void Free(void* ptr) override
//	{
//		_aligned_free(ptr);
//	}
//};

//int main() {
//	FakeAllocator all{};
//
//	ucsl::containers::arrays::Array<int> arr{ &all };
//	for (int i = 0; i < 100; i++)
//		arr.push_back(i);
//
//	auto it = arr.begin() + 10;
//	const int d = 3;
//	arr.insert(it, size_t(4), d);
//
//	for (auto x : arr)
//		std::cout << x << std::endl;
//
//	{
//		std::ifstream ifs{ "chr_sonic.asm", std::ios::binary | std::ios::ate };
//		size_t fileSize = ifs.tellg();
//
//		char* data = new char[fileSize];
//
//		ifs.seekg(std::ios::beg);
//		ifs.read(data, fileSize);
//
//		rip::binary::containers::binary_file::v2::BinaryFileResolver resolver{ data };
//
//		std::ofstream of{ "test.bin", std::ios::binary };
//		rip::binary::binary_ostream bof{ of };
//		rip::binary::containers::binary_file::v2::BinaryFileWriter writer{ bof };
//
//		Exp exp{
//			.u8 = u'W',
//			.i8 = 'C',
//			.u16 = 876,
//			.i16 = 123,
//			.u32 = 85,
//			.i32 = 97,
//			.u64 = 174,
//			.i64 = 1332,
//			.f32 = 9573,
//			.f64 = 374,
//			.cstr = "hello there!",
//			.vstr = { "hi peeps", &all },
//			.intarr = { &all },
//		};
//
//		for (int i : arr)
//			exp.intarr.push_back(i);
//
//		auto chunk = writer.addDataChunk();
//		rip::binary::ReflectionSerializer rfls{ chunk };
//		rfls.serialize<rip::reflection::simplerfl_traversal>([&](auto& t) { t(&exp); });
//
//		auto* asmData = (stf::anim::animation_state_machine::v2::AsmData*)resolver.getData(0);
//		rip::binary::JsonSerializer json{ "chr_sonic.json" };
//		json.serialize<rip::reflection::simplerfl_traversal>([&](auto& t) { t(asmData); });
//	}
//
//	{
//		std::ifstream ifs{ "test.bin", std::ios::binary | std::ios::ate };
//		size_t fileSize = ifs.tellg();
//
//		char* data = new char[fileSize];
//
//		ifs.seekg(std::ios::beg);
//		ifs.read(data, fileSize);
//
//		rip::binary::containers::binary_file::v2::BinaryFileResolver resolver{ data };
//
//		Exp* exp = (Exp*)resolver.getData(0);
//
//		std::cout
//			<< "u8: " << exp->u8 << std::endl
//			<< "i8: " << exp->i8 << std::endl
//			<< "u16: " << exp->u16 << std::endl
//			<< "i16: " << exp->i16 << std::endl
//			<< "u32: " << exp->u32 << std::endl
//			<< "i32: " << exp->i32 << std::endl
//			<< "u64: " << exp->u64 << std::endl
//			<< "i64: " << exp->i64 << std::endl
//			<< "f32: " << exp->f32 << std::endl
//			<< "f64: " << exp->f64 << std::endl
//			<< "cstr: " << exp->cstr << std::endl
//			<< "vstr: " << exp->vstr << std::endl
//			<< std::endl;
//
//		for (int i : exp->intarr)
//			std::cout << i << std::endl;
//
//		delete[] data;
//	}
//}
