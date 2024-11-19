#pragma once
#include <optional>

namespace rip::binary {
	namespace serialized_types {
		template<typename U, typename T> struct offset_t : public std::optional<U> {
		public:
			using std::optional<U>::optional;
		};

		template<typename T> using o64_t = offset_t<size_t, T>;
		template<typename T> using o32_t = offset_t<unsigned int, T>;
	};
}
