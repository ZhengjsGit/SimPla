/*
 * fetl_test.cpp
 *
 *  created on: 2013-12-28
 *      Author: salmon
 */

#include "../../diff_geometry/diff_scheme/fdm.h"
#include "../../diff_geometry/geometry/cartesian.h"
#include "../../diff_geometry/interpolator/interpolator.h"
#include "../../diff_geometry/mesh.h"
#include "../../diff_geometry/topology/structured.h"
#include "../../gtl/ntuple.h"
#include "../field.h"

using namespace simpla;

//typedef Manifold<CartesianCoordinates<StructuredMesh, CARTESIAN_ZAXIS>,
//		FiniteDiffMethod, InterpolatorLinear> m_type;

typedef CartesianCoordinates<StructuredMesh, CARTESIAN_ZAXIS> g_type;
typedef Real v_type;

#include "field_diff_calculus_test.h"
