//
// Created by salmon on 17-4-24.
//

#ifndef SIMPLA_STRUCTUREDMESH_H
#define SIMPLA_STRUCTUREDMESH_H

#include <simpla/algebra/Array.h>
#include <simpla/algebra/all.h>
#include <simpla/data/all.h>
#include <simpla/engine/Attribute.h>
#include <simpla/geometry/Chart.h>
namespace simpla {
namespace mesh {

/**
 *  Structured Mesh
 *  - index space and local coordinates have same origin coordinates
 *
 */
class StructuredMesh {
    SP_OBJECT_BASE(StructuredMesh)
   public:
    static constexpr unsigned int NDIMS = 3;
    typedef EntityId entity_id_type;

    StructuredMesh();
    ~StructuredMesh();
    StructuredMesh(StructuredMesh const &) = delete;
    StructuredMesh(StructuredMesh &&) = delete;
    StructuredMesh &operator=(StructuredMesh const &) = delete;
    StructuredMesh &operator=(StructuredMesh &&) = delete;

    point_type map(point_type const &p) const;

    void SetChart(std::shared_ptr<geometry::Chart> const &);
    geometry::Chart const *GetChart() const;

    void SetBlock(const engine::MeshBlock &);
    const engine::MeshBlock &GetBlock() const;
    id_type GetBlockId() const;

    point_type GetCellWidth() const;
    point_type GetOrigin() const;
    box_type GetBox() const;

    index_tuple GetIndexOrigin() const;
    size_tuple GetDimensions() const;

    index_tuple GetGhostWidth(int tag = VERTEX) const;

    index_box_type GetIndexBox(int tag) const;

    point_type point(entity_id_type s) const;

    virtual point_type local_coordinates(entity_id_type s, Real const *r) const;
    template <typename... Args>
    point_type global_coordinates(Args &&... args) const {
        return map(local_coordinates(std::forward<Args>(args)...));
    }

    ZSFC<NDIMS> GetSpaceFillingCurve(int iform, int nsub = 0) const {
        return ZSFC<NDIMS>{GetIndexBox(EntityIdCoder::m_sub_index_to_id_[iform][nsub])};
    }

   protected:
    point_type m_dx_{1, 1, 1};
    point_type m_x0_{0, 0, 0};

   public:
    size_type GetNumberOfEntity(int IFORM = VERTEX) const {
        index_box_type m_index_box_ = GetBlock().GetIndexBox();
        return calculus::reduction<tags::multiplication>(std::get<1>(m_index_box_) - std::get<0>(m_index_box_)) *
               ((IFORM == VERTEX || IFORM == VOLUME) ? 1 : 3);
    }

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};
}  // namespace mesh {

namespace engine {

template <typename TM>
struct domain_traits<TM, std::enable_if_t<std::is_base_of<mesh::StructuredMesh, TM>::value>> {
    static constexpr int NDIMS = 3;
    typedef EntityId entity_id_type;
    typedef engine::Attribute attribute_type;
    template <typename U>
    using array_type = Array<U, ZSFC<NDIMS>>;
};
}  // namespace engine{
}  // namespace simpla {

#endif  // SIMPLA_STRUCTUREDMESH_H
