/**
 * @file MeshBlock.h
 * @author salmon
 * @date 2015-10-28.
 */

#ifndef SIMPLA_MESH_MESHBLOCK_H
#define SIMPLA_MESH_MESHBLOCK_H

#include "../gtl/macro.h"
#include "../gtl/primitives.h"
#include "../gtl/ntuple.h"
#include "../gtl/type_traits.h"
#include "../gtl/Log.h"
#include "MeshIds.h"
#include "../model/GeoAlgorithm.h"

namespace simpla { namespace mesh
{

struct MeshBlock : public MeshIDs, public base::Object
{


    SP_OBJECT_HEAD(MeshBlock, base::Object)

private:

    typedef MeshBlock this_type;
    typedef MeshIDs m;

public:

    static constexpr int ndims = 3;


    using typename m::id_type;
    using typename m::id_tuple;
    using typename m::index_type;
    using typename m::range_type;
    using typename m::iterator;





    /**
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
 *	0 = m_idx_min_
 *	5 = m_idx_max_
 *
 *	1 = m_idx_memory_min_
 *	4 = m_idx_memory_max_
 *
 *	2 = m_idx_local_min_
 *	3 = m_idx_local_max_
 *
 *
 */

    index_tuple m_idx_min_{0, 0, 0};
    index_tuple m_idx_max_{1, 1, 1};
    index_tuple m_idx_local_min_{0, 0, 0};
    index_tuple m_idx_local_max_{1, 1, 1};
    index_tuple m_idx_memory_min_{0, 0, 0};
    index_tuple m_idx_memory_max_{1, 1, 1};

    index_tuple m_dimensions_{1, 1, 1};
    index_tuple m_ghost_width_{0, 0, 0};
    int m_ndims_ = 3;

public:

    MeshBlock();

    virtual  ~MeshBlock();

    MeshBlock(this_type const &other) = delete;


    void dimensions(index_tuple const &d);

    void ghost_width(index_tuple const &d);


    void decompose(index_tuple const &dist_dimensions, index_tuple const &dist_coord);


    virtual void deploy() { deploy2(); }

private:
    void deploy2();

public:


    int number_of_dims() const { return m_ndims_; }

    index_tuple const &dimensions() const { return m_dimensions_; }

    index_tuple const &ghost_width() const { return m_ghost_width_; }


    bool is_periodic(int n) const { return m_ghost_width_[n % 3] == 0; }

    /**
     *  remove periodic axis, which  ghost_width==0
     */
    id_type periodic_id_mask() const
    {
        id_type M0 = ((1UL << ID_DIGITS) - 1);
        id_type M1 = ((1UL << (MESH_RESOLUTION)) - 1);
        return FULL_OVERFLOW_FLAG
               | (is_periodic(0) ? M1 : M0)
               | ((is_periodic(1) ? M1 : M0) << ID_DIGITS)
               | ((is_periodic(2) ? M1 : M0) << (ID_DIGITS * 2));
    }

    size_t id_mask() const
    {
        id_type M0 = ((1UL << ID_DIGITS) - 1);
        id_type M1 = ((1UL << (MESH_RESOLUTION)) - 1);
        return FULL_OVERFLOW_FLAG
               | ((m_idx_max_[0] - m_idx_min_[0] > 1) ? M0 : M1)
               | (((m_idx_max_[1] - m_idx_min_[1] > 1) ? M0 : M1) << ID_DIGITS)
               | (((m_idx_max_[2] - m_idx_min_[2] > 1) ? M0 : M1) << (ID_DIGITS * 2));
    }


    index_box_type index_box() const
    {
        return std::make_tuple(m_idx_min_, m_idx_max_);
    }


//    index_box_type cell_index_box(id_type const &s) const
//    {
//        return std::make_tuple(m::unpack_index(s - _DA), m::unpack_index(s + _DA));
//    }
    index_box_type index_box(box_type const &b) const
    {

        point_type b0, b1, x0, x1;

        std::tie(b0, b1) = local_index_box();

        std::tie(x0, x1) = b;

        if (geometry::box_intersection(b0, b1, &x0, &x1))
        {
            return std::make_tuple(m::unpack_index(id(x0)),
                                   m::unpack_index(id(x1) + (m::_DA << 1)));

        }
        else
        {
            index_tuple i0, i1;
            i0 = 0;
            i1 = 0;
            return std::make_tuple(i0, i1);
        }

    }

    index_box_type local_index_box() const
    {
        return (std::make_tuple(m_idx_local_min_, m_idx_local_max_));
    }

    index_box_type memory_index_box() const
    {
        return (std::make_tuple(m_idx_memory_min_, m_idx_memory_max_));
    }

    bool in_box(index_tuple const &x) const
    {
        return (m_idx_local_min_[1] <= x[1]) && (m_idx_local_min_[2] <= x[2]) && (m_idx_local_min_[0] <= x[0])  //
               && (m_idx_local_max_[1] > x[1]) && (m_idx_local_max_[2] > x[2]) && (m_idx_local_max_[0] > x[0]);

    }

