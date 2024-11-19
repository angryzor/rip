#pragma once

#include <map>
#include <queue>
#include <functional>
#include <cassert>
#include "BlobWorker.h"

namespace rip::binary {
	template<typename Backend>
	class BlobSerializer {
		Backend& backend;
		BlobWorker<> worker{};

	public:
		BlobSerializer(Backend& backend) : backend{ backend } {}

		template<typename F>
		size_t enqueueBlock(size_t size, size_t alignment, F processFunc) {
			return worker.enqueueBlock(size, alignment, [=](size_t offset, size_t alignment) {
				backend.write_padding(alignment);
				assert(backend.tellp() == offset);

				processFunc();
			});
		}

		void processQueuedBlocks() {
			worker.processQueuedBlocks();
		}
	};
}
