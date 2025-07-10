#pragma once
#include <type_traits>
#include <ucsl-reflection/reflections/basic-types.h>
#include <ucsl-reflection/traversals/types.h>
#include <ucsl-reflection/traversals/traversal.h>
#include <ucsl-reflection/opaque.h>
#include <rip/binary/types.h>
#include "BlobWorker.h"
#include "PointerDisambiguationStore.h"
#include <iostream>

namespace rip::binary {
	using namespace ucsl::reflection;
	using namespace ucsl::reflection::traversals;

	template<typename Backend>
	class ReflectionSerializer {
		Backend& backend;

		SerializationPointerDisambiguationStore<size_t> ptrDisambiguationStore{};

		class SerializeChunk {
		public:
			constexpr static size_t arity = 1;
			typedef int result_type;

			ReflectionSerializer& serializer;
			BlobWorker<> worker;
			size_t dbgStructStartLoc{};
			void* currentStructAddr{};

			SerializeChunk(ReflectionSerializer& serializer) : serializer{ serializer }, worker{ serializer.backend.tellp() } {}

			template<typename T, typename F>
			offset_t<T> enqueueBlock(const T* ptr, size_t bufferSize, size_t alignment, F processFunc) {
				if (ptr == nullptr)
					return offset_t<T>{};

				if (auto ref = serializer.ptrDisambiguationStore.get_reference(ptr, bufferSize))
					return ref.value();

				size_t offset = worker.enqueueBlock(bufferSize, alignment, [this, bufferSize, processFunc](size_t offset, size_t alignment) {
					serializer.backend.write_padding(alignment);
					assert(serializer.backend.tellp() == offset);

					processFunc();

					assert(serializer.backend.tellp() == offset + bufferSize);
				});

				serializer.ptrDisambiguationStore.set_reference(ptr, bufferSize, offset);

				return offset;
			}

			template<typename T, std::enable_if_t<!std::is_fundamental_v<T>, bool> = true>
			int visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				serializer.backend.template write<T>(obj);
				return 0;
			}

			template<typename T, std::enable_if_t<std::is_fundamental_v<T>, bool> = true>
			int visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				serializer.backend.write(info.erased ? T{} : obj);
				return 0;
			}

			void write_string(const char* obj) {
				if constexpr (Backend::hasNativeStrings)
					serializer.backend.write(obj);
				else
					serializer.backend.write(obj == nullptr ? offset_t<char>{} : enqueueBlock(obj, strlen(obj) + 1, 1, [this, obj]() {
						serializer.backend.write_string(obj);
					}));
			}

			int visit_primitive(const char*& obj, const PrimitiveInfo<const char*>& info) {
				write_string(obj);
				return 0;
			}

			int visit_primitive(ucsl::strings::VariableString& obj, const PrimitiveInfo<ucsl::strings::VariableString>& info) {
				write_string(reinterpret_cast<const char*&>(obj));
				serializer.backend.write(static_cast<size_t>(0));
				return 0;
			}

			int visit_primitive(void*& obj, const PrimitiveInfo<void*>& info) {
				if (obj == nullptr)
					serializer.backend.write(offset_t<opaque_obj>{});
				else if (auto ref = serializer.ptrDisambiguationStore.get_reference(obj, 0))
					serializer.backend.write(offset_t<opaque_obj>{ ref.value() });
				else
					assert(false && "cannot find backreference");
				return 0;
			}

			template<typename T, typename O>
			int visit_enum(T& obj, const EnumInfo<O>& info) {
				return visit_primitive(obj, PrimitiveInfo<T>{});
			}

			template<typename T, typename O>
			int visit_flags(T& obj, const FlagsInfo<O>& info) {
				return visit_primitive(obj, PrimitiveInfo<T>{});
			}

			template<typename F, typename C, typename D, typename A>
			int visit_array(A& arr, const ArrayInfo& info, C c, D d, F f) {
				serializer.backend.write(enqueueBlock(&arr[0], arr.size() * info.itemSize, info.itemAlignment, [arr, f]() {
					A myarr{ arr };
					for (opaque_obj& item : myarr)
						f(item);
				}));
				serializer.backend.write(arr.size());
				serializer.backend.write(arr.capacity());
				serializer.backend.write(static_cast<size_t>(0));
				return 0;
			}

			template<typename F, typename C, typename D, typename A>
			int visit_tarray(A& arr, const ArrayInfo& info, C c, D d, F f) {
				serializer.backend.write(enqueueBlock(&arr[0], arr.size() * info.itemSize, info.itemAlignment, [arr, f]() {
					A myarr{ arr };
					for (opaque_obj& item : myarr)
						f(item);
				}));
				serializer.backend.write(arr.size());
				serializer.backend.write(static_cast<intptr_t>(arr.capacity()));
				return 0;
			}

			template<typename F, typename A, typename S>
			int visit_pointer(opaque_obj*& obj, const PointerInfo<A, S>& info, F f) {
				serializer.backend.write(enqueueBlock(obj, info.getTargetSize(), info.getTargetAlignment(), [obj, f]() {
					f(*obj);
				}));
				return 0;
			}

			template<typename F>
			int visit_carray(opaque_obj* obj, const CArrayInfo& info, F f) {
				for (size_t i = 0; i < info.size; i++)
					f(*addptr(obj, i * info.stride));
				return 0;
			}

			template<typename F>
			int visit_union(opaque_obj& obj, const UnionInfo& info, F f) {
				f(obj);
				return 0;
			}

			template<typename F>
			int visit_type(opaque_obj& obj, const TypeInfo& info, F f) {
				serializer.backend.write_padding(info.alignment);

				size_t typeStart = serializer.backend.tellp();

				// Catch alignment issues.
				if (currentStructAddr)
					assert((serializer.backend.tellp() - dbgStructStartLoc) == (reinterpret_cast<size_t>(&obj) - reinterpret_cast<size_t>(currentStructAddr)));

				f(obj);

				serializer.backend.write_padding_bytes(info.size - (serializer.backend.tellp() - typeStart));

				// Catch alignment issues.
				if (currentStructAddr)
					assert((serializer.backend.tellp() - dbgStructStartLoc) == (reinterpret_cast<size_t>(&obj) + info.size - reinterpret_cast<size_t>(currentStructAddr)));
				return 0;
			}

			template<typename F>
			int visit_field(opaque_obj& obj, const FieldInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			int visit_base_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				f(obj);
				return 0;
			}

			template<typename F>
			int visit_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				size_t prevDbgStructStartLoc = dbgStructStartLoc;
				void* prevStructAddr = currentStructAddr;

				dbgStructStartLoc = serializer.backend.tellp();
				currentStructAddr = &obj;

				if (!serializer.ptrDisambiguationStore.get_reference(&obj, 0))
					serializer.ptrDisambiguationStore.set_reference(&obj, 0, dbgStructStartLoc);

				f(obj);

				dbgStructStartLoc = prevDbgStructStartLoc;
				currentStructAddr = prevStructAddr;
				return 0;
			}

			template<typename F>
			int visit_root(opaque_obj& obj, const RootInfo& info, F f) {
				enqueueBlock(&obj, info.size, info.alignment, [&obj, f]() { f(obj); });
				worker.processQueuedBlocks();
				return 0;
			}
		};

	public:
		ReflectionSerializer(Backend& backend) : backend{ backend } { }

		template<typename T, typename R>
		void serialize(T& data, R refl) {
			ucsl::reflection::traversals::traversal<SerializeChunk> operation{ *this };
			operation(data, refl);
		}
	};
}