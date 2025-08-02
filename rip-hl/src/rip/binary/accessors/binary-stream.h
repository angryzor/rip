#pragma once
#include <ucsl/containers/arrays/array.h>
#include <ucsl/containers/arrays/tarray.h>
#include <ucsl-reflection/providers/types.h>
#include <ucsl-reflection/util/memory.h>
#include <ucsl-reflection/bound-reflection.h>

namespace ucsl::reflection::accessors {
	template<typename Stream>
	struct binary_istream {
		struct opaque_value {};

		struct Reference {
			Stream& stream;
			size_t offset;

			inline Reference(Stream& stream, size_t offset) : stream{ stream }, offset{ offset } {}
			inline Reference(Stream& stream) : stream{ stream }, offset{ stream.tellg() } {}

			auto withStream(auto f) {
				size_t prevOff{ offset };
				stream.seekg(offset);
				auto res = f(stream);
				stream.seekg(prevOff);
				return res;
			}
		};

		template<typename Refl>
		class AccessorBase {
		public:
			Reference reference;
			Refl refl;

			inline AccessorBase(Reference reference, const Refl& refl) : reference{ reference }, refl{ refl } {}
		};

		template<typename Refl>
		class ValueAccessor;

		template<typename Refl>
		class PrimitiveDataAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl, typename Refl::repr>::AccessorBase;

			operator typename Refl::repr() const {
				typename Refl::repr res;
				this->reference.withStream([&](auto& stream) {
					stream.read(res);
				});
				return res;
			}
		};

		template<typename Refl>
		class PrimitiveAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl>::AccessorBase;

