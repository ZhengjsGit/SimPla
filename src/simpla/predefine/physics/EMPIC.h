//
// Created by salmon on 16-6-5.
//

#ifndef SIMPLA_EMPIC_H
#define SIMPLA_EMPIC_H

#include "../../src/physics/Field.h"
#include "../../src/physics/PhysicalConstants.h"
#include "../../src/simulation/ProblemDomain.h"
#include "../../src/mesh/Mesh.h"
#include "../../src/mesh/EntityRange.h"
#include "../../src/physics/Particle.h"
//#include "../../src/particle/pre_define/BorisParticle.h"
#include "../../src/physics/ParticleGenerator.h"
#include "../../src/mesh/Model.h"
#include "BorisYee.h"

namespace simpla
{
using namespace mesh;


template<typename TM>
class EMPIC : public ProblemDomain
{
    typedef EMPIC<TM> this_type;
    typedef ProblemDomain base_type;

public:
    virtual bool is_a(std::type_info const &info) const { return typeid(this_type) == info; }

    template<typename _UOTHER_>
    bool is_a() const { return is_a(typeid(_UOTHER_)); }


    virtual std::string getClassName() const { return class_name(); }


    static std::string class_name() { return "EMPIC<" + traits::type_id<TM>::name() + ">"; }


public:
    typedef TM mesh_type;
    typedef typename mesh_type::scalar_type scalar_type;
    mesh_type   m_mesh_;

    EMPIC(const mesh_type *mp) : base_type(mp), m(mp) { }

    virtual ~EMPIC() { }

    virtual void init(ConfigParser const &options);

    virtual void next_step(Real dt);

    virtual io::IOStream &check_point(io::IOStream &os) const;

//    virtual io::IOStream &Save(io::IOStream &os) const;

    Mesh* GetMesh() { return &m_mesh_; }
    Mesh const* GetMesh() const { return &m_mesh_; }

    EntityRange limiter_boundary;
    EntityRange vertex_boundary;
    EntityRange edge_boundary;
    EntityRange face_boundary;

    EntityRange plasma_region_volume;
    EntityRange plasma_region_vertex;


    template<typename ValueType, size_t IFORM> using field_t =  Field<ValueType, TM, std::integral_constant<size_t, IFORM> >;;

    EntityRange J_src_range;
    std::function<Vec3(Real, point_type const &, vector_type const &v)> J_src_fun;


//    field_type<scalar_type, EDGE> E0/*   */{*this, "E0"};
    field_t<scalar_type, FACE> B0/*   */{*this, "B0"};
    field_t<scalar_type, FACE> B1/*   */{*this, "B1"};
    field_t<scalar_type, EDGE> E1/*   */{*this, "E1"};
    field_t<scalar_type, EDGE> J1/*   */{*this, "J1"};

//    field_type<scalar_type, NODE> n{*this, "n"};

    typedef particle::BorisParticle<mesh_type> particle_type;
    particle_type H{*this, "H"};


};

template<typename TM>
void EMPIC<TM>::init(ConfigParser const &options)
{
    if (options["Constraints"]["J"])
    {
        options["Constraints"]["J"]["Value"].as(&J_src_fun);

        mesh::select(*m, m->range(EDGE), options["Constraints"]["J"]["RectMesh"].as<box_type>()).swap(J_src_range);

    }
    dt(options["MeshBase"]["dt"].as<Real>(1.0));

    time(options["MeshBase"]["time"].as<Real>(0.0));

    J1.clear();
    B1.clear();
    E1.clear();
    B0.clear();

    if (options["InitValue"])
    {
        if (options["InitValue"]["B0"])
        {
            std::function<vector_type(point_type const &)> fun;
            options["InitValue"]["B0"]["Value"].as(&fun);
            parallel::parallel_foreach(
                    m->range(FACE), [&](EntityId const &s)
                    {
                        B0[s] = m->template sample<FACE>(s, fun(m->point(s)));
                    });
        }

        if (options["InitValue"]["B1"])
        {
            std::function<vector_type(point_type const &)> fun;
            options["InitValue"]["B1"]["Value"].as(&fun);
            parallel::parallel_foreach(
                    m->range(FACE), [&](EntityId const &s)
                    {
                        B1[s] = m->template sample<FACE>(s, fun(m->point(s)));
                    });
        }

        if (options["InitValue"]["E1"])
        {
            std::function<vector_type(point_type const &)> fun;
            options["InitValue"]["E1"]["Value"].as(&fun);
            parallel::parallel_foreach(
                    m->range(EDGE), [&](EntityId const &s)
                    {
                        E1[s] = m->template sample<EDGE>(s, fun(m->point(s)));
                    });
        }
    }

    H.mass(options["Particles"]["H"]["mass"].as<Real>(1.0));
    H.charge(options["Particles"]["H"]["charge"].as<Real>(1.0));
    size_t pic = options["Particles"]["H"]["pic"].as<int>(10);

    H.deploy();
    particle::ParticleGenerator<> gen;

    particle::generate_particle(
            &H, gen, pic,
            [&](nTuple<Real, 3> const &x0, nTuple<Real, 3> const &v0, typename particle_type::value_type *res)
            {

            });
}

template<typename TM>
io::IOStream &EMPIC<TM>::check_point(io::IOStream &os) const
{
    os.write("E1", E1.dataset(), io::SP_RECORD);
    os.write("B1", B1.dataset(), io::SP_RECORD);
    os.write("J1", J1.dataset(), io::SP_RECORD);
    return os;
}

//template<typename TM>
//io::IOStream &EMPIC<TM>::Save(io::IOStream &os) const
//{
//    os.write("H", H.dataset(), io::SP_NEW);
//    return os;
//}


template<typename TM>
void EMPIC<TM>::next_step(Real dt)
{

    DEFINE_PHYSICAL_CONST

    J1.clear();

    H.gather_all(&J1);

    H.push_all(dt, E1, B0);

    if (J_src_fun)
    {
        Real current_time = time();

        auto f = J_src_fun;
        parallel::parallel_foreach(
                J_src_range, [&](EntityId const &s)
                {
                    auto x0 = m->point(s);
                    auto v = J_src_fun(current_time, x0, J1(x0));
                    J1[s] += m->template sample<EDGE>(s, v);
                });
    }

    LOG_CMD(B1 -= curl(E1) * (dt * 0.5));

    B1.apply(face_boundary, [](EntityId const &) -> Real { return 0.0; });

    LOG_CMD(E1 += (curl(B1) * speed_of_light2 - J1 / epsilon0) * dt);

    E1.apply(edge_boundary, [](EntityId const &) -> Real { return 0.0; });


}


}//namespace simpla  {
#endif //SIMPLA_EMPIC_H
