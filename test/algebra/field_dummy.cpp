//
// Created by salmon on 16-12-26.
//

#include "simpla/algebra/Algebra.h"
#include "simpla/algebra/Array.h"
#include "simpla/algebra/Field.h"

#include "simpla/predefine/CartesianGeometry.h"
#include "simpla/predefine/CalculusPolicy.h"

using namespace simpla;

int main(int argc, char **argv)
{
    size_type dims[3] = {10, 1, 1};
    Real xmin[3] = {0, 0, 0};
    Real xmax[3] = {1, 2, 3};
//        m->dimensions(dims);
//        m->box(xmin, xmax);

//    size_type gw[3] = {2, 2, 2};
//    index_type lo[3] = {0, 0, 0};
//    index_type hi[3];//= {dims[0], dims[1], dims[2]}
//

    typedef mesh::CartesianGeometry mesh_type;

    mesh_type m;//(&dims[0], &xmin[0], &xmax[0]);

    Field<Real, mesh_type> f(&m);
    Field<Real, mesh_type> g(&m);
    f.clear();
    f(1, 2, 3) = 1990;
    f = 1;
    g = 2;
//    f = f + g;

}