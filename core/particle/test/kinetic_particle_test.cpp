/**
 * @file kinetic_particle_test.cpp
 *
 * @date 2015年2月11日
 * @author salmon
 */

#include <gtest/gtest.h>
#include "../kinetic_particle.h"
#include "../particle_engine.h"
#include "../particle_generator.h"
#include "../../mesh/simple_mesh.h"
#include "../../field/field.h"

#include "../../numeric/rectangle_distribution.h"
#include "../../numeric/multi_normal_distribution.h"

using namespace simpla;

class TestKineticParticle: public testing::Test
{
protected:
	virtual void SetUp()
	{
		LOGGER.set_stdout_visable_level(LOG_DEBUG);
	}
public:
	typedef SimpleMesh mesh_type;
	typedef SimpleParticleEngine engine_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef KineticParticle<mesh_type, engine_type> particle_type;

	static constexpr size_t pic = 10;

	mesh_type mesh;

	typedef typename engine_type::Point_s Point_s;

};
constexpr size_t TestKineticParticle::pic;

TEST_F(TestKineticParticle,Add)
{

	particle_type p(mesh);

	auto extents = mesh.extents();

	ParticleGenerator<engine_type,

	rectangle_distribution<mesh_type::ndims>,

	multi_normal_distribution<mesh_type::ndims> > p_generator(

	static_cast<engine_type const &>(p),

	rectangle_distribution<mesh_type::ndims>(extents),

	multi_normal_distribution<mesh_type::ndims>()

	);

	std::mt19937 rnd_gen;

	auto range = mesh.range();

	size_t num = pic * (mesh.hash(*end(range)) - mesh.hash(*begin(range)));

	p.insert(p_generator.begin(rnd_gen), p_generator.end(rnd_gen, num));

	EXPECT_EQ(p.size(), num);

//	sync(&p);

//	VERBOSE << "update_ghosts particle DONE " << p.size() << std::endl;

}

TEST_F(TestKineticParticle, scatter_n)
{

	SimpleField<mesh_type, Real> n(mesh), n0(mesh);

	particle_type ion(mesh);

	SimpleField<mesh_type, Real> E(mesh);
	SimpleField<mesh_type, Real> B(mesh);

	E.clear();
	B.clear();
	n0.clear();
	n.clear();

//	scatter(ion, &n, E, B);

	Real q = ion.charge;
	Real variance = 0.0;

	Real average = 0.0;

//	for (auto s : mesh.range())
//	{
//		coordinates_type x = mesh.id_to_coordinates(s);
//
//		Real expect = q * n(x[0], x[1], x[2]).template as<Real>();
//
//		n0[s] = expect;
//
//		Real actual = n.get(s);
//
//		average += abs(actual);
//
//		variance += std::pow(abs(expect - actual), 2.0);
//	}

//	if (std::is_same<engine_type, PICEngineFullF<mesh_type> >::value)
//	{
//		Real relative_error = std::sqrt(variance) / abs(average);
//		CHECK(relative_error);
//		EXPECT_LE(relative_error, 1.0 / std::sqrt(pic));
//	}
//	else
	{
		Real error = 1.0 / std::sqrt(static_cast<double>(ion.size()));

		EXPECT_LE(abs(average), error);
	}

}
