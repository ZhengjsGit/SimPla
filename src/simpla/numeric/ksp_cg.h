/**
 *  @file ksp_cg.h
 *
 *  created on: 2012-3-30
 *      Author: salmon
 */

#ifndef KSP_CG_H_
#define KSP_CG_H_

#include <iostream>
#include "simpla/utilities/Log.h"

namespace simpla {

/**@ingroup numeric*/
namespace linear_solver {

template <typename TF1, typename TF2>  //
void ksp_cg(TF1 const &Ax, TF2 &x, size_t max_iterative_num = 1000, double residual = 1.0e-10) {
    //	if (!CheckEquationHasVariable(Ax, x_))
    //	{
    //		THROW_EXCEPTION_LOGIC_ERROR << "Unsolvable Equation!";
    //	}

    typedef decltype(x[0]) ValueType;

   auto const &mesh = x.mesh();

    TF2 r(mesh), Ap(mesh), p(mesh), b(mesh);

    INFORM << "KSP_CG Solver: Start";

    ValueType rsold, rsnew, alpha;

    p.clear();
    p.swap(x);
    b = Ax;
    p.swap(x);
    r.clear();
    p = r;

    rsold = InnerProduct(r, r);

    for (int k = 0; k < max_iterative_num; ++k) {
        x.swap(p);
        Ap = Ax;
        p.swap(x);
        alpha = rsold / InnerProduct(p, Ap);
        x = x + alpha * (p);
        r = r - alpha * Ap;

        rsnew = InnerProduct(r, r);

        if (rsnew < residual) { break; }
        p = r + rsnew / rsold * p;
        rsold = rsnew;
    }

    INFORM << "KSP_CG Solver: DONE! ";

    // [ Residual = " 	<< rsnew / static_cast<double>(num_of_ele) << ", iterate " << k	<< " times]
}
}
// namespace linear_solver
}
// namespace simpla

#endif /* KSP_CG_H_ */
