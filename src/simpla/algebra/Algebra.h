//
// Created by salmon on 16-12-22.
//

#ifndef SIMPLA_ALGEBRACOMMON_H
#define SIMPLA_ALGEBRACOMMON_H

#include <simpla/SIMPLA_config.h>
#include <simpla/mpl/integer_sequence.h>
#include <simpla/mpl/type_traits.h>
#include <utility>

namespace simpla {
enum { VERTEX = 0, EDGE = 1, FACE = 2, VOLUME = 3, FIBER = 6 };

namespace algebra {

namespace declare {
template <typename...>
struct Expression;
}
namespace calculus {
template <typename, class Enable = void>
struct calculator {};
}
namespace traits {
template <typename>
struct num_of_dimension : public index_const<3> {};

template <typename T>
struct value_type {
    typedef T type;
};

template <typename T>
using value_type_t = typename value_type<T>::type;

template <typename T>
struct value_type<T&> {
    typedef T& type;
};
template <typename T>
struct value_type<T const&> {
    typedef T const& type;
};

template <typename T>
struct value_type<T*> {
    typedef T type;
};
template <typename T, int N>
struct value_type<T[N]> {
    typedef T type;
};
template <typename T>
struct value_type<T const*> {
    typedef T type;
};
template <typename T>
struct value_type<T const[]> {
    typedef T type;
};

template <typename T>
struct scalar_type {
    typedef Real type;
};

template <typename T>
using scalar_type_t = typename scalar_type<T>::type;

template <typename...>
struct is_complex : public std::integral_constant<bool, false> {};

template <typename T>
struct is_complex<std::complex<T>> : public std::integral_constant<bool, true> {};

template <typename...>
struct is_scalar : public std::integral_constant<bool, false> {};

template <typename T>
struct is_scalar<T>
    : public std::integral_constant<bool, std::is_arithmetic<std::decay_t<T>>::value ||
                                              is_complex<std::decay_t<T>>::value> {};
template <typename First, typename... Others>
struct is_scalar<First, Others...>
    : public std::integral_constant<bool, is_scalar<First>::value && is_scalar<Others...>::value> {
};

template <typename...>
struct is_field;

template <typename T>
struct is_field<T> : public std::integral_constant<bool, false> {};
template <typename First, typename... Others>
struct is_field<First, Others...>
    : public std::integral_constant<bool, is_field<First>::value || is_field<Others...>::value> {};
template <typename...>
struct is_array;

template <typename T>
struct is_array<T> : public std::integral_constant<bool, false> {};

template <typename First, typename... Others>
struct is_array<First, Others...>
    : public std::integral_constant<bool, (is_array<First>::value && !is_field<First>::value) ||
                                              is_array<Others...>::value> {};

template <typename...>
struct is_nTuple;

template <typename T>
struct is_nTuple<T> : public std::integral_constant<bool, false> {};

template <typename First, typename... Others>
struct is_nTuple<First, Others...>
    : public std::integral_constant<bool,
                                    (is_nTuple<First>::value &&
                                     !(is_field<Others...>::value || is_array<Others...>::value)) ||
                                        is_nTuple<Others...>::value> {};
template <typename...>
struct is_expression;

template <typename T>
struct is_expression<T> : public std::integral_constant<bool, false> {};

template <typename... T>
struct is_expression<declare::Expression<T...>> : public std::integral_constant<bool, true> {};

template <typename T>
struct reference {
    typedef T type;
};
template <typename T>
using reference_t = typename reference<T>::type;
template <typename T, int N>
struct reference<T[N]> {
    typedef T* type;
};
template <typename T, int N>
struct reference<const T[N]> {
    typedef T const* type;
};

//***********************************************************************************************************************

template <typename>
struct iform : public index_const<VERTEX> {};
template <typename T>
struct iform<const T> : public iform<T> {};

template <typename>
struct dof : public index_const<1> {};
template <typename T>
struct dof<const T> : public dof<T> {};

template <typename>
struct rank : public index_const<3> {};
template <typename T>
struct rank<const T> : public rank<T> {};

template <typename>
struct extent : public index_const<0> {};
template <typename T>
struct extent<const T> : public index_const<extent<T>::value> {};
template <typename T>
struct extents : public index_sequence<> {};



}  // namespace traits

}  // namespace algebra

// template <typename T, int... N>
// using nTuple = algebra::declare::nTuple_<T, N...>;
//
// template <typename T, int N>
// using Vector = algebra::declare::nTuple_<T, N>;
//
// template <typename T, int M, int N>
// using Matrix = algebra::declare::nTuple_<T, M, N>;
//
// template <typename T, int... N>
// using Tensor = algebra::declare::nTuple_<T, N...>;
//
// template <typename T, int N, bool is_slow_first = true>
// using Array = algebra::declare::Array_<T, N, is_slow_first>;

}  // namespace simpla
#endif  // SIMPLA_ALGEBRACOMMON_H
