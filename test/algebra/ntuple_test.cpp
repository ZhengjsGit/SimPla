/**
 * @file ntuple_test.cpp
 *
 *  created on: 2012-1-10
 *      Author: salmon
 */

#include <gtest/gtest.h>

#include <complex>
#include <iostream>
#include <typeinfo>
#include "simpla/algebra/nTuple.ext.h"
#include "simpla/algebra/nTuple.h"
#include "simpla/utilities/type_traits.h"
#include "simpla/utilities/utility.h"
using namespace simpla;

#define EQUATION(_A, _B, _C) (-(_A + TestFixture::a) / (_B * TestFixture::b - TestFixture::c) - _C)

template <typename T>
class TestNTuple : public testing::Test {
   protected:
    virtual void SetUp() {
        a = 1;
        b = 3;
        c = 4;
        d = 7;

        traits::seq_for_each(extents(), [&](int const *idx) {
            traits::get_v(aA, idx) = static_cast<value_type>(idx[0] * 2);
            traits::get_v(aB, idx) = static_cast<value_type>(5 - idx[0]);
            traits::get_v(aC, idx) = static_cast<value_type>(idx[0] * 5 + 1);
            traits::get_v(aD, idx) = static_cast<value_type>(0);
            traits::get_v(vA, idx) = traits::get_v(aA, idx);
            traits::get_v(vB, idx) = traits::get_v(aB, idx);
            traits::get_v(vC, idx) = traits::get_v(aC, idx);
            traits::get_v(vD, idx) = static_cast<value_type>(0);
            traits::get_v(res, idx) =
                -(traits::get_v(aA, idx) + a) / (traits::get_v(aB, idx) * b - c) - traits::get_v(aC, idx);

        });
    }

   public:
    typedef T type;

    typedef traits::extents<type> extents;

    nTuple<int, std::rank<type>::value> DIMENSIONS;

    typedef traits::value_type_t<type> value_type;

    type vA, vB, vC, vD;

    typename traits::pod_type_t<type> aA, aB, aC, aD, res;

    value_type a, b, c, d;
};

typedef testing::Types<nTuple<double, 3>,                        //
                       nTuple<double, 3, 3>,                     //
                       nTuple<double, 3, 4, 5>,                  //
                       nTuple<int, 3, 4, 5, 6>,                  //
                       nTuple<std::complex<double>, 3>,          //
                       nTuple<std::complex<double>, 3, 4, 5, 6>  //
                       >
    ntuple_type_lists;

TYPED_TEST_CASE(TestNTuple, ntuple_type_lists);

TYPED_TEST(TestNTuple, swap) {
    TestFixture::vA.swap(TestFixture::vB);

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        EXPECT_DOUBLE_EQ(0, simpla::abs(traits::get_v(TestFixture::aA, idx) - traits::get_v(TestFixture::vB, idx)));
        EXPECT_DOUBLE_EQ(0, simpla::abs(traits::get_v(TestFixture::aB, idx) - traits::get_v(TestFixture::vA, idx)));
    });
}

TYPED_TEST(TestNTuple, assign_Scalar) {
    TestFixture::vA = TestFixture::a;

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        EXPECT_DOUBLE_EQ(0, simpla::abs(TestFixture::a - traits::get_v(TestFixture::vA, idx)));
    });
}
//
TYPED_TEST(TestNTuple, assign_Array) {
    TestFixture::vA = TestFixture::aA;

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        EXPECT_DOUBLE_EQ(0, simpla::abs(traits::get_v(TestFixture::aA, idx) - traits::get_v(TestFixture::vA, idx)));
    });
}

TYPED_TEST(TestNTuple, self_assign) {
    TestFixture::vB += TestFixture::vA;

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        EXPECT_DOUBLE_EQ(simpla::abs(traits::get_v(TestFixture::vB, idx)),
                         simpla::abs(traits::get_v(TestFixture::aA, idx) + traits::get_v(TestFixture::aB, idx)));

    });
}

TYPED_TEST(TestNTuple, cross) {
    nTuple<typename TestFixture::value_type, 3> vA, vB, vC, vD;

    for (int i = 0; i < 3; ++i) {
        vA[i] = (i * 2);
        vB[i] = (5 - i);
    }

    for (int i = 0; i < 3; ++i) { vD[i] = vA[(i + 1) % 3] * vB[(i + 2) % 3] - vA[(i + 2) % 3] * vB[(i + 1) % 3]; }

    vC = cross(vA, vB);
    vD -= vC;
    EXPECT_DOUBLE_EQ(0, abs(vD[0]) + abs(vD[1]) + abs(vD[2]));
}

TYPED_TEST(TestNTuple, dot) {
    typename TestFixture::value_type expect;
    expect = 0;

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        expect += traits::get_v(TestFixture::aA, idx) * traits::get_v(TestFixture::aB, idx);
    });

    EXPECT_DOUBLE_EQ(0.0, abs(expect - (dot(TestFixture::vA, TestFixture::vB))));
}

TYPED_TEST(TestNTuple, arithmetic) {
    TestFixture::vD = EQUATION(TestFixture::vA, TestFixture::vB, TestFixture::vC);

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        auto ta = traits::get_v(TestFixture::vA, idx);
        auto tb = traits::get_v(TestFixture::vB, idx);
        auto tc = traits::get_v(TestFixture::vC, idx);
        auto td = traits::get_v(TestFixture::vD, idx);
        EXPECT_DOUBLE_EQ(abs(td), abs(EQUATION(ta, tb, tc)));
    });
}

TYPED_TEST(TestNTuple, expression_construct) {
    auto tD = typename TestFixture::type(EQUATION(TestFixture::vA, TestFixture::vB, TestFixture::vC));

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        auto ta = traits::get_v(TestFixture::vA, idx);
        auto tb = traits::get_v(TestFixture::vB, idx);
        auto tc = traits::get_v(TestFixture::vC, idx);
        auto td = traits::get_v(tD, idx);
        EXPECT_DOUBLE_EQ(abs(td), abs(EQUATION(ta, tb, tc)));
    });
}

TYPED_TEST(TestNTuple, expression_construct2) {
    typename TestFixture::type tD = EQUATION(TestFixture::vA, TestFixture::vB, TestFixture::vC);

    traits::seq_for_each(typename TestFixture::extents(), [&](int const *idx) {
        auto ta = traits::get_v(TestFixture::vA, idx);
        auto tb = traits::get_v(TestFixture::vB, idx);
        auto tc = traits::get_v(TestFixture::vC, idx);
        auto td = traits::get_v(tD, idx);
        EXPECT_DOUBLE_EQ(abs(td), abs(EQUATION(ta, tb, tc)));
    });
}

TYPED_TEST(TestNTuple, compare) {
    EXPECT_TRUE(TestFixture::vA == TestFixture::vA);
    EXPECT_FALSE(TestFixture::vA != TestFixture::vA);
    EXPECT_FALSE(TestFixture::vA == TestFixture::vB);
    EXPECT_TRUE(TestFixture::vA != TestFixture::vB);
}