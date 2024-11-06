#pragma once
#include <simple-reflection/simple-reflection.h>
#include <rip/util/memory.h>
#include "types.h"

namespace rip::reflection {
	using namespace simplerfl;

	template<typename Algorithm, typename = std::make_index_sequence<Algorithm::arity>>
	class simplerfl_traversal;
	template<typename Algorithm, std::size_t... S>
	class simplerfl_traversal<Algorithm, std::index_sequence<S...>> {
		template<std::size_t, typename T> using Spread = T;
		template<std::size_t, typename T = void> using Obj = T*;

		Algorithm algorithm;

		std::tuple<Obj<S>...> parents{};

		template<typename Fields, typename I = std::make_index_sequence<std::tuple_size_v<Fields>>>
		struct FieldIterator;
		template<typename Fields, size_t... Is>
		struct FieldIterator<Fields, std::index_sequence<Is...>> {
			simplerfl_traversal<Algorithm, std::index_sequence<S...>>& t;

			typename Algorithm::result_type operator()(Obj<S>... objs, Obj<S>... parents, size_t& offset) {
				typename Algorithm::result_type result{};

				((
					offset = align(offset, align_of_v<typename std::tuple_element_t<Is, Fields>::type>),
					result |= t.process_field<std::tuple_element_t<Is, Fields>>(addptr(objs, offset)..., parents...),
					offset += size_of_v<typename std::tuple_element_t<Is, Fields>::type>
				), ...);

				return result;
			}
		};

		template<typename T>
		typename Algorithm::result_type process_primitive_repr(Obj<S, T>... objs, const PrimitiveReprInfo& info) {
			return algorithm.visit_primitive_repr(objs..., info);
		}

		//template<typename T>
		//typename Algorithm::result_type process_primitive(Obj<S, ucsl::containers::arrays::Array<T>>... objs, const PrimitiveInfo& info) {
		//	return algorithm.visit_array(
		//		static_cast<ucsl::containers::arrays::Array<representation_t<T>>*>(objs)...,
		//		[]() { return new representation_t<T>{}; },
		//		[](representation_t<T>* obj) { delete obj; },
		//		[=](Obj<S>... objs) { return process_type<T>(objs...); }
		//	);
		//}

		//template<typename T>
		//typename Algorithm::result_type process_primitive(Obj<S, csl::ut::MoveArray32<T>>... objs, const PrimitiveInfo& info) {
		//	return algorithm.visit_array(
		//		static_cast<csl::ut::MoveArray32<representation_t<T>>*>(objs)...,
		//		[]() { return new representation_t<T>{}; },
		//		[](representation_t<T>* obj) { delete obj; },
		//		[=](Obj<S>... objs) { return process_type<T>(objs...); }
		//	);
		//}

		template<typename T>
		typename Algorithm::result_type process_primitive(Obj<S>... objs) {
			static_assert(desugar_t<T>::desc_type == DescType::PRIMITIVE);

			using Repr = typename desugar_t<T>::repr;

			return algorithm.visit_primitive(objs..., PrimitiveInfo{ align_of_v<T>, size_of_v<T> }, [=](Obj<S>...objs) { return process_primitive_repr(static_cast<Repr*>(objs)..., PrimitiveReprInfo{ .erased = stf::reflection::is_erased_v<T> }); });
		}

		template<typename T>
		typename Algorithm::result_type process_pointer(Obj<S>... objs, Obj<S>... parents) {
			static_assert(desugar_t<T>::desc_type == DescType::POINTER);

			using Target = typename desugar_t<T>::target;

			return algorithm.visit_pointer((Obj<S>*)objs..., PointerInfo{ align_of_v<Target>, simplerfl::dynamic_size_of<Target>(parents, objs) }..., [=](Obj<S>...objs) { return process_type<Target>(objs..., parents...); });
		}

		template<typename T>
		typename Algorithm::result_type process_dynamic_carray(Obj<S>... objs, Obj<S>... parents) {
			static_assert(desugar_t<T>::desc_type == DescType::DYNAMIC_CARRAY);

			using Parent = typename desugar_t<T>::parent;
			using InnerType = typename desugar_t<T>::type;

			return algorithm.visit_carray(objs..., CArrayInfo{ desugar_t<T>::resolver(*(Parent*)parents), size_of_v<InnerType> }..., [=](Obj<S>...objs) { return process_type<InnerType>(objs..., parents...); });
		}

