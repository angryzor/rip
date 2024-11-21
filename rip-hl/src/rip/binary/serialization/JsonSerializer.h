#pragma once
#include <ucsl-reflection/reflections/basic-types.h>
#include <ucsl-reflection/traversals/types.h>
#include <ucsl-reflection/opaque.h>
#include <yyjson.h>
#include <iomanip>
#include <sstream>

namespace rip::binary {
	using namespace ucsl::reflection;
	using namespace ucsl::reflection::traversals;

	class JsonSerializer {
		yyjson_mut_doc* doc{ yyjson_mut_doc_new(nullptr) };
		const char* filename;
		yyjson_mut_val* currentStruct{};

		class SerializeChunk {
		public:
			constexpr static size_t arity = 1;
			struct result_type {
				yyjson_mut_val* value{};

				result_type() {}
				result_type(yyjson_mut_val* value) : value{ value } {}
				result_type& operator|=(const result_type& other) { return *this; }
			};

			JsonSerializer& serializer;

			SerializeChunk(JsonSerializer& serializer) : serializer{ serializer } {}

			template<std::integral T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
			result_type visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				return yyjson_mut_sint(serializer.doc, obj);
			}

			template<std::integral T, std::enable_if_t<!std::is_signed_v<T>, bool> = true>
			result_type visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				return yyjson_mut_uint(serializer.doc, obj);
			}

			result_type visit_primitive(float& obj, const PrimitiveInfo<float>& info) {
				return yyjson_mut_float(serializer.doc, obj);
			}

			result_type visit_primitive(double& obj, const PrimitiveInfo<double>& info) {
				return yyjson_mut_real(serializer.doc, obj);
			}

			result_type visit_primitive(bool& obj, const PrimitiveInfo<bool>& info) {
				return yyjson_mut_bool(serializer.doc, obj);
			}

