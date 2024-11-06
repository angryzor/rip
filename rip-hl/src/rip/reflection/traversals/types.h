#pragma once

namespace rip::reflection {
	struct RootInfo {
		size_t alignment{};
		size_t size{};
	};

	struct StructureInfo {
		const char* name{};
		size_t alignment{};
	};

	struct FieldInfo {
		const char* name{};
	};

	struct ArrayFieldInfo : FieldInfo {
		size_t size{};
	};

	struct ArrayFieldItemInfo : ArrayFieldInfo {
		size_t index{};
	};

	struct PrimitiveInfo {
		size_t alignment{};
		size_t size{};
	};

	struct PointerInfo {
		size_t targetAlignment{};
		size_t targetSize{};
	};

	struct CArrayInfo {
		size_t size{};
		size_t stride{};
	};

	struct PrimitiveReprInfo {
		bool erased{};
	};
}
