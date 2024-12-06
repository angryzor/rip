#pragma once

#include <map>
#include <queue>
#include <functional>
#include <cassert>
#include <rip/util/memory.h>

namespace rip::binary {
	struct SequentialBlockAllocator {
		size_t nextOffset{};

		size_t allocate(size_t size, size_t alignment) {
			size_t offset = align(nextOffset, alignment);
			nextOffset = offset + size;
			return offset;
		}
	};

	template<typename T = void>
	struct SequentialMemoryBlockAllocator {
		T* origin{};
		SequentialBlockAllocator seqAllocator{};

		T* allocate(size_t size, size_t alignment) {
			return addptr(origin, seqAllocator.allocate(size, alignment));
		}
	};

	template<typename GameInterface, typename T = void>
	struct HeapBlockAllocator {
		size_t sizeRequired{};
		std::vector<T*> live_objs{};

		T* allocate(size_t size, size_t alignment) {
			size_t offset = align(sizeRequired, alignment);
			sizeRequired = offset + size;

			return (T*)GameInterface::AllocatorSystem::get_allocator()->Alloc(size, alignment);
		}

		void cleanup() {
			for (auto* ptr : live_objs)
				GameInterface::AllocatorSystem::get_allocator()->Free(ptr);

			live_objs.clear();
		}
	};

	template<typename T = size_t, typename Allocator = SequentialBlockAllocator, bool deferred = true>
	class BlobWorker {
		struct WorkQueueEntry {
			T offset;
			size_t alignment;
			std::function<void(T, size_t)> processFunc;
		};

		std::queue<WorkQueueEntry> workQueue{};
		void processChunk(const WorkQueueEntry chunk) {
			chunk.processFunc(chunk.offset, chunk.alignment);
		}

	public:
		Allocator allocator;

		template<typename... Args>
		BlobWorker(Args&&... args) : allocator{ std::forward<Args>(args)... } {}

		template<typename F>
		T enqueueBlock(size_t size, size_t alignment, F processFunc) {
			T offset = allocator.allocate(size, alignment);
			if constexpr (deferred)
				workQueue.push(WorkQueueEntry{ offset, alignment, processFunc });
			else
				processChunk(WorkQueueEntry{ offset, alignment, processFunc });
			return offset;
		}

		void processQueuedBlocks() {
			while (!workQueue.empty()) {
				auto chunk = workQueue.front();
				workQueue.pop();

				processChunk(chunk);
			}
		}
	};
}