			// These can probably be replaced by a recursive simplerfl traversal.
			result_type visit_primitive(ucsl::math::Vector2& obj, const PrimitiveInfo<ucsl::math::Vector2>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "x", obj.x);
				yyjson_mut_obj_add_float(serializer.doc, res, "y", obj.y);
				return res;
			}

			result_type visit_primitive(ucsl::math::Vector3& obj, const PrimitiveInfo<ucsl::math::Vector3>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "x", obj.x);
				yyjson_mut_obj_add_float(serializer.doc, res, "y", obj.y);
				yyjson_mut_obj_add_float(serializer.doc, res, "z", obj.z);
				return res;
			}

			result_type visit_primitive(ucsl::math::Position& obj, const PrimitiveInfo<ucsl::math::Position>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "x", obj.x);
				yyjson_mut_obj_add_float(serializer.doc, res, "y", obj.y);
				yyjson_mut_obj_add_float(serializer.doc, res, "z", obj.z);
				return res;
			}

			yyjson_mut_val* make_vec4(ucsl::math::Vector4& obj) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "x", obj.x);
				yyjson_mut_obj_add_float(serializer.doc, res, "y", obj.y);
				yyjson_mut_obj_add_float(serializer.doc, res, "z", obj.z);
				yyjson_mut_obj_add_float(serializer.doc, res, "w", obj.w);
				return res;
			}

			result_type visit_primitive(ucsl::math::Vector4& obj, const PrimitiveInfo<ucsl::math::Vector4>& info) {
				return make_vec4(obj);
			}

			result_type visit_primitive(ucsl::math::Quaternion& obj, const PrimitiveInfo<ucsl::math::Quaternion>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "x", obj.x);
				yyjson_mut_obj_add_float(serializer.doc, res, "y", obj.y);
				yyjson_mut_obj_add_float(serializer.doc, res, "z", obj.z);
				yyjson_mut_obj_add_float(serializer.doc, res, "w", obj.w);
				return res;
			}

			result_type visit_primitive(ucsl::math::Matrix34& obj, const PrimitiveInfo<ucsl::math::Matrix34>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_val(serializer.doc, res, "t", make_vec4(obj.t));
				yyjson_mut_obj_add_val(serializer.doc, res, "u", make_vec4(obj.u));
				yyjson_mut_obj_add_val(serializer.doc, res, "v", make_vec4(obj.v));
				yyjson_mut_obj_add_val(serializer.doc, res, "w", make_vec4(obj.w));
				return res;
			}

			result_type visit_primitive(ucsl::math::Matrix44& obj, const PrimitiveInfo<ucsl::math::Matrix44>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_val(serializer.doc, res, "t", make_vec4(obj.t));
				yyjson_mut_obj_add_val(serializer.doc, res, "u", make_vec4(obj.u));
				yyjson_mut_obj_add_val(serializer.doc, res, "v", make_vec4(obj.v));
				yyjson_mut_obj_add_val(serializer.doc, res, "w", make_vec4(obj.w));
				return res;
			}

			result_type visit_primitive(ucsl::colors::Color8& obj, const PrimitiveInfo<ucsl::colors::Color8>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_uint(serializer.doc, res, "r", obj.r);
				yyjson_mut_obj_add_uint(serializer.doc, res, "g", obj.g);
				yyjson_mut_obj_add_uint(serializer.doc, res, "b", obj.b);
				yyjson_mut_obj_add_uint(serializer.doc, res, "a", obj.a);
				return res;
			}

			result_type visit_primitive(ucsl::colors::Colorf& obj, const PrimitiveInfo<ucsl::colors::Colorf>& info) {
				yyjson_mut_val* res = yyjson_mut_obj(serializer.doc);
				yyjson_mut_obj_add_float(serializer.doc, res, "r", obj.r);
				yyjson_mut_obj_add_float(serializer.doc, res, "g", obj.g);
				yyjson_mut_obj_add_float(serializer.doc, res, "b", obj.b);
				yyjson_mut_obj_add_float(serializer.doc, res, "a", obj.a);
				return res;
			}

			result_type visit_primitive(ucsl::objectids::ObjectIdV1& obj, const PrimitiveInfo<ucsl::objectids::ObjectIdV1>& info) {
				return yyjson_mut_uint(serializer.doc, obj);
			}

			result_type visit_primitive(ucsl::objectids::ObjectIdV2& obj, const PrimitiveInfo<ucsl::objectids::ObjectIdV2>& info) {
				std::ostringstream oss{};
				oss << std::setfill('0') << std::setw(16) << std::hex << obj.groupId << std::setfill('0') << std::setw(16) << std::hex << obj.objectId;
				return yyjson_mut_strcpy(serializer.doc, oss.str().c_str());
			}

			result_type visit_primitive(ucsl::strings::VariableString& obj, const PrimitiveInfo<ucsl::strings::VariableString>& info) {
				return yyjson_mut_str(serializer.doc, obj.c_str());
			}

			result_type visit_primitive(const char*& obj, const PrimitiveInfo<const char*>& info) {
				return yyjson_mut_str(serializer.doc, obj);
			}

			template<typename O, typename F>
			result_type visit_enum(opaque_obj& obj, const EnumInfo<O>& info, F f) {
				yyjson_mut_val* val = f(obj).value;
				auto v = static_cast<unsigned int>(yyjson_mut_get_uint(val));
				for (auto& option : info.options)
					if (option.GetIndex() == v)
						return yyjson_mut_strcpy(serializer.doc, option.GetEnglishName());
				return nullptr;
			}

			template<typename O, typename F>
			result_type visit_flags(opaque_obj& obj, const FlagsInfo<O>& info, F f) {
				return f(obj);
			}

			template<typename F, typename C, typename D, typename A>
			result_type visit_array(A& arr, const ArrayInfo& info, C c, D d, F f) {
				yyjson_mut_val* jarr = yyjson_mut_arr(serializer.doc);
				for (auto& obj : arr)
					yyjson_mut_arr_add_val(jarr, f(obj).value);
				return jarr;
			}

			template<typename F, typename C, typename D, typename A>
			result_type visit_tarray(A& arr, const ArrayInfo& info, C c, D d, F f) {
				yyjson_mut_val* jarr = yyjson_mut_arr(serializer.doc);
				for (auto& obj : arr)
					yyjson_mut_arr_add_val(jarr, f(obj).value);
				return jarr;
			}

			template<typename F>
			result_type visit_pointer(opaque_obj*& obj, const PointerInfo& info, F f) {
				return obj == nullptr ? yyjson_mut_null(serializer.doc) : f(*obj);
			}

			template<typename F>
			result_type visit_carray(opaque_obj* obj, const CArrayInfo& info, F f) {
				yyjson_mut_val* jarr = yyjson_mut_arr(serializer.doc);
				for (size_t i = 0; i < info.size; i++)
					yyjson_mut_arr_add_val(jarr, f(*addptr(obj, i * info.stride)).value);
				return jarr;
			}

			template<typename F>
			result_type visit_union(opaque_obj& obj, const UnionInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_type(opaque_obj& obj, const TypeInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_field(opaque_obj& obj, const FieldInfo& info, F f) {
				if (!info.erased)
					yyjson_mut_obj_add_val(serializer.doc, serializer.currentStruct, info.name, f(obj).value);
				return nullptr;
			}

			template<typename F>
			result_type visit_base_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				yyjson_mut_val* thisStruct = yyjson_mut_obj(serializer.doc);

				if (!serializer.currentStruct)
					yyjson_mut_doc_set_root(serializer.doc, thisStruct);

				yyjson_mut_val* prevStruct = serializer.currentStruct;
				serializer.currentStruct = thisStruct;
				result_type res = f(obj);
				serializer.currentStruct = prevStruct;

				return thisStruct;
			}

			template<typename F>
			result_type visit_root(opaque_obj& obj, const RootInfo& info, F f) {
				return f(obj);
			}
		};

	public:
		JsonSerializer(const char* filename) : filename{ filename } {
		}
		~JsonSerializer() {
			yyjson_mut_doc_free(doc);
		}

		template<template<typename> typename Traversal, typename T>
		void serialize(T& data) {
			Traversal<SerializeChunk> operation{ *this };
			operation(data);
			yyjson_write_err err;
			yyjson_mut_write_file(filename, doc, YYJSON_WRITE_PRETTY_TWO_SPACES, nullptr, &err);

			if (err.code != YYJSON_WRITE_SUCCESS) {
				std::cout << "Error writing json: " << err.msg << std::endl;
			}
		}
	};
}