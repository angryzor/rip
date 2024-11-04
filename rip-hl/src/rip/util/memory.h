#pragma once

namespace rip {
	template<typename T, typename A>
	inline T align(T addr, A alignment) {
		return (T)(((size_t)addr + alignment - 1) & ~(alignment - 1));
	}

	template<typename T>
	inline T* addptr(T* addr, size_t off) {
		return reinterpret_cast<T*>(reinterpret_cast<size_t>(addr) + off);
	}
}
