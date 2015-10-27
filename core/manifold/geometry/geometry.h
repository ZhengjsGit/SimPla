/**
 * @file mesh.h
 * @author salmon
 * @date 2015-10-13.
 */

#ifndef SIMPLA_GEOMETRY_H
#define SIMPLA_GEOMETRY_H

#include "../../gtl/primitives.h"
#include "../../gtl/macro.h"
#include "../../gtl/type_traits.h"
#include "../../geometry/coordinate_system.h"

#include "../topology/topology_common.h"
#include "geometry_traits.h"

namespace simpla
{


template<typename ...>
struct Geometry;


template<typename CS, typename TopologyTags>
struct Geometry<CS, TopologyTags> : public Topology<TopologyTags>
{
public:
    typedef CS coordinates_system_type;

    geometry::mertic<coordinates_system_type> m_metric_;

    typedef Topology<TopologyTags> topology_type;

    using topology_type::ndims;

    typedef typename topology_type::point_type topology_point_type;


    typedef geometry::traits::scalar_type_t<coordinates_system_type> scalar_type;

    typedef geometry::traits::point_type_t<coordinates_system_type> point_type;

    typedef geometry::traits::vector_type_t<coordinates_system_type> vector_type;

private:

    typedef Geometry<CS, TopologyTags> this_type;

    point_type m_coords_min_ = {0, 0, 0};

    point_type m_coords_max_ = {1, 1, 1};

    vector_type m_dx_ = {1, 1, 1};; //!< width of cell, except m_dx_[i]=0 when m_dims_[i]==1

    vector_type m_delta_ = {1, 1, 1};; //!< equiv. m_dx_, except m_delta_[i]=1 when m_dims_[i]==1

    topology_point_type m_from_topology_orig_ = {0, 0, 0};

    topology_point_type m_to_topology_orig_ = {0, 0, 0};

    topology_point_type m_to_topology_scale_ = {1, 1, 1};

    topology_point_type m_from_topology_scale_ = {1, 1, 1};

public:
    Geometry() { }

    Geometry(this_type const &other) : topology_type(other),

                                       m_coords_min_(other.m_coords_min_),

                                       m_coords_max_(other.m_coords_max_),

                                       m_dx_(other.m_dx_),

                                       m_delta_(other.m_delta_),

                                       m_from_topology_orig_(other.m_from_topology_orig_),

                                       m_to_topology_orig_(other.m_to_topology_orig_),

                                       m_to_topology_scale_(other.m_to_topology_scale_),

                                       m_from_topology_scale_(other.m_from_topology_scale_) { }

    virtual ~Geometry() { }


    template<typename TDict>
    void load(TDict const &dict)
    {
        auto d = dict["Geometry"];

        topology_type::load(d);

        extents(d["Box"].template as<std::tuple<point_type, point_type> >());
    }

    template<typename OS>
    OS &print(OS &os) const
    {
        os << "\t Gemetry = {" << std::endl;

        os << "\t\tCoordinateSystem = { " << std::endl
        << "\t\t Type = \"" << traits::type_id<CS>::name() << "\"," << std::endl
        << "\t\t Box = " << extents() << "," << std::endl
        //				<< "\t\t xmax = " << m_coords_max_ << "," << std::endl
        << "\t\t}," << std::endl;

        topology_type::print(os);


        os << "\t }, #Gemetry " << std::endl;

        return os;
    }

    virtual void deploy();


    point_type epsilon() const { return topology_type::EPSILON * m_from_topology_scale_; }


    template<typename X0, typename X1>
    void extents(X0 const &x0, X1 const &x1)
    {

        m_coords_min_ = x0;
        m_coords_max_ = x1;
    }

    template<typename T0>
    void extents(T0 const &box)
    {
        extents(simpla::traits::get<0>(box), simpla::traits::get<1>(box));
    }

    constexpr std::tuple<point_type, point_type> extents() const
    {
        return (std::make_tuple(m_coords_min_, m_coords_max_));
    }

//	constexpr std::pair<point_type, point_type> local_extents() const
//	{
//		return (std::make_pair(point(m_id_local_min_), point(m_id_local_max_)));
//	}

    vector_type const &dx() const
    {
        return m_dx_;
    }

/**
 * @name  Coordinate map
 * @{
 *
 *        Topology Mesh       Geometry Mesh
 *                        map
 *              M      ---------->      G
 *              x                       y
 **/
    point_type point(id_type const &s) const { return std::move(map(topology_type::coordinates(s))); }

