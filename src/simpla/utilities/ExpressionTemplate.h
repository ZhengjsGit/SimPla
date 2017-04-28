//
// Created by salmon on 17-4-28.
//

#ifndef SIMPLA_EXPRESSIONTEMPLATE_H
#define SIMPLA_EXPRESSIONTEMPLATE_H

#include <type_traits>
#include <utility>
#include "integer_sequence.h"
#include "type_traits.h"

namespace simpla {

namespace tags {
struct _assign {
    template <typename TL, typename TR>
    void operator()(TL &lhs, TR const &rhs) const {
        lhs = rhs;
    }
};
}
namespace traits {

template <typename...>
struct reference;
template <typename T>
struct reference<T> {
    typedef T type;
};
template <typename T>
using reference_t = typename reference<T>::type;

template <typename T>
struct value_type {
    typedef T type;
};
template <typename T>
using value_type_t = typename value_type<T>::type;

}  // namespace traits{

/**
*  @ingroup calculus
*  @addtogroup expression_template  Expression Template
*  @{
*/
template <typename...>
class Expression;

template <typename...>
struct AssignmentExpression;

namespace traits {

template <typename...>
struct expr_parser;

template <typename TRes, typename TR>
struct expr_parser<TRes, TR> {
    static TRes eval(TR const &expr) { return static_cast<TRes>(expr); };
};
template <typename TRes, typename TOP, typename... Args>
struct expr_parser<TRes, Expression<TOP, Args...>> {
    template <size_type... index>
    static decltype(auto) _invoke_helper(Expression<TOP, Args...> const &expr, std::index_sequence<index...>) {
        return expr.m_op_(expr_parser<TRes, std::remove_cv_t<Args>>::eval(std::get<index>(expr.m_args_))...);
    }

    static TRes eval(Expression<TOP, Args...> const &expr) {
        return _invoke_helper(expr, std::index_sequence_for<Args...>());
    };
};
}

template <typename TOP, typename... Args>
struct Expression<TOP, Args...> {
    typedef Expression<TOP, Args...> this_type;

    typename std::tuple<traits::reference_t<Args>...> m_args_;
    typedef std::true_type is_expression;
    typedef std::false_type prefer_pass_by_reference;
    typedef std::true_type prefer_pass_by_value;

    TOP m_op_;

    Expression(this_type const &that) : m_args_(that.m_args_) {}
    Expression(this_type &&that) noexcept : m_args_(that.m_args_) {}
    explicit Expression(Args &... args) noexcept : m_args_(args...) {}
    virtual ~Expression() = default;

    this_type &operator=(this_type const &) = delete;
    this_type &operator=(this_type &&) = delete;

