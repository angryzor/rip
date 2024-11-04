#include <map>
#include <queue>
#include <functional>
#include <cassert>
#include <rip/util/memory.h>

namespace rip::binary {
	template<typename Backend, typename ChunkProcessor>
	class BlobSerializer {
		struct WorkQueueEntry {
			size_t dbgExpectedOffset;
			size_t alignment;
			std::function<void()> processFunc;
		};

		Backend& backend;
		ChunkProcessor& chunkProcessor;
		std::queue<WorkQueueEntry> workQueue{};
		size_t nextOffset{};

		size_t allocateBlock(size_t size, size_t alignment) {
			size_t offset = align(nextOffset, alignment);
			nextOffset = offset + size;
			return offset;
		}

	public:
		BlobSerializer(Backend& backend, ChunkProcessor& chunkProcessor) : backend{ backend }, chunkProcessor{ chunkProcessor } {}

		template<typename F>
		size_t enqueueBlock(size_t size, size_t alignment, F processFunc) {
			auto offset = allocateBlock(size, alignment);
			workQueue.push(WorkQueueEntry{ offset, alignment, processFunc });
			return offset;
		}

		void processQueuedBlocks() {
			while (!workQueue.empty()) {
				auto chunk = workQueue.front();
				workQueue.pop();

				backend.write_padding(chunk.alignment);
				assert(backend.tellp() == chunk.dbgExpectedOffset);

				chunk.processFunc();
			}
		}
	};
}
