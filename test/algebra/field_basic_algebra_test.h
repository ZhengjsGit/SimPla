/*
 * field_basic_algebra_test.h
 *
 *  created on: 2014-2-20
 *      Author: salmon
 */

#ifndef FIELD_BASIC_ALGEBRA_TEST_H_
#define FIELD_BASIC_ALGEBRA_TEST_H_

#include <stddef.h>
#include <memory>
#include <random>

#include <gtest/gtest.h>

#include <simpla/mpl/macro.h>
#include <simpla/mpl/type_traits.h>
#include <simpla/toolbox/Log.h>
#include <simpla/toolbox/sp_def.h>
#include "simpla/algebra/Field.h"

using namespace simpla;

template<typename TField>
class TestField : public testing::Test
{
protected:
    virtual void SetUp()
    {
        logger::set_stdout_level(10);

        size_tuple dims = {10, 1, 1};
        point_type xmin = {0, 0, 0};
        point_type xmax = {1, 2, 3};
//        m->dimensions(dims);
//        m->box(xmin, xmax);

        size_type gw[3] = {2, 2, 2};
        index_type lo[3] = {0, 0, 0};
        index_type hi[3];//= {dims[0], dims[1], dims[2]};

        m = std::make_shared<mesh_type>(&dims[0], &xmin[0], &xmax[0]);
        m->deploy();


    }

public:

    typedef TField field_type;

    typedef typename field_type::mesh_type mesh_type;

    typedef typename field_type::value_type value_type;

    typedef Real scalar_type;

//    typedef  traits::scalar_type_t<manifold_type> scalar_type;

    static constexpr size_type iform = algebra::traits::iform<TField>::value;
    static constexpr size_type dof = algebra::traits::dof<TField>::value;


    value_type default_value;


    std::shared_ptr<mesh_type> m;


//    typedef Field<value_type, manifold_type, index_const<static_cast<size_t>(iform)> > field_type;
    typedef Field<value_type, mesh_type, iform> scalar_field_type;
    typedef Field<value_type, mesh_type, iform, 3> vector_field_type;

//    auto make_scalarField() const DECL_RET_TYPE((field_t<value_type, manifold_type, iform>(m)))
//
//    auto make_vectorField() const DECL_RET_TYPE((field_t<nTuple<value_type, 3>, manifold_type, iform>(m)))
};

TYPED_TEST_CASE_P(TestField);

TYPED_TEST_P(TestField, assign)
{


    typedef typename TestFixture::value_type value_type;
    typedef typename TestFixture::field_type field_type;

    typename TestFixture::field_type f1(TestFixture::m);

    f1.clear();
    value_type va;
    va = 2.0;
    f1 = va;
    size_type count = 0;
    TestFixture::m->foreach(
            [&](typename TestFixture::mesh_type::id_type const &s) { EXPECT_LE(abs(va - f1[s]), EPSILON); },
            TestFixture::iform, TestFixture::dof);
}

TYPED_TEST_P(TestField, index)
{


    typedef typename TestFixture::value_type value_type;

    typename TestFixture::field_type f1(TestFixture::m);


    f1.clear();

    value_type va;

    va = 2.0;

    TestFixture::m->foreach(
            [&](typename TestFixture::mesh_type::id_type const &s)
            {
                f1[s] = va * TestFixture::m->hash(s);
            }, TestFixture::iform);

    TestFixture::m->foreach(
            [&](typename TestFixture::mesh_type::id_type const &s)
            {
                EXPECT_LE(abs(va * TestFixture::m->hash(s) - f1[s]), EPSILON);
            }, TestFixture::iform);

}

TYPED_TEST_P(TestField, constant_real)
{


    typedef typename TestFixture::value_type value_type;
    typedef typename TestFixture::field_type field_type;

    typename TestFixture::field_type f1(TestFixture::m);
    typename TestFixture::field_type f2(TestFixture::m);
    typename TestFixture::field_type f3(TestFixture::m);


    f1.deploy();
    f2.deploy();
    f3.deploy();
    Real a, b, c;
    a = -1.1, b = 1.34, c = 3.2;

    value_type va, vb;

    va = 2.0;
    vb = 3.0;
    std::mt19937 gen;

    std::uniform_real_distribution<Real> uniform_dist(0, 1.0);

    f1.assign([&](typename TestFixture::mesh_type::id_type const &s) { return va * uniform_dist(gen); });
    f2.assign([&](typename TestFixture::mesh_type::id_type const &s) { return vb * uniform_dist(gen); });


    LOG_CMD(f3 = -f1 + f1 * a + f2 * c - f1 / b);

    TestFixture::m->foreach(
            [&](typename TestFixture::mesh_type::id_type const &s)
            {
                value_type expect;
                expect = -f1[s] + f1[s] * a + f2[s] * c - f1[s] / b;

                // FIXME： truncation error is too big . why ??
                EXPECT_LE(abs(expect - f3[s]), EPSILON);
            }, TestFixture::iform);
}

