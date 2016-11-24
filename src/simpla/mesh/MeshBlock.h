//
// Created by salmon on 16-10-9.
//

#ifndef SIMPLA_BOX_H
#define SIMPLA_BOX_H

#include <simpla/SIMPLA_config.h>
#include <iomanip>
#include <simpla/data/DataBase.h>
#include <simpla/data/DataSpace.h>
#include <simpla/toolbox/nTuple.h>
#include <simpla/concept/Object.h>
#include <simpla/concept/Serializable.h>
#include <simpla/concept/Printable.h>
#include <simpla/toolbox/BoxUtility.h>
#include "MeshCommon.h"
#include "EntityId.h"
#include "DataBlock.h"

namespace simpla { namespace mesh
{

/**
 *  block represent a n-dims block in the index space;
 *
 *
 *   -----------------------------5
 *   |                            |
 *   |     ---------------4       |
 *   |     |              |       |
 *   |     |  ********3   |       |
 *   |     |  *       *   |       |
 *   |     |  *       *   |       |
 *   |     |  *       *   |       |
 *   |     |  2********   |       |
 *   |     1---------------       |
 *   0-----------------------------
 *
 *	5-0 = dimensions
 *	4-1 = e-d = ghosts
 *	2-1 = counts
 *
 *	0 = id_begin
 *	5 = id_end
 *
 *	1 = id_local_outer_begin
 *	4 = id_local_outer_end
 *
 *	2 = id_local_inner_begin
 *	3 = id_local_inner_end
 *
 *
 *
 *   ********************global **************
 *   *      +---------memory  ---------+     *
 *   *      |       +-----outer ---+   |     *
 *   *      |       |              |   |     *
 *   *      |       |  **inner**   |   |     *
 *   *      |       |  *       *   |   |     *
 *   *      |       |  *       *   |   |     *
 *   *      |       |  *       *   |   |     *
 *   *      |       |  *********   |   |     *
 *   *      |       +--------------+   |     *
 *   *      |                          |     *
 *   *      |                          |     *
 *   *      +--------------------------+     *
 *   *                                       *
 *   *****************************************
 *
 *    global > memory> outer > inner
 *    inner <  ghost < outer
 *
 */

class MeshBlock :
        public Object,
        public concept::Serializable,
        public concept::Printable
{

public:

    SP_OBJECT_HEAD(MeshBlock, Object)

    MeshBlock();

    MeshBlock(int ndims, index_type const *lo, index_type const *up, Real const *dx, Real const *x_lo, id_type id);

    MeshBlock(MeshBlock const &);

    MeshBlock(MeshBlock &&other);

    virtual ~MeshBlock();

    MeshBlock &operator=(MeshBlock const &other)= delete;


    template<typename TV, MeshEntityType IFORM, size_type DOF = 1> using data_block_type=  DataBlockArray<TV, IFORM, DOF>;


    template<typename TV, mesh::MeshEntityType IFORM, size_type DOF>
    std::shared_ptr<DataBlock> create_data_block(void *p) const
    {
        int n_dof = DOF;
        int ndims = 3;
        if (IFORM == mesh::EDGE || IFORM == mesh::FACE)
        {
            n_dof *= 3;
            ++ndims;
        }
        auto b = outer_index_box();
        index_type lo[4] = {std::get<0>(b)[0], std::get<0>(b)[1], std::get<0>(b)[2], 0};
        index_type hi[4] = {std::get<1>(b)[0], std::get<1>(b)[1], std::get<0>(b)[2], n_dof};
        return std::dynamic_pointer_cast<DataBlock>(
                std::make_shared<data_block_type<TV, IFORM, DOF>>(static_cast<TV *>(p), ndims, lo, hi));
    };


    virtual void initialize() { DO_NOTHING; }

    virtual void deploy();

    virtual bool is_deployed() const { return m_is_deployed_; };

    /** for Printable @{*/

    virtual std::ostream &print(std::ostream &os, int indent = 0) const;

    /** @}*/

    /** for Serializable @{*/
    virtual std::string name() const { return string_cast(id()); }

    virtual void load(const data::DataBase &) {};

    virtual void save(data::DataBase *) const {};

    /** @}*/


    /**
     *
     *   inc_level <  0 => create a new mesh in coarser  index space
     *   inc_level == 0 => create a new mesh in the same index space
     *   inc_level >  0 => create a new mesh in finer    index space
     *
     *   dx_new= dx_old* 2^ (-inc_level)
     *   offset_new = b.first * 2^ (-inc_level)
     *   count_new  = b.second * 2^ (-inc_level) - offset_new
     */
    virtual std::shared_ptr<MeshBlock>
    create(int inc_level, const index_type *lo, const index_type *hi) const;

    virtual std::shared_ptr<MeshBlock>
    create(int inc_level, index_box_type const &b) const
    {
        return create(inc_level, &std::get<0>(b)[0], &std::get<1>(b)[0]);
    }

    /**
     * create a sub-mesh of this mesh, with same m_root_id
     * @param other_box
     */
    std::shared_ptr<MeshBlock> intersection(index_box_type const &other_box, int inc_level = 0);


    int level() const { return m_level_; }

    virtual bool is_overlap(index_box_type const &) { return true; }

    virtual bool is_overlap(box_type const &) { return true; }

    virtual bool is_overlap(MeshBlock const &) { return true; }

    /**
     *  Set unique ID of index space
     * @param id
     */


    void shift(index_type x, index_type y = 0, index_type z = 0) {}

    void shift(index_type const *) {}


    virtual bool is_valid()
    {
        return m_is_deployed_ &&
               toolbox::is_valid(m_g_box_) &&
               toolbox::is_valid(m_m_box_) &&
               toolbox::is_valid(m_inner_box_) &&
               toolbox::is_valid(m_outer_box_);
    }

    size_tuple dimensions() const { return toolbox::dimensions(m_g_box_); }

    size_tuple const &ghost_width() const { return m_ghost_width_; }

    index_box_type const &global_index_box() const { return m_g_box_; }

    index_box_type const &memory_index_box() const { return m_m_box_; }

    index_box_type const &inner_index_box() const { return m_inner_box_; }

    index_box_type const &outer_index_box() const { return m_outer_box_; }

    box_type
    get_box(index_box_type const &b) const
    {
        return std::make_tuple(point(std::get<0>(b)), point(std::get<1>(b)));
    }

    virtual box_type box() const { return get_box(inner_index_box()); };

    virtual box_type global_box() const { return get_box(global_index_box()); };

    virtual box_type memory_box() const { return get_box(memory_index_box()); };

    virtual box_type inner_box() const { return get_box(inner_index_box()); };

    virtual box_type outer_box() const { return get_box(outer_index_box()); };

    point_type const &global_origin() const { return m_global_origin_; }

    point_type const &dx() const { return m_dx_; }

    point_type const &inv_dx() const { return m_inv_dx_; }

    virtual point_type point(index_type x, index_type y = 0, index_type z = 0) const
    {
        return point_type{std::fma(x, m_dx_[0], m_global_origin_[0]),
                          std::fma(y, m_dx_[1], m_global_origin_[1]),
                          std::fma(z, m_dx_[2], m_global_origin_[2])};
    };

//    virtual point_type point(MeshEntityId const &s) const { return point(s.x >> 1, s.y >> 1, s.z >> 1); }
//
    virtual point_type point(index_tuple const &x) const { return point(x[0], x[1], x[2]); };
//
//    virtual index_tuple index(point_type const &x) const
//    {
//        return index_tuple{static_cast<index_type>(floor((x[0] + 0.5 * m_dx_[0]) * m_inv_dx_[0])),
//                           static_cast<index_type>(floor((x[1] + 0.5 * m_dx_[0]) * m_inv_dx_[1])),
//                           static_cast<index_type>(floor((x[2] + 0.5 * m_dx_[0]) * m_inv_dx_[2]))
//        };
//    }
//
//    virtual point_type point_global_to_local(point_type const &x, int iform = 0) const
//    {
//        return point_type{static_cast<Real>(x[0] - floor((x[0] + 0.5 * m_dx_[0]) * m_inv_dx_[0])),
//                          static_cast<Real>(x[1] - floor((x[1] + 0.5 * m_dx_[0]) * m_inv_dx_[1])),
//                          static_cast<Real>(x[2] - floor((x[2] + 0.5 * m_dx_[0]) * m_inv_dx_[2]))};
//    }


    size_type number_of_entities(int iform) const
    {
        return max_hash() * ((iform == VERTEX || iform == VOLUME) ? 1 : 3);
    }

    size_type max_hash() const { return toolbox::size(m_m_box_); }

    typedef MeshEntityIdCoder m;


    virtual int get_adjacent_entities(MeshEntityType entity_type, MeshEntityId s, MeshEntityId *p = nullptr) const
    {
        return m::get_adjacent_entities(entity_type, entity_type, s, p);
    }

//    virtual index_tuple point_to_index(point_type const &g, int nId = 0) const
//    {
//        return m::unpack_index(std::get<0>(m::point_global_to_local(g, nId)));
//    };


    virtual point_type
    point(MeshEntityId const &s) const
    {
        point_type p = m::point(s);

        p[0] = std::fma(p[0], m_l2g_scale_[0], m_l2g_shift_[0]);
        p[1] = std::fma(p[1], m_l2g_scale_[1], m_l2g_shift_[1]);
        p[2] = std::fma(p[2], m_l2g_scale_[2], m_l2g_shift_[2]);

        return std::move(p);

    }

    virtual point_type
    point_local_to_global(MeshEntityId s, point_type const &r) const
    {
        point_type p = m::point_local_to_global(s, r);

        p[0] = std::fma(p[0], m_l2g_scale_[0], m_l2g_shift_[0]);
        p[1] = std::fma(p[1], m_l2g_scale_[1], m_l2g_shift_[1]);
        p[2] = std::fma(p[2], m_l2g_scale_[2], m_l2g_shift_[2]);

        return std::move(p);
    }

    virtual    //std::tuple<MeshEntityId, point_type>
    point_type
    point_global_to_local(point_type const &g, int nId = 0) const
    {

        return
//                m::point_global_to_local(
                point_type{
                        std::fma(g[0], m_g2l_scale_[0], m_g2l_shift_[0]),
                        std::fma(g[1], m_g2l_scale_[1], m_g2l_shift_[1]),
                        std::fma(g[2], m_g2l_scale_[2], m_g2l_shift_[2])
                }
//                        , nId)
                ;
    }

    virtual index_tuple
    point_to_index(point_type const &g, int nId = 0) const
    {
        return m::unpack_index(std::get<0>(m::point_global_to_local(
                point_type{
                        std::fma(g[0], m_g2l_scale_[0], m_g2l_shift_[0]),
                        std::fma(g[1], m_g2l_scale_[1], m_g2l_shift_[1]),
                        std::fma(g[2], m_g2l_scale_[2], m_g2l_shift_[2])
                }, nId)));
    };

    virtual EntityIdRange range(MeshEntityType entityType, MeshZoneTag status = SP_ES_ALL, index_type dof = 1) const;

    virtual EntityIdRange range(MeshEntityType entityType, index_box_type const &b, index_type dof = 1) const;

    virtual EntityIdRange range(MeshEntityType entityType, box_type const &b, index_type dof = 1) const;

    template<typename TFun>
    void foreach(MeshEntityType const &iform, MeshZoneTag tag, TFun const &fun) const
    {
        int n = iform == VERTEX || iform == VOLUME ? 1 : 3;
        index_type ib = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[0] : std::get<0>(m_outer_box_)[0];
        index_type ie = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[0] : std::get<1>(m_outer_box_)[0];
        index_type jb = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[1] : std::get<0>(m_outer_box_)[1];
        index_type je = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[1] : std::get<1>(m_outer_box_)[1];
        index_type kb = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[2] : std::get<0>(m_outer_box_)[2];
        index_type ke = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[2] : std::get<1>(m_outer_box_)[2];

        //#pragma omp parallel for
        for (index_type i = ib; i < ie; ++i)
            for (index_type j = jb; j < je; ++j)
                for (index_type k = kb; k < ke; ++k)
                    for (index_type l = 0; l < n; ++l)
                    {
                        fun(i, j, k, l);
                    }

    }

    template<typename TFun>
    void foreach(MeshZoneTag tag, TFun const &fun) const
    {
        index_type ib = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[0] : std::get<0>(m_outer_box_)[0];
        index_type ie = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[0] : std::get<1>(m_outer_box_)[0];
        index_type jb = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[1] : std::get<0>(m_outer_box_)[1];
        index_type je = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[1] : std::get<1>(m_outer_box_)[1];
        index_type kb = tag == SP_ES_LOCAL ? std::get<0>(m_inner_box_)[2] : std::get<0>(m_outer_box_)[2];
        index_type ke = tag == SP_ES_LOCAL ? std::get<1>(m_inner_box_)[2] : std::get<1>(m_outer_box_)[2];

        //#pragma omp parallel for
        for (index_type i = ib; i < ie; ++i)
            for (index_type j = jb; j < je; ++j)
                for (index_type k = kb; k < ke; ++k)
                {
                    fun(i, j, k);
                }

    }

    virtual bool is_inside(point_type const &p) const { return toolbox::is_inside(p, box()); }

    virtual bool is_inside(index_tuple const &p) const { return toolbox::is_inside(p, m_inner_box_); }

protected:
    bool m_is_deployed_ = false;

    id_type m_space_id_ = 0;

    int m_level_ = 0;

    int m_ndims_;

    point_type m_dx_{{1, 1, 1}};

    point_type m_inv_dx_{{1, 1, 1}};

    point_type m_global_origin_{{0, 0, 0}};

    size_tuple m_ghost_width_{{0, 0, 0}};        //!<     ghost width
    index_box_type m_g_box_;         //!<     global index block
    index_box_type m_m_box_;         //!<     memory index block
    index_box_type m_inner_box_;     //!<    inner block
    index_box_type m_outer_box_;     //!<    outer block

    vector_type m_l2g_scale_{{1, 1, 1}}, m_l2g_shift_{{0, 0, 0}};
    vector_type m_g2l_scale_{{1, 1, 1}}, m_g2l_shift_{{0, 0, 0}};


};

}} //namespace simpla{namespace mesh_as
#endif //SIMPLA_BOX_H
