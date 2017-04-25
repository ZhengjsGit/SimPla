//
// Created by salmon on 17-2-16.
//

#ifndef SIMPLA_CONTEXT_H
#define SIMPLA_CONTEXT_H

#include <simpla/SIMPLA_config.h>
#include <map>
#include "Atlas.h"
#include "Model.h"
#include "Patch.h"
#include "Domain.h"
#include "simpla/data/Serializable.h"
namespace simpla {
namespace engine {

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
class Context : public data::Serializable {
    SP_OBJECT_BASE(Context)
   public:
    Context();
    ~Context() override;
    SP_DEFAULT_CONSTRUCT(Context)
    std::shared_ptr<DataTable> Serialize() const override;
    void Deserialize(std::shared_ptr<DataTable>) override;

    void Initialize();  //!< initialize data on current patch
    void Finalize();    //!< release data on current patch
    void TearDown();
    void SetUp();

    void InitializeCondition(Patch *p, Real time_now);
    void BoundaryCondition(Patch *p, Real time_now, Real time_dt);
    void Advance(Patch *p, Real time_now, Real time_dt);

    void Register(AttributeGroup *);

    Model &GetModel() const;

    void SetWorker(std::string const &k, std::shared_ptr<Domain>);
    std::shared_ptr<Domain> GetWorker(std::string const &k);
    std::shared_ptr<Domain> GetWorker(std::string const &k) const;

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};
}  // namespace engine{
}  // namespace simpla{

#endif  // SIMPLA_CONTEXT_H