			template<typename F>
			inline const auto visit(F f) const {
				return this->refl.visit([&](auto r){ return f(PrimitiveDataAccessor<decltype(r)>{ this->reference, r }); });
			}
		};

		template<typename Refl>
		class EnumAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl>::AccessorBase;
			
			operator long long () const {
				return this->refl.visit([&](auto r){
					PrimitiveDataAccessor<decltype(r)> pd{ this->reference, r };

					return static_cast<long long>(pd);
				});
			}
		};

		template<typename Refl>
		class StructureAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl>::AccessorBase;

			template<typename FieldRefl>
			inline const auto operator[](const FieldRefl& field_refl) const {
				auto type = field_refl.get_type(*this);

				return ValueAccessor<decltype(type)>{ { this->reference.stream, this->reference.offset + field_refl.get_offset() }, type };
			}

			inline auto get_base() const {
				auto base = this->refl.get_base();

				return base.has_value() ? std::make_optional(StructureAccessor<std::remove_reference_t<decltype(base.value())>>{ this->reference, base.value() }) : std::nullopt;
			}
		};

		template<typename Refl>
		class CArrayAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl>::AccessorBase;

			inline size_t get_length() const {
				return this->refl.get_length();
			}

			inline const auto operator[](size_t idx) const {
				auto item_refl = this->refl.get_item_type();

				assert(idx < this->refl.get_length());

				return ValueAccessor<decltype(item_refl)>{ { this->reference.stream, this->reference.offset + idx * item_refl.get_size(*this) }, item_refl };
			}
		};

		template<typename Refl>
		class PointerAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl, opaque_value*>::AccessorBase;

			inline auto get() const {
				auto target_type = this->refl.get_target_type();

				this->reference.withStream([&](auto& stream) {
					offset_t<opaque_obj> offset{};
					stream.read(offset);

					return !offset.has_value() ? std::nullopt : std::make_optional<const ValueAccessor<decltype(target_type)>>({ { this->reference.stream, offset.value() }, target_type });
				});
			}
		};

		template<template<typename, typename> typename A, typename Refl>
		class ArrayAccessor : public AccessorBase<Refl, RflArray<A, Refl>> {
		public:
			class const_iterator {
				ArrayAccessor& accessor;
				size_t idx{};

			public:
				const_iterator(ArrayAccessor& accessor, size_t idx) : accessor{ accessor }, idx{ idx } {}
				const_iterator(const const_iterator& other) : accessor{ other.accessor }, idx{ other.idx } {}

				const_iterator& operator++() {
					idx++;
					return *this;
				}

				const_iterator operator++(int) {
					const_iterator result{ *this };
					idx++;
					return result;
				}

				const_iterator& operator--() {
					idx--;
					return *this;
				}

				const_iterator operator--(int) {
					const_iterator result{ *this };
					idx--;
					return result;
				}

				bool operator==(const const_iterator& other) const { return idx == other.idx; }
				bool operator!=(const const_iterator& other) const { return idx != other.idx; }
				bool operator<(const const_iterator& other) const { return idx < other.idx; }
				bool operator>(const const_iterator& other) const { return idx > other.idx; }
				bool operator<=(const const_iterator& other) const { return idx <= other.idx; }
				bool operator>=(const const_iterator& other) const { return idx >= other.idx; }
				const auto operator*() const { return accessor[idx]; }
			};

			//template<typename T> OpaqueReflArray(A<T, typename GameInterface::AllocatorSystem>& underlying, Refl refl) : underlying{ static_cast<RflArray<A>&>(underlying) }, refl{ refl } {}
			//template<typename T> OpaqueReflArray(const OpaqueReflArray<A, GameInterface, Refl>& other, Refl refl) : underlying{ other.underlying }, refl{ other.refl } {}
			
			using AccessorBase<Refl, RflArray<A, Refl>>::AccessorBase;

			const_iterator begin() const { return { *this, 0 }; }
			const_iterator cbegin() const { return { *this, 0 }; }
			const_iterator end() const { return { *this, size() }; }
			const_iterator cend() const { return { *this, size() }; }

			const auto operator[](size_t i) const {
				auto item_type = this->refl.get_item_type();

				return ValueAccessor<decltype(item_type)>{ this->reference.at(*this, i), item_type };
			}

			size_t size() const { return this->reference.size(); }
			size_t capacity() const { return this->reference.capacity(); }
		};

		template<typename Refl>
		class ValueAccessor : public AccessorBase<Refl> {
		public:
			using AccessorBase<Refl>::AccessorBase;

			const auto visit(auto f) const {
				return this->refl.visit([&](auto r) {
					if constexpr (decltype(r)::kind == providers::TypeKind::PRIMITIVE) return f(PrimitiveAccessor<decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::ENUM) return f(EnumAccessor<decltype(r)>{ this->reference, r });
					//else if constexpr (decltype(r)::kind == providers::TypeKind::FLAGS) return f(flags(this->reference, refl.is_erased(), r));
					else if constexpr (decltype(r)::kind == providers::TypeKind::ARRAY) return f(ArrayAccessor<ucsl::containers::arrays::Array, decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::TARRAY) return f(ArrayAccessor<ucsl::containers::arrays::TArray, decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::POINTER) return f(PointerAccessor<decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::CARRAY) return f(CArrayAccessor<decltype(r)>{ this->reference, r });
					//else if constexpr (decltype(r)::kind == providers::TypeKind::UNION) return f(union(this->reference, parent, r));
					else if constexpr (decltype(r)::kind == providers::TypeKind::STRUCTURE) return f(StructureAccessor<decltype(r)>{ this->reference, r });
					else static_assert(false, "invalid type kind");
				});
			}

			auto visit(auto f) {
				return this->refl.visit([&](auto r) {
					if constexpr (decltype(r)::kind == providers::TypeKind::PRIMITIVE) return f(PrimitiveAccessor<decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::ENUM) return f(EnumAccessor<decltype(r)>{ this->reference, r });
					//else if constexpr (decltype(r)::kind == providers::TypeKind::FLAGS) return f(flags(this->reference, refl.is_erased(), r));
					else if constexpr (decltype(r)::kind == providers::TypeKind::ARRAY) return f(ArrayAccessor<ucsl::containers::arrays::Array, decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::TARRAY) return f(ArrayAccessor<ucsl::containers::arrays::TArray, decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::POINTER) return f(PointerAccessor<decltype(r)>{ this->reference, r });
					else if constexpr (decltype(r)::kind == providers::TypeKind::CARRAY) return f(CArrayAccessor<decltype(r)>{ this->reference, r });
					//else if constexpr (decltype(r)::kind == providers::TypeKind::UNION) return f(union(this->reference, parent, r));
					else if constexpr (decltype(r)::kind == providers::TypeKind::STRUCTURE) return f(StructureAccessor<decltype(r)>{ this->reference, r });
					else static_assert(false, "invalid type kind");
				});
			}

			auto as_primitive() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::PRIMITIVE) return PrimitiveAccessor<decltype(r)>{ this->reference, r }; else static_assert(false, "not a primitive"); });
			}

			auto as_enum() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::ENUM) return EnumAccessor<decltype(r)>{ this->reference, r }; else static_assert(false, "not an enum"); });
			}

			auto as_array() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::ARRAY) return ArrayAccessor<ucsl::containers::arrays::Array, decltype(r)>{ this->reference, r }; else static_assert(false, "not a array"); });
			}

			auto as_tarray() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::TARRAY) return ArrayAccessor<ucsl::containers::arrays::TArray, decltype(r)>{ this->reference, r }; else static_assert(false, "not a tarray"); });
			}

			auto as_carray() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::CARRAY) return CArrayAccessor<decltype(r)>{ this->reference, r }; else static_assert(false, "not a carray"); });
			}

			auto as_pointer() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::POINTER) return PointerAccessor<decltype(r)>{ this->reference, r }; else static_assert(false, "not a pointer"); });
			}

			auto as_structure() {
				return this->refl.visit([&](auto r) { if constexpr (decltype(r)::kind == providers::TypeKind::STRUCTURE) return StructureAccessor<decltype(r)>{ this->reference, r }; else static_assert(false, "not a structure"); });
			}
		};
	};
}
