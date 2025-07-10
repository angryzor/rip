#pragma once

#include <catch2/catch_test_macros.hpp>
#include <ucsl-reflection/accessors/binary.h>
#include <ucsl-reflection/providers/simplerfl.h>
#include <ucsl-reflection/game-interfaces/standalone/game-interface.h>
#include <ucsl-reflection/traversals/fold.h>
#include <ucsl-reflection/reflections/basic-types.h>
#include <rip/binary/accessors/json.h>

using namespace simplerfl;
using namespace std;
using namespace ucsl::reflection;

using GI = game_interfaces::standalone::StandaloneGameInterface;

enum class TestEnum1Repr : unsigned short {
	OPT1,
	OPT2,
};

struct Test1Repr {
	bool b;
	unsigned char u8;
	char s8;
	unsigned short u16;
	short s16;
	unsigned int u32;
	int s32;
	unsigned long long u64;
	long long s64;
	float f32;
	ucsl::math::Vector2 v2;
	ucsl::math::Vector3 v3;
	ucsl::math::Vector4 v4;
	ucsl::math::Quaternion quat;
	ucsl::math::Position pos;
	ucsl::math::Rotation rot;
	ucsl::math::Matrix34 m34;
	ucsl::math::Matrix44 m44;
	const char* str;
	ucsl::strings::VariableString vstr;
	ucsl::objectids::ObjectIdV1 oid1;
	ucsl::objectids::ObjectIdV2 oid2;
	ucsl::colors::Color8RGBA col8rgba;
	ucsl::colors::Color8ABGR col8abgr;
	ucsl::colors::ColorfRGBA colfrgba;
	ucsl::colors::ColorfABGR colfabgr;
	TestEnum1Repr enm;
	alignas(16) int barCount;
	int* bars;
	ucsl::containers::arrays::Array<int, GI::AllocatorSystem> arr;
	ucsl::containers::arrays::TArray<int, GI::AllocatorSystem> tarr;
};

using TestEnum1 = enumeration<TestEnum1Repr, "TestEnum1", unsigned short,
	option<"OPT1">,
	option<"OPT2">
>;

using Test1 = structure<Test1Repr, "Test1", void,
	field<bool, "b">,
	field<unsigned char, "u8">,
	field<char, "s8">,
	field<unsigned short, "u16">,
	field<short, "s16">,
	field<unsigned int, "u32">,
	field<int, "s32">,
	field<unsigned long long, "u64">,
	field<long long, "s64">,
	field<float, "f32">,
	field<ucsl::math::Vector2, "v2">,
	field<ucsl::math::Vector3, "v3">,
	field<ucsl::math::Vector4, "v4">,
	field<ucsl::math::Quaternion, "quat">,
	field<ucsl::math::Position, "pos">,
	field<ucsl::math::Rotation, "rot">,
	field<ucsl::math::Matrix34, "m34">,
	field<ucsl::math::Matrix44, "m44">,
	field<const char*, "str">,
	field<ucsl::strings::VariableString, "vstr">,
	field<ucsl::objectids::ObjectIdV1, "oid1">,
	field<ucsl::objectids::ObjectIdV2, "oid2">,
	field<ucsl::colors::Color8RGBA, "col8rgba">,
	field<ucsl::colors::Color8ABGR, "col8abgr">,
	field<ucsl::colors::ColorfRGBA, "colfrgba">,
	field<ucsl::colors::ColorfABGR, "colfabgr">,
	field<TestEnum1, "enm">,
	field<aligned<16, int>, "barCount">,
	field<dynamic_carray<int, field_resolver<size_t, "barCount">>*, "bars">,
	field<ucsl::containers::arrays::Array<int, GI::AllocatorSystem>, "arr">,
	field<ucsl::containers::arrays::TArray<int, GI::AllocatorSystem>, "tarr">
>;

