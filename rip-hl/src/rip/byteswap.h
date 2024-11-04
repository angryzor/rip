#pragma once
#include <iostream>
#include <bit>

#ifndef __cpp_lib_byteswap
namespace std {
	// Reference implementation from cppreference.com
	template<integral T> constexpr T byteswap(T value) noexcept
	{
		static_assert(has_unique_object_representations_v<T>, "T may not have padding bits");
		auto value_representation = bit_cast<array<byte, sizeof(T)>>(value);
		ranges::reverse(value_representation);
		return bit_cast<T>(value_representation);
	}
}
#endif

namespace rip {
	template<std::integral T> T byteswap(T value) noexcept { return std::byteswap(value); }
	template<> inline unsigned long long byteswap(unsigned long long value) noexcept { return _byteswap_uint64(value); }
	template<> inline unsigned int byteswap(unsigned int value) noexcept { return _byteswap_ulong(value); }
	template<> inline unsigned short byteswap(unsigned short value) noexcept { return _byteswap_ushort(value); }

	template<std::integral T>
	T byteswap_to_native(std::endian endianness, T value) noexcept {
		return std::endian::native != endianness ? byteswap(value) : value;
	}

	template<typename T> void byteswap_deep(T& value) noexcept { value.byteswap_deep(); }
	template<std::integral T> void byteswap_deep(T& value) noexcept { value = byteswap(value); }

	template<typename T>
	void byteswap_deep_to_native(std::endian endianness, T& value) noexcept {
		if (std::endian::native != endianness)
			byteswap_deep(value);
	}
}
