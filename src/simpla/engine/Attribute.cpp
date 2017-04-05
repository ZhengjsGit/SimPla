//
// Created by salmon on 16-10-20.
//

#include "Attribute.h"
#include <simpla/data/DataBlock.h>
#include <set>
#include <typeindex>
#include "Domain.h"
#include "Mesh.h"
#include "MeshBlock.h"
#include "Patch.h"
namespace simpla {
namespace engine {

struct AttributeBundle::pimpl_s {
    Domain *m_domain_ = nullptr;
    Mesh const *m_mesh_;
    std::set<Attribute *> m_attributes_;
};

AttributeBundle::AttributeBundle(Domain *d) : m_pimpl_(new pimpl_s) { Connect(d); }
AttributeBundle::~AttributeBundle() { Disconnect(); }

void AttributeBundle::Connect(Domain *d) {
    if (d != m_pimpl_->m_domain_) {
        Disconnect();
        m_pimpl_->m_domain_ = d;
        m_pimpl_->m_domain_->Attach(this);
    }
};
void AttributeBundle::Disconnect() {
    if (m_pimpl_->m_domain_ != nullptr) {
        auto t = m_pimpl_->m_domain_;
        m_pimpl_->m_domain_ = nullptr;
        t->Detach(this);
    }
};
void AttributeBundle::Attach(Attribute *p) {
    if (p != nullptr) { m_pimpl_->m_attributes_.emplace(p); }
}

void AttributeBundle::Detach(Attribute *p) {
    if (p != nullptr) { m_pimpl_->m_attributes_.erase(p); }
}

void AttributeBundle::SetMesh(Mesh const *m) {
    if (m == nullptr) { return; }
    m_pimpl_->m_mesh_ = m;
    for (Attribute *v : m_pimpl_->m_attributes_) { v->SetMesh(m); }
}
Mesh const *AttributeBundle::GetMesh() const { return m_pimpl_->m_mesh_; }

void AttributeBundle::PushData(std::shared_ptr<MeshBlock> const &m, std::shared_ptr<data::DataTable> const &p) {
    if (GetMesh() == nullptr || m == nullptr || GetMesh()->GetMeshBlockId() != m->GetGUID()) {
        RUNTIME_ERROR << " data and mesh mismatch!" << std::endl;
    }

    for (auto *v : m_pimpl_->m_attributes_) { v->PushData(m, p->GetTable(v->name())); }
}
std::pair<std::shared_ptr<MeshBlock>, std::shared_ptr<data::DataTable>> AttributeBundle::PopData() {
    auto res = std::make_shared<data::DataTable>();
    for (auto *v : m_pimpl_->m_attributes_) { res->Set(v->name(), v->PopData().second); }

    return std::make_pair(m_pimpl_->m_mesh_->GetMeshBlock(), res);
}

std::set<Attribute *> const &AttributeBundle::GetAllAttributes() const { return m_pimpl_->m_attributes_; }

struct Attribute::pimpl_s {
    AttributeBundle *m_bundle_ = nullptr;
    Mesh const *m_mesh_ = nullptr;
};
Attribute::Attribute() : m_pimpl_(new pimpl_s) {}
Attribute::Attribute(AttributeBundle *b, std::shared_ptr<data::DataTable> const &t)
    : SPObject(t), m_pimpl_(new pimpl_s) {
    if (b != nullptr && b != m_pimpl_->m_bundle_) { b->Attach(this); }
    m_pimpl_->m_bundle_ = b;
};

Attribute::~Attribute() {
    if (m_pimpl_->m_bundle_ != nullptr) { m_pimpl_->m_bundle_->Detach(this); }
    m_pimpl_->m_bundle_ = nullptr;
}

void Attribute::SetMesh(Mesh const *m) {
    if (m == nullptr) { return; }
    if ((m_pimpl_->m_mesh_ == nullptr || m_pimpl_->m_mesh_->GetTypeInfo() == m->GetTypeInfo())) {
        m_pimpl_->m_mesh_ = m;
    } else {
        RUNTIME_ERROR << "Can not change the mesh type of a worker![ from " << m_pimpl_->m_mesh_->GetClassName()
                      << " to " << m->GetClassName() << "]" << std::endl;
    }
}

Mesh const *Attribute::GetMesh() const {
    return m_pimpl_->m_mesh_ != nullptr ? m_pimpl_->m_mesh_
                                        : (m_pimpl_->m_bundle_ != nullptr ? m_pimpl_->m_bundle_->GetMesh() : nullptr);
}
/**
*@startuml
*start
* if (m_domain_ == nullptr) then (yes)
* else   (no)
*   : m_current_block_id = m_domain-> current_block_id();
* endif
*stop
*@enduml
*/
bool Attribute::Update() { return SPObject::Update(); }

bool Attribute::isNull() const { return false; }

}  //{ namespace engine
}  // namespace simpla
