#pragma once
#include <string>
#include <iostream>
#include <ucsl/containers/arrays/array.h>
#include <ucsl/containers/arrays/tarray.h>
#include <ucsl/math.h>
#include <ucsl/colors.h>
#include <ucsl/strings/variable-string.h>
#include <ucsl/rfl/ranges.h>
#include <ucsl-reflection/game-interfaces/standalone/reflection-db.h>
#include <ucsl-reflection/game-interfaces/standalone/rflsystem.h>
#include <rfl.hpp>
#include <rfl/json.hpp>

namespace rip::schemas::hedgeset {
	namespace json_reflections {
		//using BoolType = rfl::Literal<"bool">;
		//using Int8Type = rfl::Literal<"int8">;
		//using Uint8Type = rfl::Literal<"uint8">;
		//using Int16Type = rfl::Literal<"int16">;
		//using Uint16Type = rfl::Literal<"uint16">;
		//using Int32Type = rfl::Literal<"int32">;
		//using Uint32Type = rfl::Literal<"uint32">;
		//using Int64Type = rfl::Literal<"int64">;
		//using Uint64Type = rfl::Literal<"uint64">;
		//using Float32Type = rfl::Literal<"float32">;
		//using Vector2Type = rfl::Literal<"vector2">;
		//using Vector3Type = rfl::Literal<"vector3">;
		//using Vector4Type = rfl::Literal<"vector4">;
		//using QuaternionType = rfl::Literal<"quaternion">;
		//using ObjectIdType = rfl::Literal<"object_reference">;
		//using StringType = rfl::Literal<"string">;
		//using Float64Type = rfl::Literal<"float64">;
		//using CharType = rfl::Literal<"char">;
		//using ArrayType = rfl::Literal<"array">;

		//template<typename T> struct hson_type {};
		//template<> struct hson_type<bool> { static constexpr const char* type = "bool"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = BoolType; };
		//template<> struct hson_type<int8_t> { static constexpr const char* type = "int8"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Int8Type; };
		//template<> struct hson_type<uint8_t> { static constexpr const char* type = "uint8"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Uint8Type; };
		//template<> struct hson_type<int16_t> { static constexpr const char* type = "int16"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Int16Type; };
		//template<> struct hson_type<uint16_t> { static constexpr const char* type = "uint16"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Uint16Type; };
		//template<> struct hson_type<int32_t> { static constexpr const char* type = "int32"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Int32Type; };
		//template<> struct hson_type<uint32_t> { static constexpr const char* type = "uint32"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Uint32Type; };
		//template<> struct hson_type<int64_t> { static constexpr const char* type = "int64"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Int64Type; };
		//template<> struct hson_type<uint64_t> { static constexpr const char* type = "uint64"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Uint64Type; };
		//template<> struct hson_type<float> { static constexpr const char* type = "float32"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Float32Type; };
		//template<> struct hson_type<ucsl::math::Vector2> { static constexpr const char* type = "vector2"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Vector2Type; };
		//template<> struct hson_type<ucsl::math::Vector3> { static constexpr const char* type = "vector3"; static constexpr std::optional<unsigned int> alignment = std::make_optional(16); using lit = Vector3Type; };
		//template<> struct hson_type<ucsl::math::Vector4> { static constexpr const char* type = "vector4"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Vector4Type; };
		//template<> struct hson_type<ucsl::math::Quaternion> { static constexpr const char* type = "quaternion"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = QuaternionType; };
		//template<> struct hson_type<ucsl::objectids::ObjectIdV1> { static constexpr const char* type = "object_reference"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = ObjectIdType; };
		//template<> struct hson_type<ucsl::objectids::ObjectIdV2> { static constexpr const char* type = "object_reference"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = ObjectIdType; };
		//template<> struct hson_type<ucsl::strings::VariableString> { static constexpr const char* type = "string"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = StringType; };
		//template<> struct hson_type<double> { static constexpr const char* type = "float64"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = Float64Type; };
		//template<> struct hson_type<char> { static constexpr const char* type = "char"; static constexpr std::optional<unsigned int> alignment = std::nullopt; using lit = CharType; };

