#pragma once
#include <ucsl-reflection/reflections/basic-types.h>
#include <ucsl-reflection/traversals/types.h>
#include <ucsl-reflection/opaque.h>
#include <rip/util/object-id-guids.h>
#include <yyjson.h>
#include <iomanip>
#include <sstream>
#include "BlobWorker.h"
#include "PointerDisambiguationStore.h"

namespace rip::binary {
	using namespace ucsl::reflection;
	using namespace ucsl::reflection::traversals;

	template<typename GameInterface, bool arrayVectors = false>
	class JsonDeserializer {
		yyjson_doc* doc{};
		const char* filename;
		opaque_obj* result{};

		template<typename OpState>
		class OperationBase {
		public:
			constexpr static size_t arity = 1;
			using result_type = int;
			OpState& state;

			template<typename F>
			void with_val(yyjson_val* val, F f) {
				yyjson_val* ref;
				while (yyjson_is_obj(val) && (ref = yyjson_obj_get(val, "$ref")))
					val = yyjson_ptr_get(state.deserializer.doc->root, yyjson_get_str(ref));

				yyjson_val* prevVal = state.currentVal;
				state.currentVal = val;
				f();
				state.currentVal = prevVal;
			}

			template<typename T>
			void enqueue_block(T*& ptr, auto&& allocationDataGetter, auto processFunc) {
				yyjson_val* reference = state.currentVal;

				state.ptrDisambiguationStore.add_pointer(reference, (void*&)ptr);

				state.worker.enqueueBlock(
					[this, reference, allocationDataGetter]() {
						return !state.ptrDisambiguationStore.is_resolved(reference, allocationDataGetter().size);
					},
					[this, reference, allocationDataGetter](opaque_obj* target) {
						state.ptrDisambiguationStore.set_resolved_target(reference, allocationDataGetter().size, target);
					},
					std::forward<decltype(allocationDataGetter)>(allocationDataGetter),
					[this, reference, processFunc](opaque_obj* offset, size_t alignment) {
						with_val(reference, [processFunc, offset]() { processFunc(offset); });
					}
				);
			}

			OperationBase(OpState& state) : state{ state } {}

			template<std::integral T, std::enable_if_t<std::is_signed_v<T>, bool> = true>
			result_type visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				obj = static_cast<T>(info.erased ? 0ll : info.constantValue.has_value() ? info.constantValue.value() : yyjson_get_sint(state.currentVal));
				return 0;
			}

			template<std::integral T, std::enable_if_t<!std::is_signed_v<T>, bool> = true>
			result_type visit_primitive(T& obj, const PrimitiveInfo<T>& info) {
				obj = static_cast<T>(info.erased ? 0ull : info.constantValue.has_value() ? info.constantValue.value() : yyjson_get_uint(state.currentVal));
				return 0;
			}

			result_type visit_primitive(float& obj, const PrimitiveInfo<float>& info) {
				obj = info.erased ? 0.0f : info.constantValue.has_value() ? info.constantValue.value() : static_cast<float>(yyjson_get_num(state.currentVal));
				return 0;
			}

			result_type visit_primitive(double& obj, const PrimitiveInfo<double>& info) {
				obj = info.erased ? 0.0 : info.constantValue.has_value() ? info.constantValue.value() : yyjson_get_num(state.currentVal);
				return 0;
			}

			result_type visit_primitive(bool& obj, const PrimitiveInfo<bool>& info) {
				obj = info.erased ? false : info.constantValue.has_value() ? info.constantValue.value() : yyjson_get_bool(state.currentVal);
				return 0;
			}

