#pragma once
#include <unordered_map>
#include <optional>
#include <ucsl-reflection/opaque.h>

namespace rip::binary {
	using namespace ucsl::reflection;

	template<typename ReferenceType>
	class SerializationPointerDisambiguationStore {
		struct DisambiguatedPointer {
			ReferenceType reference;
			size_t bufferSize;
		};

		std::unordered_map<const void*, DisambiguatedPointer> knownPtrs{};

	public:
		std::optional<ReferenceType> get_reference(const void* ptr, size_t size) {
			auto it = knownPtrs.find(ptr);
			if (it != knownPtrs.end() && size <= it->second.bufferSize)
				return std::make_optional(it->second.reference);

			return std::nullopt;
		}

		void set_reference(const void* ptr, size_t size, ReferenceType reference) {
			knownPtrs[ptr] = { reference, size };
		}
	};

	template<typename ReferenceType>
	class DeserializationPointerDisambiguationStore {
		// We keep the buffer size as well, since sometimes an earlier reference serialized a smaller slice
		// of the buffer, and in that case we can't simply point back to this already stored version.
		// We could do a lot of work to roll back and instead store a larger buffer, but for now I'm just
		// duplicating the data in that case, since it mostly happens with dangling pointers of empty MoveArrays.
		struct DisambiguatedReference {
			struct Resolution {
				void* ptr;
				size_t size;
			};

			std::optional<Resolution> resolved{};
			std::vector<void**> ptrs{};
		};

		std::unordered_map<ReferenceType, DisambiguatedReference> knownReferences{};

	public:
		void add_pointer(ReferenceType reference, void*& ptr) {
			auto it = knownReferences.find(reference);
			if (it != knownReferences.end()) {
				it->second.ptrs.push_back(&ptr);
				
				if (it->second.resolved.has_value())
					ptr = it->second.resolved.value().ptr;
			}
			else
				knownReferences.emplace(reference, DisambiguatedReference{ std::nullopt, { &ptr } });
		}

		bool is_resolved(ReferenceType reference, size_t size) {
			auto it = knownReferences.find(reference);
			if (!it->second.resolved.has_value())
				return false;

			auto& resolved = it->second.resolved.value();

			if (resolved.size < size)
				return false;

			return true;
		}

		void set_resolved_target(ReferenceType reference, size_t size, void* target) {
			auto& disamb = knownReferences.find(reference)->second;

			for (void** ptr : disamb.ptrs)
				*ptr = target;

			disamb.resolved = { target, size };
		}
	};
}
