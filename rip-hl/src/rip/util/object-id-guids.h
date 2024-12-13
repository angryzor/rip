#pragma once
#include <ucsl/object-id.h>
#include <string>
#include <cinttypes>

namespace rip::util {
	inline bool fromGUID(ucsl::objectids::ObjectIdV1& id, const char* str) {
		auto& b = reinterpret_cast<uint8_t(&)[4]>(id);
		uint8_t i[12];
		int res = sscanf_s(str, "{%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "}", &b[3], &b[2], &b[1], &b[0], &i[0], &i[1], &i[2], &i[3], &i[4], &i[5], &i[6], &i[7], &i[8], &i[9], &i[10], &i[11]);
		return res == 16;
	}

	inline bool fromGUID(ucsl::objectids::ObjectIdV2& id, const char* str) {
		auto& b = reinterpret_cast<uint8_t(&)[16]>(id);
		int res = sscanf_s(str, "{%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "}", &b[3], &b[2], &b[1], &b[0], &b[5], &b[4], &b[7], &b[6], &b[8], &b[9], &b[10], &b[11], &b[12], &b[13], &b[14], &b[15]);
		return res == 16;
	}

	template<size_t Len, typename = std::enable_if_t<Len >= 39>>
	inline void toGUID(const ucsl::objectids::ObjectIdV1& id, char (&guid)[Len]) {
		auto& b = reinterpret_cast<const uint8_t(&)[4]>(id);
		sprintf_s(guid, "{%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "}", b[3], b[2], b[1], b[0], 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}

	template<size_t Len, typename = std::enable_if_t<Len >= 39>>
	inline void toGUID(const ucsl::objectids::ObjectIdV2& id, char(&guid)[Len]) {
		auto& b = reinterpret_cast<const uint8_t(&)[16]>(id);
		sprintf_s(guid, "{%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "-%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "%02" SCNx8 "}", b[3], b[2], b[1], b[0], b[5], b[4], b[7], b[6], b[8], b[9], b[10], b[11], b[12], b[13], b[14], b[15]);
	}

	template<typename T>
	inline T fromGUID(const char* str) {
		T id;
		fromGUID(id, str);
		return id;
	}

	template<typename T>
	inline std::string toGUID(const T& id) {
		char guid[39];
		toGUID(id, guid);
		return guid;
	}
}