			// These can probably be replaced by a recursive simplerfl traversal.
			result_type visit_primitive(ucsl::math::Vector2& obj, const PrimitiveInfo<ucsl::math::Vector2>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 2);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Vector3& obj, const PrimitiveInfo<ucsl::math::Vector3>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 3);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
					obj.z() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "z")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Position& obj, const PrimitiveInfo<ucsl::math::Position>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 3);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
					obj.z() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "z")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Rotation& obj, const PrimitiveInfo<ucsl::math::Rotation>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 4);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj.coeffs()(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
					obj.z() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "z")));
					obj.w() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "w")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Vector4& obj, const PrimitiveInfo<ucsl::math::Vector4>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 4);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
					obj.z() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "z")));
					obj.w() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "w")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Quaternion& obj, const PrimitiveInfo<ucsl::math::Quaternion>& info) {
				if constexpr (arrayVectors) {
					assert(yyjson_arr_size(state.currentVal) == 4);
					size_t i, max;
					yyjson_val* item;
					yyjson_arr_foreach(state.currentVal, i, max, item) {
						obj.coeffs()(i, 0) = yyjson_get_num(item);
					}
				}
				else {
					obj.x() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "x")));
					obj.y() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "y")));
					obj.z() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "z")));
					obj.w() = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "w")));
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Matrix34& obj, const PrimitiveInfo<ucsl::math::Matrix34>& info) {
				assert(yyjson_arr_size(state.currentVal) == 12);
				size_t i, max;
				yyjson_val* item;
				yyjson_arr_foreach(state.currentVal, i, max, item) {
					obj(i / 4, i % 4) = yyjson_get_num(item);
				}
				return 0;
			}

			result_type visit_primitive(ucsl::math::Matrix44& obj, const PrimitiveInfo<ucsl::math::Matrix44>& info) {
				assert(yyjson_arr_size(state.currentVal) == 16);
				size_t i, max;
				yyjson_val* item;
				yyjson_arr_foreach(state.currentVal, i, max, item) {
					obj(i / 4, i % 4) = yyjson_get_num(item);
				}
				return 0;
			}

			template<ucsl::colors::ChannelOrder order>
			result_type visit_primitive(ucsl::colors::Color8<order>& obj, const PrimitiveInfo<ucsl::colors::Color8<order>>& info) {
				obj.r = static_cast<uint8_t>(yyjson_get_uint(yyjson_obj_get(state.currentVal, "r")));
				obj.g = static_cast<uint8_t>(yyjson_get_uint(yyjson_obj_get(state.currentVal, "g")));
				obj.b = static_cast<uint8_t>(yyjson_get_uint(yyjson_obj_get(state.currentVal, "b")));
				obj.a = static_cast<uint8_t>(yyjson_get_uint(yyjson_obj_get(state.currentVal, "a")));
				return 0;
			}

			template<ucsl::colors::ChannelOrder order>
			result_type visit_primitive(ucsl::colors::Colorf<order>& obj, const PrimitiveInfo<ucsl::colors::Colorf<order>>& info) {
				obj.r = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "r")));
				obj.g = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "g")));
				obj.b = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "b")));
				obj.a = static_cast<float>(yyjson_get_num(yyjson_obj_get(state.currentVal, "a")));
				return 0;
			}

			result_type visit_primitive(ucsl::objectids::ObjectIdV1& obj, const PrimitiveInfo<ucsl::objectids::ObjectIdV1>& info) {
				util::fromGUID(obj, yyjson_get_str(state.currentVal));
				return 0;
			}

			result_type visit_primitive(ucsl::objectids::ObjectIdV2& obj, const PrimitiveInfo<ucsl::objectids::ObjectIdV2>& info) {
				util::fromGUID(obj, yyjson_get_str(state.currentVal));
				return 0;
			}

			result_type visit_primitive(ucsl::strings::VariableString& obj, const PrimitiveInfo<ucsl::strings::VariableString>& info) {
				auto buffer = (const char**)addptr(&obj, sizeof(size_t) * 0);
				auto allocator = (void**)addptr(&obj, sizeof(size_t) * 1);
				if (yyjson_is_null(state.currentVal))
					*buffer = nullptr;
				else if (!strcmp("", yyjson_get_str(state.currentVal)))
					*buffer = nullptr;
				else
					visit_primitive(*buffer, PrimitiveInfo<const char*>{});
				*allocator = nullptr;
				return 0;
			}

			result_type visit_primitive(const char*& obj, const PrimitiveInfo<const char*>& info) {
				if (yyjson_is_null(state.currentVal))
					obj = nullptr;
				else {
					const char* str = yyjson_get_str(state.currentVal);

					enqueue_block(obj, [size = strlen(str) + 1]() { return BlockAllocationData{ size, 1 }; }, [str](opaque_obj* target) {
						strcpy_s((char*)target, strlen(str) + 1, str);
					});
				}
				return 0;
			}

			result_type visit_primitive(void*& obj, const PrimitiveInfo<void*>& info) {
				return 0;
			}

			template<typename T, typename O>
			result_type visit_enum(T& obj, const EnumInfo<O>& info) {
				if (yyjson_is_int(state.currentVal)) {
					obj = static_cast<T>(yyjson_get_sint(state.currentVal));
					return 0;
				}
				if (yyjson_is_str(state.currentVal)) {
					const char* str = yyjson_get_str(state.currentVal);
					for (auto& option : info.options) {
						if (!strcmp(option.GetEnglishName(), str)) {
							obj = static_cast<T>(option.GetIndex());
							return 0;
						}
					}
				}
				assert("unhandled enum");
				return 0;
			}

			template<typename T, typename O>
			result_type visit_flags(T& obj, const FlagsInfo<O>& info) {
				return visit_primitive(obj, PrimitiveInfo<T>{});
			}

			template<typename F, typename C, typename D, typename A>
			result_type visit_array(A& arr, const ArrayInfo& info, C c, D d, F f) {
				size_t arrsize = yyjson_arr_size(state.currentVal);
				auto buffer = (opaque_obj**)addptr(&arr.underlying, sizeof(size_t) * 0);
				auto length = (size_t*)addptr(&arr.underlying, sizeof(size_t) * 1);
				auto capacity = (size_t*)addptr(&arr.underlying, sizeof(size_t) * 2);
				auto allocator = (void**)addptr(&arr.underlying, sizeof(size_t) * 3);
				if (arrsize == 0)
					*buffer = nullptr;
				else
					enqueue_block(*buffer, [info, arrsize]() { return BlockAllocationData{ arrsize * info.itemSize, info.itemAlignment }; }, [this, itemSize = info.itemSize, f](opaque_obj* target) {
						size_t i, max;
						yyjson_val* item;
						yyjson_arr_foreach (state.currentVal, i, max, item) {
							with_val(item, [target, itemSize, i, f]() { f(*addptr(target, i * itemSize)); });
						}
					});
				*length = arrsize;
				*capacity = arrsize;
				*allocator = nullptr;
				return 0;
			}

			template<typename F, typename C, typename D, typename A>
			result_type visit_tarray(A& arr, const ArrayInfo& info, C c, D d, F f) {
				size_t arrsize = yyjson_arr_size(state.currentVal);
				auto buffer = (opaque_obj**)addptr(&arr.underlying, sizeof(size_t) * 0);
				auto length = (size_t*)addptr(&arr.underlying, sizeof(size_t) * 1);
				auto capacity = (intptr_t*)addptr(&arr.underlying, sizeof(size_t) * 2);
				if (arrsize == 0)
					*buffer = nullptr;
				else
					enqueue_block(*buffer, [info, arrsize]() { return BlockAllocationData{ arrsize * info.itemSize, info.itemAlignment }; }, [this, itemSize = info.itemSize, arrsize, f](opaque_obj* target) {
						size_t i, max;
						yyjson_val* item;
						yyjson_arr_foreach (state.currentVal, i, max, item) {
							with_val(item, [target, itemSize, i, f]() { f(*addptr(target, i * itemSize)); });
						}
					});
				*length = arrsize;
				*capacity = arrsize;
				return 0;
			}

			template<typename F, typename A, typename S>
			result_type visit_pointer(opaque_obj*& obj, const PointerInfo<A, S>& info, F f) {
				if (yyjson_is_null(state.currentVal))
					obj = nullptr;
				else if (info.isWeak)
					state.ptrDisambiguationStore.add_pointer(state.currentVal, (void*&)obj);
				else
					enqueue_block(obj, [info]() { return BlockAllocationData{ info.getTargetSize(), info.getTargetAlignment() }; }, [f](opaque_obj* target) { f(*target); });
				return 0;
			}

			template<typename F>
			result_type visit_carray(opaque_obj* obj, const CArrayInfo& info, F f) {
				assert(yyjson_arr_size(state.currentVal) == info.size);
				size_t i, max;
				yyjson_val* item;
				yyjson_arr_foreach (state.currentVal, i, max, item) {
					with_val(item, [f, obj, i, stride = info.stride]() { f(*addptr(obj, i * stride)); });
				}
				return 0;
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
				with_val(yyjson_obj_get(state.currentVal, info.name), [f, &obj]() { f(obj); });
				return 0;
			}

			template<typename F>
			result_type visit_base_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_struct(opaque_obj& obj, const StructureInfo& info, F f) {
				return f(obj);
			}

			template<typename F>
			result_type visit_root(opaque_obj& obj, const RootInfo& info, F f) {
				opaque_obj* ptr;
				with_val(yyjson_doc_get_root(state.deserializer.doc), [f, this, &ptr, &info]() {
					enqueue_block(ptr, [info]() { return BlockAllocationData{ info.size, info.alignment }; }, [f](opaque_obj* target) {
						f(*target);
					});
				});
				state.worker.processQueuedBlocks();
				return 0;
			}
		};

		template<typename Allocator>
		struct OperationState {
			JsonDeserializer& deserializer;
			yyjson_val* currentVal{};
			BlobWorker<opaque_obj*, Allocator, DeferredAllocationBlobWorkerScheduler> worker{};
			DeserializationPointerDisambiguationStore<yyjson_val*> ptrDisambiguationStore{};
		};

		using MeasureState = OperationState<HeapBlockAllocator<GameInterface, opaque_obj>>;
		using WriteState = OperationState<SequentialMemoryBlockAllocator<opaque_obj>>;

		MeasureState measureState{ *this };
		WriteState writeState{ *this };

	public:
		JsonDeserializer(const char* filename) : filename{ filename } {
		}

		~JsonDeserializer() {
			yyjson_doc_free(doc);
		}

		template<typename T, typename R>
		T* deserialize(R refl) {
			yyjson_read_err err;
			doc = yyjson_read_file(filename, 0, nullptr, &err);
			if (err.code != YYJSON_READ_SUCCESS) {
				std::cout << "Error reading json: " << err.msg << std::endl;
				return nullptr;
			}

			T* stub{};
			ucsl::reflection::traversals::traversal<OperationBase<MeasureState>> measureOp{ measureState };
			measureOp.operator()<T>(*stub, refl);
			size_t size = measureState.worker.allocator.sizeRequired;

			result = (opaque_obj*)GameInterface::AllocatorSystem::get_allocator()->Alloc(size, 16);
			writeState.worker.allocator.origin = result;

			memset(result, 0, size);

			ucsl::reflection::traversals::traversal<OperationBase<WriteState>> writeOp{ writeState };
			writeOp.operator()<T>(*(T*)result, refl);

			return (T*)result;
		}

		template<typename T, typename R>
		T* deserializeString(R refl) {
			yyjson_read_err err;
			doc = yyjson_read_opts((char*)filename, strlen(filename), 0, nullptr, &err);
			if (err.code != YYJSON_READ_SUCCESS) {
				std::cout << "Error reading json: " << err.msg << std::endl;
				return nullptr;
			}

			T* stub{};
			ucsl::reflection::traversals::traversal<OperationBase<MeasureState>> measureOp{ measureState };
			measureOp.operator()<T>(*stub, refl);
			size_t size = measureState.worker.allocator.sizeRequired;

			result = (opaque_obj*)GameInterface::AllocatorSystem::get_allocator()->Alloc(size, 16);
			writeState.worker.allocator.origin = result;

			memset(result, 0, size);

			ucsl::reflection::traversals::traversal<OperationBase<WriteState>> writeOp{ writeState };
			writeOp.operator()<T>(*(T*)result, refl);

			return (T*)result;
		}
	};
}