    template <typename T>
    explicit operator T() const {
        return traits::expr_parser<T, this_type>::eval(*this);
    }
};

template <typename TReduction, typename T>
std::complex<T> reduction(std::complex<T> const &expr) {
    return expr;
}
template <typename TReduction, typename TExpr>
TExpr reduction(TExpr const &expr) {
    return expr;
}

//
// template<typename TOP, typename TL, typename TR>
// struct AssignmentExpression<TOP, TL, TR>
//{
//    typedef AssignmentExpression<TOP, TL, TR> this_type;
//    TL &lhs;
//    typename traits::reference<TR>::value_type_info rhs;
//    TOP op_;
//
//    AssignmentExpression(this_type const &that) : lhs(that.lhs),
//    rhs(that.rhs), op_(that.op_) {}
//
//    AssignmentExpression(this_type &&that) : lhs(that.lhs), rhs(that.rhs),
//    op_(that.op_) {}
//
//    AssignmentExpression(TL &l, TR const &r) : lhs(l), rhs(r), op_() {}
//
//    AssignmentExpression(TOP op, TL &l, TR const &r) : lhs(l), rhs(r), op_(op)
//    {}
//
//    virtual   ~AssignmentExpression() {}
//
//    template<typename IndexType>
//    inline auto operator[](IndexType const &s) const
//    AUTO_RETURN (((op_(traits::GetValue(lhs, s), traits::GetValue(rhs,
//    s)))))
//
//};

#define _SP_DEFINE_EXPR_BINARY_OPERATOR(_NAME_, _OP_)                                                \
    namespace tags {                                                                                 \
    struct _NAME_ {                                                                                  \
        template <typename TL, typename TR>                                                          \
        static constexpr auto eval(TL const &l, TR const &r) {                                       \
            return ((l _OP_ r));                                                                     \
        }                                                                                            \
        template <typename TL, typename TR>                                                          \
        constexpr auto operator()(TL const &l, TR const &r) const {                                  \
            return ((l _OP_ r));                                                                     \
        }                                                                                            \
    };                                                                                               \
    }                                                                                                \
    template <typename... TL, typename TR>                                                           \
    auto operator _OP_(Expression<TL...> const &lhs, TR const &rhs) {                                \
        return Expression<tags::_NAME_, const Expression<TL...>, const TR>(lhs, rhs);                \
    };                                                                                               \
    template <typename TL, typename... TR>                                                           \
    auto operator _OP_(TL const &lhs, Expression<TR...> const &rhs) {                                \
        return Expression<tags::_NAME_, const TL, const Expression<TR...>>(lhs, rhs);                \
    };                                                                                               \
    template <typename... TL, typename... TR>                                                        \
    auto operator _OP_(Expression<TL...> const &lhs, Expression<TR...> const &rhs) {                 \
        return Expression<tags::_NAME_, const Expression<TL...>, const Expression<TR...>>(lhs, rhs); \
    };

#define _SP_DEFINE_EXPR_UNARY_OPERATOR(_NAME_, _OP_)                  \
    namespace tags {                                                  \
    struct _NAME_ {                                                   \
        template <typename TL>                                        \
        static constexpr auto eval(TL const &l) {                     \
            return (_OP_(l));                                         \
        }                                                             \
        template <typename TL>                                        \
        constexpr auto operator()(TL const &l) const {                \
            return _OP_(l);                                           \
        }                                                             \
    };                                                                \
    }                                                                 \
    template <typename... T>                                          \
    auto operator _OP_(Expression<T...> const &lhs) {                 \
        return Expression<tags::_NAME_, const Expression<T...>>(lhs); \
    }

#define _SP_DEFINE_EXPR_BINARY_FUNCTION(_NAME_)                                                      \
    namespace tags {                                                                                 \
    struct _NAME_ {                                                                                  \
        template <typename TL, typename TR>                                                          \
        static constexpr auto eval(TL const &l, TR const &r) {                                       \
            return (_NAME_(l, r));                                                                   \
        }                                                                                            \
        template <typename TL, typename TR>                                                          \
        constexpr auto operator()(TL const &l, TR const &r) const {                                  \
            return (_NAME_(l, r));                                                                   \
        }                                                                                            \
    };                                                                                               \
    }                                                                                                \
    template <typename... TL, typename TR>                                                           \
    auto _NAME_(Expression<TL...> const &lhs, TR const &rhs) {                                       \
        return Expression<tags::_NAME_, const Expression<TL...>, const TR>(lhs, rhs);                \
    };                                                                                               \
    template <typename TL, typename... TR>                                                           \
    auto _NAME_(TL const &lhs, Expression<TR...> const &rhs) {                                       \
        return Expression<tags::_NAME_, const TL, const Expression<TR...>>(lhs, rhs);                \
    };                                                                                               \
    template <typename... TL, typename... TR>                                                        \
    auto _NAME_(Expression<TL...> const &lhs, Expression<TR...> const &rhs) {                        \
        return Expression<tags::_NAME_, const Expression<TL...>, const Expression<TR...>>(lhs, rhs); \
    };

#define _SP_DEFINE_EXPR_UNARY_FUNCTION(_NAME_)                        \
    namespace tags {                                                  \
    struct _NAME_ {                                                   \
        template <typename TL>                                        \
        static constexpr auto eval(TL &l) {                           \
            return (_NAME_(l));                                       \
        }                                                             \
        template <typename TL>                                        \
        constexpr auto operator()(TL &l) const {                      \
            return _NAME_(l);                                         \
        }                                                             \
    };                                                                \
    }                                                                 \
    template <typename... T>                                          \
    auto _NAME_(Expression<T...> const &lhs) {                        \
        return Expression<tags::_NAME_, const Expression<T...>>(lhs); \
    }

_SP_DEFINE_EXPR_BINARY_OPERATOR(addition, +)
_SP_DEFINE_EXPR_BINARY_OPERATOR(subtraction, -)
_SP_DEFINE_EXPR_BINARY_OPERATOR(multiplication, *)
_SP_DEFINE_EXPR_BINARY_OPERATOR(division, /)
_SP_DEFINE_EXPR_BINARY_OPERATOR(modulo, %)

_SP_DEFINE_EXPR_UNARY_OPERATOR(bitwise_not, ~)
_SP_DEFINE_EXPR_BINARY_OPERATOR(bitwise_xor, ^)
_SP_DEFINE_EXPR_BINARY_OPERATOR(bitwise_and, &)
_SP_DEFINE_EXPR_BINARY_OPERATOR(bitwise_or, |)
_SP_DEFINE_EXPR_BINARY_OPERATOR(bitwise_left_shift, <<)
_SP_DEFINE_EXPR_BINARY_OPERATOR(bitwise_right_shifit, >>)

_SP_DEFINE_EXPR_UNARY_OPERATOR(unary_plus, +)
_SP_DEFINE_EXPR_UNARY_OPERATOR(unary_minus, -)

_SP_DEFINE_EXPR_UNARY_OPERATOR(logical_not, !)
_SP_DEFINE_EXPR_BINARY_OPERATOR(logical_and, &&)
_SP_DEFINE_EXPR_BINARY_OPERATOR(logical_or, ||)

_SP_DEFINE_EXPR_UNARY_FUNCTION(cos)
_SP_DEFINE_EXPR_UNARY_FUNCTION(acos)
_SP_DEFINE_EXPR_UNARY_FUNCTION(cosh)
_SP_DEFINE_EXPR_UNARY_FUNCTION(sin)
_SP_DEFINE_EXPR_UNARY_FUNCTION(asin)
_SP_DEFINE_EXPR_UNARY_FUNCTION(sinh)
_SP_DEFINE_EXPR_UNARY_FUNCTION(tan)
_SP_DEFINE_EXPR_UNARY_FUNCTION(tanh)
_SP_DEFINE_EXPR_UNARY_FUNCTION(atan)
_SP_DEFINE_EXPR_UNARY_FUNCTION(exp)
_SP_DEFINE_EXPR_UNARY_FUNCTION(log)
_SP_DEFINE_EXPR_UNARY_FUNCTION(log10)
_SP_DEFINE_EXPR_UNARY_FUNCTION(sqrt)
_SP_DEFINE_EXPR_BINARY_FUNCTION(atan2)
_SP_DEFINE_EXPR_BINARY_FUNCTION(pow)

#undef _SP_DEFINE_EXPR_BINARY_OPERATOR
#undef _SP_DEFINE_EXPR_UNARY_OPERATOR
#undef _SP_DEFINE_EXPR_BINARY_FUNCTION
#undef _SP_DEFINE_EXPR_UNARY_FUNCTION

#define _SP_DEFINE_COMPOUND_OP(_OP_)                              \
    template <typename TL, typename... TR>                        \
    TL &operator _OP_##=(TL &lhs, Expression<TR...> const &rhs) { \
        lhs = lhs _OP_ rhs;                                       \
        return lhs;                                               \
    }

