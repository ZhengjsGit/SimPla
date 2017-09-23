//
// Created by salmon on 17-3-1.
//
#include "simpla/SIMPLA_config.h"

#include <simpla/data/DataNode.h>
#include <boost/functional/hash.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "simpla/algebra/EntityId.h"

#include "MeshBlock.h"

namespace simpla {
namespace engine {
MeshBlock::MeshBlock() {}
MeshBlock::MeshBlock(index_box_type b, int id, int level, int global_rank)
    : m_global_rank_(global_rank), m_level_(level), m_local_id_(id), m_index_box_(std::move(b)) {}

MeshBlock::~MeshBlock() = default;
std::shared_ptr<MeshBlock> MeshBlock::New(std::shared_ptr<simpla::data::DataNode> const &tdb) {
    auto res = std::shared_ptr<MeshBlock>(new MeshBlock);
    res->Deserialize(tdb);
    return res;
}
std::shared_ptr<simpla::data::DataNode> MeshBlock::Serialize() const {
    auto res = data::DataNode::New(data::DataNode::DN_TABLE);
    res->SetValue("LowIndex", std::get<0>(IndexBox()));
    res->SetValue("HighIndex", std::get<1>(IndexBox()));
    res->SetValue("Level", GetLevel());
    res->SetValue("GUID", GetGUID());
    return res;
}
void MeshBlock::Deserialize(std::shared_ptr<simpla::data::DataNode> const &cfg) {
    std::get<0>(m_index_box_) = cfg->GetValue<index_tuple>("LowIndex", index_tuple{0, 0, 0});
    std::get<1>(m_index_box_) = cfg->GetValue<index_tuple>("HighIndex", index_tuple{1, 1, 1});
    m_level_ = cfg->GetValue<int>("Level", 0);
    m_local_id_ = cfg->GetValue<id_type>("GUID", 0);
}
std::shared_ptr<MeshBlock> MeshBlock::New(index_box_type const &box, int id, int level, int global_rank) {
    return std::shared_ptr<MeshBlock>(new MeshBlock(box, id, level, global_rank));
};

id_type MeshBlock::hash_id(id_type id, int level, int owner) {
    return static_cast<id_type>((owner * MAX_LEVEL_NUMBER + level) * MAX_LOCAL_ID_NUMBER) + id;
}

id_type MeshBlock::GetGUID() const { return hash_id(m_local_id_, m_level_, m_global_rank_); }

// MeshBlock &MeshBlock::operator=(MeshBlock const &other) {
//    MeshBlock(other).swap(*this);
//    return *this;
//}
// MeshBlock &MeshBlock::operator=(MeshBlock &&other) noexcept {
//    MeshBlock(other).swap(*this);
//    return *this;
//}
// index_tuple MeshBlock::GetGhostWidth() const { return m_ghost_width_; };
// index_box_type MeshBlock::GetOuterIndexBox() const {
//    auto ibox = IndexBox();
//    std::get<0>(ibox) -= GetGhostWidth();
//    std::get<1>(ibox) += GetGhostWidth();
//    return std::move(ibox);
//}
// index_box_type MeshBlock::GetInnerIndexBox() const { return IndexBox(); }
//
// index_tuple MeshBlock::GetIndexOrigin() const { return std::get<0>(m_index_box_); }
//
// size_tuple MeshBlock::GetDimensions() const {
//    return std::get<1>(m_index_box_) - std::get<0>(m_index_box_);
//}

}  // namespace engine {
}  // namespace simpla {