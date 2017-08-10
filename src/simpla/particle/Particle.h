//
// Created by salmon on 17-6-8.
//

#ifndef SIMPLA_PARTICLE_H
#define SIMPLA_PARTICLE_H

#include <simpla/algebra/Field.h>
#include "simpla/SIMPLA_config.h"
#include "simpla/algebra/EntityId.h"
#include "simpla/algebra/nTuple.h"
#include "simpla/engine/Attribute.h"

#include "ParticlePool.h"

namespace simpla {

class ParticleBase : public engine::EnableCreateFromDataTable<ParticleBase> {};

template <typename TM>
class Particle : public engine::Attribute, public engine::EnableCreateFromDataTable<ParticleBase> {
   private:
    typedef Particle<TM> particle_type;
    SP_OBJECT_HEAD(particle_type, engine::Attribute);

   public:
    typedef TM mesh_type;
    static constexpr int iform = FIBER;
    static constexpr int ndims = 3;

   private:
    mesh_type const* m_host_ = nullptr;
    ParticlePool* m_data_ = nullptr;

   public:
    template <typename... Args>
    Particle(mesh_type* grp, int DOF, Args&&... args)
        : base_type(grp->GetMesh(), FIBER, DOF, typeid(Real),
                    std::make_shared<data::DataTable>(std::forward<Args>(args)...)),
          m_host_(grp) {}

    ~Particle() override = default;

    Particle(this_type const& other) = delete;
    Particle(this_type&& other) noexcept = delete;
    this_type& operator=(this_type const& other) = delete;
    this_type& operator=(this_type&& other) = delete;

    void DoInitialize() override {
        ASSERT(GetDataBlock()->isA(typeid(ParticlePool)));
        m_data_ = dynamic_cast<ParticlePool*>(GetDataBlock());
    }

    void DoFinalize() override { m_data_ = nullptr; }
    size_type GetSize() const { return m_data_->GetSize(); }
    auto Get() { return m_data_->GetAttributes(); }
    auto Get() const { return m_data_->GetAttributes(); }

    void Sort() {
        ASSERT(m_data_ != nullptr);
        m_data_->Sort();
    }

};  // class Particle
}  // namespace simpla{

#endif  // SIMPLA_PARTICLE_H
