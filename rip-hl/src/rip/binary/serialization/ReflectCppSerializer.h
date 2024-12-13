#pragma once
#include <ucsl-reflection/reflections/basic-types.h>
#include <ucsl-reflection/traversals/types.h>
#include <ucsl-reflection/opaque.h>
#include <iomanip>
#include <sstream>
#include <rfl.hpp>

namespace rip::binary {
	using namespace ucsl::reflection;
	using namespace ucsl::reflection::traversals;

	struct SerializedVector2 { float x{}; float y{}; };
	struct SerializedVector3 { float x{}; float y{}; float z{}; };
	struct SerializedVector4 { float x{}; float y{}; float z{}; float w{}; };

	using UCSLType = rfl::Variant<
		bool,
		char,
		unsigned char,
		short,
		unsigned short,
		int,
		unsigned int,
		long long,
		unsigned long long,
		float,
		ucsl::math::Vector2,
		ucsl::math::Vector3,
		ucsl::math::Vector4,
		ucsl::math::Quaternion,
		ucsl::math::Matrix34,
		ucsl::math::Matrix44,
		ucsl::math::Position,
		ucsl::objectids::ObjectIdV1,
		ucsl::objectids::ObjectIdV2,
		ucsl::colors::Color8,
		ucsl::colors::Colorf,
		std::nullopt_t,
		std::vector<UCSLType>,
		rfl::Object<UCSLType>,
		std::string
	>;

	template<typename GameInterface>
	class ReflectCppSerializer {
		rfl::Object<UCSLType<typename GameInterface::AllocatorSystem>> currentStruct{};

		class SerializeChunk {
		public:
			constexpr static size_t arity = 1;
			struct result_type {
				UCSLType<typename GameInterface ::AllocatorSystem>value{};

				result_type() {}
				result_type(const UCSLType<typename GameInterface::AllocatorSystem>& value) : value{ value } {}
				result_type(UCSLType<typename GameInterface::AllocatorSystem>&& value) : value{ std::move(value) } {}
				result_type& operator|=(const result_type& other) { return *this; }
			};

			ReflectCppSerializer& serializer;

			SerializeChunk(ReflectCppSerializer& serializer) : serializer{ serializer } {}

			template<typename T>
			result_type visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				return obj;
			}

			result_type visit_primitive(ucsl::strings::VariableString& obj, const PrimitiveInfo<ucsl::strings::VariableString>& info) {
				return std::string{ obj.c_str() };
			}

			result_type visit_primitive(const char*& obj, const PrimitiveInfo<const char*>& info) {
				return std::string{ obj };
			}

			result_type visit_primitive(void*& obj, const PrimitiveInfo<void*>& info) {
				return std::nullopt;
			}

			template<typename T, typename O>
			result_type visit_enum(T& obj, const EnumInfo<O>& info) {
				yyjson_mut_val* val = visit_primitive(obj, PrimitiveInfo<T>{}).value;
				auto v = yyjson_mut_get_sint(val);
				for (auto& option : info.options)
					if (option.GetIndex() == v)
						return std::string{ option.GetEnglishName() };
				return nullptr;
			}

			template<typename T, typename O>
			result_type visit_flags(T& obj, const FlagsInfo<O>& info) {
				return visit_primitive(obj, PrimitiveInfo<T>{});
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

			template<typename F, typename A, typename S>
			result_type visit_pointer(opaque_obj*& obj, const PointerInfo<A, S>& info, F f) {
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
		ReflectCppSerializer(yyjson_mut_doc* doc) : doc{ doc } {
		}
		~ReflectCppSerializer() {
		}

		template<typename T, typename R>
		yyjson_mut_val* serialize(T& data, R refl) {
			return ucsl::reflection::traversals::traversal<SerializeChunk>{ *this }(data, refl).value;
		}
	};
}