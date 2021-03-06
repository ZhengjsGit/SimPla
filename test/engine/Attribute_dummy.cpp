//
// Created by salmon on 17-9-9.
//

#include <iostream>
#include "simpla/data/Data.h"
#include "simpla/engine/Attribute.h"
using namespace simpla;
using namespace simpla::data;
int main(int argc, char** argv) {
    engine::AttributeT<Real, NODE> f{nullptr, "F"_};
    engine::AttributeT<Real, NODE, 3> g{nullptr, "G"_};

    std::cout << f.GetRank() << std::endl;
    std::cout << g.GetRank() << std::endl;

    engine::AttributeT<Real, EDGE> h{nullptr, "H"_};
    h = [&](int w, index_type x, index_type y, index_type z) { return 1.0; };

    engine::AttributeT<Real, EDGE, 3> h3{nullptr, "H3"_};
    h3 = [&](int w, index_type x, index_type y, index_type z) { return 1.0; };
}