		template<typename T>
		typename Algorithm::result_type process_static_carray(Obj<S>... objs, Obj<S>... parents) {
			static_assert(desugar_t<T>::desc_type == DescType::STATIC_CARRAY);

			using InnerType = typename desugar_t<T>::type;

			return algorithm.visit_carray(objs..., (objs, CArrayInfo{ desugar<T>::size, size_of_v<InnerType> })..., [=](Obj<S>...objs) { return process_type<InnerType>(objs..., parents...); });
		}

		template<typename T>
		typename Algorithm::result_type process_type(Obj<S>... objs, Obj<S>... parents) {
			if constexpr (desugar_t<T>::desc_type == DescType::STRUCTURE)
				return process_struct<T>(objs...);
			else if constexpr (desugar_t<T>::desc_type == DescType::PRIMITIVE)
				return process_primitive<T>(objs...);
			else if constexpr (desugar_t<T>::desc_type == DescType::POINTER)
				return process_pointer<T>(objs..., parents...);
			else if constexpr (desugar_t<T>::desc_type == DescType::DYNAMIC_CARRAY)
				return process_dynamic_carray<T>(objs..., parents...);
			else if constexpr (desugar_t<T>::desc_type == DescType::STATIC_CARRAY)
				return process_static_carray<T>(objs..., parents...);
			else
				static_assert("unknown desc type");
		}

		template<typename T>
		typename Algorithm::result_type process_field(Obj<S>... objs, Obj<S>... parents) {
			using Type = typename T::type;

			return algorithm.visit_field(align(objs, align_of_v<Type>)..., FieldInfo{ T::name }, [=](Obj<S>... objs) { return process_type<Type>(objs..., parents...); });
		}

		template<typename T>
		typename Algorithm::result_type process_fields(Obj<S>... objs, Obj<S>... parents) {
			using Base = typename T::base;
			using Fields = typename T::fields;

			typename Algorithm::result_type result{};
			size_t offset{};

			if constexpr (!std::is_same_v<Base, primitive<void>>) {
				result |= process_base_struct<Base>(objs..., parents...);
				offset = size_of_v<Base>;
			}

			result |= FieldIterator<Fields>{ *this }(objs..., parents..., offset);

			return result;
		}

		template<typename T>
		typename Algorithm::result_type process_base_struct(Obj<S>... objs, Obj<S>... parents) {
			return algorithm.visit_base_struct(objs..., StructureInfo{ T::name, align_of_v<T> }, [=](Obj<S>... objs) { return process_fields<T>(objs..., parents...); });
		}

		template<typename T>
		typename Algorithm::result_type process_struct(Obj<S>... objs) {
			static_assert(desugar_t<T>::desc_type == DescType::STRUCTURE);
			return algorithm.visit_struct(objs..., StructureInfo{ T::name, align_of_v<T> }, [=](Obj<S>... objs) { return process_fields<T>(objs..., objs...); });
		}

		template<typename T>
		typename Algorithm::result_type process_root(Obj<S>... objs) {
			return algorithm.visit_root(objs..., RootInfo{ align_of_v<T>, size_of_v<T> }, [=](Obj<S>...objs) { return process_type<T>(objs..., objs...); });
		}

	public:
		template<typename ...Args>
		simplerfl_traversal(Args&& ...objs) : algorithm{ std::forward<Args>(objs)... } {}

		template<typename T>
		typename Algorithm::result_type operator()(Obj<S>... objs) {
			return process_root<T>(objs...);
		}

		template<typename T>
		typename Algorithm::result_type operator()(Obj<S, T>... objs) {
			return process_root<simplerfl::canonical_t<T>>(((Obj<S>)objs)...);
		}

		template<typename T, typename ...Args>
		static typename Algorithm::result_type apply(Obj<S>... objs, Args&& ...args) {
			simplerfl_traversal<Algorithm> t{ std::forward<Args>(args)... };

			return t.operator()<T>(objs...);
		}

		//template<typename T, typename R, typename ...Args>
		//static typename Algorithm::result_type apply(Obj<S, R*>... objs, Args&& ...args) {
		//	return traversal<Algorithm>::apply(((void*)objs)..., std::forward<Args>(args)...);
		//}
	};
}
