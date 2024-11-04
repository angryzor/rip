#pragma once
#include <ranges>
#include "C:\\Users\\Ruben Tytgat\\source\\repos\\miller-sdk\\miller-api\\miller-api\\miller-api.h"

namespace rip::rfl::db {
	class InGameDB {
	public:
		using RflClassId = const hh::fnd::RflClass*;
		using RflClassMemberId = const hh::fnd::RflClassMember*;
		using RflEnumClassId = const hh::fnd::RflClassEnum*;

		auto getClassMembers(RflClassId classId) {
			return std::span(classId->m_pMembers.items, classId->m_pMembers.count);
		}

		auto getClassEnums(RflClassId classId) {
			return std::span(classId->m_pEnums.items, classId->m_pEnums.count);
		}

		auto getClassSize(RflClassId classId) {
			return classId->GetSizeInBytes();
		}

		auto getClassAlignment(RflClassId classId) {
			return classId->GetAlignment();
		}

		auto getClassMemberType(RflClassMemberId memberId) {
			return memberId->GetType();
		}

		auto getClassMemberSubType(RflClassMemberId memberId) {
			return memberId->GetSubType();
		}

		auto getClassMember
	};
}