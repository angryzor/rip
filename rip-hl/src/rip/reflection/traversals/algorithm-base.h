#pragma once

namespace rip::reflection {
	template<typename ResultType, size_t arity, typename... Types>
	class AlgorithmBase {
	public:
		using result_type = ResultType;
		constexpr static size_t arity = arity;
	};
}
