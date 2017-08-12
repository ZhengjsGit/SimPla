//
// Created by salmon on 16-10-20.
//

#include "Attribute.h"
#include <set>
#include <typeindex>
#include "Domain.h"
#include "MeshBlock.h"
#include "Patch.h"
#include "simpla/data/DataBlock.h"
//#include "simpla/mesh/MeshBase.h"
namespace simpla {
namespace engine {

id_type AttributeDesc::GetDescID() const {
    static std::hash<std::string> s_hasher;
    return s_hasher(GetPrefix() +                       //
                    "." + value_type_info().name() +    //
                    "." + std::to_string(GetIFORM()) +  //
                    "." + std::to_string(GetDOF()));
}
const AttributeDesc &AttributeDesc::GetDescription() const { return *this; };

AttributeGroup::AttributeGroup() = default;

AttributeGroup::~AttributeGroup() {
    for (auto *item : m_attributes_) { item->Deregister(this); }
}

void AttributeGroup::Push(Patch *p) {
    for (auto *item : m_attributes_) { item->Push(p->GetDataBlock(item->GetDescID())); }
}

void AttributeGroup::Pop(Patch *p) {
    for (auto *item : m_attributes_) { p->SetDataBlock(item->GetDescID(), item->Pop()); }
}

void AttributeGroup::Attach(Attribute *p) { m_attributes_.emplace(p); }
void AttributeGroup::Detach(Attribute *p) { m_attributes_.erase(p); }
void AttributeGroup::RegisterAttributes() {
    m_register_desc_.clear();
    for (auto const *item : m_attributes_) {
        m_register_desc_[item->GetName()] = std::make_shared<AttributeDesc>(item->GetDescription());
    }
}
std::shared_ptr<AttributeDesc> AttributeGroup::GetAttributeDescription(std::string const &k) {
    auto it = m_register_desc_.find(k);
    return it != m_register_desc_.end() ? it->second : nullptr;
}

// void AttributeGroup::RegisterDescription(std::map<std::string, std::shared_ptr<AttributeDesc>> *m) const {
//    for (auto &item : m_pimpl_->m_attributes_) { (*m)[item.first] = item.second->GetDescription(); }
//};
//
// void AttributeGroup::RegisterAt(AttributeGroup *other) {
//    for (auto *item : m_pimpl_->m_attributes_) { item->Register(other); }
//};
// void AttributeGroup::DeregisterFrom(AttributeGroup *other) {
//    for (auto *item : m_pimpl_->m_attributes_) { item->Deregister(other); }
//};
//
// std::map<std::string, Attribute *> &AttributeGroup::GetAllAttributes() { return m_pimpl_->m_attributes_; };
// std::map<std::string, Attribute *> const &AttributeGroup::GetAll() const { return m_pimpl_->m_attributes_; };
// bool AttributeGroup::has(std::string const &k) const {
//    return m_pimpl_->m_attributes_.find(k) != m_pimpl_->m_attributes_.end();
//}
// bool AttributeGroup::check(std::string const &k, std::type_info const &t_info) const {
//    auto it = m_pimpl_->m_attributes_.find(k);
//    return (it != m_pimpl_->m_attributes_.end() && it->second->isA(t_info));
//}
// Attribute *AttributeGroup::GetPatch(std::string const &k) {
//    auto it = m_pimpl_->m_attributes_.find(k);
//    Attribute *res = nullptr;
//    if (it != m_pimpl_->m_attributes_.end()) {
//        res = it->second;
//    } else {
//        VERBOSE << "Can not find field [" << k << "] in [";
//        for (auto const &item : m_pimpl_->m_attributes_) { VERBOSE << item.first << ","; }
//        VERBOSE << std::endl;
//    }
//
//    return res;
//}
// Attribute const *AttributeGroup::GetPatch(std::string const &k) const {
//    auto it = m_pimpl_->m_attributes_.find(k);
//    Attribute *res = nullptr;
//    if (it != m_pimpl_->m_attributes_.end()) { res = it->second; }
//    if (res == nullptr) {
//        VERBOSE << "Can not find field [" << k << "] in [";
//        for (auto const &item : m_pimpl_->m_attributes_) { VERBOSE << item.first << ","; }
//        VERBOSE << std::endl;
//    }
//
//    return res;
//}
//
// Attribute::Attribute(Attribute const &other) : SPObject(other), AttributeDesc(other) {
//    for (auto *grp : other.m_bundle_) { Register(grp); }
//    Initialize();
//}
// Attribute::Attribute(Attribute &&other) noexcept : SPObject(std::move(other)), AttributeDesc(std::move(other)) {
//    for (auto *grp : m_bundle_) {
//        grp->Detach(&other);
//        grp->Attach(this);
//    }
//}
Attribute::~Attribute() {
    for (auto *grp : m_bundle_) { grp->Detach(this); }
}
void Attribute::swap(Attribute &other) {
    SPObject::swap(other);
    AttributeDesc::swap(other);
    std::swap(m_data_block_, other.m_data_block_);

    for (auto *grp : m_bundle_) {
        grp->Detach(this);
        grp->Attach(&other);
    }
    for (auto *grp : other.m_bundle_) {
        grp->Detach(&other);
        grp->Attach(this);
    }
}

void Attribute::Register(AttributeGroup *attr_b) {
    if (attr_b != nullptr) {
        auto res = m_bundle_.emplace(attr_b);
        if (res.second) { attr_b->Attach(this); }
    }
}
void Attribute::Deregister(AttributeGroup *attr_b) {
    if (attr_b != nullptr) {
        attr_b->Detach(this);
        m_bundle_.erase(attr_b);
    }
}
void Attribute::Push(const std::shared_ptr<DataBlock> &d) {
    m_data_block_ = d;
    Initialize();
}
std::shared_ptr<data::DataBlock> Attribute::Pop() {
    Finalize();
    return m_data_block_;
}
data::DataBlock *Attribute::GetDataBlock() { return m_data_block_.get(); }
data::DataBlock const *Attribute::GetDataBlock() const { return m_data_block_.get(); }

bool Attribute::isNull() const { return m_data_block_ == nullptr; }

}  //{ namespace engine
}  // namespace simpla
