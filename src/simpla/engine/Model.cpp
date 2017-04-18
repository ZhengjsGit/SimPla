//
// Created by salmon on 16-6-2.
//
#include "Model.h"
#include <simpla/utilities/sp_def.h>
#include "Attribute.h"
#include "MeshBlock.h"
namespace simpla {
namespace engine {

struct Model::pimpl_s {
    std::map<std::string, std::shared_ptr<geometry::GeoObject>> m_g_objs_;
    box_type m_bound_box_{{0, 0, 0}, {0, 0, 0}};
};

Model::Model() : m_pimpl_(new pimpl_s) {}
Model::~Model() {}

void Model::Initialize() { LOGGER << "Model is initialized " << std::endl; }
void Model::Finalize() {}

void Model::SetUp() {
    auto it = m_pimpl_->m_g_objs_.begin();
    if (it == m_pimpl_->m_g_objs_.end()) { return; }
    m_pimpl_->m_bound_box_ = it->second->GetBoundBox();
    ++it;
    for (; it != m_pimpl_->m_g_objs_.end(); ++it) {
        m_pimpl_->m_bound_box_ = geometry::BoundBox(m_pimpl_->m_bound_box_, it->second->GetBoundBox());
    }
};
int Model::GetNDims() const { return 3; }
box_type const& Model::GetBoundBox() const { return m_pimpl_->m_bound_box_; };

// id_type Model::GetMaterialId(std::string const& k) const { return GetMaterial(k)->GetValue<id_type>("GUID"); }
//
// std::shared_ptr<data::DataTable> Model::GetMaterial(std::string const& s) const {
//    return s == "" ? db() : db()->GetTable(s);
//}
//
// std::shared_ptr<data::DataTable> Model::SetMaterial(std::string const &s, std::shared_ptr<DataTable> other) {
//    db()->Set("/Material/" + s, other, false);
//    db()->Get("/Material/" + s)->cast_as<DataTable>().SetValue("GUID", std::hash<std::string>{}(s));
//    return nullptr;
//}

std::pair<std::shared_ptr<geometry::GeoObject>, bool> Model::AddObject(std::string const& key,
                                                                       std::shared_ptr<DataTable> cfg) {
    auto geo = m_pimpl_->m_g_objs_.emplace(key, nullptr);
    //    if (geo.first->second == nullptr) {}
    return std::make_pair(geo.first->second, false);
};
id_type Model::AddObject(std::string const& key, std::shared_ptr<geometry::GeoObject> const& g_obj) {
    m_pimpl_->m_g_objs_.emplace(key, g_obj);
    return 0;
}

std::shared_ptr<geometry::GeoObject> Model::GetObject(std::string const& k) const {
    auto it = m_pimpl_->m_g_objs_.find(k);
    return it == m_pimpl_->m_g_objs_.end() ? nullptr : it->second;
}

size_type Model::DeleteObject(std::string const& key) { return m_pimpl_->m_g_objs_.erase(key); }
std::map<std::string, std::shared_ptr<geometry::GeoObject>> const& Model::GetAll() const {
    return m_pimpl_->m_g_objs_;
};

}  // namespace engine
}  // namespace simpla{;