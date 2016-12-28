//
// Created by salmon on 16-11-19.
//

#ifndef SIMPLA_GEOMETRY_H
#define SIMPLA_GEOMETRY_H

#include <simpla/design_pattern/Observer.h>
#include <simpla/mesh/MeshBlock.h>

namespace simpla { namespace mesh
{
class Patch;

/**
 *  Define:
 *   A bundle is a triple $(E, p, B)$ where $E$, $B$ are sets and $p:E→B$ a map
 *   - $E$ is called the total space
 *   - $B$ is the base space of the bundle
 *   - $p$ is the projection
 *
 */
class Chart :
        public concept::Printable,
        public concept::LifeControllable
{
public:
    SP_OBJECT_BASE(Chart);

    Chart();

    virtual ~Chart();

    virtual std::ostream &print(std::ostream &os, int indent) const;

    virtual void accept(Patch *)=0;

    virtual void pre_process();

    virtual void post_process();

    virtual void initialize(Real data_time = 0, Real dt = 0);

    virtual void finalize(Real data_time = 0, Real dt = 0);


    virtual std::shared_ptr<MeshBlock> const &mesh_block() const
    {
        ASSERT(m_mesh_block_ != nullptr);
        return m_mesh_block_;
    }

    virtual point_type point(index_type i, index_type j, index_type k) const { return m_mesh_block_->point(i, j, k); };

    template<typename TV, size_type IFORM, size_type DOF = 1> using data_block_type=mesh::DataBlockArray<TV, IFORM, DOF>;

    size_type size(size_type IFORM = VERTEX, size_type DOF = 1) const
    {
        return 1;// return m_dims_[0] * m_dims_[1] * m_dims_[2] * DOF * ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3);
    }

    template<typename TV, size_type IFORM, size_type DOF>
    bool create_data_block(std::shared_ptr<data_block_type<TV, IFORM, DOF> > *p, void *d = nullptr) const
    {
        if (p == nullptr || (*p) != nullptr) { return false; }
        else
        {
            size_type s = size(IFORM, DOF);
            *p = sp_alloc_array<TV>(s);
        }
    };

protected:

    std::shared_ptr<MeshBlock> m_mesh_block_;

};


}}//namespace simpla { namespace mesh

#endif //SIMPLA_GEOMETRY_H
