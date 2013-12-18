/*
 * CoRectMesh.h
 *
 *  Created on: 2013年12月12日
 *      Author: salmon
 */

#ifndef RECT_MESH_H_
#define RECT_MESH_H_

#include <cmath>
#include <functional>
#include <initializer_list>
#include <iostream>
//#include <limits>
#include <memory>
#include <string>
#include <utility>
#include <valarray>
#include <vector>
#include <map>
#include <unordered_map>
#include "../../src/engine/basecontext.h"
//#include "../../src/fetl/fetl.h"
#include "../../src/fetl/field_function.h"
#include "../../src/fetl/field.h"
#include "../../src/fetl/ntuple.h"
#include "../../src/fetl/primitives.h"
#include "../../src/io/data_stream.h"
#include "../../src/mesh/media_tag.h"
#include "../../src/particle/particle.h"
#include "../../src/particle/pic_engine_default.h"
#include "../../src/particle/pic_engine_deltaf.h"
#include "../../src/utilities/log.h"
#include "../../src/utilities/lua_state.h"
#include "../../src/utilities/singleton_holder.h"
#include "../pic/pic_engine_ggauge.h"
#include "../solver/electromagnetic/cold_fluid.h"

namespace simpla
{
template<typename TM>
struct Context: public BaseContext
{
public:
	typedef BaseContext base_type;
	typedef TM mesh_type;
	typedef typename mesh_type::scalar scalar;
	typedef LuaObject configure_type;

	DEFINE_FIELDS (TM)
public:
	typedef Context<TM> this_type;

	Context();
	~Context();
	void Deserialize(configure_type const & cfg);
	void Serialize(configure_type * cfg) const;
	std::ostream & Serialize(std::ostream & os) const;
	void NextTimeStep(double dt);
	void DumpData() const;

	inline ParticleCollection<mesh_type> & GetParticleCollection()
	{
		return particle_collection_;
	}
public:

	mesh_type mesh;
	typedef typename mesh_type::scalar_type scalar_type;
	typedef typename mesh_type::index_type index_type;
	typedef typename mesh_type::coordinates_type coordinates_type;
	typedef MediaTag<mesh_type> mediatag_type;
	typedef typename mediatag_type::tag_type tag_type;

	mediatag_type media_tag;

	Form<1> E1;
	Form<1> J1;
	Form<2> B1;
	RVectorForm<0> B0;
	RForm<0> n0;

	ColdFluidEM<mesh_type> cold_fluid_;
	ParticleCollection<mesh_type> particle_collection_;

	typedef typename ParticleCollection<mesh_type>::particle_type particle_type;

	bool isCompactStored_;

//	typedef typename Form<1>::field_value_type field_value_type;
//	typedef std::function<field_value_type(Real, Real, Real, Real)> field_function;
	typedef LuaObject field_function;
	FieldFunction<decltype(J1), field_function> j_src_;

	std::unordered_multimap<std::string, std::function<void()> > fun_;
}
;

template<typename TM>
Context<TM>::Context()
		: E1(mesh), B1(mesh), J1(mesh), B0(mesh), n0(mesh),

		cold_fluid_(mesh), particle_collection_(mesh), isCompactStored_(true),

