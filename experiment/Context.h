//
// Created by salmon on 17-2-16.
//

#ifndef SIMPLA_CONTEXT_H
#define SIMPLA_CONTEXT_H

#include "simpla/SIMPLA_config.h"

#include <map>

#include "SPObject.h"

namespace simpla {
namespace engine {
class Model;
class Patch;
class DomainBase;
class MeshBase;
/**
 * @brief
 *
 * @startuml
 * start
 * repeat
 *  : DomainView::Dispatch(Domain & d);
 *  : DomainView::Update() ;
 *  : DomainView:: ;
 * repeat while (more Domain?)
 *
 * stop
 * @enduml
 *
 * @startuml
 *  Context -> DomainView: Dispatch(Domain &)
 *  DomainView -->Context : Done
 *  Context -> DomainView: Update()
 *  activate DomainView
 *      DomainView -> DomainView :Update
 *      activate DomainView
 *          DomainView -> MeshView: Dispatch(Domain::mesh_block)
 *          MeshView --> DomainView: Done
 *          DomainView -> MeshView: Update()
 *          activate MeshView
 *              MeshView -> MeshView : Update
 *              MeshView --> DomainView : Done
 *          deactivate MeshView
 *          DomainView -> Worker  : Update()
 *          activate Worker
 *              activate Worker
 *                    Worker -> AttributeView : Update
 *                    activate AttributeView
 *                          AttributeView -> DomainView : require DataBlock
 *                          DomainView --> AttributeView: return DataBlock
 *                          AttributeView --> Worker : Done
 *                    deactivate AttributeView
 *              deactivate Worker
 *              Worker --> DomainView : Done
 *          deactivate Worker
 *      deactivate DomainView
 *      DomainView --> Context: Done
 *  deactivate DomainView
 * @enduml
 */

/**
 * Context is a container of Model,Atlas,Domains
 */
class Context : public SPObject {
    SP_OBJECT_HEAD(Context, SPObject)

    void DoInitialize() override;
    void DoFinalize() override;
    void DoUpdate() override;
    void DoTearDown() override;

    box_type BoundingBox() const;
    index_box_type IndexBox() const;

    template <typename TM, typename... Args>
    std::shared_ptr<TM> NewMesh(Args &&... args) {
        SetMesh(TM::New(std::forward<Args>(args)...));
        return GetMesh();
    };
    void SetMesh(std::shared_ptr<MeshBase> const &);
    std::shared_ptr<const MeshBase> GetMesh() const;
    std::shared_ptr<MeshBase> GetMesh();

    template <typename TM, typename... Args>
    std::shared_ptr<TM> NewModel(std::string const &k, Args &&... args) {
        SetModel(k, TM::New(std::forward<Args>(args)...));
        return GetModel(k);
    };
    void SetModel(std::string const &k, std::shared_ptr<Model> const &);
    std::shared_ptr<const Model> GetModel(std::string const &k) const;

    template <typename TD>
    std::shared_ptr<TD> NewDomain(std::string const &k, std::shared_ptr<Model> const &m = nullptr) {
        if (m != nullptr) { SetModel(k, m); }
        SetDomain(k, TD::New(GetMesh(), GetModel(k)));
        return GetDomain(k);
    };
    std::shared_ptr<DomainBase> NewDomain(std::string const &k, const std::shared_ptr<data::DataEntity> &);
    std::shared_ptr<DomainBase> GetDomain(std::string const &k) const;

    std::map<std::string, std::shared_ptr<DomainBase>> &GetAllDomains();
    std::map<std::string, std::shared_ptr<DomainBase>> const &GetAllDomains() const;

    void Pop(const std::shared_ptr<Patch> &p);
    void Push(const std::shared_ptr<Patch> &p);

    void InitialCondition(Real time_now);
    void BoundaryCondition(Real time_now, Real dt);
    void ComputeFluxes(Real time_now, Real time_dt);
    Real ComputeStableDtOnPatch(Real time_now, Real time_dt);
    void Advance(Real time_now, Real dt);
    void TagRefinementCells(Real time_now);

   private:
    void SetDomain(std::string const &k, std::shared_ptr<DomainBase> const &);
};
}  // namespace engine{
}  // namespace simpla{

#endif  // SIMPLA_CONTEXT_H