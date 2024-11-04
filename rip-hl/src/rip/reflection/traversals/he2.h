#pragma once

template<typename Algorithm, typename = std::make_index_sequence<Algorithm::arity>>
class rflclass_traversal;
template<typename Algorithm, std::size_t... S>
class rflclass_traversal<Algorithm, std::index_sequence<S...>> {
	template<std::size_t, typename T = void*> using Obj = T;

	Algorithm algorithm;

	template<typename U>
	typename Algorithm::result_type ProcessPrimitive(Obj<S>... objs) {
		return algorithm.VisitPrimitive(static_cast<U*>(objs)...);
	}

	typename Algorithm::result_type ProcessPrimitive(Obj<S>... objs, const hh::fnd::RflClassMember::Type type) {
		switch (type) {
		case hh::fnd::RflClassMember::TYPE_BOOL: return ProcessPrimitive<bool>(objs...);
		case hh::fnd::RflClassMember::TYPE_SINT8: return ProcessPrimitive<int8_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_UINT8: return ProcessPrimitive<uint8_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_SINT16: return ProcessPrimitive<int16_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_UINT16: return ProcessPrimitive<uint16_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_SINT32: return ProcessPrimitive<int32_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_UINT32: return ProcessPrimitive<uint32_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_SINT64: return ProcessPrimitive<int64_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_UINT64: return ProcessPrimitive<uint64_t>(objs...);
		case hh::fnd::RflClassMember::TYPE_FLOAT: return ProcessPrimitive<float>(objs...);
		case hh::fnd::RflClassMember::TYPE_VECTOR2: return ProcessPrimitive<csl::math::Vector2>(objs...);
		case hh::fnd::RflClassMember::TYPE_VECTOR3: return ProcessPrimitive<csl::math::Vector3>(objs...);
		case hh::fnd::RflClassMember::TYPE_VECTOR4: return ProcessPrimitive<csl::math::Vector4>(objs...);
		case hh::fnd::RflClassMember::TYPE_QUATERNION: return ProcessPrimitive<csl::math::Quaternion>(objs...);
		case hh::fnd::RflClassMember::TYPE_MATRIX34: return ProcessPrimitive<csl::math::Matrix34>(objs...);
		case hh::fnd::RflClassMember::TYPE_MATRIX44: return ProcessPrimitive<csl::math::Matrix44>(objs...);
		case hh::fnd::RflClassMember::TYPE_CSTRING: return ProcessPrimitive<const char*>(objs...);
		case hh::fnd::RflClassMember::TYPE_STRING: return ProcessPrimitive<csl::ut::VariableString>(objs...);
		case hh::fnd::RflClassMember::TYPE_OBJECT_ID: return ProcessPrimitive<hh::game::ObjectId>(objs...);
		case hh::fnd::RflClassMember::TYPE_COLOR_BYTE: return ProcessPrimitive<csl::ut::Color8>(objs...);
		case hh::fnd::RflClassMember::TYPE_COLOR_FLOAT: return ProcessPrimitive<csl::ut::Color<float>>(objs...);
		case hh::fnd::RflClassMember::TYPE_POSITION: return ProcessPrimitive<csl::math::Position>(objs...);
		default:
			assert(!"reflective operation assertion failed: unknown primitive type");
			return typename Algorithm::result_type{};
		}
	}

	typename Algorithm::result_type ProcessSingle(Obj<S>... objs, const hh::fnd::RflClassMember* member, const hh::fnd::RflClassMember::Type type)
	{
		switch (type) {
		case hh::fnd::RflClassMember::TYPE_STRUCT:
			return ProcessStruct(objs..., member->m_pClass);
		default:
			return ProcessPrimitive(objs..., type);
		}
	}

