//
// Created by salmon on 16-11-19.
//

#ifndef SIMPLA_GEOMETRY_H
#define SIMPLA_GEOMETRY_H

#include <simpla/concept/Printable.h>
#include <simpla/geometry/GeoObject.h>
#include "Attribute.h"
#include "SPObject.h"
#include "simpla/data/EnableCreateFromDataTable.h"

namespace simpla {
namespace engine {
class MeshBlock;
class Patch;
class Chart;
class Mesh;

template <typename TChart>
class MeshView;

/**
 *  Define:
 *   A bundle is a triple \f$(E, p, B)\f$ where \f$E\f$, \f$B\f$ are sets and \f$p:E \rightarrow B\f$ a map
 *   - \f$E\f$ is called the total space
 *   - \f$B\f$ is the base space of the bundle
 *   - \f$p\f$ is the projection
 *
 */
class Mesh : public AttributeGroup, public data::Serializable, public data::EnableCreateFromDataTable<Mesh> {
    SP_OBJECT_BASE(Mesh);

   public:
    explicit Mesh(std::shared_ptr<Chart> c = nullptr);
    ~Mesh() override;

    Mesh(Mesh const &) = delete;
    Mesh(Mesh &&) = delete;
    Mesh &operator=(Mesh const &) = delete;
    Mesh &operator=(Mesh &&sss) = delete;

    std::shared_ptr<data::DataTable> Serialize() const override;
    void Deserialize(std::shared_ptr<data::DataTable> t) override;

    virtual Range<EntityId> GetRange(int iform) const;

    void Push(Patch *p) override;
    void Pop(Patch *p) override;

    virtual void InitializeData(Real time_now);

    virtual void SetUp();
    virtual void TearDown();
    virtual void Initialize();
    virtual void Finalize();

    Real GetTime() const;
    id_type GetBlockId() const;
    void SetBlock(std::shared_ptr<MeshBlock>);
    std::shared_ptr<MeshBlock> GetBlock() const;

    void SetGeoObject(std::shared_ptr<geometry::GeoObject>);
    std::shared_ptr<geometry::GeoObject> GetGeoObject() const;

    void SetChart(std::shared_ptr<Chart> c);
    std::shared_ptr<Chart> GetChart() const;

   protected:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};

}  // namespace engine
}  // namespace simpla

#endif  // SIMPLA_GEOMETRY_H