_SP_DEFINE_COMPOUND_OP(+)
_SP_DEFINE_COMPOUND_OP(-)
_SP_DEFINE_COMPOUND_OP(*)
_SP_DEFINE_COMPOUND_OP(/)
_SP_DEFINE_COMPOUND_OP(%)
_SP_DEFINE_COMPOUND_OP(&)
_SP_DEFINE_COMPOUND_OP(|)
_SP_DEFINE_COMPOUND_OP (^)
_SP_DEFINE_COMPOUND_OP(<<)
_SP_DEFINE_COMPOUND_OP(>>)

#undef _SP_DEFINE_COMPOUND_OP

#define _SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(_NAME_, _REDUCTION_, _OP_)                              \
    namespace tags {                                                                                    \
    struct _NAME_ {                                                                                     \
        template <typename TL, typename TR>                                                             \
        static constexpr auto eval(TL const &l, TR &r) {                                                \
            return ((l _OP_ r));                                                                        \
        }                                                                                               \
        template <typename TL, typename TR>                                                             \
        constexpr auto operator()(TL const &l, TR &r) const {                                           \
            return ((l _OP_ r));                                                                        \
        }                                                                                               \
    };                                                                                                  \
    }                                                                                                   \
    template <typename... TL, typename TR>                                                              \
    auto operator _OP_(Expression<TL...> const &lhs, TR &rhs) {                                         \
        return reduction<_REDUCTION_>(Expression<tags::_NAME_, const Expression<TL...>, TR>(lhs, rhs)); \
    };                                                                                                  \
    template <typename TL, typename... TR>                                                              \
    auto operator _OP_(TL &lhs, Expression<TR...> const &rhs) {                                         \
        return reduction<_REDUCTION_>(Expression<tags::_NAME_, TL, const Expression<TR...>>(lhs, rhs)); \
    };                                                                                                  \
    template <typename... TL, typename... TR>                                                           \
    auto operator _OP_(Expression<TL...> const &lhs, Expression<TR...> const &rhs) {                    \
        return reduction<_REDUCTION_>(                                                                  \
            Expression<tags::_NAME_, const Expression<TL...>, const Expression<TR...>>(lhs, rhs));      \
    };

_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(not_equal_to, tags::logical_or, !=)
_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(equal_to, tags::logical_and, ==)
_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(less, tags::logical_and, <)
_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(greater, tags::logical_and, >)
_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(less_equal, tags::logical_and, <=)
_SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR(greater_equal, tags::logical_and, >=)

//_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(real)                                          \
//_SP_DEFINE_##_CONCEPT_##_EXPR_UNARY_FUNCTION(imag)                                          \
//
// template <typename T1>
// auto operator<<(T1 const &l, unsigned int r) {
//    return ((Expression<tags::shift_left, const T1, unsigned int>(l, r)));
//}
// template <typename T1>
// auto operator>>(T1 const &l, unsigned int r) {
//    return ((Expression<tags::shift_right, const T1, unsigned int>(l, r)));
//}

#undef _SP_DEFINE_EXPR_BINARY_RIGHT_OPERATOR
#undef _SP_DEFINE_EXPR_BINARY_BOOLEAN_OPERATOR
#undef _SP_DEFINE_EXPR_UNARY_BOOLEAN_OPERATOR

template <typename TL, typename TR>
auto inner_product(TL const &l, TR const &r) {
    return reduction<tags::addition>(l * r);
}

}  // namespace simpla

#endif  // SIMPLA_EXPRESSIONTEMPLATE_H
