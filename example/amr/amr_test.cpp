//
// Created by salmon on 16-11-4.
//

#include <simpla/SIMPLA_config.h>

#include <iostream>

#include <simpla/mesh/MeshCommon.h>
#include <simpla/mesh/Atlas.h>
#include <simpla/simulation/Worker.h>
#include <simpla/physics/Field.h>

using namespace simpla;

class DummyMesh : public mesh::MeshBlock
{
public:
    virtual std::shared_ptr<mesh::MeshBlock> clone() const
    {
        return std::dynamic_pointer_cast<mesh::MeshBlock>(std::make_shared<DummyMesh>());
    };

};

template<typename TM>
struct AMRTest : public simulation::Worker
{

    typedef TM mesh_type;


    template<typename TV, mesh::MeshEntityType IFORM> using field_type=Field<TV, mesh_type, index_const<static_cast<size_t>(IFORM)>>;

    field_type<Real, mesh::EDGE> E{this, "E"};
};

int main(int argc, char **argv)
{

    auto m = std::make_shared<DummyMesh>();

    auto atlas = std::make_shared<mesh::Atlas>();

    atlas->insert(*m);

    auto worker = std::make_shared<AMRTest<mesh::MeshBlock>>();

    worker->E.deploy(m.get());

    std::cout << worker->E << std::endl;
    std::cout << *worker << std::endl;

//
//    auto m = std::make_shared<mesh::MeshBlock>();
//
//    auto attr = mesh::Attribute::create();

//    auto f = attr->create(m);
}