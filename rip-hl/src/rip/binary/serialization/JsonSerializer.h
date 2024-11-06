#pragma once
#include <stf/basic-types.h>
#include <rip/reflection/traversals/types.h>
#include <yyjson.h>

namespace rip::binary {
	using namespace rip::reflection;

	class JsonSerializer {
		yyjson_mut_doc* doc{ yyjson_mut_doc_new(nullptr) };
		const char* filename;

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
			yyjson_mut_val* currentStruct{};

			SerializeChunk(JsonSerializer& serializer) : serializer{ serializer } {}

			template<std::integral T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
			result_type visit_primitive_repr(T* obj, const PrimitiveReprInfo& info) {
				return yyjson_mut_sint(serializer.doc, *obj);
			}

			template<std::integral T, std::enable_if_t<!std::is_signed_v<T>, bool> = true>
			result_type visit_primitive_repr(T* obj, const PrimitiveReprInfo& info) {
				return yyjson_mut_uint(serializer.doc, *obj);
			}

			template<std::floating_point T>
			result_type visit_primitive_repr(T* obj, const PrimitiveReprInfo& info) {
				return yyjson_mut_real(serializer.doc, *obj);
			}

			result_type visit_primitive_repr(ucsl::strings::VariableString* obj, const PrimitiveReprInfo& info) {
				return yyjson_mut_str(serializer.doc, obj->c_str());
			}

			result_type visit_primitive_repr(const char** obj, const PrimitiveReprInfo& info) {
				return yyjson_mut_str(serializer.doc, *obj);
			}

			//template<typename F>
			//result_type VisitEnum(void* obj, hh::fnd::RflClassMember::Type type, const hh::fnd::RflClassEnum* enumClass, F f) {
			//	return f(obj);
			//}

			//template<typename F>
			//result_type VisitFlags(void* obj, hh::fnd::RflClassMember::Type type, const hh::fnd::RflArray<const hh::fnd::RflClassEnumMember>* enumEntries, F f) {
			//	return f(obj);
			//}

			//template<typename F, typename C, typename D>
			//result_type visit_array(RflArrayAccessor<csl::ut::MoveArray>& arr, C c, D d, F f) {
			//	serializer.backend.write(arr[0] == nullptr ? serialized_types::o64_t<void*>{} : serializer.enqueueBlock(arr.size() * arr.item_size(), arr.item_alignment(), [=]() {
			//		for (auto item : arr)
			//			f(item);
			//	}));
			//	serializer.backend.write(arr.size());
			//	serializer.backend.write(arr.capacity());
			//	serializer.backend.write(0ull);
			//	return 0;
			//}

			//template<typename F, typename C, typename D>
			//result_type visit_array(RflArrayAccessor<csl::ut::MoveArray32>& arr, C c, D d, F f) {
			//	serializer.backend.write(arr[0] == nullptr ? serialized_types::o64_t<void*>{} : serializer.enqueueBlock(arr.size()* arr.item_size(), arr.item_alignment(), [=]() {
			//		for (auto item : arr)
			//			f(item);
			//	}));
			//	serializer.backend.write(arr.size());
			//	serializer.backend.write(arr.capacity());
			//	serializer.backend.write(0ull);
			//	return 0;
			//}

			//template<typename F, typename C, typename D>
			//result_type visit_array(RflArrayAccessor<hh::TArray>& arr, C c, D d, F f) {
			//	serializer.backend.write(arr[0] == nullptr ? serialized_types::o64_t<void*>{} : serializer.enqueueBlock(arr.size() * arr.item_size(), arr.item_alignment(), [=]() {
			//		for (auto item : arr)
			//			f(item);
			//	}));
			//	serializer.backend.write(arr.size());
			//	serializer.backend.write(static_cast<int64_t>(arr.capacity()));
			//	return 0;
			//}

			template<typename F>
			result_type visit_pointer(void** obj, const PointerInfo& info, F f) {
				return f(*obj);
			}

			template<typename F>
			result_type visit_carray(void* obj, const CArrayInfo& info, F f) {
				yyjson_mut_val* arr = yyjson_mut_arr(serializer.doc);
				for (size_t i = 0; i < info.size; i++)
					yyjson_mut_arr_add_val(arr, f(addptr(obj, i * info.stride)).value);
				return arr;
			}

			template<typename F>
			result_type visit_primitive(void* obj, const PrimitiveInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_field(void* obj, const FieldInfo& info, F f) {
				yyjson_mut_obj_add_val(serializer.doc, currentStruct, info.name, f(obj).value);
				return nullptr;
			}

			template<typename F>
			result_type visit_array_field(void* obj, const ArrayFieldInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_array_field_item(void* obj, const ArrayFieldItemInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_base_struct(void* obj, const StructureInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_struct(void* obj, const StructureInfo& info, F f) {
				yyjson_mut_val* thisStruct = yyjson_mut_obj(serializer.doc);

				if (!currentStruct)
					yyjson_mut_doc_set_root(serializer.doc, thisStruct);

				yyjson_mut_val* prevStruct = currentStruct;
				currentStruct = thisStruct;
				result_type res = f(obj);
				currentStruct = prevStruct;

				return thisStruct;
			}

			template<typename F>
			result_type visit_root(void* obj, const RootInfo& info, F f) {
				return f(obj);
			}
		};

	public:
		JsonSerializer(const char* filename) : filename{ filename } {
		}
		~JsonSerializer() {
			yyjson_mut_doc_free(doc);
		}

		template<template<typename> typename Traversal, typename F>
		void serialize(F f) {
			Traversal<SerializeChunk> operation{ *this };
			f(operation);
			yyjson_mut_write_file(filename, doc, 0, nullptr, nullptr);
		}
	};
}