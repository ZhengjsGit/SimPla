//
// Created by salmon on 17-7-16.
//

#ifndef SIMPLA_MESH_H
#define SIMPLA_MESH_H

#include "simpla/SIMPLA_config.h"

#include "simpla/data/EnableCreateFromDataTable.h"

#include "Attribute.h"
#include "SPObject.h"

#include "PoliciesCommon.h"

namespace simpla {
namespace geometry {
struct Chart;
struct GeoObject;
}
namespace engine {
class MeshBlock;
class Patch;

struct MeshBase : public SPObject, public AttributeGroup, public data::EnableCreateFromDataTable<MeshBase> {
    SP_OBJECT_HEAD(MeshBase, SPObject)
    DECLARE_REGISTER_NAME(MeshBase)
   public:
    using AttributeGroup::attribute_type;

    MeshBase();
    ~MeshBase() override;

    MeshBase(MeshBase const &other) = delete;
    MeshBase(MeshBase &&other) noexcept = delete;
    void swap(MeshBase &other) = delete;
    MeshBase &operator=(this_type const &other) = delete;
    MeshBase &operator=(this_type &&other) noexcept = delete;

    virtual const geometry::Chart &GetChart() const = 0;
    virtual geometry::Chart &GetChart() = 0;

    virtual this_type *GetMesh() { return this; }
    virtual this_type const *GetMesh() const { return this; }

    virtual void AddEmbeddedBoundary(std::string const &prefix, const std::shared_ptr<geometry::GeoObject> &g){};

    virtual index_box_type GetIndexBox(int tag = 0) const;
    virtual box_type GetBox() const;

    void SetBlock(const MeshBlock &blk);
    virtual const MeshBlock &GetBlock() const;
    virtual id_type GetBlockId() const;

    std::shared_ptr<data::DataTable> Serialize() const override;
    void Deserialize(std::shared_ptr<data::DataTable> const &t) override;

    void DoInitialize() override;
    void DoFinalize() override;
    void DoUpdate() override;
    void DoTearDown() override;

    virtual void DoInitialCondition(Real time_now) {}
    virtual void DoBoundaryCondition(Real time_now, Real dt) {}
    virtual void DoAdvance(Real time_now, Real dt) {}

    void InitialCondition(Real time_now);
    void BoundaryCondition(Real time_now, Real dt);
    void Advance(Real time_now, Real dt);

    void Pull(Patch *p) override;
    void Push(Patch *p) override;

    void InitialCondition(Patch *patch, Real time_now);
    void BoundaryCondition(Patch *patch, Real time_now, Real dt);
    void Advance(Patch *patch, Real time_now, Real dt);

    void SetRange(std::string const &, Range<EntityId> const &);
    Range<EntityId> &GetRange(std::string const &k);
    Range<EntityId> GetRange(std::string const &k) const;

   private:
    MeshBlock m_mesh_block_;
    std::shared_ptr<geometry::Chart> m_chart_ = nullptr;

    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};

template <typename TChart, template <typename> class... Policies>
class Mesh : public MeshBase, public Policies<Mesh<TChart, Policies...>>... {
   public:
    DECLARE_REGISTER_NAME(Mesh)

    typedef Mesh<TChart, Policies...> this_type;
    Mesh() : Policies<this_type>(this)... {};
    ~Mesh() override = default;

    TChart &GetChart() override { return m_chart_; };
    const TChart &GetChart() const override { return m_chart_; };

    const engine::MeshBlock &GetBlock() const override { return MeshBase::GetBlock(); }

    this_type *GetMesh() override { return this; }
    this_type const *GetMesh() const override { return this; }

    void DoInitialCondition(Real time_now) override;
    void DoBoundaryCondition(Real time_now, Real dt) override;
    void DoAdvance(Real time_now, Real dt) override;

    void Deserialize(std::shared_ptr<data::DataTable> const &cfg) override;
    std::shared_ptr<data::DataTable> Serialize() const override;

    void AddEmbeddedBoundary(std::string const &prefix, const std::shared_ptr<geometry::GeoObject> &g) override;

    template <typename TL, typename TR>
    void Fill(TL &lhs, TR &&rhs) const {
        FillRange(lhs, std::forward<TR>(rhs), Range<EntityId>{}, true);
    };

    template <typename TL, typename TR>
    void FillRange(TL &lhs, TR &&rhs, Range<EntityId> r = Range<EntityId>{},
                   bool full_fill_if_range_is_null = false) const;

    template <typename TL, typename TR>
    void FillRange(TL &lhs, TR &&rhs, std::string const &k = "", bool full_fill_if_range_is_null = false) const {
        FillRange(lhs, std::forward<TR>(rhs), GetRange(k), full_fill_if_range_is_null);
    };

    template <typename TL, typename TR>
    void FillBody(TL &lhs, TR &&rhs, std::string const &prefix = "") const {
        FillRange(lhs, std::forward<TR>(rhs), prefix + "_BODY_" + std::to_string(TL::iform), true);
    };

    template <typename TL, typename TR>
    void FillBoundary(TL &lhs, TR &&rhs, std::string const &prefix = "") const {
        FillRange(lhs, std::forward<TR>(rhs), prefix + "_BOUNDARY_" + std::to_string(TL::iform), false);
    };

   private:
    TChart m_chart_;
};

namespace _detail {
DEFINE_INVOKE_HELPER(SetEmbeddedBoundary)
DEFINE_INVOKE_HELPER(Calculate)
}

template <typename TM, template <typename> class... Policies>
void Mesh<TM, Policies...>::DoInitialCondition(Real time_now) {
    traits::_try_invoke_InitialCondition<Policies...>(this, time_now);
}
template <typename TM, template <typename> class... Policies>
void Mesh<TM, Policies...>::DoBoundaryCondition(Real time_now, Real dt) {
    traits::_try_invoke_BoundaryCondition<Policies...>(this, time_now, dt);
}
template <typename TM, template <typename> class... Policies>
void Mesh<TM, Policies...>::DoAdvance(Real time_now, Real dt) {
    traits::_try_invoke_Advance<Policies...>(this, time_now, dt);
}
template <typename TM, template <typename> class... Policies>
std::shared_ptr<data::DataTable> Mesh<TM, Policies...>::Serialize() const {
    auto res = MeshBase::Serialize();
    traits::_try_invoke_Serialize<Policies...>(this, res.get());
    return res;
};
template <typename TM, template <typename> class... Policies>
void Mesh<TM, Policies...>::Deserialize(std::shared_ptr<data::DataTable> const &cfg) {
    traits::_try_invoke_Deserialize<Policies...>(this, cfg);
    MeshBase::Deserialize(cfg);
};
template <typename TM, template <typename> class... Policies>
void Mesh<TM, Policies...>::AddEmbeddedBoundary(std::string const &prefix,
                                                const std::shared_ptr<geometry::GeoObject> &g) {
    _detail::_try_invoke_SetEmbeddedBoundary<Policies...>(this, prefix, g);
};

template <typename TM, template <typename> class... Policies>
template <typename LHS, typename RHS>
void Mesh<TM, Policies...>::FillRange(LHS &lhs, RHS &&rhs, Range<EntityId> r, bool full_fill_if_range_is_null) const {
    if (r.isNull() && full_fill_if_range_is_null) {
        _detail::_try_invoke_once_Calculate<Policies...>(this, lhs, std::forward<RHS>(rhs));
    } else {
        _detail::_try_invoke_once_Calculate<Policies...>(this, lhs, std::forward<RHS>(rhs), r);
    }
};

}  // namespace mesh
}  // namespace simpla{

#endif  // SIMPLA_MESH_H
