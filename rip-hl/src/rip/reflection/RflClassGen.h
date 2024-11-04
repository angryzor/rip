#pragma once
#include <simple-reflection/declarations.h>

using namespace simple_reflection::declarations {
	template<typename T> struct RflClassMemberSize { static constexpr size_t size = sizeof(T); };
	template<DynamicRflClassResolver classResolver> struct RflClassMemberSize<RflDynamicStructPointer<classResolver>> { static constexpr size_t size = sizeof(void*); };
	template<DynamicRflClassResolver classResolver> struct RflClassMemberSize<RflDynamicStructSelfPointer<classResolver>> { static constexpr size_t size = sizeof(void*); };
	template<DynamicRflClassResolver classResolver, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberSize<RflDynamicStructPointerArray<classResolver, arraySizeResolver>> { static constexpr size_t size = sizeof(void*); };
	template<DynamicRflClassResolver classResolver, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberSize<RflDynamicStructSelfPointerArray<classResolver, arraySizeResolver>> { static constexpr size_t size = sizeof(void*); };
	template<typename T, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberSize<RflDynamicPointerArray<T, arraySizeResolver>> { static constexpr size_t size = sizeof(void*); };
	template<typename T, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberSize<RflDynamicInlineArray<T, arraySizeResolver>> { static constexpr size_t size = 0; };

	template<typename T> struct RflClassMemberAlignment { static constexpr size_t align = alignof(T); };
	template<DynamicRflClassResolver classResolver> struct RflClassMemberAlignment<RflDynamicStructPointer<classResolver>> { static constexpr size_t align = alignof(void*); };
	template<DynamicRflClassResolver classResolver> struct RflClassMemberAlignment<RflDynamicStructSelfPointer<classResolver>> { static constexpr size_t align = alignof(void*); };
	template<DynamicRflClassResolver classResolver, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberAlignment<RflDynamicStructPointerArray<classResolver, arraySizeResolver>> { static constexpr size_t align = alignof(void*); };
	template<DynamicRflClassResolver classResolver, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberAlignment<RflDynamicStructSelfPointerArray<classResolver, arraySizeResolver>> { static constexpr size_t align = alignof(void*); };
	template<typename T, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberAlignment<RflDynamicPointerArray<T, arraySizeResolver>> { static constexpr size_t align = alignof(void*); };
	template<typename T, DynamicRflArraySizeResolver arraySizeResolver> struct RflClassMemberAlignment<RflDynamicInlineArray<T, arraySizeResolver>> { static constexpr size_t align = alignof(void*); };
}
