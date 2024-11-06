#pragma once
#include <tuple>
#include "types.h"

namespace rip::reflection {
	namespace internal {
		template<typename RflSystem>
		struct rflclass_traversal_for {
			template<typename Algorithm, typename = std::make_index_sequence<Algorithm::arity>>
			class rflclass_traversal;
			template<typename Algorithm, std::size_t... S>
			class rflclass_traversal<Algorithm, std::index_sequence<S...>> {
				template<std::size_t, typename T = void> using Obj = T*;

				Algorithm algorithm;

				using MemberType = typename RflSystem::RflClassMember::Type;

				template<typename U>
				typename Algorithm::result_type process_primitive_repr(Obj<S>... objs) {
					return algorithm.visit_primitive_repr(static_cast<U*>(objs)...);
				}

				typename Algorithm::result_type process_primitive(Obj<S>... objs, const MemberType type) {
					switch (type) {
					case MemberType::BOOL: return process_primitive_repr<bool>(objs...);
					case MemberType::SINT8: return process_primitive_repr<int8_t>(objs...);
					case MemberType::UINT8: return process_primitive_repr<uint8_t>(objs...);
					case MemberType::SINT16: return process_primitive_repr<int16_t>(objs...);
					case MemberType::UINT16: return process_primitive_repr<uint16_t>(objs...);
					case MemberType::SINT32: return process_primitive_repr<int32_t>(objs...);
					case MemberType::UINT32: return process_primitive_repr<uint32_t>(objs...);
					case MemberType::SINT64: return process_primitive_repr<int64_t>(objs...);
					case MemberType::UINT64: return process_primitive_repr<uint64_t>(objs...);
					case MemberType::FLOAT: return process_primitive_repr<float>(objs...);
					case MemberType::VECTOR2: return process_primitive_repr<csl::math::Vector2>(objs...);
					case MemberType::VECTOR3: return process_primitive_repr<csl::math::Vector3>(objs...);
					case MemberType::VECTOR4: return process_primitive_repr<csl::math::Vector4>(objs...);
					case MemberType::QUATERNION: return process_primitive_repr<csl::math::Quaternion>(objs...);
					case MemberType::MATRIX34: return process_primitive_repr<csl::math::Matrix34>(objs...);
					case MemberType::MATRIX44: return process_primitive_repr<csl::math::Matrix44>(objs...);
					case MemberType::CSTRING: return process_primitive_repr<const char*>(objs...);
					case MemberType::STRING: return process_primitive_repr<csl::ut::VariableString>(objs...);
					case MemberType::OBJECT_ID: return process_primitive_repr<hh::game::ObjectId>(objs...);
					case MemberType::COLOR_BYTE: return process_primitive_repr<csl::ut::Color8>(objs...);
					case MemberType::COLOR_FLOAT: return process_primitive_repr<csl::ut::Color<float>>(objs...);
					case MemberType::POSITION: return process_primitive_repr<csl::math::Position>(objs...);
					default:
						assert(!"reflective operation assertion failed: unknown primitive type");
						return typename Algorithm::result_type{};
					}
				}

				typename Algorithm::result_type process_single(Obj<S>... objs, const RflSystem::RflClassMember* member, const MemberType type)
				{
					switch (type) {
					case MemberType::STRUCT:
						return process_struct(objs..., member->GetClass());
					default:
						return process_primitive(objs..., type);
					}
				}

				//typename Algorithm::result_type process_array(Obj<S>... objs, const RflSystem::RflClassMember* member)
				//{
				//	std::tuple<Obj<S, RflArrayAccessor<csl::ut::MoveArray>>...> arrs{ { *static_cast<csl::ut::MoveArray<void*>*>(objs), member }... };

				//	return algorithm.VisitArray(
				//		std::get<S>(arrs)...,
				//		[=]() {
				//			void* obj = new (std::align_val_t(16), RflSystem::MemoryRouter::GetModuleAllocator()) char[member->GetSubTypeSizeInBytes()];

				//			if (member->GetSubType() == MemberType::STRUCT)
				//				RflSystem::RflTypeInfoRegistry::GetInstance()->ConstructObject(RflSystem::MemoryRouter::GetModuleAllocator(), obj, member->GetStructClass()->GetName());

				//			return obj;
				//		},
				//		[=](void* obj) {
				//			if (member->GetSubType() == MemberType::STRUCT)
				//				RflSystem::RflTypeInfoRegistry::GetInstance()->CleanupLoadedObject(obj, member->GetStructClass()->GetName());

				//			RflSystem::MemoryRouter::GetModuleAllocator()->Free(obj);
				//		},
				//		[=](Obj<S>... objs) { return process_single(objs..., member, member->GetSubType()); }
				//	);
				//}

				//typename Algorithm::result_type process_old_array(Obj<S>... objs, const RflSystem::RflClassMember* member)
				//{
				//	std::tuple<Obj<S, RflArrayAccessor<hh::TArray>>...> arrs{ { *static_cast<hh::TArray<void*>*>(objs), member }... };

				//	return algorithm.VisitArray(
				//		std::get<S>(arrs)...,
				//		[=]() {
				//			void* obj = new (std::align_val_t(16), RflSystem::MemoryRouter::GetModuleAllocator()) char[member->GetSubTypeSizeInBytes()];

				//			if (member->GetSubType() == MemberType::STRUCT)
				//				RflSystem::RflTypeInfoRegistry::GetInstance()->ConstructObject(RflSystem::MemoryRouter::GetModuleAllocator(), obj, member->GetStructClass()->GetName());

				//			return obj;
				//		},
				//		[=](void* obj) {
				//			if (member->GetSubType() == MemberType::STRUCT)
				//				RflSystem::RflTypeInfoRegistry::GetInstance()->CleanupLoadedObject(obj, member->GetStructClass()->GetName());

