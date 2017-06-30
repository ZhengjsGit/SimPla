/**
 * @file field.h
 * @author salmon
 * @date 2015-10-16.
 */

#ifndef SIMPLA_FIELD_H
#define SIMPLA_FIELD_H

#include <simpla/SIMPLA_config.h>
#include <simpla/utilities/ExpressionTemplate.h>
#include <simpla/utilities/Range.h>
#include <simpla/utilities/SPObject.h>
#include <simpla/utilities/nTuple.h>
#include "Algebra.h"
#include "CalculusPolicy.h"
namespace simpla {

template <typename>
class calculator;
template <typename TM, typename TV, int...>
class Field;

template <typename TM, typename TV, int IFORM, int... DOF>
class Field<TM, TV, IFORM, DOF...> : public SPObject {
   private:
    typedef Field<TM, TV, IFORM, DOF...> field_type;
    SP_OBJECT_HEAD(field_type, SPObject);

   public:
    typedef TV value_type;
    typedef TM mesh_type;
    static constexpr int iform = IFORM;
    static constexpr int NUM_OF_SUB = (IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3;
    typedef std::conditional_t<sizeof...(DOF) == 0, value_type, nTuple<value_type, DOF...>> element_type;
    typedef std::conditional_t<(IFORM == VERTEX || IFORM == VOLUME), element_type, nTuple<element_type, 3>>
        field_value_type;
    typedef typename mesh_type::entity_id entity_id;

   private:
    typedef typename mesh_type::template array_type<value_type> array_type;
    nTuple<array_type, NUM_OF_SUB, DOF...> m_data_;
    EntityRange m_range_;
    mesh_type* m_mesh_ = nullptr;

   public:
    template <typename... Args>
    explicit Field(mesh_type* m, Args&&... args) : SPObject(m, std::forward<Args>(args)...), m_mesh_(m) {}

    Field(this_type const& other)
        : SPObject(other), m_data_(other.m_data_), m_mesh_(other.m_mesh_), m_range_(other.m_range_) {}
    Field(this_type&& other)
        : SPObject(other), m_data_(other.m_data_), m_mesh_(other.m_mesh_), m_range_(other.m_range_) {}

    Field(this_type const& other, EntityRange r)
        : SPObject(other), m_data_(other.m_data_), m_mesh_(other.m_mesh_), m_range_(std::move(r)) {}
    ~Field() = default;

    size_type size() const { return m_range_.size(); }

    void Clear() { Fill(0); }
    void SetUndefined() { Fill(std::numeric_limits<value_type>::signaling_NaN()); }
    void Fill(value_type v) { m_data_ = v; }

    this_type& operator=(this_type const& other) {
        Assign(other);
        return *this;
    }

    void swap(this_type& other) {
        SPObject::swap(other);
        m_data_.swap(other.m_data_);
        m_range_.swap(other.m_range_);
        std::swap(m_mesh_, other.m_mesh_);
    }

    template <typename TR>
    this_type& operator=(TR const& rhs) {
        Assign(rhs);
        return *this;
    };
    auto& operator[](int n) { return m_data_[n]; }
    auto const& operator[](int n) const { return m_data_[n]; }

    //*****************************************************************************************************************
    this_type operator()(EntityRange const& d) const { return this_type(*this, d); }

    typedef calculator<mesh_type> calculus_policy;

    value_type const& at(entity_id s) const { return calculus_policy::getValue(*this, *m_mesh_, s); }

    value_type& at(entity_id s) { return calculus_policy::getValue(*this, *m_mesh_, s); }

    value_type const& operator()(entity_id s) const { return at(s); }

    value_type& operator()(entity_id s) { return at(s); }

    template <typename... Args>
    auto gather(Args&&... args) const {
        return calculus_policy::gather(*m_mesh_, *this, std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto scatter(Args&&... args) {
        return calculus_policy::scatter(*m_mesh_, *this, std::forward<Args>(args)...);
    }

    void Update() override { m_mesh_->UpdateArray(m_data_); }

    //    void TearDown() override {
    //        m_range_.reset();
    //        m_mesh_ = nullptr;
    //    }
    //
    //    void Push(const std::shared_ptr<data::DataBlock>& d, const EntityRange& r) override {
    //        if (d != nullptr) {
    //            m_range_ = r;
    //            for (int i = 0; i < NUM_OF_SUB; ++i) { m_data_[i].reset(); };
    //
    //            m_data_ = std::dynamic_pointer_cast<array_type>(d);
    //            Click();
    //        }
    //        DoUpdate();
    //    }
    //
    //    std::shared_ptr<data::DataBlock> Pop() override {
    //        auto res = std::dynamic_pointer_cast<data::DataMultiArray<value_type, NDIMS>>();
    //        DoTearDown();
    //        return res;
    //    }
    template <int IR, int... DR>
    void DeepCopy(Field<TM, TV, IR, DR...> const& other) {
        m_data_ = other.m_data_;
    }

    template <typename Other>
    void Assign(Other const& other) {
        DoUpdate();
        ASSERT(m_mesh_ != nullptr);
        //        m_mesh_->Assign(*this, m_range_, other);
        //        m_mesh_->Assign(*this, m_mesh_->GetRange(std::string(EntityIFORMName[IFORM]) + "_PATCH_BOUNDARY"), 0);
    }

};  // class Field
namespace traits {

template <typename TM, typename TV, int IFORM, int... DOF>
struct reference<Field<TM, TV, IFORM, DOF...>> {
    typedef Field<TM, TV, IFORM, DOF...> const& type;
};
}
template <typename TM, typename TL, int... NL>
auto operator<<(Field<TM, TL, NL...> const& lhs, int n) {
    return Expression<tags::bitwise_left_shift, Field<TM, TL, NL...>, int>(lhs, n);
};

template <typename TM, typename TL, int... NL>
auto operator>>(Field<TM, TL, NL...> const& lhs, int n) {
    return Expression<tags::bitwise_right_shifit, Field<TM, TL, NL...>, int>(lhs, n);
};

#define _SP_DEFINE_FIELD_BINARY_FUNCTION(_TAG_, _FUN_)                                        \
    template <typename TM, typename TL, int... NL, typename TR>                               \
    auto _FUN_(Field<TM, TL, NL...> const& lhs, TR const& rhs) {                              \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>, TR>(lhs, rhs);                   \
    };                                                                                        \
    template <typename TL, typename TM, typename TR, int... NR>                               \
    auto _FUN_(TL const& lhs, Field<TM, TR, NR...> const& rhs) {                              \
        return Expression<tags::_TAG_, TL, Field<TM, TR, NR...>>(lhs, rhs);                   \
    };                                                                                        \
    template <typename TM, typename TL, int... NL, typename... TR>                            \
    auto _FUN_(Field<TM, TL, NL...> const& lhs, Expression<TR...> const& rhs) {               \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>, Expression<TR...>>(lhs, rhs);    \
    };                                                                                        \
    template <typename... TL, typename TM, typename TR, int... NR>                            \
    auto _FUN_(Expression<TL...> const& lhs, Field<TM, TR, NR...> const& rhs) {               \
        return Expression<tags::_TAG_, Expression<TL...>, Field<TM, TR, NR...>>(lhs, rhs);    \
    };                                                                                        \
    template <typename ML, typename TL, int... NL, typename MR, typename TR, int... NR>       \
    auto _FUN_(Field<ML, TL, NL...> const& lhs, Field<MR, TR, NR...> const& rhs) {            \
        return Expression<tags::_TAG_, Field<ML, TL, NL...>, Field<MR, TR, NR...>>(lhs, rhs); \
    };

#define _SP_DEFINE_FIELD_UNARY_FUNCTION(_TAG_, _FUN_)              \
    template <typename TM, typename TL, int... NL>                 \
    auto _FUN_(Field<TM, TL, NL...> const& lhs) {                  \
        return Expression<tags::_TAG_, Field<TM, TL, NL...>>(lhs); \
    }

_SP_DEFINE_FIELD_UNARY_FUNCTION(cos, cos)
_SP_DEFINE_FIELD_UNARY_FUNCTION(acos, acos)
_SP_DEFINE_FIELD_UNARY_FUNCTION(cosh, cosh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sin, sin)
_SP_DEFINE_FIELD_UNARY_FUNCTION(asin, asin)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sinh, sinh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(tan, tan)
_SP_DEFINE_FIELD_UNARY_FUNCTION(tanh, tanh)
_SP_DEFINE_FIELD_UNARY_FUNCTION(atan, atan)
_SP_DEFINE_FIELD_UNARY_FUNCTION(exp, exp)
_SP_DEFINE_FIELD_UNARY_FUNCTION(log, log)
_SP_DEFINE_FIELD_UNARY_FUNCTION(log10, log10)
_SP_DEFINE_FIELD_UNARY_FUNCTION(sqrt, sqrt)
_SP_DEFINE_FIELD_BINARY_FUNCTION(atan2, atan2)
_SP_DEFINE_FIELD_BINARY_FUNCTION(pow, pow)

_SP_DEFINE_FIELD_BINARY_FUNCTION(addition, operator+)
_SP_DEFINE_FIELD_BINARY_FUNCTION(subtraction, operator-)
_SP_DEFINE_FIELD_BINARY_FUNCTION(multiplication, operator*)
_SP_DEFINE_FIELD_BINARY_FUNCTION(division, operator/)
_SP_DEFINE_FIELD_BINARY_FUNCTION(modulo, operator%)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_xor, operator^)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_and, operator&)
_SP_DEFINE_FIELD_BINARY_FUNCTION(bitwise_or, operator|)
_SP_DEFINE_FIELD_BINARY_FUNCTION(logical_and, operator&&)
_SP_DEFINE_FIELD_BINARY_FUNCTION(logical_or, operator||)

_SP_DEFINE_FIELD_UNARY_FUNCTION(bitwise_not, operator~)
_SP_DEFINE_FIELD_UNARY_FUNCTION(unary_plus, operator+)
_SP_DEFINE_FIELD_UNARY_FUNCTION(unary_minus, operator-)
_SP_DEFINE_FIELD_UNARY_FUNCTION(logical_not, operator!)

#undef _SP_DEFINE_FIELD_BINARY_FUNCTION
#undef _SP_DEFINE_FIELD_UNARY_FUNCTION

#define _SP_DEFINE_FIELD_COMPOUND_OP(_OP_)                                                            \
    template <typename TM, typename TL, int... NL, typename TR>                                       \
    Field<TM, TL, NL...>& operator _OP_##=(Field<TM, TL, NL...>& lhs, TR const& rhs) {                \
        lhs = lhs _OP_ rhs;                                                                           \
        return lhs;                                                                                   \
    }                                                                                                 \
    template <typename TM, typename TL, int... NL, typename... TR>                                    \
    Field<TM, TL, NL...>& operator _OP_##=(Field<TM, TL, NL...>& lhs, Expression<TR...> const& rhs) { \
        lhs = lhs _OP_ rhs;                                                                           \
        return lhs;                                                                                   \
    }