    point_type map(topology_point_type const &x) const
    {

        point_type res;


        res[0] = std::fma(x[0], m_from_topology_scale_[0], m_from_topology_orig_[0]);

        res[1] = std::fma(x[1], m_from_topology_scale_[1], m_from_topology_orig_[1]);

        res[2] = std::fma(x[2], m_from_topology_scale_[2], m_from_topology_orig_[2]);


        return std::move(res);
    }

    topology_point_type inv_map(point_type const &y) const
    {

        topology_point_type res;


        res[0] = std::fma(y[0], m_to_topology_scale_[0], m_to_topology_orig_[0]);

        res[1] = std::fma(y[1], m_to_topology_scale_[1], m_to_topology_orig_[1]);

        res[2] = std::fma(y[2], m_to_topology_scale_[2], m_to_topology_orig_[2]);

        return std::move(res);
    }


/** @} */
/** @name Volume
 * @{
 */

    Real m_volume_[9];
    Real m_inv_volume_[9];
    Real m_dual_volume_[9];
    Real m_inv_dual_volume_[9];

    Real volume_(id_type s) const
    {
        return m_metric_.dual_volume(topology_type::node_id(s), point(s), m_delta_);
    }

    Real dual_volume_(id_type s) const
    {
        return m_metric_.dual_volume(topology_type::node_id(s), point(s), m_delta_);
    }

    Real volume(id_type s) const
    {
        return m_volume_[topology_type::node_id(s)];//* volume_(s);
    }

    Real dual_volume(id_type s) const
    {
        return m_dual_volume_[topology_type::node_id(s)];// * dual_volume_(s);
    }

    Real inv_volume(id_type s) const
    {
        return m_inv_volume_[topology_type::node_id(s)];// / volume_(s);
    }

    Real inv_dual_volume(id_type s) const
    {
        return m_inv_dual_volume_[topology_type::node_id(s)];// / dual_volume_(s);
    }

/**@}*/

    template<typename TX>
    topology_point_type coordinates_to_topology(TX const &y) const { return inv_map(y); }

    template<typename TX>
    point_type coordinates_from_topology(TX const &x) const { return map(x); }

/**
 * @bug: truncation error of coordinates transform larger than 1000
 *     epsilon (1e4 epsilon for cylindrical coordinates)
 * @param args
 * @return
 */
    point_type coordinates_local_to_global(std::tuple<id_type, topology_point_type> const &t) const
    {
        return std::move(map(topology_type::coordinates_local_to_global(t)));
    }

    std::tuple<id_type, topology_point_type> coordinates_global_to_local(
            point_type x, int n_id = 0) const
    {
        return std::move(topology_type::coordinates_global_to_local(coordinates_to_topology(x), n_id));
    }

