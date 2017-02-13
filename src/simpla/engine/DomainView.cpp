//
// Created by salmon on 17-2-12.
//
#include "DomainView.h"
#include <simpla/SIMPLA_config.h>
#include "AttributeView.h"
#include "MeshView.h"
#include "Object.h"
#include "Worker.h"
namespace simpla {
namespace engine {
struct DomainView::pimpl_s {
    id_type m_current_block_id_ = NULL_ID;
    std::shared_ptr<MeshView> m_mesh_;
    std::shared_ptr<Worker> m_worker_;
    std::set<AttributeView *> m_attrs_;
};
DomainView::DomainView() : m_pimpl_(new pimpl_s) {}
DomainView::~DomainView() {}

/**
 *
 * @startuml
 * actor Main
 * Main -> DomainView : Set  MeshView as U
 * activate DomainView
 *     alt if MeshView=nullptr
 *          create MeshView
 *     DomainView -> MeshView : create U as MeshView
 *     MeshView --> DomainView: return MeshView
 *     end
 *     DomainView --> Main : Done
 * deactivate DomainView
 * @enduml
 * @startuml
 * actor Main
 * Main -> DomainView : Dispatch
 * activate DomainView
 *     DomainView->MeshView:  Dispatch
 *     MeshView->MeshView: SetMeshBlock
 *     activate MeshView
 *     deactivate MeshView
 *     MeshView -->DomainView:  Done
*      DomainView --> Main : Done
 * deactivate DomainView
 * @enduml
 * @startuml
 * Main ->DomainView: Update
 * activate DomainView
 *     DomainView -> AttributeView : Update
 *     activate AttributeView
 *          AttributeView -> Field : Update
 *          Field -> AttributeView : Update
 *          activate AttributeView
 *               AttributeView -> DomainView : get DataBlock at attr.id()
 *               DomainView --> AttributeView : return DataBlock at attr.id()
 *               AttributeView --> Field : return DataBlock is ready
 *          deactivate AttributeView
 *          alt if data_block.isNull()
 *              Field -> Field :  create DataBlock
 *              Field -> AttributeView : send DataBlock
 *              AttributeView --> Field : Done
 *          end
 *          Field --> AttributeView : Done
 *          AttributeView --> DomainView : Done
 *     deactivate AttributeView
 *     DomainView -> MeshView : Update
 *     activate MeshView
 *          alt if isFirstTime
 *              MeshView -> AttributeView : Set Initialize Value
 *              activate AttributeView
 *                   AttributeView --> MeshView : Done
 *              deactivate AttributeView
 *          end
 *          MeshView --> DomainView : Done
 *     deactivate MeshView
 *     DomainView -> Worker : Update
 *     activate Worker
 *          alt if isFirstTime
 *              Worker -> AttributeView : set initialize value
 *              activate AttributeView
 *                  AttributeView --> Worker : Done
 *              deactivate AttributeView
 *          end
 *          Worker --> DomainView : Done
 *     deactivate Worker
 *     DomainView --> Main : Done
 * deactivate DomainView
 * deactivate Main
 * @enduml
 */
void DomainView::Dispatch(Domain const &d) {
    ASSERT(m_pimpl_->m_mesh_ != nullptr);
    m_pimpl_->m_mesh_->Dispatch(d.mesh_block());
};
id_type DomainView::current_block_id() const { return m_pimpl_->m_current_block_id_; }

bool DomainView::isUpdated() const {
    return m_pimpl_->m_mesh_ != nullptr && m_pimpl_->m_mesh_->current_block_id() == m_pimpl_->m_current_block_id_ &&
           m_pimpl_->m_current_block_id_ != NULL_ID;
}
void DomainView::Update() {
    if (!isUpdated()) {
        m_pimpl_->m_mesh_->Update();
        for (auto &attr : m_pimpl_->m_attrs_) { attr->Update(); }
        m_pimpl_->m_worker_->Update();
        m_pimpl_->m_current_block_id_ = m_pimpl_->m_mesh_->current_block_id();
    }
}

void DomainView::Evaluate() {
    if (m_pimpl_->m_worker_ != nullptr) { m_pimpl_->m_worker_->Evaluate(); }
}

void DomainView::SetMesh(std::shared_ptr<MeshView> const &m) { m_pimpl_->m_mesh_ = m; };
void DomainView::UnsetMesh() { m_pimpl_->m_mesh_ = nullptr; };
std::shared_ptr<MeshView> const &DomainView::GetMesh() const { return m_pimpl_->m_mesh_; }

void DomainView::AppendWorker(std::shared_ptr<Worker> w) {
    if (w == nullptr) { return; }
};
void DomainView::PrependWorker(std::shared_ptr<Worker> w) {
    if (w == nullptr) { return; }
};
void DomainView::RemoveWorker(std::shared_ptr<Worker> w) {
    if (w == nullptr) { return; }
};

void DomainView::AddAttribute(AttributeView *attr) {
    //    attr->AddAttribute(this);
    m_pimpl_->m_attrs_.insert(attr);
};
void DomainView::RemoveAttribute(AttributeView *attr) {
    //    attr->RemoveAttribute(this);
    m_pimpl_->m_attrs_.erase(attr);
}

std::shared_ptr<DataBlock> DomainView::data_block(id_type) const {}

void DomainView::data_block(id_type, std::shared_ptr<DataBlock>) {}

std::ostream &DomainView::Print(std::ostream &os, int indent) const {
    for (auto const &attr : m_pimpl_->m_attrs_) { os << attr->description().db << " , "; }
    os << std::endl;
    return os;
};

}  // namespace engine
}  // namespace simpla