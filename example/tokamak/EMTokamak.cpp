//
// Created by salmon on 16-11-29.
//
#include "simpla/SIMPLA_config.h"

#include <simpla/application/SPInit.h>
#include <simpla/engine/Engine.h>
#include <simpla/geometry/Cube.h>
#include <simpla/geometry/csCartesian.h>
#include <simpla/geometry/csCylindrical.h>
#include <simpla/mesh/CoRectMesh.h>
#include <simpla/mesh/EBMesh.h>
#include <simpla/mesh/RectMesh.h>
#include <simpla/predefine/device/ICRFAntenna.h>
#include <simpla/predefine/device/Tokamak.h>
#include <simpla/predefine/engine/SimpleTimeIntegrator.h>
#include <simpla/predefine/physics/EMFluid.h>
#include <simpla/predefine/physics/Maxwell.h>
#include <simpla/predefine/physics/PICBoris.h>
#include <simpla/scheme/FVM.h>
#include <simpla/third_part/SAMRAITimeIntegrator.h>
#include <simpla/utilities/Logo.h>

namespace simpla {
typedef engine::Domain<geometry::csCylindrical, scheme::FVM, mesh::CoRectMesh /*, mesh::EBMesh*/> domain_type;
}  // namespace simpla {

using namespace simpla;
using namespace simpla::engine;

int main(int argc, char** argv) {
    simpla::Initialize(argc, argv);
    //    auto scenario = SAMRAITimeIntegrator::New();
    auto scenario = SimpleTimeIntegrator::New();
    scenario->SetName("EAST");
    scenario->db()->SetValue("DumpFileSuffix", "h5");
    scenario->db()->SetValue("CheckPointFilePrefix", "EAST");
    scenario->db()->SetValue("CheckPointFileSuffix", "xdmf");
    scenario->SetCheckPointInterval(1);
//    scenario->SetDumpInterval(1);

    scenario->GetAtlas()->SetChart<simpla::geometry::csCartesian>();
    scenario->GetAtlas()->GetChart()->SetScale({0.1, 0.1, 0.1});
    scenario->GetAtlas()->GetChart()->SetOrigin({0, 0, 0});
    //    scenario->GetAtlas()->SetBoundingBox(box_type{{1.4, -PI / 4, -1.4}, {2.8, PI / 4, 1.4}});
    scenario->GetAtlas()->SetBoundingBox(box_type{{-1, -1, -1}, {1, 1, 1}});
    //    auto tokamak = Tokamak::New("/home/salmon/workspace/SimPla/scripts/gfile/g038300.03900");
    //    auto* p = new domain::Maxwell<domain_type>;/*tokamak->Limiter()*/
    scenario->SetDomain<domain::Maxwell<domain_type>>("Limiter", geometry::Cube::New({{-1, -1, -1}, {1, 1, 1}}));
    scenario->GetDomain("Limiter")->PostInitialCondition.Connect([=](DomainBase* self, Real time_now) {
        if (auto d = dynamic_cast<domain::Maxwell<domain_type>*>(self)) {
            //            d->B[0].FillNaN();
            //            d->B[0].GetSelection({{-10, -10, -10}, {11, 10, 10}}) = [&](index_type x, auto&&... others) {
            //                return static_cast<Real>(x);
            //            };
            d->E = [&](point_type const& x) { return point_type{0, std::cos(PI * x[0]), 0}; };
            //            d->B = [&](point_type const& x) { return point_type{std::cos(PI * x[0]), 0, 0}; };
        }
    });

    //    scenario->SetDomain<Domain<mesh_type, EMFluid>>("Plasma", tokamak->Boundary());
    //    scenario->GetDomain("Plasma")->PreInitialCondition.Connect([=](DomainBase* self, Real time_now) {
    //        if (auto d = dynamic_cast<Domain<mesh_type, EMFluid>*>(self)) { d->ne = tokamak->profile("ne"); }
    //    });
    //    scenario->SetTimeNow(0);
    scenario->SetTimeEnd(2.0e-8);
    scenario->SetMaxStep(50);
    scenario->SetUp();

    //    INFORM << "Attributes" << *scenario->GetAttributes() << std::endl;

    TheStart();
    scenario->Run();
    //    scenario->Dump();

    //    std::cout << *scenario->Serialize() << std::endl;

    TheEnd();
    scenario->TearDown();

    simpla::Finalize();
}
