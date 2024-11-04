#pragma once
#include <stf/basic-types.h>
#include <rip/reflection/traversals/types.h>

namespace rip::binary {
	template<typename Stream>
	class JsonSerializer {
		yyjson_doc*
		class SerializeChunk {
		public:
			constexpr static size_t arity = 1;
			typedef int result_type;

			ReflectionSerializer& serializer;
			size_t dbgStructStartLoc{};
			void* currentStructAddr{};

			SerializeChunk(ReflectionSerializer& serializer) : serializer{ serializer } {}

			template<typename T, std::enable_if_t<!std::is_fundamental_v<T>, bool> = true>
			int visit_primitive(T* obj, const PrimitiveInfo& info) {
				std::cout << "writing NF " << std::hex << obj << " as " << *obj << std::endl;
				serializer.backend.write<T>(*obj);
				return 0;
			}

			template<typename T, std::enable_if_t<std::is_fundamental_v<T>, bool> = true>
			int visit_primitive(T* obj, const PrimitiveInfo& info) {
				std::cout << "writing " << std::hex << obj << " as " << *obj << std::endl;
				serializer.backend.write(info.erased ? T{} : *obj);
				return 0;
			}

			int visit_primitive(const char** obj, const PrimitiveInfo& info) {
				std::cout << "writing STR " << std::hex << obj << " as " << *obj << std::endl;
				if constexpr (Backend::hasNativeStrings)
					serializer.backend.write(*obj);
				else
					serializer.backend.write(*obj == nullptr ? serialized_types::o64_t<const char>{} : serializer.enqueueBlock(strlen(*obj) + 1, 1, [=]() {
					serializer.backend.write(*obj);
						}));
				return 0;
			}

			int visit_primitive(csl::ut::VariableString* obj, const PrimitiveInfo& info) {
				visit_primitive(reinterpret_cast<const char**>(obj));
				serializer.backend.write(0ull);
				return 0;
			}

			//template<typename F>
			//int VisitEnum(void* obj, hh::fnd::RflClassMember::Type type, const hh::fnd::RflClassEnum* enumClass, F f) {
			//	return f(obj);
			//}

			//template<typename F>
			//int VisitFlags(void* obj, hh::fnd::RflClassMember::Type type, const hh::fnd::RflArray<const hh::fnd::RflClassEnumMember>* enumEntries, F f) {
			//	return f(obj);
			//}

			//template<typename F, typename C, typename D>
			//int visit_array(RflArrayAccessor<csl::ut::MoveArray>& arr, C c, D d, F f) {
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
			//int visit_array(RflArrayAccessor<csl::ut::MoveArray32>& arr, C c, D d, F f) {
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
			//int visit_array(RflArrayAccessor<hh::TArray>& arr, C c, D d, F f) {
			//	serializer.backend.write(arr[0] == nullptr ? serialized_types::o64_t<void*>{} : serializer.enqueueBlock(arr.size() * arr.item_size(), arr.item_alignment(), [=]() {
			//		for (auto item : arr)
			//			f(item);
			//	}));
			//	serializer.backend.write(arr.size());
			//	serializer.backend.write(static_cast<int64_t>(arr.capacity()));
			//	return 0;
			//}

			template<typename F>
			int visit_pointer(void** obj, const PointerInfo& info, F f) {
				serializer.backend.write(*obj == nullptr ? serialized_types::o64_t<void*>{} : serializer.enqueueBlock(info.targetSize, info.targetAlignment, [=]() {
					f(*obj);
					}));
				return 0;
			}

			template<typename F>
			int visit_field_data(void* obj, const FieldDataInfo& info, F f) {
				serializer.backend.write_padding(info.alignment);

				// Catch alignment issues.
				assert((serializer.backend.tellp() - dbgStructStartLoc) == (reinterpret_cast<size_t>(obj) - reinterpret_cast<size_t>(currentStructAddr)));

				f(obj);

				// Catch alignment issues.
				assert((serializer.backend.tellp() - dbgStructStartLoc) == (reinterpret_cast<size_t>(obj) + info.size - reinterpret_cast<size_t>(currentStructAddr)));
				return 0;
			}

			template<typename F>
			int visit_field(void* obj, const FieldInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			int visit_array_field(void* obj, const ArrayFieldInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			int visit_array_field_item(void* obj, const ArrayFieldItemInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			int visit_base_struct(void* obj, const StructureInfo& info, F f) {
				f(obj);

				serializer.backend.write_padding(info.alignment);
				return 0;
			}

			template<typename F>
			int visit_struct(void* obj, const StructureInfo& info, F f) {
				size_t prevDbgStructStartLoc = dbgStructStartLoc;
				void* prevStructAddr = currentStructAddr;

				dbgStructStartLoc = serializer.backend.tellp();
				currentStructAddr = obj;

				f(obj);

				serializer.backend.write_padding(info.alignment);

				dbgStructStartLoc = prevDbgStructStartLoc;
				currentStructAddr = prevStructAddr;
				return 0;
			}
		};

	public:

	};
}