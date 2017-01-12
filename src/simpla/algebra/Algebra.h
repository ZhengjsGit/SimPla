//
// Created by salmon on 16-12-22.
//

#ifndef SIMPLA_ALGEBRACOMMON_H
#define SIMPLA_ALGEBRACOMMON_H

#include <simpla/SIMPLA_config.h>
#include <simpla/concept/CheckConcept.h>
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

namespace traits {
template <typename>
struct num_of_dimension : public index_const<3> {};

CHECK_MEMBER_TYPE(value_type, value_type)

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
struct reference {
   private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template <typename U>
    static auto test(int) -> typename U::prefer_pass_by_reference;
    template <typename>
    static no test(...);

   public:
    typedef std::conditional_t<std::is_same<decltype(test<T>(0)), no>::value, T&, T> type;
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

//**************************************************************************************************

template <typename T>
struct field_value_type {
   private:
    typedef std::true_type yes;
    typedef std::false_type no;

    template <typename U>
    static auto test(int) -> typename U::field_value_type;
    template <typename>
    static no test(...);

   public:
    typedef std::conditional_t<std::is_same<decltype(test<T>(0)), no>::value, value_type_t<T>,
                               field_value_type>
        type;
};

template <typename T>
using field_value_t = typename field_value_type<T>::type;

HAS_MEMBER_TYPE_BOOLEAN(is_array, is_array)
HAS_MEMBER_TYPE_BOOLEAN(is_field, is_field)
HAS_MEMBER_TYPE_BOOLEAN(is_nTuple, is_nTuple)
HAS_MEMBER_TYPE_BOOLEAN(is_expression, is_expression)

template <typename First, typename... Others>
struct is_field<First, Others...>
    : public std::integral_constant<bool, is_field<First>::value || is_field<Others...>::value> {};

template <typename First, typename... Others>
struct is_array<First, Others...>
    : public std::integral_constant<bool, (is_array<First>::value && !is_field<First>::value) ||
                                              is_array<Others...>::value> {};

template <typename First, typename... Others>
struct is_nTuple<First, Others...>
    : public std::integral_constant<bool,
                                    (is_nTuple<First>::value &&
                                     !(is_field<Others...>::value || is_array<Others...>::value)) ||
                                        is_nTuple<Others...>::value> {};

template <typename T>
struct is_primary_field
    : public std::integral_constant<bool, is_field<T>::value && (!is_expression<T>::value)> {};

template <typename _T>
struct iform_ {
   private:
    template <typename U>
    static auto test(int) -> std::integral_constant<int, U::iform>;
    template <typename>
    static std::integral_constant<int, 0> test(...);

   public:
    static constexpr int value = decltype(test<_T>(0))::value;
};
template <typename T>
struct iform : public int_constant<iform_<T>::value> {};

template <typename _T>
struct dof_ {
   private:
    template <typename U>
    static auto test(int) -> std::integral_constant<int, U::dof>;
    template <typename>
    static std::integral_constant<int, 1> test(...);

   public:
    static constexpr int value = decltype(test<_T>(0))::value;
};
template <typename T>
struct dof : public int_constant<dof_<T>::value> {};
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
