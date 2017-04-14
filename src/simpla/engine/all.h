//
// Created by salmon on 17-3-19.
//

#ifndef SIMPLA_ENGINE_ALL_H
#define SIMPLA_ENGINE_ALL_H

#include "Atlas.h"
#include "Attribute.h"
#include "Context.h"
#include "Domain.h"

#include "Mesh.h"
#include "Model.h"
#include "Patch.h"
#include "Task.h"
#include "TimeIntegrator.h"
/**
 *
 * @startuml
 *

 *
 * class GeoObject{
 * }
 * note right
 *    <latex> \mathcal{O}_{\alpha}  </latex>
 * end note
 * class MeshBlock{
 * }
 * note right
 *      If <latex> \mathcal{O}_{\alpha,m} = \bar{\varphi} ^{-1}_{\alpha} \left[ \mathbb{Z}^n \right] _m </latex>,
 *      it is called as  <b>Mesh Block</b>.
 * end note

 * class Atlas{
 *      Patch * Pop(index_box_type,int level)
 *      void Push(Patch *)
 *      id_type AddBlock(index_box_type,int level)
 *      index_box_type  GetBlock(id_type)
 * }
 * note right
 *   <latex>  \mathcal{G}_{\alpha}\supseteq \bigcup_{m}  \mathcal{O}_{\alpha,m}^0 ,</latex>
 *   <latex>  \mathcal{O}_{\alpha} \cap  \mathcal{O}_{\alpha,m}^l \neq  \oslash,</latex>
 *   <latex>  \mathcal{O}_{\alpha,m}^l \cap \mathcal{O}_{\alpha,n}^l =\oslash </latex>
 * end note
 * Atlas o-- "0..*" Patch
 * Atlas .. Domain
 * class Atlas{
 * }
  note right
 *   A collection of charts  <latex>\mathcal{A}\equiv\left\{ \mathcal{O}_{\alpha},\varphi_{\alpha}\right\} </latex>
 *   is called as <b>atlas</b> .
 *      <latex>X=\bigcup_{\alpha} \mathcal{O}_{\alpha} </latex>
 *  The set <latex> \mathcal{O} </latex> is known as <b>coordinate patch </b>,
 * end note
 *
 *
 * class Domain{
 *      GeoObject * m_geo_object_
 *      Chart * m_chart_
 *      Worker * m_worker_center_
 *      Worker * m_worker_boundary_[]
 *      int CheckOverlap(MeshBlock)
 *      void Push(Patch)
 *      Patch Pop()
 *      Worker & GetWorker()
 * }
 * note right
 *    <latex>\left\{ \mathcal{O}_{\alpha},\varphi_{\alpha}\right\} </latex>
 * end note
 * Domain *-- GeoObject
 * Domain *-- "0..*" Worker
 * Domain *-- "1" Chart
 * class IdRange{
 * }
 *
 * class Worker{
 *      void SetMesh(Chart*)
 *      {abstract} void Register(AttributeGroup *);
 *      {abstract} void Deregister(AttributeGroup *);
 *      {abstract} void Push(Patch const &);
 *      {abstract} void Pop(Patch *) const;
 * }
 * Worker *-- "1" Mesh
 *
 * class ConcreteWorker<TMesh>{
 *      void Register(AttributeGroup*)
 *      void Initialize(Real time_now)
 *      void Run(Real time_now,Real dt)
 * }
 * ConcreteWorker --|> Worker
 * ConcreteWorker o-- Field
 *
 * class Chart{
 *      point_type origin;
 *      point_type dx;
 *      Chart* Coarsen(int)const
 *      Chart* Refine(int)const
 *      Chart* Shift(point_type)const
 *      Chart* Rotate(Real a[])const
 *      {abstract} Mesh * CreateView(index_box_type)const
 * }
 * note bottom of Chart
 *    <latex>  \varphi_{\alpha}</latex>
 * end note
 *

 *
 * note as LocalCoordinates
 *   <b>Chart/Local Coordinates</b>
 *   A homeomorphism
 *     <latex>\varphi:\mathcal{O}\rightarrow\mathbb{R}^{n}\left[x^{0},...,x^{n-1}\right] </latex>
 *     <latex>\bar{\varphi}:\mathcal{O}\rightarrow\mathbb{Z}^{n}\left[x^{0},...,x^{n-1}\right] </latex>
 *    is called a <b>chart</b> or alternatively <b>local coordinates</b>.
 *    Each point <latex> x\in\mathcal{O} </latex> is then uniquely associated with
 *    an n-tuple of real numbers - its coordinates.
 *    The boundary of Chart is not defined.
 *    <latex>\varphi:\left(x^{n}\right)\mapsto\left(z^{n},r^{n}\right),z^{n}\in\mathbb{Z},r^{n}\in\left[0,1\right)</latex>
 *    <latex>z^{n}</latex> is the index of mesh vertex, and <latex>r^{n}</latex> is the local coordinates in cell
 *    <latex>x= \sum _{i=0}^{m} p_i w_i\left( r_0,r_1,...,r_{n-1} \right) </latex>
 *     where <latex>p_i</latex> is coordinate  of vertex i, and m is the number of vertices in the cell,
 *     <latex>w_i</latex> is the interpolation function
 * end note
 * MeshBlock .. DataBlock
 *
 * abstract  Mesh {
 *      {abstract} GeoObject boundary()const
 *      Range range(iform)
 *      {abstract} void Register(AttributeGroup *);
 *      {abstract} void Deregister(AttributeGroup *);
 *      {abstract} void Push(Patch const &);
 *      {abstract} void Pop(Patch *) const;
 * }
 * note right of Mesh
 *    <latex>\left\{ \mathcal{O}_{\alpha,m},\varphi_{\alpha}\right\} </latex>
 * end note
 * Mesh *-- Chart
 * Mesh *-- GeoObject
 * Mesh <.. MeshBlock
 *
 *
 * class Patch {
 * }
 * Patch *-- "1" MeshBlock
 * Patch *-- "0..*" DataBlock
 * Patch *-- "1..*" IdRange
 *
 * class IdRange{
 * }
 *
 * class RectMesh<TGeometry> {
 * }
 * RectMesh --|> Mesh
 * class EBMesh<TGeometry> {
 * }
 * EBMesh --|> Mesh
 *
 * ConcreteWorker ..> RectMesh
 * ConcreteWorker ..> EBMesh
 * class Attribute {
 *      void Register(AttributeGroup*)
 *      void SetMesh(Mesh *);
 *      {abstract} int GetIFORM()const;
 *      {abstract} int GetDOF()const
 *      Push(DataBlock);
 *      DataBlock Pop();
 * }
 * Attribute <.. IdRange: push/pop
 * class AttributeGroup {
 *      void Register(AttributeGroup*);
 *      void Detach(Attribute *attr);
 *      void Attach(Attribute *attr);
 *      void Push(Patch );
 *      Patch Pop();
 * }
 * AttributeGroup o-- Attribute

 * class Field<TMesh>{
  *    int GetIFORM()const;
 *     int GetDOF()const
 * }
 *
 * Field --|> Attribute
 *
 * Chart <|-- CartesianGeometry
 * Chart <|-- CylindricalGeometry
 *
 * Patch <..> Domain : push/pop
 * DataBlock <..> Attribute : push/pop
 * @enduml

 * @startuml
 * start
 * repeat
 *  repeat
 *      : push Patch to Domain;
 *    if (MeshBlock inside Domain) then (yes)
 *      : Domain.center_worker.run();
 *    elseif (MeshBlock inside Domain[A]) then (yes)
 *      : Domain.boundary_worker[A].run();
 *    else (outside)
 *      : do sth.;
 *    endif
 *      : Pop Patch from Domain;
 *  repeat while (more Domain?)
 * repeat while (more MeshBlock ?)
 * end
 * @enduml
 *
 */

/**
 *
 * @startuml
 * actor Main
 * Main -> DomainView : Set U as MeshView
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
#endif  // SIMPLA_ENGINE_ALL_H