_SP_DEFINE_FIELD_COMPOUND_OP(+)
_SP_DEFINE_FIELD_COMPOUND_OP(-)
_SP_DEFINE_FIELD_COMPOUND_OP(*)
_SP_DEFINE_FIELD_COMPOUND_OP(/)
_SP_DEFINE_FIELD_COMPOUND_OP(%)
_SP_DEFINE_FIELD_COMPOUND_OP(&)
_SP_DEFINE_FIELD_COMPOUND_OP(|)
_SP_DEFINE_FIELD_COMPOUND_OP (^)
_SP_DEFINE_FIELD_COMPOUND_OP(<<)
_SP_DEFINE_FIELD_COMPOUND_OP(>>)
#undef _SP_DEFINE_FIELD_COMPOUND_OP

#define _SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(_TAG_, _REDUCTION_, _OP_)                                  \
    template <typename TM, typename TL, int... NL, typename TR>                                             \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, TR const& rhs) {                                    \
        return traits::reduction<_REDUCTION_>(Expression<tags::_TAG_, Field<TM, TL, NL...>, TR>(lhs, rhs)); \
    };                                                                                                      \
    template <typename TL, typename TM, typename TR, int... NR>                                             \
    bool operator _OP_(TL const& lhs, Field<TM, TR, NR...> const& rhs) {                                    \
        return traits::reduction<_REDUCTION_>(Expression<tags::_TAG_, TL, Field<TM, TR, NR...>>(lhs, rhs)); \
    };                                                                                                      \
    template <typename TM, typename TL, int... NL, typename... TR>                                          \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, Expression<TR...> const& rhs) {                     \
        return traits::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Field<TM, TL, NL...>, Expression<TR...>>(lhs, rhs));                    \
    };                                                                                                      \
    template <typename... TL, typename TM, typename TR, int... NR>                                          \
    bool operator _OP_(Expression<TL...> const& lhs, Field<TM, TR, NR...> const& rhs) {                     \
        return traits::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Expression<TL...>, Field<TM, TR, NR...>>(lhs, rhs));                    \
    };                                                                                                      \
    template <typename TM, typename TL, int... NL, typename TR, int... NR>                                  \
    bool operator _OP_(Field<TM, TL, NL...> const& lhs, Field<TM, TR, NR...> const& rhs) {                  \
        return traits::reduction<_REDUCTION_>(                                                              \
            Expression<tags::_TAG_, Field<TM, TL, NL...>, Field<TM, TR, NR...>>(lhs, rhs));                 \
    };

