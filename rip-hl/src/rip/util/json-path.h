#pragma once
#include <vector>
#include <string>
#include <sstream>

namespace rip::util {
	class json_path_builder {
		std::vector<std::string> fragments{};

	public:
		void push(const std::string& fragment) {
			fragments.push_back(fragment);
		}

		void push(size_t idx) {
			std::ostringstream oss{};
			oss << idx;
			push(oss.str());
		}

		void pop() {
			fragments.pop_back();
		}

		std::string str() const {
			std::string res{ "#" };

			for (auto& frag : fragments) {
				res += "/";
				res += frag;
			}

			return res;
		}
	};
}