		media_tag(mesh)

{

	particle_collection_.template RegisterFactory<GGauge<mesh_type, 0>>("GuidingCenter");
	particle_collection_.template RegisterFactory<GGauge<mesh_type, 8>>("GGauge8");
	particle_collection_.template RegisterFactory<GGauge<mesh_type, 32>>("GGauge32");
	particle_collection_.template RegisterFactory<PICEngineDefault<mesh_type> >("Default");
	particle_collection_.template RegisterFactory<PICEngineDeltaF<mesh_type> >("DeltaF");
}

template<typename TM>
Context<TM>::~Context()
{
}
template<typename TM>
void Context<TM>::Deserialize(LuaObject const & cfg)
{
	base_type::description = cfg["Description"].as<std::string>();

	mesh.Deserialize(cfg["Grid"]);

	cold_fluid_.Deserialize(cfg["FieldSolver"]["ColdFluid"]);

//	particle_collection_.Deserialize(cfg["Particles"]);
//
//	LOGGER << " Load Particles [Done]!";

	auto init_value = cfg["InitValue"];

	auto gfile = cfg["GFile"];

	if (gfile.empty())
	{
		n0.Init();
		LoadField(init_value["n0"], &n0);
		B0.Init();
		LoadField(init_value["B0"], &B0);
		E1.Init();
		LoadField(init_value["E1"], &E1);
		B1.Init();
		LoadField(init_value["B1"], &B1);
		J1.Init();
		LoadField(init_value["J1"], &J1);
	}
	else
	{
		UNIMPLEMENT << "TODO: use g-file initialize field, set boundary condition!";
	}
	LOGGER << " Load Initial Fields [Done]!";

	LuaObject jSrcCfg = cfg["CurrentSrc"];

	if (!jSrcCfg.empty())
	{
		j_src_.SetFunction(jSrcCfg["Fun"]);

		j_src_.SetDefineDomain(mesh, jSrcCfg["Points"].as<std::vector<coordinates_type>>());

		LOGGER << " Load Current Source [Done]!";
	}

	media_tag.Deserialize(cfg["Media"]);

	LuaObject boundary = cfg["Boundary"];

	for (auto const & obj : boundary)
	{
		std::string type = "";

		obj.second["Type"].as<std::string>(&type);

		CHECK(type);

		tag_type in = media_tag.GetTagFromString(obj.second["In"].as<std::string>());
		tag_type out = media_tag.GetTagFromString(obj.second["Out"].as<std::string>());

		if (type == "PEC")
		{
			fun_.emplace(

			"Set PEC boundary on E1",

			[in,out,this]()
			{

				media_tag.template SelectBoundaryCell<1>(
						[this](index_type const &s)
						{
							CHECK(s);
							(this->E1)[s]=0;
						}
						,in,out,mediatag_type::ON_BOUNDARY,mesh_type::DO_PARALLEL
				);
			}

			);
		}
		else
		{
			UNIMPLEMENT << "Unknown boundary type [" << type << "]";
		}
	}

	LOGGER << " Load Boundary [Done]!";

}

template<typename TM>
void Context<TM>::Serialize(configure_type * cfg) const
{
}
template<typename TM>
std::ostream & Context<TM>::Serialize(std::ostream & os) const
{

	os << "Description=\"" << base_type::description << "\" \n";

	os << mesh << "\n"

	<< media_tag << "\n"

	<< " FieldSolver={ \n"

	<< cold_fluid_ << "\n"

	<< "} \n";

//	os << particle_collection_ << "\n"

	;

	os << "Function={";
	for (auto const & p : fun_)
	{
		os << "\"" << p.first << "\",\n";
	}
	os << "}\n";

	GLOBAL_DATA_STREAM.OpenGroup("/InitValue");

	os

	<< "InitValue={" << "\n"

	<< "	n0 = " << Data(n0.data(), "n0", n0.GetShape()) << ",\n"

	<< "	E1 = " << Data(E1.data(), "E1", E1.GetShape()) << ",\n"

	<< "	B1 = " << Data(B1.data(), "B1", B1.GetShape()) << ",\n"

	<< "	J1 = " << Data(J1.data(), "J1", J1.GetShape()) << ",\n"

	<< "	B0 = " << Data(B0.data(), "B0", n0.GetShape()) << "\n"

	<< "}" << "\n"

	;
	return os;
}
template<typename TM>
void Context<TM>::NextTimeStep(double dt)
{
	dt = std::isnan(dt) ? mesh.GetDt() : dt;

	base_type::NextTimeStep(dt);

	LOGGER

	<< " SimTime = "

	<< (base_type::GetTime() / mesh.constants["s"]) << "[s]"

	<< " dt = " << (dt / mesh.constants["s"]) << "[s]";

	J1 = 0;

	j_src_(&J1, base_type::GetTime());

//	particle_collection_.CollectAll(dt, &J1, E1, B1);
//
//	if (cold_fluid_.IsEmpty())
	{
		const double mu0 = mesh.constants["permeability of free space"];
		const double epsilon0 = mesh.constants["permittivity of free space"];
		const double speed_of_light = mesh.constants["speed of light"];
		const double proton_mass = mesh.constants["proton mass"];
		const double elementary_charge = mesh.constants["elementary charge"];

		E1 += (Curl(B1 / mu0) - J1) / epsilon0 * dt;
		B1 -= Curl(E1) * dt;
	}
//	else
//	{
//		cold_fluid_.NextTimeStep(dt, J1, &E1, &B1);
//	}

	for (auto const & p : fun_)
	{
		p.second();
		LOGGER << p.first << " Done!";
	}

//	particle_collection_.Push(dt, E1, B1);

}
template<typename TM>
void Context<TM>::DumpData() const
{
	GLOBAL_DATA_STREAM.OpenGroup("/DumpData");

	LOGGER << "Dump E1 to " << Data(E1.data(), "E1", E1.GetShape(), isCompactStored_);

	LOGGER << "Dump B1 to " << Data(B1.data(), "B1", B1.GetShape(), isCompactStored_);

	LOGGER << "Dump J1 to " << Data(J1.data(), "J1", J1.GetShape(), isCompactStored_);

	cold_fluid_.DumpData();

//	particle_collection_.DumpData();

}
}
 // namespace simpla

#endif /* RECT_MESH_H_ */
