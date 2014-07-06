/*
 * load_particle.h
 *
 *  Created on: 2013-12-21
 *      Author: salmon
 */

#ifndef LOAD_PARTICLE_H_
#define LOAD_PARTICLE_H_

#include <random>
#include <string>
#include <functional>

#include "../fetl/fetl.h"
#include "../fetl/load_field.h"
#include "../numeric/multi_normal_distribution.h"
#include "../numeric/rectangle_distribution.h"
#include "../utilities/log.h"
#include "../physics/physical_constants.h"
#include "../parallel/update_ghosts.h"
namespace simpla
{

template<typename TP, typename TDict, typename TModel, typename TN, typename TT>
std::shared_ptr<TP> LoadParticle(TDict const &dict, TModel const & model, TN const & ne0, TT const & T0)
{
	if (!dict || (TP::GetTypeAsString() != dict["Type"].template as<std::string>()))
	{
		PARSER_ERROR("illegal particle configure!");
	}

	typedef typename TP::mesh_type mesh_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	std::shared_ptr<TP> p(new TP(dict, model.mesh));

	auto range = model.SelectByConfig(TP::IForm, dict["Select"]);

	std::function<Real(coordinates_type const&)> ns;

	std::function<Real(coordinates_type const&)> Ts;

	if (!T0.empty())
	{
		Ts = [&T0](coordinates_type x)->Real
		{	return T0(x);};
	}
	else if (dict["Temperature"].is_number())
	{
		Real T = dict["Temperature"].template as<Real>();
		Ts = [T](coordinates_type x)->Real
		{	return T;};
	}
	else if (dict["Temperature"].is_function())
	{
		Ts = dict["Temperature"].template as<std::function<Real(coordinates_type const&)>>();
	}

	if (!ne0.empty())
	{
		Real ratio = dict["Ratio"].template as<Real>(1.0);
		ns = [&ne0,ratio](coordinates_type x)->Real
		{	return ne0(x)*ratio;};
	}
	else if (dict["Density"].is_number())
	{
		Real n0 = dict["Density"].template as<Real>();
		ns = [n0](coordinates_type x)->Real
		{	return n0;};
	}
	else if (dict["Density"].is_function())
	{
		ns = dict["Density"].template as<std::function<Real(coordinates_type const&)>>();
	}

	  unsigned int   pic = dict["PIC"].template as<size_t>(100);

	InitParticle(p.get(), range, pic, ns, Ts);

	LoadParticleConstriant(p.get(), range, model, dict["Constriants"]);

	LOGGER << "Create Particles:[ Engine=" << p->GetTypeAsString() << ", Number of Particles=" << p->size() << "]";

	LOGGER << DONE;

	return p;

}

template<typename TP, typename TRange, typename TModel, typename TDict>
void LoadParticleConstriant(TP *p, TRange const &range, TModel const & model, TDict const & dict)
{
	for (auto const & key_item : dict)
	{
		auto const & item = std::get<1>(key_item);

		auto r = model.SelectByConfig(range, item["Select"]);

		auto type = item["Type"].template as<std::string>("Modify");

		if (type == "Modify")
		{
			p->AddConstraint([=]()
			{	p->Modify(r, item["Operations"]);});
		}
		else if (type == "Remove")
		{
			if (item["Operation"])
			{
				p->AddConstraint([=]()
				{	p->Remove(r);});
			}
			else if (item["Condiition"])
			{
				p->AddConstraint([=]()
				{	p->Remove(r,item["Condiition"]);});
			}
		}

	}
}

template<typename TP, typename TR, typename TN, typename TT>
void InitParticle(TP *p, TR range, size_t pic, TN const & ns, TT const & Ts)
{
	typedef typename TP::engine_type engine_type;

	typedef typename TP::mesh_type mesh_type;

	typedef typename mesh_type::coordinates_type coordinates_type;

	static constexpr  unsigned int  NDIMS = mesh_type::NDIMS;

	mesh_type const &mesh = p->mesh;

	DEFINE_PHYSICAL_CONST

	nTuple<NDIMS, Real> dxmin = { -0.5, -0.5, -0.5 };
	nTuple<NDIMS, Real> dxmax = { 0.5, 0.5, 0.5 };
	rectangle_distribution<NDIMS> x_dist(dxmin, dxmax);
	multi_normal_distribution<NDIMS> v_dist;

	std::mt19937 rnd_gen(NDIMS * 2);

	nTuple<3, Real> x, v;

	auto buffer = p->create_child();
	for (auto s : range)
	{

		Real inv_sample_density = mesh.CellVolume(s) / pic;

		p->n[s] = mesh.Sample(Int2Type<TP::IForm>(), s, p->q * ns(mesh.GetCoordinates(s)));

		for (int i = 0; i < pic; ++i)
		{
			x_dist(rnd_gen, &x[0]);

			v_dist(rnd_gen, &v[0]);

			x = mesh.CoordinatesLocalToGlobal(s, x);

			v *= std::sqrt(boltzmann_constant * Ts(x) / p->m);

			buffer.push_back(engine_type::make_point(x, v, ns(x) * inv_sample_density));
		}
	}

	p->Add(&buffer);
	p->Sort();
//	UpdateGhosts(p);
}
}  // namespace simpla

#endif /* LOAD_PARTICLE_H_ */