		struct EnumValueDef {
			unsigned int value{};
			std::optional<rfl::Object<std::string>> descriptions{};
		};

		struct EnumDef {
			std::string type{};
			rfl::Object<EnumValueDef> values{};
		};

		template<typename T> struct rfl_range_rep {};
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeSint32> { using type = int32_t; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeUint32> { using type = uint32_t; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeSint64> { using type = int64_t; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeUint64> { using type = uint64_t; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeFloat> { using type = float; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeVector2> { using type = std::array<float, 2>; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeVector3> { using type = std::array<float, 3>; };
		template<> struct rfl_range_rep<ucsl::rfl::ranges::RangeVector4> { using type = std::array<float, 4>; };

		template<typename T>
		using rfl_range_rep_t = typename rfl_range_rep<T>::type;

		using Ranges = rfl::Variant<
			rfl_range_rep_t<ucsl::rfl::ranges::RangeSint32>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeUint32>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeSint64>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeUint64>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeFloat>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeVector2>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeVector3>,
			rfl_range_rep_t<ucsl::rfl::ranges::RangeVector4>
		>;

		struct MemberDef {
			std::string name{};
			std::string type{};
			std::optional<std::string> subtype{};
			std::optional<rfl::Object<EnumValueDef>> flags{};
			std::optional<unsigned int> array_size{};
			std::optional<unsigned int> alignment{};
			std::optional<Ranges> min_range{};
			std::optional<Ranges> max_range{};
			std::optional<Ranges> step{};
			std::optional<rfl::Object<std::string>> descriptions{};
		};

		struct RflClassDef {
			std::optional<std::string> parent{};
			std::optional<std::vector<MemberDef>> fields{};
		};

		struct ObjectDef {
			rfl::Rename<"struct", std::string> rflClassName{};
			std::string category{};
		};

		struct Template {
			unsigned int version{};
			std::string format{};
			rfl::Object<EnumDef> enums{};
			rfl::Object<RflClassDef> structs{};
			rfl::Object<ObjectDef> objects{};
		};
	}

	template<typename RflSystem>
	class Template {
		std::map<std::string, std::shared_ptr<typename RflSystem::RflClassEnum>> enums{};
		std::map<std::string, std::shared_ptr<typename RflSystem::RflClass>> rflClasses{};
		std::map<std::string, std::string> objects{};

		struct TypeConversionResult {
			typename RflSystem::RflClassMember::Type type{ RflSystem::RflClassMember::Type::VOID };
			typename RflSystem::RflClassMember::Type subtype{ RflSystem::RflClassMember::Type::VOID };
			std::optional<std::shared_ptr<typename RflSystem::RflClass>> structt{ std::nullopt };
			std::optional<std::shared_ptr<typename RflSystem::RflClassEnum>> enumm{ std::nullopt };
			std::optional<std::vector<typename RflSystem::RflClassEnumMember>> flagValues{ std::nullopt };
		};

		typename RflSystem::RflClassMember::Type get_primitive_type(const std::string& type) {
			if (type == "int8") return RflSystem::RflClassMember::Type::SINT8;
			if (type == "uint8") return RflSystem::RflClassMember::Type::UINT8;
			if (type == "int16") return RflSystem::RflClassMember::Type::SINT16;
			if (type == "uint16") return RflSystem::RflClassMember::Type::UINT16;
			if (type == "int32") return RflSystem::RflClassMember::Type::SINT32;
			if (type == "uint32") return RflSystem::RflClassMember::Type::UINT32;
			if (type == "int64") return RflSystem::RflClassMember::Type::SINT64;
			if (type == "uint64") return RflSystem::RflClassMember::Type::UINT64;
			if (type == "bool") return RflSystem::RflClassMember::Type::BOOL;
			if (type == "float32") return RflSystem::RflClassMember::Type::FLOAT;
			if (type == "vector2") return RflSystem::RflClassMember::Type::VECTOR2;
			if (type == "vector3") return RflSystem::RflClassMember::Type::VECTOR3;
			if (type == "vector4") return RflSystem::RflClassMember::Type::VECTOR4;
			if (type == "quaternion") return RflSystem::RflClassMember::Type::QUATERNION;
			if (type == "matrix34") return RflSystem::RflClassMember::Type::MATRIX34;
			if (type == "matrix44") return RflSystem::RflClassMember::Type::MATRIX44;
			if (type == "color8") return RflSystem::RflClassMember::Type::COLOR_BYTE;
			if (type == "colorf") return RflSystem::RflClassMember::Type::COLOR_FLOAT;
			if (type == "string") return RflSystem::RflClassMember::Type::STRING;
			if (type == "object_reference") return RflSystem::RflClassMember::Type::OBJECT_ID;
			return RflSystem::RflClassMember::Type::VOID;
		}

		std::vector<typename RflSystem::RflClassEnumMember> get_flag_values(const json_reflections::MemberDef& member) {
			std::vector<typename RflSystem::RflClassEnumMember> flagValues{};

			for (auto& [name, enumValueDef] : member.flags.value())
				flagValues.push_back(typename RflSystem::RflClassEnumMember{ enumValueDef.value, name, enumValueDef.descriptions.has_value() ? enumValueDef.descriptions.value().get("ja").value_or("") : "" });

			return std::move(flagValues);
		}

		TypeConversionResult get_type(const json_reflections::MemberDef& member, const json_reflections::Template& templ) {
			if (templ.structs.get(member.type))
				return { .type = RflSystem::RflClassMember::Type::STRUCT, .structt = load_rfl_class(member.type, templ.structs.get(member.type).value(), templ) };

			if (templ.enums.get(member.type)) {
				auto e = templ.enums.get(member.type).value();

				return { .type = RflSystem::RflClassMember::Type::ENUM, .subtype = get_primitive_type(e.type), .enumm = load_enum(member.type, e) };
			}

			if (member.type == "flags")
				return { .type = RflSystem::RflClassMember::Type::FLAGS, .subtype = get_primitive_type(member.subtype.value()), .flagValues = get_flag_values(member) };

			if (member.type != "array")
				return { .type = get_primitive_type(member.type) };

			auto subtype = member.subtype.value();

			if (member.array_size.has_value() && member.array_size.value() > 0) {
				if (templ.structs.get(subtype))
					return { .type = RflSystem::RflClassMember::Type::STRUCT, .structt = load_rfl_class(subtype, templ.structs.get(subtype).value(), templ) };

				if (templ.enums.get(subtype)) {
					auto e = templ.enums.get(subtype).value();

					return { .type = RflSystem::RflClassMember::Type::ENUM, .subtype = get_primitive_type(e.type), .enumm = load_enum(subtype, e) };
				}

				return { .type = get_primitive_type(subtype) };
			}

			if constexpr (RflSystem::TypeSet::supports_old_array) {
				if (templ.format == "gedit_v3") {
					if (templ.structs.get(subtype))
						return { .type = RflSystem::RflClassMember::Type::OLD_ARRAY, .subtype = RflSystem::RflClassMember::Type::STRUCT, .structt = load_rfl_class(subtype, templ.structs.get(subtype).value(), templ) };

					return { .type = RflSystem::RflClassMember::Type::OLD_ARRAY, .subtype = get_primitive_type(subtype) };
				}
			}

			if (templ.structs.get(subtype))
				return { .type = RflSystem::RflClassMember::Type::ARRAY, .subtype = RflSystem::RflClassMember::Type::STRUCT, .structt = load_rfl_class(subtype, templ.structs.get(subtype).value(), templ)};

			return { .type = RflSystem::RflClassMember::Type::ARRAY, .subtype = get_primitive_type(subtype) };
		}

		std::shared_ptr<typename RflSystem::RflClassEnum> load_enum(const std::string& name, const json_reflections::EnumDef& enumDef) {
			if (enums.contains(name))
				return enums[name];

			auto [resIt, resSuccess] = enums.emplace(name, std::make_shared<typename RflSystem::RflClassEnum>(name, std::vector<typename RflSystem::RflClassEnumMember>{}));
			auto [resName, res] = *resIt;

			std::vector<typename RflSystem::RflClassEnumMember> enumMembers{};

			for (auto& [name, enumValueDef] : enumDef.values)
				enumMembers.push_back(typename RflSystem::RflClassEnumMember{ enumValueDef.value, name, enumValueDef.descriptions.has_value() ? enumValueDef.descriptions.value().get("ja").value_or("") : "" });

			res->values = std::move(enumMembers);

			return enums[name];
		}

		std::shared_ptr<typename RflSystem::RflClass> load_rfl_class(const std::string& name, const json_reflections::RflClassDef& rflClassDef, const json_reflections::Template& templ) {
			if (rflClasses.contains(name))
				return rflClasses[name];

			auto [resIt, resSuccess] = rflClasses.emplace(name, std::make_shared<typename RflSystem::RflClass>(name, std::nullopt, 0, std::vector<std::shared_ptr<typename RflSystem::RflClassEnum>>{}, std::vector<std::shared_ptr<typename RflSystem::RflClassMember>>{}, 0));
			auto [resName, res] = *resIt;

			std::vector<std::shared_ptr<typename RflSystem::RflClassEnum>> rflClassEnums{};
			std::vector<std::shared_ptr<typename RflSystem::RflClassMember>> rflClassMembers{};

			unsigned int offset{};

			std::optional<std::shared_ptr<typename RflSystem::RflClass>> parent{};

			if (rflClassDef.parent.has_value()) {
				parent = templ.structs.get(rflClassDef.parent.value()).transform([&](const auto& e) -> std::optional<std::shared_ptr<typename RflSystem::RflClass>> { return load_rfl_class(rflClassDef.parent.value(), e, templ); }).value();
				offset = parent.value()->GetSize();
			}

			res->parent = parent;

			if (rflClassDef.fields.has_value()) {
				for (auto& memberDef : rflClassDef.fields.value()) {
					auto convertedTypes = get_type(memberDef, templ);

					if (convertedTypes.enumm.has_value())
						rflClassEnums.emplace_back(convertedTypes.enumm.value());

					auto member = rflClassMembers.emplace_back(std::make_shared<typename RflSystem::RflClassMember>(
						memberDef.name,
						convertedTypes.structt,
						convertedTypes.enumm,
						std::move(convertedTypes.flagValues),
						convertedTypes.type,
						convertedTypes.subtype,
						(memberDef.array_size.has_value() && memberDef.array_size.value() > 0) ? memberDef.array_size.value() : 0,
						0
					));

					member->offset = offset = align(offset, member->GetAlignment());
					offset += (unsigned int)member->GetSize();
				}
			}

			res->enums = std::move(rflClassEnums);
			res->members = std::move(rflClassMembers);
			res->size = align(offset, res->GetAlignment());

			return rflClasses[name];
		}

	public:
		inline Template(const std::string& filename) {
			auto json = rfl::json::load<json_reflections::Template>(filename);
			auto err = json.error();
			if (err.has_value())
				std::cout << err.value().what() << std::endl;
			auto templ = json.value();

			for (auto& [name, enumDef] : templ.enums)
				load_enum(name, enumDef);

			for (auto& [name, rflClassDef] : templ.structs)
				load_rfl_class(name, rflClassDef, templ);

			for (auto& [name, objectDef] : templ.objects)
				objects[name] = objectDef.rflClassName.value();
		}

		void load(ucsl::reflection::game_interfaces::standalone::ReflectionDB<RflSystem>& db) {
			for (auto& [name, rflClassDef] : rflClasses)
				db.rflClasses[name] = rflClassDef;

			for (auto& [name, structName] : objects)
				db.spawnerDataRflClasses[name] = structName;
		}
	};
}