std::ostream& operator<<(std::ostream& o$, const ucsl::math::Vector2& v) { return o$ << "<" << v.x() << ", " << v.y() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Vector3& v) { return o$ << "<" << v.x() << ", " << v.y() << ", " << v.z() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Vector4& v) { return o$ << "<" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Quaternion& v) { return o$ << "<" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Position& v) { return o$ << "<" << v.x() << ", " << v.y() << ", " << v.z() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Rotation& v) { return o$ << "<" << v.x() << ", " << v.y() << ", " << v.z() << ", " << v.w() << ">"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Matrix34& v) {
	o$ << "<";
	for (size_t i = 0; i < v.rows(); i++)
		for (size_t j = 0; j < v.cols(); j++)
			o$ << v(i, j) << (i == v.rows() - 1 && j == v.cols() - 1 ? "" : ", ");
	return o$ << ">";
}
std::ostream& operator<<(std::ostream& o$, const ucsl::math::Matrix44& v) {
	o$ << "<";
	for (size_t i = 0; i < v.rows(); i++)
		for (size_t j = 0; j < v.cols(); j++)
			o$ << v(i, j) << (i == v.rows() - 1 && j == v.cols() - 1 ? "" : ", ");
	return o$ << ">";
}
std::ostream& operator<<(std::ostream& o$, const ucsl::colors::Color8RGBA& v) { return o$ << "(" << static_cast<unsigned int>(v.r) << ", " << static_cast<unsigned int>(v.g) << ", " << static_cast<unsigned int>(v.b) << ", " << static_cast<unsigned int>(v.a) << ")"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::colors::Color8ABGR& v) { return o$ << "(" << static_cast<unsigned int>(v.r) << ", " << static_cast<unsigned int>(v.g) << ", " << static_cast<unsigned int>(v.b) << ", " << static_cast<unsigned int>(v.a) << ")"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::colors::ColorfRGBA& v) { return o$ << "(" << v.r << ", " << v.g << ", " << v.b << ", " << v.a << ")"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::colors::ColorfABGR& v) { return o$ << "(" << v.r << ", " << v.g << ", " << v.b << ", " << v.a << ")"; }
std::ostream& operator<<(std::ostream& o$, const ucsl::objectids::ObjectIdV1& v) { return o$ << rip::util::toGUID(v); }
std::ostream& operator<<(std::ostream& o$, const ucsl::objectids::ObjectIdV2& v) { return o$ << rip::util::toGUID(v); }
std::ostream& operator<<(std::ostream& o$, const ucsl::strings::VariableString v) { return o$ << v.c_str(); }

struct Null {
	static constexpr size_t arity = 2;
	struct result_type {
		int v;

		result_type() : v{} {}
		result_type(int v) : v{ v } {}
		result_type(const result_type& other) : v{ other.v } {}
		result_type& operator|=(const result_type& other) { v += other.v; return *this; }
	};
	template<accessors::PrimitiveAccessor T> result_type visit_primitive(T obj1, T obj2) {
		return obj1.visit([&](auto obj1Data) {
			return obj2.visit([&](auto obj2Data) {
				if constexpr (std::is_same_v<typename decltype(obj1Data.refl)::repr, unsigned char> || std::is_same_v<typename decltype(obj1Data.refl)::repr, char>)
					std::cout << "obj1: " << static_cast<int>(obj1Data) << ", obj2: " << static_cast<int>(obj2Data) << std::endl;
				else
					std::cout << "obj1: " << obj1Data << ", obj2: " << obj2Data << std::endl;
				return 1;
			});
		});
	}
	template<typename T> result_type visit_enum(T obj1, T obj2) {
		std::cout << "obj1: " << obj1 << ", obj2: " << obj2 << std::endl;
		return 1;
	}
	//template<typename T> result_type visit_flags(T obj1, T obj2) { return 0; }
	template<typename T, typename F> result_type visit_array(T obj1, T obj2, F f) {
		result_type res{};
		size_t size = std::min(obj1.size(), obj2.size());

		for (size_t i = 0; i < size; i++) {
			auto item1 = obj1[i];
			auto item2 = obj2[i];

			res |= f(item1, item2);
		}

		return res;
	}
	template<typename T, typename F> result_type visit_tarray(T obj1, T obj2, F f) {
		result_type res{};
		size_t size = std::min(obj1.size(), obj2.size());

		for (size_t i = 0; i < size; i++) {
			auto item1 = obj1[i];
			auto item2 = obj2[i];

			res |= f(item1, item2);
		}

		return res;
	}
	template<accessors::PointerAccessor T, typename F> result_type visit_pointer(T obj1, T obj2, F f) {
		auto obj1Target = obj1.get().value();
		auto obj2Target = obj2.get().value();

		return f(obj1Target, obj2Target);
	}
	template<accessors::CArrayAccessor T, typename F> result_type visit_carray(T obj1, T obj2, F f) {
		result_type res{};
		for (size_t i = 0; i < obj1.get_length(); i++) {
			auto obj1Item = obj1[i];
			auto obj2Item = obj2[i];

			res |= f(obj1Item, obj2Item);

			std::cout << res.v << std::endl;
		}
		return res;
	}
	//template<typename T, typename F> result_type visit_union(T& obj1, T& obj2, F f) { return 0; }
	template<typename T, typename F> result_type visit_type(T obj1, T obj2, F f) { return f(obj1, obj2); }
	template<accessors::ValueAccessor T, typename F> result_type visit_field(T obj1, T obj2, const traversals::FieldInfo& info, F f) { return f(obj1, obj2); }
	template<accessors::StructureAccessor T, typename F> result_type visit_base_struct(T obj1, T obj2, F f) { return f(obj1, obj2); }
	template<accessors::StructureAccessor T, typename F> result_type visit_struct(T obj1, T obj2, F f) { return f(obj1, obj2); }
	template<accessors::ValueAccessor T, typename F> result_type visit_root(T obj1, T obj2, F f) { return f(obj1, obj2); }
};