				//			RflSystem::MemoryRouter::GetModuleAllocator()->Free(obj);
				//		},
				//		[=](Obj<S>... objs) { return process_single(objs..., member, member->GetSubType() == MemberType::UINT32 ? MemberType::OBJECT_ID : member->GetSubType()); } // We're overriding this here because Forces seems to use Type::OLD_ARRAY,Type::UINT32 for object ID arrays...
				//	);
				//}

				//typename Algorithm::result_type process_enum(Obj<S>... objs, const RflSystem::RflClassMember* member) {
				//	return algorithm.visit_enum(objs..., member->GetSubType(), member->GetEnumClass(), [=](Obj<S>... objs) { return process_single(objs..., member, member->GetSubType()); });
				//}

				//typename Algorithm::result_type process_flags(Obj<S>... objs, const RflSystem::RflClassMember* member) {
				//	auto* enumEntries = reinterpret_cast<const RflSystem::RflArray<const RflSystem::RflClassEnumMember>*>(member->GetAttribute("DisplayIndex")->GetData());

				//	return algorithm.visit_flags(objs..., member->GetSubType(), enumEntries, [=](Obj<S>... objs) { return process_single(objs..., member, member->GetSubType()); });
				//}

				typename Algorithm::result_type process_pointer(Obj<S>... objs, const RflSystem::RflClassMember* member) {
					return algorithm.visit_pointer(static_cast<Obj<S>*>(objs)..., PointerInfo{ member->GetSubTypeAlignment(), member->GetSubTypeSize() }, [=](Obj<S>... objs) { return process_single(objs..., member, member->GetSubType()); });
				}

				typename Algorithm::result_type process_type(Obj<S>... objs, const RflSystem::RflClassMember* member) {
					switch (member->GetType()) {
					case MemberType::ARRAY: return process_array(objs..., member);
					case MemberType::OLD_ARRAY: return process_old_array(objs..., member);
					case MemberType::POINTER: return process_pointer(objs..., member);
					//case MemberType::ENUM: return process_enum(objs..., member);
					//case MemberType::FLAGS: return process_flags(objs..., member);
					case MemberType::SIMPLE_ARRAY:
						assert(!"This RflClass member type (SIMPLE_ARRAY) is not implemented yet because it is unused.");
						return typename Algorithm::result_type{};
					default: return process_single(objs..., member, member->GetType());
					}
				}

				typename Algorithm::result_type process_class_member(Obj<S>... objs, const RflSystem::RflClassMember* member) {
					//size_t constArrSizeOrZero = member->m_Flags & 0x40000 ? std::min({ reinterpret_cast<RflSystem::DynamicRflArraySizeResolver>(member->m_pEnum)(parents)... }) : member->GetCstyleArraySize();

					//if (constArrSizeOrZero == 0)
					return algorithm.visit_field(objs..., FieldInfo{ member->GetName() }, [=](Obj<S>... objs) { return process_type(objs..., member); });
					//else
					//	return algorithm.VisitArrayClassMember(objs..., member, constArrSizeOrZero, [=](Obj<S>... objs) {
					//		typename Algorithm::result_type arrayResult{};
					//		for (size_t j = 0; j < constArrSizeOrZero; j++)
					//			arrayResult |= algorithm.VisitArrayClassMemberItem(
					//				reinterpret_cast<void*>(reinterpret_cast<size_t>(objs) + j * member->GetSingleSizeInBytes())...,
					//				member,
					//				j,
					//				[=](Obj<S>... objs) { return process_type(objs..., member); }
					//			);
					//		return arrayResult;
					//	});
				}

				typename Algorithm::result_type process_class_members(Obj<S>... objs, const RflSystem::RflClass* rflClass) {
					const RflSystem::RflClass* parent = rflClass->GetParent();

					typename Algorithm::result_type result{};

					if (parent != nullptr)
						result |= process_base_struct(objs..., parent);

					for (auto& member : rflClass->GetMembers())
						result |= process_class_member(addptr(objs, member.GetOffset())..., &member);

					return result;
				}

				typename Algorithm::result_type process_base_struct(Obj<S>... objs, const RflSystem::RflClass* rflClass) {
					return algorithm.visit_base_struct(objs..., StructureInfo{ rflClass->GetName(), rflClass->GetAlignment() }, [=](Obj<S>... objs) { return process_class_members(objs..., rflClass); });
				}

				typename Algorithm::result_type process_struct(Obj<S>... objs, const RflSystem::RflClass* rflClass) {
					return algorithm.visit_struct(objs..., StructureInfo{ rflClass->GetName(), rflClass->GetAlignment() }, [=](Obj<S>... objs) { return process_class_members(objs..., rflClass); });
				}

			public:
				template<typename ...Args>
				rflclass_traversal(Args&& ...objs) : algorithm{ std::forward<Args>(objs)... } {}

				typename Algorithm::result_type operator()(Obj<S>... objs, const RflSystem::RflClass* rflClass) {
					return process_struct(objs..., rflClass);
				}

				template<typename ...Args>
				static typename Algorithm::result_type apply(Obj<S>... objs, const RflSystem::RflClass* rflClass, Args&& ...objs) {
					return rflclass_traversal<Algorithm>{ std::forward<Args>(objs)... }(objs..., rflClass);
				}
				template<typename R, typename ...Args>
				static typename Algorithm::result_type apply(Obj<S, R*>... objs, Args&& ...objs) {
					return rflclass_traversal<Algorithm>(objs..., &R::rflClass, std::forward<Args>(objs)...);
				}
			};
		}
	};

	template<typename T>
	using rflclass_traversal = typename internal::rflclass_traversal_for<T>::traversal;
}