TYPED_TEST_P(TestField, scalarField)
{


    typedef typename TestFixture::value_type value_type;

    typename TestFixture::field_type f1(TestFixture::m);
    typename TestFixture::field_type f2(TestFixture::m);
    typename TestFixture::field_type f3(TestFixture::m);
    typename TestFixture::field_type f4(TestFixture::m);

    typename TestFixture::scalar_field_type fa(TestFixture::m);
    typename TestFixture::scalar_field_type fb(TestFixture::m);
    typename TestFixture::scalar_field_type fc(TestFixture::m);

    Real ra = 1.0, rb = 10.0, rc = 100.0;

    value_type va, vb, vc;

    va = ra;
    vb = rb;
    vc = rc;

    fa = (va);
    fb = (vb);
    fc = (vc);

    f1.deploy();
    f2.deploy();
    f3.deploy();
    f4.deploy();
    f4.clear();
    size_type count = 0;

    std::mt19937 gen;
    std::uniform_real_distribution<Real> uniform_dist(0, 1.0);

    f1.assign([&](typename TestFixture::mesh_type::id_type const &s) { return va * uniform_dist(gen); });
    f2.assign([&](typename TestFixture::mesh_type::id_type const &s) { return vb * uniform_dist(gen); });
    f3.assign([&](typename TestFixture::mesh_type::id_type const &s) { return vc * uniform_dist(gen); });

    LOG_CMD(f4 = -f1 * fa + f2 * fb - f3 / fc - f1);

//	Plus( Minus(Negate(Wedge(f1,fa)),Divides(f2,fb)),Multiplies(f3,fc) )

/**           (+)
 *           /   \
 *         (-)    (*)
 *        /   \    | \
 *      (^)    (/) f1 c
 *     /  \   /  \
 *-f1      a f2   b
 *
 * */

//    TestFixture::m->foreach([&](typename TestFixture::mesh_type::id_type const &s)
//                            {
//                                CHECK(fa[s]);
//                                CHECK(fb[s]);
//                                CHECK(fc[s]);
//                            });

    TestFixture::m->foreach(
            [&](typename TestFixture::mesh_type::id_type const &s)
            {
                value_type res = -f1[s] * ra + f2[s] * rb - f3[s] / rc - f1[s];



                EXPECT_DOUBLE_EQ(abs(res), abs(f4[s]));
                EXPECT_LE(abs(res - f4[s]), EPSILON);
            }, TestFixture::iform, TestFixture::dof);


}

REGISTER_TYPED_TEST_CASE_P(TestField, assign, index, constant_real, scalarField);

//#include <gtest/gtest.h>
//
//#include "field.h"
//#include "../geometry/domain_traits.h"
//using namespace simpla;
//
////#include "../utilities/log.h"
////#include "../utilities/pretty_stream.h"
////
////using namespace simpla;
////
//class Bundle;
//class Container;
//
//class TestFIELD: public testing::TestWithParam<
//		std::tuple<typename domain_traits<Bundle>::coordinate_tuple,
//				typename Bundle::coordinate_tuple,
//				nTuple<Bundle::NDIMS, size_t>, nTuple<Bundle::NDIMS, Real> > >
//{
//
//protected:
//	void SetUp()
//	{
//		LOGGER.set_stdout_level(LOG_INFORM);
//		auto _fdtd_param = GetParam();
//
//		xmin = std::get<0>(_fdtd_param);
//
//		xmax = std::get<1>(_fdtd_param);
//
//		topology_dims = std::get<2>(_fdtd_param);
//
//		K_real = std::get<3>(_fdtd_param);
//
//		SetDefaultValue(&default_value);
//
//		for (int i = 0; i < NDIMS; ++i)
//		{
//			if (topology_dims[i] <= 1 || (xmax[i] <= xmin[i]))
//			{
//				topology_dims[i] = 1;
//				K_real[i] = 0.0;
//				xmax[i] = xmin[i];
//			}
//		}
//
//		geometry.set_dimensions(topology_dims);
//		geometry.set_extents(xmin, xmax);
//
//		geometry.sync();
//
//	}
//public:
//
//	typedef Bundle domain_type;
//	typedef Real value_type;
//	typedef domain_type::scalar_type scalar_type;
//	typedef domain_type::iterator iterator;
//	typedef domain_type::coordinate_tuple coordinate_tuple;
//
//	domain_type geometry;
//
//	static constexpr unsigned int NDIMS = domain_type::NDIMS;
//
//	nTuple<NDIMS, Real> xmin;
//
//	nTuple<NDIMS, Real> xmax;
//
//	nTuple<NDIMS, size_t> topology_dims;
//
//	nTuple<3, Real> K_real; // @NOTE must   k = n TWOPI, period condition
//
//	nTuple<3, scalar_type> K_imag;
//
//	value_type default_value;
//
//	template<typename T>
//	void SetDefaultValue(T* v)
//	{
//		*v = 1;
//	}
//	template<typename T>
//	void SetDefaultValue(std::complex<T>* v)
//	{
//		T r;
//		SetDefaultValue(&r);
//		*v = std::complex<T>();
//	}
//
//	template<unsigned int N, typename T>
//	void SetDefaultValue(nTuple<T,N>* v)
//	{
//		for (int i = 0; i < N; ++i)
//		{
//			(*v)[i] = i;
//		}
//	}
//
//	virtual ~TestFIELD()
//	{
//
//	}
//
//};

#endif /* FIELD_BASIC_ALGEBRA_TEST_H_ */