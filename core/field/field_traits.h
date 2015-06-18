/**
 * @file field_traits.h
 *
 * @date 2015年6月12日
 * @author salmon
 */

#ifndef CORE_FIELD_FIELD_TRAITS_H_
#define CORE_FIELD_FIELD_TRAITS_H_

#include <stddef.h>
#include <cstdbool>
#include <memory>
#include <type_traits>

#include "../gtl/integer_sequence.h"
#include "../gtl/type_traits.h"
#include "../mesh/domain.h"
#include "../mesh/mesh_ids.h"

namespace simpla
{

template<typename ...> struct Domain;
template<typename ... >struct _Field;

namespace tags
{

class sequence_container;

class associative_container;

class function;

}  // namespace tags
namespace traits
{

template<typename TM, size_t IFORM, typename ValueType, typename ...Policies>
struct field_type
{
	typedef _Field<
			Domain<TM, std::integral_constant<size_t, IFORM>, Policies...>,
			ValueType, tags::sequence_container> type;
};

template<typename TM, size_t IFORM, typename ValueType, typename ...Policies>
using field_t= typename field_type<TM,IFORM,ValueType,Policies...>::type;

template<typename > struct is_field: public std::integral_constant<bool, false>
{
};

template<typename ...T> struct is_field<_Field<T...>> : public std::integral_constant<
bool, true>
{
};

template<typename TM, typename TV, typename ...Others>
struct reference<_Field<TM, TV, Others...> >
{
	typedef _Field<TM, TV, Others...> const & type;
};

template<typename ...T, size_t M>
struct extent<_Field<T ...>, M> : public std::integral_constant<size_t,
		simpla::_impl::seq_get<M, extents_t<_Field<T ...> >>::value>
{
};

template<typename ...T>
struct key_type<_Field<T ...> >
{
	typedef size_t type;
};

namespace _impl
{

template<typename ...> struct field_traits;

template<typename T> struct field_traits<T>
{

	typedef std::integral_constant<size_t, 1> domain_type;

	typedef T value_type;

	static constexpr bool is_field = false;

	static constexpr size_t iform = 0;

	static constexpr size_t ndims = 3;

};

template<typename ...T>
struct field_traits<_Field<T ...>>
{
	static constexpr bool is_field = true;

	typedef typename _Field<T ...>::domain_type domain_type;

	typedef typename _Field<T ...>::value_type value_type;

	static constexpr size_t iform = traits::iform<domain_type>::value;

	static constexpr size_t ndims = traits::rank<domain_type>::value;

};

}  // namespace _impl

template<typename ... T>
struct value_type<_Field<T ...>>
{
	typedef typename _impl::field_traits<_Field<T ...> >::value_type type;
};

template<typename T> struct domain_type;

template<typename ...T> using domain_t= typename domain_type<T...>::type;

template<typename > struct mesh_type;

template<typename ...T> using mesh_t= typename mesh_type<T...>::type;

template<typename T>
struct iform: public std::integral_constant<size_t, 0>
{
};

template<typename ...T>
struct iform<_Field<T...>> : public std::integral_constant<size_t,
		iform<typename domain_type<_Field<T...> >::type>::value>
{
};

template<typename ...T>
struct rank<_Field<T...> >

: public std::integral_constant<size_t,
		rank<typename domain_type<_Field<T...>>::type>::value>
{
};

template<typename > struct field_value_type;

template<typename T>
struct field_value_type
{
	typedef typename std::conditional<
			(iform<T>::value == VERTEX || iform<T>::value == VOLUME),
			value_type_t<T>, nTuple<value_type_t<T>, 3> >::type type;
};

template<typename T> using field_value_t = typename field_value_type<T>::type;

template<typename T>
struct container_tag
{
	typedef std::nullptr_t type;
};

namespace _impl
{

template<typename T> using container_tag_t=typename container_tag<T>::type;

template<typename TV, typename TAG> struct container_type_helper;

template<typename TV>
struct container_type_helper<TV, tags::sequence_container>
{
	typedef std::shared_ptr<TV> type;
};
}  // namespace _impl

template<typename > struct container_type;

template<typename T> struct container_type
{
	typedef typename _impl::container_type_helper<traits::value_type_t<T>,
			_impl::container_tag_t<T> >::type type;
};

template<typename T> using container_t=typename container_type<T>::type;

}  // namespace traits
}  // namespace simpla

#endif /* CORE_FIELD_FIELD_TRAITS_H_ */
