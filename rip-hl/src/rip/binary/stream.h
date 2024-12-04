#pragma once
#include <iostream>
#include <bit>
#include <ranges>
#include <rip/util/memory.h>
#include "types.h"

namespace rip::binary {
	namespace internal {
		inline char zeroes[8192]{};
	}

	class binary_istream {
		std::istream& stream;
	public:
		binary_istream(std::istream& stream) : stream{ stream } {}

		template<typename T>
		void read(T& obj) {
			stream.read(reinterpret_cast<char*>(&obj), sizeof(T));
		}
	};

	class binary_ostream {
		std::ostream& stream;
		std::endian endianness;
		size_t shadow_pos; // prevent doing slow tellp() calls. this does of course mean that other modules writing to the stream will cause incorrect results.

	public:
		binary_ostream(std::ostream& stream, std::endian endianness = std::endian::native) : stream{ stream }, endianness{ endianness }, shadow_pos{ (size_t)stream.tellp() } {}

		template<typename T>
		void write(const T& obj, size_t count = 1) {
			size_t size = sizeof(T) * count;
			stream.write(reinterpret_cast<const char*>(&obj), size);
			shadow_pos += size;
		}

		void write_padding(size_t alignment) {
			write_padding_bytes(align(shadow_pos, alignment) - shadow_pos);
		}

		void write_padding_bytes(size_t size) {
			stream.write(internal::zeroes, size);
			shadow_pos += size;
		}

		void seekp(size_t loc) {
			stream.seekp(loc);
			shadow_pos = loc;
		}

		size_t tellp() const {
			return shadow_pos;
		}
	};

	class offset_binary_ostream {
		binary_ostream& stream;
		size_t offset;

	public:
		offset_binary_ostream(binary_ostream& stream, size_t offset) : stream{ stream }, offset{ offset } {}
		offset_binary_ostream(binary_ostream& stream) : offset_binary_ostream{ stream, stream.tellp() } {}

		template<typename T>
		void write(const T& obj, size_t count = 1) {
			stream.write(obj, count);
		}

		void write_padding(size_t alignment) {
			stream.write_padding(alignment);
		}

		void write_padding_bytes(size_t size) {
			stream.write_padding_bytes(size);
		}

		void seekp(size_t loc) {
			stream.seekp(loc + offset);
		}

		size_t tellp() const {
			return stream.tellp() - offset;
		}
	};
}
