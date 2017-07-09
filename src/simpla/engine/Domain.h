//
// Created by salmon on 17-4-5.
//

#ifndef SIMPLA_DOMAIN_H
#define SIMPLA_DOMAIN_H

#include <simpla/data/all.h>
#include <simpla/utilities/Signal.h>
#include <memory>
#include "Attribute.h"

namespace simpla {

class MeshBase;
template <typename...>
class EBMesh;

namespace model {
class GeoObject;
}
namespace engine {
class Patch;
class AttributeGroup;

/**
* @brief
*/
class Domain : public SPObject,
               public AttributeGroup,
               public data::EnableCreateFromDataTable<Domain, std::shared_ptr<MeshBase> const &,
                                                      std::shared_ptr<model::GeoObject> const &> {
    SP_OBJECT_HEAD(Domain, SPObject)
   public:
    explicit Domain(const std::shared_ptr<MeshBase> &m, const std::shared_ptr<model::GeoObject> &g);
    ~Domain() override;
    Domain(Domain const &other);
    Domain(Domain &&other) noexcept;
    void swap(Domain &other);
    Domain &operator=(this_type const &other) {
        Domain(other).swap(*this);
        return *this;
    }
    Domain &operator=(this_type &&other) noexcept {
        Domain(other).swap(*this);
        return *this;
    }

    DECLARE_REGISTER_NAME(Domain)

    std::shared_ptr<data::DataTable> Serialize() const override;
    void Deserialize(const std::shared_ptr<data::DataTable> &t) override;

    void Pull(Patch *) override;
    void Push(Patch *) override;

    std::string GetDomainPrefix() const override;

    MeshBase const *GetMesh() const override;
    MeshBase const *GetBodyMesh() const override;
    MeshBase const *GetBoundaryMesh() const override;

    template <typename TL, typename TR>
    void Fill(TL &lhs, TR &&rhs) const {
        FillBody(lhs, std::forward<TR>(rhs));
    };

    template <typename LHS, typename RHS>
    void FillBody(LHS &lhs, RHS &&rhs) const {
        if (GetBodyMesh() != nullptr) {
            dynamic_cast<typename LHS::mesh_type const *>(GetBodyMesh())->FillBody(lhs, std::forward<RHS>(rhs));
        }
    }
    template <typename LHS, typename RHS>
    void FillBoundary(LHS &lhs, RHS &&rhs) const {
        if (GetBoundaryMesh() != nullptr) {
            dynamic_cast<EBMesh<typename LHS::mesh_type> const *>(GetBodyMesh())->FillBody(lhs, std::forward<RHS>(rhs));
        }
    }
    engine::Domain *GetDomain() { return this; }
    engine::Domain const *GetDomain() const { return this; }

    void SetGeoObject(std::shared_ptr<model::GeoObject> g);
    const model::GeoObject *GetGeoObject() const;

    void DoInitialize() override;
    void DoFinalize() override;
    void DoUpdate() override;
    void DoTearDown() override;

    design_pattern::Signal<void(Domain *, Real)> PreInitialCondition;
    design_pattern::Signal<void(Domain *, Real)> PostInitialCondition;
    design_pattern::Signal<void(Domain *, Real, Real)> PreBoundaryCondition;
    design_pattern::Signal<void(Domain *, Real, Real)> PostBoundaryCondition;
    design_pattern::Signal<void(Domain *, Real, Real)> PreAdvance;
    design_pattern::Signal<void(Domain *, Real, Real)> PostAdvance;

    virtual void DoInitialCondition(Real time_now) {}
    virtual void DoBoundaryCondition(Real time_now, Real dt) {}
    virtual void DoAdvance(Real time_now, Real dt) {}

    void InitialCondition(Real time_now);
    void BoundaryCondition(Real time_now, Real dt);
    void Advance(Real time_now, Real dt);

    void InitialCondition(Patch *, Real time_now);
    void BoundaryCondition(Patch *, Real time_now, Real dt);
    void Advance(Patch *, Real time_now, Real dt);

    template <typename T>
    T GetAttribute(std::string const &k) const {
        return T(AttributeGroup::Get(k)->cast_as<T>());
    };

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};

#define DOMAIN_HEAD(_DOMAIN_NAME_, _MESH_TYPE_)                                              \
   public:                                                                                   \
    template <typename... Args>                                                              \
    explicit _DOMAIN_NAME_(Args &&... args) : engine::Domain(std::forward<Args>(args)...) {} \
    ~_DOMAIN_NAME_() override = default;                                                     \
    SP_DEFAULT_CONSTRUCT(_DOMAIN_NAME_);                                                     \
    std::string GetRegisterName() const override { return RegisterName(); }                  \
    static std::string RegisterName() {                                                      \
        return std::string(__STRING(_DOMAIN_NAME_)) + "." + _MESH_TYPE_::RegisterName();     \
    }                                                                                        \
    static bool is_registered;                                                               \
    typedef _MESH_TYPE_ mesh_type;

#define DOMAIN_DECLARE_FIELD(_NAME_, _IFORM_) \
    Field<mesh_type, typename mesh_type::scalar_type, _IFORM_> _NAME_{this, "name"_ = __STRING(_NAME_)};
}
}
#endif  // SIMPLA_DOMAIN_H
