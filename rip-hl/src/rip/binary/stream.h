#pragma once
#include <iostream>
#include <bit>
#include <ranges>
#include <rip/util/memory.h>
#include "types.h"

namespace rip::binary {
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

	public:
		binary_ostream(std::ostream& stream, std::endian endianness = std::endian::native) : stream{ stream }, endianness{ endianness } {}

		template<typename T>
		void write(const T& obj, size_t count = 1) {
			stream.write(reinterpret_cast<const char*>(&obj), sizeof(T) * count);
		}

		void write_padding(size_t alignment) {
			size_t pos = stream.tellp();
			size_t offset = align(pos, alignment);
			for (size_t i = 0; i < offset - pos; i++)
				stream.put(0);
		}

		void seekp(size_t loc) {
			stream.seekp(loc);
		}

		size_t tellp() {
			return stream.tellp();
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

		void seekp(size_t loc) {
			stream.seekp(loc + offset);
		}

		size_t tellp() {
			return stream.tellp() - offset;
		}
	};
}