_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(not_equal_to, tags::logical_or, !=)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(equal_to, tags::logical_and, ==)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(less, tags::logical_and, <)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(greater, tags::logical_and, >)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(less_equal, tags::logical_and, <=)
_SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR(greater_equal, tags::logical_and, >=)
#undef _SP_DEFINE_FIELD_BINARY_BOOLEAN_OPERATOR

}  // namespace simpla
//        static int tag[4][3] = {{0, 0, 0}, {1, 2, 4}, {6, 5, 3}, {7, 7, 7}};
//        for (int j = 0; j < NUMBER_OF_SUB; ++j) {
//            VERBOSE << m_holder_[j].GetIndexBox() << "~" << m_mesh_->GetIndexBox(tag[IFORM][(j / DOF) % 3]) <<
//            std::endl;
//        }
//        VERBOSE << s.x << "," << s.y << "," << s.z << "   " << std::boolalpha
//                << m_holder_[EntityIdCoder::SubIndex<IFORM, DOF>(s)].empty() << std::endl;
//        static constexpr int id_2_sub_edge[3] = {1, 2, 4};
//        static constexpr int id_2_sub_face[3] = {6, 5, 3};
//        if (m_range_.empty()) {
//            for (int i = 0; i < NUMBER_OF_SUB; ++i) {
//                int16_t w = 0;
//                switch (IFORM) {
//                    case VERTEX:
//                        w = static_cast<int16_t>(i << 3);
//                        break;
//                    case EDGE:
//                        w = static_cast<int16_t>(((i % DOF) << 3) | id_2_sub_edge[(i / DOF) % 3]);
//                        break;
//                    case FACE:
//                        w = static_cast<int16_t>(((i % DOF) << 3) | id_2_sub_face[(i / DOF) % 3]);
//                        break;
//                    case VOLUME:
//                        w = static_cast<int16_t>((i << 3) | 0b111);
//                        break;
//                    default:
//                        break;
//                }
//                m_holder_[i].Foreach([&](index_tuple const& idx, value_type& v) {
//                    EntityId s;
//                    s.w = w;
//                    s.x = static_cast<int16_t>(idx[0]);
//                    s.y = static_cast<int16_t>(idx[1]);
//                    s.z = static_cast<int16_t>(idx[2]);
//                    v = calculus_policy::getValue(*m_mesh_, other, s);
//                });
//            }
//        } else {
//        }
// namespace declare {
//
// template <typename TM, typename TV, int IFORM, int DOF>
// class Field_ : public Field<TM, TV, IFORM, DOF> {
//    typedef Field_<TM, TV, IFORM, DOF> this_type;
//    typedef Field<TM, TV, IFORM, DOF> base_type;
//
//   public:
//    template <typename... Args>
//    explicit Field_(Args&&... args) : base_type(std::forward<Args>(args)...) {}
//
//    Field_(this_type const& other) : base_type(other){};
//    //    Field_(this_type&& other) = delete;
//    ~Field_() {}
//
//    using base_type::operator[];
//    using base_type::operator=;
//    using base_type::operator();
//
//    this_type operator[](EntityRange const& d) const { return this_type(*this, d); }
//};
//}  // namespace declare
//}  // namespace algebra
//
// template <typename TM, typename TV, int IFORM = VERTEX, int DOF = 1>
// using Field = algebra::declare::Field_<TM, TV, IFORM, DOF>;

#endif  // SIMPLA_FIELD_H
