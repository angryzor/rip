#include <simple-reflection/simple-reflection.h>
#include <rip/util/memory.h>
#include "types.h"

namespace rip::reflection {
	using namespace simplerfl;

	template<typename Algorithm, typename = std::make_index_sequence<Algorithm::arity>>
	class simplerfl_traversal;
	template<typename Algorithm, std::size_t... S>
	class simplerfl_traversal<Algorithm, std::index_sequence<S...>> {
		template<std::size_t, typename T = void> using Obj = T*;

		Algorithm algorithm;

		template<typename Fields, typename I = std::make_index_sequence<std::tuple_size_v<Fields>>>
		struct FieldIterator;
		template<typename Fields, size_t... Is>
		struct FieldIterator<Fields, std::index_sequence<Is...>> {
			simplerfl_traversal<Algorithm, std::index_sequence<S...>>& t;

			typename Algorithm::result_type operator()(Obj<S>... objs, size_t& offset) {
				typename Algorithm::result_type result{};

				((
					offset = align(offset, align_of_v<typename std::tuple_element_t<Is, Fields>::type>),
					result |= t.process_field<std::tuple_element_t<Is, Fields>>(addptr(objs, offset)...),
					offset += size_of_v<typename std::tuple_element_t<Is, Fields>::type>
				), ...);

				return result;
			}
		};

		template<typename T>
		typename Algorithm::result_type process_primitive(Obj<S, T>... objs) {
			return algorithm.visit_primitive(objs..., PrimitiveInfo{ false });
		}

		template<typename T>
		typename Algorithm::result_type process_primitive(Obj<S, csl::ut::MoveArray<T>>... objs) {
			return algorithm.visit_array(
				static_cast<csl::ut::MoveArray<representation_t<T>>*>(objs)...,
				[]() { return new representation_t<T>{}; },
				[](representation_t<T>* obj) { delete obj; },
				[=](Obj<S>... objs) { return process_type<T>(objs...); }
			);
		}

		template<typename T>
		typename Algorithm::result_type process_primitive(Obj<S, csl::ut::MoveArray32<T>>... objs) {
			return algorithm.visit_array(
				static_cast<csl::ut::MoveArray32<representation_t<T>>*>(objs)...,
				[]() { return new representation_t<T>{}; },
				[](representation_t<T>* obj) { delete obj; },
				[=](Obj<S>... objs) { return process_type<T>(objs...); }
			);
		}

		template<typename T>
		typename Algorithm::result_type process_field_data(Obj<S>... objs) {
			return algorithm.visit_field_data(objs..., FieldDataInfo{ align_of_v<T>, size_of_v<T> }, [=](Obj<S>...objs) { return process_primitive(static_cast<T::type*>(objs)...); });
		}

		template<typename T>
		typename Algorithm::result_type process_type(Obj<S>... objs) {
			if constexpr (T::desc_type == DescType::STRUCTURE)
				return process_struct<T>(objs...);
			else if constexpr (T::desc_type == DescType::PRIMITIVE)
				return process_field_data<T>(objs...);
			else
				static_assert("unknown desc type");
		}

		template<typename T>
		typename Algorithm::result_type process_field(Obj<S>... objs) {
			using Type = typename T::type;

			return algorithm.visit_field(align(objs, align_of_v<Type>)..., FieldInfo{ T::name }, [=](Obj<S>... objs) { return process_type<desugar_t<Type>>(objs...); });
		}

		template<typename T>
		typename Algorithm::result_type process_fields(Obj<S>... objs) {
			using Base = typename T::base;
			using Fields = typename T::fields;

			typename Algorithm::result_type result{};
			size_t offset{};

			if constexpr (!std::is_same_v<Base, primitive<void>>) {
				result |= process_base_struct<Base>(objs...);
				offset = size_of_v<Base>;
			}

			result |= FieldIterator<Fields>{ *this }(objs..., offset);

			return result;
		}

		template<typename T>
		typename Algorithm::result_type process_base_struct(Obj<S>... objs) {
			return algorithm.visit_base_struct(objs..., StructureInfo{ T::name, align_of_v<T> }, [=](Obj<S>... objs) { return process_fields<T>(objs...); });
		}

		template<typename T>
		typename Algorithm::result_type process_struct(Obj<S>... objs) {
			return algorithm.visit_struct(objs..., StructureInfo{ T::name, align_of_v<T> }, [=](Obj<S>... objs) { return process_fields<T>(objs...); });
		}

	public:
		template<typename ...Args>
		simplerfl_traversal(Args&& ...objs) : algorithm{ std::forward<Args>(objs)... } {}

		template<typename T>
		typename Algorithm::result_type operator()(Obj<S>... objs) {
			return process_struct<T>(objs...);
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

	//template<typename T>
	//struct predefined
	//{
	//	template<typename Algorithm>
	//	class traversal : public simplerfl_traversal<Algorithm> {
	//	public:

	//	};
	//};
}
