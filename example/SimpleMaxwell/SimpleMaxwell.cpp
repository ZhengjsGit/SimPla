//
// Created by salmon on 17-10-11.
//
#include "simpla/SIMPLA_config.h"

#include <simpla/application/SPInit.h>
#include <simpla/engine/EBDomain.h>
#include <simpla/geometry/Box.h>
#include <simpla/geometry/csCartesian.h>
#include <simpla/mesh/CoRectMesh.h>
#include <simpla/mesh/RectMesh.h>
#include <simpla/predefine/engine/SimpleTimeIntegrator.h>
#include <simpla/predefine/physics/Maxwell.h>
#include <simpla/predefine/physics/PredefineDomains.h>
#include <simpla/utilities/Logo.h>

using namespace simpla;
using namespace simpla::engine;

int main(int argc, char **argv) {
    simpla::Initialize(argc, argv);
    auto scenario = SimpleTimeIntegrator::New();
    scenario->SetName("SimpleFDTD");
    scenario->GetAtlas()->NewChart<simpla::geometry::csCartesian>(point_type{0, 0, 0}, point_type{1, 1.5, 2});
    scenario->GetAtlas()->SetBoundingBox(box_type{{-20, -30, -25}, {20, 30, 25}});
    auto domain = scenario->NewDomain<domain::Maxwell<CartesianFVM>>("Center");
    domain->AddPostInitialCondition([=](auto *self, Real time_now) {
        self->B = [&](point_type const &x) {
            return point_type{std::cos(2 * PI * x[1] / 60) * std::cos(2 * PI * x[2] / 50),
                              std::cos(2 * PI * x[0] / 40) * std::cos(2 * PI * x[2] / 50),
                              std::cos(2 * PI * x[0] / 40) * std::cos(2 * PI * x[1] / 60)};
        };

    });

    scenario->SetTimeEnd(1.0e-8);
    scenario->SetMaxStep(50);
    scenario->SetUp();

    scenario->ConfigureAttribute<size_type>("E", "CheckPoint", 1);
    scenario->ConfigureAttribute<size_type>("B", "CheckPoint", 1);

    VERBOSE << "Scenario: " << *scenario->Serialize();

    scenario->Run();

    scenario->TearDown();
    simpla::Finalize();
}
