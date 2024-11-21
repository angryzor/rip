#pragma once
#include <map>
#include <ranges>

template<typename K, typename V>
std::map<V, K> reverse_map(const std::map<K, V>& m) {
	auto reverseElements = std::views::all(m) | std::views::transform([](auto& p) { return std::pair{ p.second, p.first }; });

	return { reverseElements.begin(), reverseElements.end() };
}