    bool in_box(id_type s) const { return in_box(m::unpack_index(s)); }

    template<int IFORM>
    range_type range() const
    {
        return m::template make_range<IFORM>(m_idx_local_min_, m_idx_local_max_);
    }

    template<int IFORM>
    range_type inner_range() const
    {
        index_tuple i_min = m_idx_local_min_;
        index_tuple i_max = m_idx_local_max_;
        for (int n = 0; n < ndims; ++n)
        {
            if (i_max[n] - i_min[n] > 2 * m_idx_min_[n])
            {
                i_min[n] += m_idx_min_[n];
                i_max[n] -= m_idx_min_[n];
            }

        }
        return m::template make_range<IFORM>(i_min, i_max);
    }

    template<int IFORM>
    range_type outer_range() const
    {
        return m::template make_range<IFORM>(m_idx_memory_min_, m_idx_memory_max_);
    }


    template<int IFORM>
    size_t max_hash() const
    {
        return static_cast<size_t>(m::hash(
                m::pack_index(m_idx_memory_max_ - 1, m::template sub_index_to_id<IFORM>(3UL)),
                m_idx_memory_min_, m_idx_memory_max_));
    }


    size_t hash(id_type const &s) const
    {
        return static_cast<size_t>(m::hash(s, m_idx_memory_min_, m_idx_memory_max_));
    }


    index_box_type const &center_box() const { return m_center_box_; }

    std::vector<index_box_type> const &boundary_box() const { return m_boundary_box_; }

    std::vector<index_box_type> const &ghost_box() const { return m_ghost_box_; }




    //================================================================================================
    // @name Coordinates dependent
private:
    point_type m_min_;
    point_type m_max_;
    vector_type m_dx_;
public:

    void box(box_type const &b) { std::tie(m_min_, m_max_) = b; }

    box_type box() const { return (std::make_tuple(m_min_, m_max_)); }

    vector_type const &dx() const { return m_dx_; }

    box_type cell_box(id_type const &s) const
    {
        return std::make_tuple(point(s - m::_DA), point(s + m::_DA));
    }

    int get_vertices(size_t node_id, id_type s, point_type *p = nullptr) const
    {

        int num = m::get_adjacent_cells(VERTEX, node_id, s);

        if (p != nullptr)
        {
            id_type neighbour[num];

            m::get_adjacent_cells(VERTEX, node_id, s, neighbour);

            for (int i = 0; i < num; ++i)
            {
                p[i] = point(neighbour[i]);
            }

        }

        return num;
    }

    void get_volumes(Real *v, Real *inv_v, Real *dual_v, Real *inv_dual_v);

private:
    /**
     * @name  Coordinate map
     * @{
     *
     *        Topology mesh       geometry mesh
     *                        map
     *              M      ---------->      G
     *              x                       y
     **/

    point_type m_map_orig_ = {0, 0, 0};

    point_type m_map_scale_ = {1, 1, 1};

    point_type m_inv_map_orig_ = {0, 0, 0};

    point_type m_inv_map_scale_ = {1, 1, 1};


    point_type inv_map(point_type const &x) const
    {

        point_type res;

        res[0] = std::fma(x[0], m_inv_map_scale_[0], m_inv_map_orig_[0]);

        res[1] = std::fma(x[1], m_inv_map_scale_[1], m_inv_map_orig_[1]);

        res[2] = std::fma(x[2], m_inv_map_scale_[2], m_inv_map_orig_[2]);

        return std::move(res);
    }

    point_type map(point_type const &y) const
    {

        point_type res;


        res[0] = std::fma(y[0], m_map_scale_[0], m_map_orig_[0]);

        res[1] = std::fma(y[1], m_map_scale_[1], m_map_orig_[1]);

        res[2] = std::fma(y[2], m_map_scale_[2], m_map_orig_[2]);

        return std::move(res);
    }

public:

    virtual point_type point(id_type const &s) const
    {
        return std::move(map(m::point(s)));
    }

    virtual point_type coordinates_local_to_global(id_type s, point_type const &x) const
    {
        return std::move(map(m::coordinates_local_to_global(s, x)));
    }

    virtual point_type coordinates_local_to_global(std::tuple<id_type, point_type> const &t) const
    {
        return std::move(map(m::coordinates_local_to_global(t)));
    }

    virtual std::tuple<id_type, point_type> coordinates_global_to_local(point_type const &x, int n_id = 0) const
    {
        return std::move(m::coordinates_global_to_local(inv_map(x), n_id));
    }

    virtual id_type id(point_type const &x, int n_id = 0) const
    {
        return std::get<0>(m::coordinates_global_to_local(inv_map(x), n_id));
    }


private:
    index_box_type m_center_box_;
    std::vector<index_box_type> m_boundary_box_;
    std::vector<index_box_type> m_ghost_box_;

}; // struct MeshBlock

}}//namespace simpla//namespace mesh
#endif //SIMPLA_MESH_MESHBLOCK_H