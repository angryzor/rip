#pragma once
#include <iostream>
#include <bit>
#include <ranges>
#include <string>
#include <rip/util/memory.h>
#include <rip/util/byteswap.h>
#include "types.h"

namespace rip::binary {
	namespace internal {
		inline char zeroes[8192]{};
	}

	class fast_istream {
		std::istream& stream;
		size_t shadow_pos; // prevent doing slow tellg() calls.

	public:
		fast_istream(std::istream& stream) : stream{ stream }, shadow_pos{ (size_t)stream.tellg() } {}

		void read(char* str, size_t count) {
			stream.read(str, count);
			shadow_pos += count;
		}

		void read_string(std::string& str) {
			std::getline(stream, str, '\0');
			shadow_pos += str.size() + 1;
		}

		void seekg(size_t loc) {
			stream.seekg(loc);
			shadow_pos = loc;
		}

		size_t tellg() const {
			return shadow_pos;
		}
	};

	class fast_ostream {
		std::ostream& stream;
		size_t shadow_pos; // prevent doing slow tellg() calls.

	public:
		fast_ostream(std::ostream& stream) : stream{ stream }, shadow_pos{ (size_t)stream.tellp() } {}

		void write(const char* str, size_t count) {
			stream.write(str, count);
			shadow_pos += count;
		}

		void write_string(const char* str) {
			size_t size = strlen(str) + 1;
			stream.write(str, size);
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

	template<typename AddrType>
	class binary_istream {
	protected:
		fast_istream& stream;
		size_t offset;

	public:
		std::endian endianness;

		binary_istream(fast_istream& stream, std::endian endianness = std::endian::native, size_t offset = 0) : stream{ stream }, endianness{ endianness }, offset{ offset } {}

		template<typename T, bool byteswap = true>
		void read(T& obj) {
			stream.read(reinterpret_cast<char*>(&obj), sizeof(T));
			
			if constexpr (byteswap)
				util::byteswap_deep_to_native(endianness, obj);
		}

		template<typename T>
		void read(offset_t<T>& obj) {
			AddrType offset;
			read(offset);
			obj = offset == 0 ? offset_t<T>{} : offset_t<T>{ static_cast<size_t>(offset) };
		}

		void read_string(std::string& str) {
			stream.read_string(str);
		}

		void skip_padding(size_t alignment) {
			skip_padding_bytes(align(tellg(), alignment) - tellg());
		}

		void skip_padding_bytes(size_t size) {
			seekg(tellg() + size);
		}

		void seekg(size_t loc) {
			stream.seekg(loc + offset);
		}

		size_t tellg() const {
			return stream.tellg() - offset;
		}
	};

	template<typename AddrType, std::endian endianness = std::endian::native>
	class binary_ostream {
	protected:
		fast_ostream& stream;
		size_t offset;

	public:
		binary_ostream(fast_ostream& stream, size_t offset = 0) : stream{ stream }, offset{ offset } {}

		template<typename T, bool byteswap = true>
		void write(const T& obj) {
			if constexpr (byteswap) {
				T val = obj;
				util::byteswap_deep_to_native(endianness, val);
				stream.write(reinterpret_cast<const char*>(&val), sizeof(T));
			}
			else
				stream.write(reinterpret_cast<const char*>(&obj), sizeof(T));
		}

		template<typename T, bool byteswap = true>
		void write(T&& obj) {
			if constexpr (byteswap)
				util::byteswap_deep_to_native(endianness, obj);
			stream.write(reinterpret_cast<const char*>(&obj), sizeof(T));
		}

		template<typename T>
		void write(const offset_t<T>& obj) {
			write(obj.has_value() ? static_cast<AddrType>(obj.value()) : 0);
		}

		void write_string(const char* str) {
			stream.write_string(str);
		}

		void write_padding(size_t alignment) {
			write_padding_bytes(align(tellp(), alignment) - tellp());
		}

		void write_padding_bytes(size_t size) {
			stream.write(internal::zeroes, size);
		}

		void seekp(size_t loc) {
			stream.seekp(loc + offset);
		}

		size_t tellp() const {
			return stream.tellp() - offset;
		}
	};
}