	typename Algorithm::result_type ProcessArray(Obj<S>... objs, const hh::fnd::RflClassMember* member)
	{
		std::tuple<Obj<S, RflArrayAccessor<csl::ut::MoveArray>>...> arrs{ { *static_cast<csl::ut::MoveArray<void*>*>(objs), member }... };

		return algorithm.VisitArray(
			std::get<S>(arrs)...,
			[=]() {
				void* obj = new (std::align_val_t(16), hh::fnd::MemoryRouter::GetModuleAllocator()) char[member->GetSubTypeSizeInBytes()];

				if (member->GetSubType() == hh::fnd::RflClassMember::TYPE_STRUCT)
					hh::fnd::RflTypeInfoRegistry::GetInstance()->ConstructObject(hh::fnd::MemoryRouter::GetModuleAllocator(), obj, member->GetStructClass()->GetName());

				return obj;
			},
			[=](void* obj) {
				if (member->GetSubType() == hh::fnd::RflClassMember::TYPE_STRUCT)
					hh::fnd::RflTypeInfoRegistry::GetInstance()->CleanupLoadedObject(obj, member->GetStructClass()->GetName());

				hh::fnd::MemoryRouter::GetModuleAllocator()->Free(obj);
			},
			[=](Obj<S>... objs) { return ProcessSingle(objs..., member, member->GetSubType()); }
		);
	}

	typename Algorithm::result_type ProcessOldArray(Obj<S>... objs, const hh::fnd::RflClassMember* member)
	{
		std::tuple<Obj<S, RflArrayAccessor<hh::TArray>>...> arrs{ { *static_cast<hh::TArray<void*>*>(objs), member }... };

		return algorithm.VisitArray(
			std::get<S>(arrs)...,
			[=]() {
				void* obj = new (std::align_val_t(16), hh::fnd::MemoryRouter::GetModuleAllocator()) char[member->GetSubTypeSizeInBytes()];

				if (member->GetSubType() == hh::fnd::RflClassMember::TYPE_STRUCT)
					hh::fnd::RflTypeInfoRegistry::GetInstance()->ConstructObject(hh::fnd::MemoryRouter::GetModuleAllocator(), obj, member->GetStructClass()->GetName());

				return obj;
			},
			[=](void* obj) {
				if (member->GetSubType() == hh::fnd::RflClassMember::TYPE_STRUCT)
					hh::fnd::RflTypeInfoRegistry::GetInstance()->CleanupLoadedObject(obj, member->GetStructClass()->GetName());

				hh::fnd::MemoryRouter::GetModuleAllocator()->Free(obj);
			},
			[=](Obj<S>... objs) { return ProcessSingle(objs..., member, member->GetSubType() == hh::fnd::RflClassMember::TYPE_UINT32 ? hh::fnd::RflClassMember::TYPE_OBJECT_ID : member->GetSubType()); } // We're overriding this here because Forces seems to use TYPE_OLD_ARRAY,TYPE_UINT32 for object ID arrays...
		);
	}

	typename Algorithm::result_type ProcessEnum(Obj<S>... objs, const hh::fnd::RflClassMember* member) {
		return algorithm.VisitEnum(objs..., member->GetSubType(), member->GetEnumClass(), [=](Obj<S>... objs) { return ProcessSingle(objs..., member, member->GetSubType()); });
	}

	typename Algorithm::result_type ProcessFlags(Obj<S>... objs, const hh::fnd::RflClassMember* member) {
		auto* enumEntries = reinterpret_cast<const hh::fnd::RflArray<const hh::fnd::RflClassEnumMember>*>(member->GetAttribute("DisplayIndex")->GetData());

		return algorithm.VisitFlags(objs..., member->GetSubType(), enumEntries, [=](Obj<S>... objs) { return ProcessSingle(objs..., member, member->GetSubType()); });
	}

	typename Algorithm::result_type ProcessPointer(Obj<S>... objs, const hh::fnd::RflClassMember* member) {
		return algorithm.VisitPointer(static_cast<Obj<S>*>(objs)..., [=](Obj<S>... objs) { return ProcessSingle(objs..., member, member->GetSubType()); });
	}

