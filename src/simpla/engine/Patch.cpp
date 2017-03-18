//
// Created by salmon on 17-2-22.
//
#include "Patch.h"
#include <map>
#include "MeshBlock.h"

namespace simpla {
namespace engine {
struct Patch::pimpl_s {
    mutable std::shared_ptr<MeshBlock> m_mesh_;
    mutable std::map<id_type, std::shared_ptr<data::DataBlock>> m_data_;
};
Patch::Patch(std::shared_ptr<MeshBlock> const &m) : m_pimpl_(new pimpl_s) { m_pimpl_->m_mesh_ = m; }
Patch::~Patch() {}
id_type Patch::GetMeshBlockId() const { return GetMeshBlock()->GetGUID(); }
std::shared_ptr<MeshBlock> Patch::GetMeshBlock() const { return m_pimpl_->m_mesh_; }
std::shared_ptr<data::DataBlock> Patch::GetDataBlock(id_type const &id) const {
    auto it = m_pimpl_->m_data_.find(id);
    return it == m_pimpl_->m_data_.end() ? nullptr : it->second;
}
std::map<id_type, std::shared_ptr<data::DataBlock>> &Patch::GetAllDataBlock() const { return m_pimpl_->m_data_; };
}  // namespace engine {
}  // namespace simpla {