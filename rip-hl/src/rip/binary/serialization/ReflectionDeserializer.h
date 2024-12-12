#pragma once
#include <type_traits>
#include <ucsl-reflection/reflections/basic-types.h>
#include <ucsl-reflection/traversals/types.h>
#include <ucsl-reflection/traversals/traversal.h>
#include <ucsl-reflection/opaque.h>
#include <rip/binary/types.h>
#include <rip/util/memory.h>
#include "BlobWorker.h"
#include <iostream>

namespace rip::binary {
	using namespace ucsl::reflection;
	using namespace ucsl::reflection::traversals;

	template<typename GameInterface, typename Backend>
	class ReflectionDeserializer {
		Backend& backend;

		// We keep the buffer size as well, since sometimes an earlier reference serialized a smaller slice
		// of the buffer, and in that case we can't simply point back to this already stored version.
		// We could do a lot of work to roll back and instead store a larger buffer, but for now I'm just
		// duplicating the data in that case, since it mostly happens with dangling pointers of empty MoveArrays.
		struct DisambiguatedOffset {
			opaque_obj*& ptr;
			//size_t bufferSize;
		};

		template<typename OpState>
		class OperationBase {
		public:
			constexpr static size_t arity = 1;
			typedef int result_type;

			OpState& state;
			size_t dbgStructStartLoc{};
			void* currentStructAddr{};

			OperationBase(OpState& state) : state{ state } {}

			template<typename T>
			void enqueueBlock(T*& ptr, offset_t<T> offset, auto alignmentGetter, auto processFunc) {
				if (!offset.has_value())
					return;

				size_t off = offset.value();

				auto it = state.knownOffsets.find(off);
				if (it != state.knownOffsets.end()) {
					ptr = (T*&)it->second.ptr;
					return;
				}

				state.worker.enqueueBlock((opaque_obj*&)ptr, alignmentGetter, [this, off, processFunc](opaque_obj* target, size_t alignment) {
					state.deserializer.backend.skip_padding(alignment);
					//assert(backend.tellg() == offset);

					size_t prevDbgStructStartLoc = dbgStructStartLoc;
					void* prevStructAddr = currentStructAddr;

					dbgStructStartLoc = 0;
					currentStructAddr = nullptr;

					auto pos = state.deserializer.backend.tellg();
					state.deserializer.backend.seekg(off);
					processFunc(target);
					state.deserializer.backend.seekg(pos);

					dbgStructStartLoc = prevDbgStructStartLoc;
					currentStructAddr = prevStructAddr;
					//assert(backend.tellg() == offset + size);
				});

				state.knownOffsets.emplace(off, DisambiguatedOffset{ (opaque_obj*&)ptr });
			}

			template<typename T>
			int visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				state.deserializer.backend.read(obj);
				return 0;
			}

			void read_string(const char*& obj) {
				//if constexpr (Backend::hasNativeStrings)
				//	state.deserializer.backend.read(obj);
				//else {
					offset_t<const char> offset{};
					state.deserializer.backend.read(offset);
					if (!offset.has_value())
						obj = nullptr;
					else {
						std::string temp{};

						auto pos = state.deserializer.backend.tellg();
						state.deserializer.backend.seekg(offset.value());
						state.deserializer.backend.read_string(temp);
						state.deserializer.backend.seekg(pos);

						enqueueBlock(obj, offset, [size = temp.size() + 1]() { return BlockAllocationData{ size, 1 }; }, [this](opaque_obj* target) {
							std::string str{};
							state.deserializer.backend.read_string(str);
							strcpy_s(reinterpret_cast<char*>(target), str.size() + 1, str.c_str());
						});
					}
				//}
			}

			int visit_primitive(const char*& obj, const PrimitiveInfo<const char*>& info) {
				read_string(obj);
				return 0;
			}

			int visit_primitive(ucsl::strings::VariableString& obj, const PrimitiveInfo<ucsl::strings::VariableString>& info) {
				auto buffer = (const char**)addptr(&obj, 0x0);
				auto allocator = (size_t*)addptr(&obj, sizeof(size_t));

				read_string(*buffer);
				state.deserializer.backend.read(*allocator);
				return 0;
			}

			int visit_primitive(void*& obj, const PrimitiveInfo<void*>& info) {
				offset_t<opaque_obj> offset{};
				state.deserializer.backend.read(offset);
				obj = !offset.has_value() ? nullptr : state.knownOffsets[offset.value()].pointer;
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
				auto buffer = (opaque_obj**)addptr(&arr.underlying, 0x0);
				auto length = (size_t*)addptr(&arr.underlying, 0x8);
				auto capacity = (size_t*)addptr(&arr.underlying, 0x10);
				auto allocator = (size_t*)addptr(&arr.underlying, 0x18);

				offset_t<opaque_obj> offset{};
				state.deserializer.backend.read(offset);
				state.deserializer.backend.read(*length);
				state.deserializer.backend.read(*capacity);
				state.deserializer.backend.read(*allocator);

				enqueueBlock(*buffer, offset, [info, length]() { return BlockAllocationData{ *length * info.itemSize, info.itemAlignment }; }, [length, itemSize = info.itemSize, f](opaque_obj* target) {
					for (size_t i = 0; i < *length; i++)
						f(*addptr(target, i * itemSize));
				});
				return 0;
			}