    id_type id(point_type x, int n_id = 0) const
    {
        return std::get<0>(topology_type::coordinates_global_to_local(coordinates_to_topology(x), n_id));
    }
}; //struct Geometry<CS,TopologyTags >


template<typename CS, typename TopologyTags>
void Geometry<CS, TopologyTags>::deploy()
{
    topology_type::deploy();

    point_type i_min, i_max;

    std::tie(i_min, i_max) = topology_type::local_box();

    for (int i = 0; i < ndims; ++i)
    {
        ASSERT((m_coords_max_[i] - m_coords_min_[i] > EPSILON));
        ASSERT(i_max[i] - i_min[i] >= 1);


        m_dx_[i] = (m_coords_max_[i] - m_coords_min_[i]) / (i_max[i] - i_min[i]);

        m_to_topology_scale_[i] = (i_max[i] - i_min[i]) / (m_coords_max_[i] - m_coords_min_[i]);

        m_from_topology_scale_[i] = (m_coords_max_[i] - m_coords_min_[i]) / (i_max[i] - i_min[i]);

        if (i_max[i] - i_min[i] == 1)
        {
            m_to_topology_scale_[i] = 0;

            m_from_topology_scale_[i] = 0;

        }

        m_to_topology_orig_[i] = i_min[i] - m_coords_min_[i] * m_to_topology_scale_[i];

        m_from_topology_orig_[i] = m_coords_min_[i] - i_min[i] * m_from_topology_scale_[i];
    }

    /**
     *\verbatim
     *                ^y
     *               /
     *        z     /
     *        ^    /
     *        |  110-------------111
     *        |  /|              /|
     *        | / |             / |
     *        |/  |            /  |
     *       100--|----------101  |
     *        | m |           |   |
     *        |  010----------|--011
     *        |  /            |  /
     *        | /             | /
     *        |/              |/
     *       000-------------001---> x
     *
     *\endverbatim
     */

    auto dims = topology_type::dimensions();

#define NOT_ZERO(_V_) ((_V_<EPSILON)?1.0:(_V_))
    m_volume_[0] = 1.0;

    m_volume_[1/* 001*/] = (dims[0] > 1) ? m_dx_[0] : 1.0;
    m_volume_[2/* 010*/] = (dims[1] > 1) ? m_dx_[1] : 1.0;
    m_volume_[4/* 100*/] = (dims[2] > 1) ? m_dx_[2] : 1.0;

//    m_volume_[1/* 001*/] = (m_dx_[0] <= EPSILON) ? 1 : m_dx_[0];
//    m_volume_[2/* 010*/] = (m_dx_[1] <= EPSILON) ? 1 : m_dx_[1];
//    m_volume_[4/* 100*/] = (m_dx_[2] <= EPSILON) ? 1 : m_dx_[2];

    m_volume_[3] /* 011 */= m_volume_[1] * m_volume_[2];
    m_volume_[5] /* 101 */= m_volume_[4] * m_volume_[1];
    m_volume_[6] /* 110 */= m_volume_[2] * m_volume_[4];
    m_volume_[7] /* 111 */= m_volume_[1] * m_volume_[2] * m_volume_[4];

    m_dual_volume_[7] = 1.0;

    m_dual_volume_[6] = m_volume_[1];
    m_dual_volume_[5] = m_volume_[2];
    m_dual_volume_[3] = m_volume_[4];

//    m_dual_volume_[6] = (m_dx_[0] <= EPSILON) ? 1 : m_dx_[0];
//    m_dual_volume_[5] = (m_dx_[1] <= EPSILON) ? 1 : m_dx_[1];
//    m_dual_volume_[3] = (m_dx_[2] <= EPSILON) ? 1 : m_dx_[2];

    m_dual_volume_[4] /* 011 */= m_dual_volume_[6] * m_dual_volume_[5];
    m_dual_volume_[2] /* 101 */= m_dual_volume_[3] * m_dual_volume_[6];
    m_dual_volume_[1] /* 110 */= m_dual_volume_[5] * m_dual_volume_[3];

    m_dual_volume_[0] /* 111 */= m_dual_volume_[6] * m_dual_volume_[5] * m_dual_volume_[3];

    m_inv_volume_[7] = 1.0;

    m_inv_volume_[1/* 001 */] = (dims[0] > 1) ? 1.0 / m_volume_[1] : 0;
    m_inv_volume_[2/* 010 */] = (dims[1] > 1) ? 1.0 / m_volume_[2] : 0;
    m_inv_volume_[4/* 100 */] = (dims[2] > 1) ? 1.0 / m_volume_[4] : 0;

    m_inv_volume_[3] /* 011 */= NOT_ZERO(m_inv_volume_[1]) * NOT_ZERO(m_inv_volume_[2]);
    m_inv_volume_[5] /* 101 */= NOT_ZERO(m_inv_volume_[4]) * NOT_ZERO(m_inv_volume_[1]);
    m_inv_volume_[6] /* 110 */= NOT_ZERO(m_inv_volume_[2]) * NOT_ZERO(m_inv_volume_[4]);
    m_inv_volume_[7] /* 111 */= NOT_ZERO(m_inv_volume_[1]) * NOT_ZERO(m_inv_volume_[2]) * NOT_ZERO(m_inv_volume_[4]);

    m_inv_dual_volume_[7] = 1.0;

    m_inv_dual_volume_[6/* 110 */] = (dims[0] > 1) ? 1.0 / m_dual_volume_[6] : 0;
    m_inv_dual_volume_[5/* 101 */] = (dims[1] > 1) ? 1.0 / m_dual_volume_[5] : 0;
    m_inv_dual_volume_[3/* 001 */] = (dims[2] > 1) ? 1.0 / m_dual_volume_[3] : 0;

    m_inv_dual_volume_[4] /* 011 */= NOT_ZERO(m_inv_dual_volume_[6]) * NOT_ZERO(m_inv_dual_volume_[5]);
    m_inv_dual_volume_[2] /* 101 */= NOT_ZERO(m_inv_dual_volume_[3]) * NOT_ZERO(m_inv_dual_volume_[6]);
    m_inv_dual_volume_[1] /* 110 */= NOT_ZERO(m_inv_dual_volume_[5]) * NOT_ZERO(m_inv_dual_volume_[3]);
    m_inv_dual_volume_[0] /* 111 */=
            NOT_ZERO(m_inv_dual_volume_[6]) * NOT_ZERO(m_inv_dual_volume_[5]) * NOT_ZERO(m_inv_dual_volume_[3]);
#undef NOT_ZERO
}


}//namespace simpla
#endif //SIMPLA_GEOMETRY_H