const char mockT1[] = R"({
	"b": true,
	"u8": 1,
	"s8": 2,
	"u16": 3,
	"s16": 4,
	"u32": 5,
	"s32": 6,
	"u64": 7,
	"s64": 8,
	"f32": 9.0,
	"v2": { "x": 1.0, "y": 2.0 },
	"v3": { "x": 1.0, "y": 2.0, "z": 3.0 },
	"v4": { "x": 1.0, "y": 2.0, "z": 3.0, "w": 4.0 },
	"quat": { "x": 1.0, "y": 2.0, "z": 3.0, "w": 4.0 },
	"pos": { "x": 1.0, "y": 2.0, "z": 3.0 },
	"rot": { "x": 1.0, "y": 2.0, "z": 3.0, "w": 4.0 },
	"m34": [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0],
	"m44": [1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 0.0, 1.0],
	"str": "foo",
	"vstr": "bar",
	"oid1": "{090af1d7-0000-0000-0000-000000000000}",
	"oid2": "{9abf3b74-f631-45e9-b8da-9501abd976ae}",
	"col8rgba": { "r": 120, "g": 53, "b": 74, "a": 201 },
	"col8abgr": { "r": 42, "g": 12, "b": 56, "a": 65 },
	"colfrgba": { "r": 0.4, "g": 0.3, "b": 0.6, "a": 1.0 },
	"colfabgr": { "r": 0.6, "g": 0.1, "b": 0.5, "a": 0.2 },
	"enm": "OPT2",
	"barCount": 5,
	"bars": [1, 2, 3, 4, 5],
	"arr": [1, 2],
	"tarr": [1, 2, 3]
})";

TEST_CASE( "read", "json accessors") {	
	auto* t1Imm = yyjson_read(mockT1, strlen(mockT1), 0);

	providers::BoundProvider::RootType refl{ providers::simplerfl<GI>::Type<Test1>{} };
	rip::binary::accessors::json<GI>::ValueAccessor<decltype(refl)> acc1{ t1Imm, refl };
	rip::binary::accessors::json<GI>::ValueAccessor<decltype(refl)> acc2{ t1Imm, refl };
	traversals::traversal<Null> t{};

	REQUIRE(t.call(refl, acc1, acc2).v == 74);

	yyjson_doc_free(t1Imm);
}

TEST_CASE( "write", "json accessors") {	
	auto* t1Imm = yyjson_read(mockT1, strlen(mockT1), 0);
	//auto* t2Imm = yyjson_read(t2Str, strlen(t2Str), 0);

	auto* t1 = yyjson_doc_mut_copy(t1Imm, nullptr);
	auto* t2 = yyjson_doc_mut_copy(t1Imm, nullptr);

	providers::BoundProvider::RootType refl{ providers::simplerfl<GI>::Type<Test1>{} };
	rip::binary::accessors::json_mut<GI>::ValueAccessor<decltype(refl)> acc1{ t1, refl };
	rip::binary::accessors::json_mut<GI>::ValueAccessor<decltype(refl)> acc2{ t2, refl };
	traversals::traversal<Null> t{};

	REQUIRE(t.call(refl, acc1, acc2).v == 74);

	yyjson_mut_doc_free(t1);
	yyjson_mut_doc_free(t2);

	yyjson_doc_free(t1Imm);
}