			template<typename F, typename C, typename D, typename A>
			int visit_tarray(A& arr, const ArrayInfo& info, C c, D d, F f) {
				auto buffer = (opaque_obj**)addptr(&arr.underlying, 0x0);
				auto length = (size_t*)addptr(&arr.underlying, 0x8);
				auto capacity = (size_t*)addptr(&arr.underlying, 0x10);

				offset_t<opaque_obj> offset{};
				state.deserializer.backend.read(offset);
				state.deserializer.backend.read(*length);
				state.deserializer.backend.read(*capacity);

				enqueueBlock(*buffer, offset, [info, length]() { return BlockAllocationData{ *length * info.itemSize, info.itemAlignment }; }, [length, itemSize = info.itemSize, f](opaque_obj* target) {
					for (size_t i = 0; i < *length; i++)
						f(*addptr(target, i * itemSize));
				});
				return 0;
			}

			template<typename F, typename A, typename S>
			int visit_pointer(opaque_obj*& obj, const PointerInfo<A, S>& info, F f) {
				offset_t<opaque_obj> offset{};
				state.deserializer.backend.read(offset);
				enqueueBlock(obj, offset, [info]() { return BlockAllocationData{ info.getTargetSize(), info.getTargetAlignment() }; }, [f](opaque_obj* target) {
					f(*target);
				});
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
				state.deserializer.backend.skip_padding(info.alignment);

				size_t typeStart = state.deserializer.backend.tellg();

				std::cout << "Type at " << std::hex << typeStart << " is " << std::hex << info.size << " large and aligned at " << std::hex << info.alignment << std::endl;

				// Catch alignment issues.
				if (currentStructAddr)
					assert((state.deserializer.backend.tellg() - dbgStructStartLoc) == (reinterpret_cast<size_t>(&obj) - reinterpret_cast<size_t>(currentStructAddr)));

				f(obj);

				state.deserializer.backend.skip_padding_bytes(info.size - (state.deserializer.backend.tellg() - typeStart));

				// Catch alignment issues.
				if (currentStructAddr)
					assert((state.deserializer.backend.tellg() - dbgStructStartLoc) == (reinterpret_cast<size_t>(&obj) + info.size - reinterpret_cast<size_t>(currentStructAddr)));
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

				dbgStructStartLoc = state.deserializer.backend.tellg();
				currentStructAddr = &obj;

				f(obj);

				dbgStructStartLoc = prevDbgStructStartLoc;
				currentStructAddr = prevStructAddr;
				return 0;
			}

			template<typename F>
			int visit_root(opaque_obj& obj, const RootInfo& info, F f) {
				opaque_obj* ptr;
				enqueueBlock(ptr, offset_t<opaque_obj>{ 0 }, [info]() { return BlockAllocationData{ info.size, info.alignment }; }, [f](opaque_obj* target) {
					f(*target);
				});
				state.worker.processQueuedBlocks();
				return 0;
			}
		};

		template<typename Allocator>
		struct OperationState {
			ReflectionDeserializer& deserializer;
			BlobWorker<ucsl::reflection::opaque_obj*, Allocator, DeferredAllocationBlobWorkerScheduler> worker{};
			std::map<size_t, DisambiguatedOffset> knownOffsets{};
		};

		using MeasureState = OperationState<HeapBlockAllocator<GameInterface, opaque_obj>>;
		using WriteState = OperationState<SequentialMemoryBlockAllocator<opaque_obj>>;

		MeasureState measureState{ *this };
		WriteState writeState{ *this };
		opaque_obj* result{};

	public:
		ReflectionDeserializer(Backend& backend) : backend{ backend } { }

		template<typename T, typename R>
		T* deserialize(R refl) {
			T* stub{};
			ucsl::reflection::traversals::traversal<OperationBase<MeasureState>> measureOp{ measureState };
			measureOp.operator()<T>(*stub, refl);
			size_t size = measureState.worker.allocator.sizeRequired;

			result = (opaque_obj*)GameInterface::AllocatorSystem::get_allocator()->Alloc(size, 16);
			writeState.worker.allocator.origin = result;

			memset(result, 0, size);

			backend.seekg(0);
			ucsl::reflection::traversals::traversal<OperationBase<WriteState>> writeOp{ writeState };
			writeOp.operator()<T>(*(T*)result, refl);

			return (T*)result;
		}
	};
}