	typename Algorithm::result_type ProcessSingleClassMember(Obj<S>... objs, const hh::fnd::RflClassMember* member) {
		switch (member->GetType()) {
		case hh::fnd::RflClassMember::TYPE_ARRAY: return ProcessArray(objs..., member);
		case hh::fnd::RflClassMember::TYPE_OLD_ARRAY: return ProcessOldArray(objs..., member);
		case hh::fnd::RflClassMember::TYPE_POINTER: return ProcessPointer(objs..., member);
		case hh::fnd::RflClassMember::TYPE_ENUM: return ProcessEnum(objs..., member);
		case hh::fnd::RflClassMember::TYPE_FLAGS: return ProcessFlags(objs..., member);
		case hh::fnd::RflClassMember::TYPE_SIMPLE_ARRAY:
			assert(!"This RflClass member type (SIMPLE_ARRAY) is not implemented yet because it is unused.");
			return typename Algorithm::result_type{};
		default: return ProcessSingle(objs..., member, member->GetType());
		}
	}

	typename Algorithm::result_type ProcessClassMember(Obj<S>... objs, Obj<S>... parents, const hh::fnd::RflClassMember* member) {
		size_t constArrSizeOrZero = member->m_Flags & 0x40000 ? std::min({ reinterpret_cast<hh::fnd::DynamicRflArraySizeResolver>(member->m_pEnum)(parents)... }) : member->GetCstyleArraySize();

		if (constArrSizeOrZero == 0)
			return algorithm.VisitClassMember(objs..., member, [=](Obj<S>... objs) { return ProcessSingleClassMember(objs..., member); });
		else
			return algorithm.VisitArrayClassMember(objs..., member, constArrSizeOrZero, [=](Obj<S>... objs) {
			typename Algorithm::result_type arrayResult{};
			for (size_t j = 0; j < constArrSizeOrZero; j++)
				arrayResult |= algorithm.VisitArrayClassMemberItem(
					reinterpret_cast<void*>(reinterpret_cast<size_t>(objs) + j * member->GetSingleSizeInBytes())...,
					member,
					j,
					[=](Obj<S>... objs) { return ProcessSingleClassMember(objs..., member); }
				);
			return arrayResult;
				});
	}

	typename Algorithm::result_type ProcessClassMembers(Obj<S>... objs, const hh::fnd::RflClass* rflClass) {
		const hh::fnd::RflClass* parent = rflClass->GetBaseType();

		typename Algorithm::result_type result{};

		if (parent != nullptr)
			result |= ProcessBaseStruct(objs..., parent);

		for (size_t i = 0; i < rflClass->m_pMembers.count; i++) {
			auto* member = &rflClass->m_pMembers.items[i];

			result |= ProcessClassMember(reinterpret_cast<void*>(reinterpret_cast<size_t>(objs) + member->m_Offset)..., objs..., member);
		}

		return result;
	}

	typename Algorithm::result_type ProcessBaseStruct(Obj<S>... objs, const hh::fnd::RflClass* rflClass) {
		return algorithm.VisitBaseStruct(objs..., rflClass, [=](Obj<S>... objs) { return ProcessClassMembers(objs..., rflClass); });
	}

	typename Algorithm::result_type ProcessStruct(Obj<S>... objs, const hh::fnd::RflClass* rflClass) {
		return algorithm.VisitStruct(objs..., rflClass, [=](Obj<S>... objs) { return ProcessClassMembers(objs..., rflClass); });
	}

public:
	template<typename ...Args>
	rflclass_traversal(Args&& ...objs) : algorithm{ std::forward<Args>(objs)... } {}

	typename Algorithm::result_type operator()(Obj<S>... objs, const hh::fnd::RflClass* rflClass) {
		return ProcessStruct(objs..., rflClass);
	}

	template<typename ...Args>
	static typename Algorithm::result_type Apply(Obj<S>... objs, const hh::fnd::RflClass* rflClass, Args&& ...objs) {
		return rflclass_traversal<Algorithm>{ std::forward<Args>(objs)... }(objs..., rflClass);
	}
	template<typename R, typename ...Args>
	static typename Algorithm::result_type Apply(Obj<S, R*>... objs, Args&& ...objs) {
		return rflclass_traversal<Algorithm>(objs..., &R::rflClass, std::forward<Args>(objs)...);
	}
};
