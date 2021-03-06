//
// Created by salmon on 17-11-2.
//

#include "IntersectionCurveSurface.h"
#include <simpla/utilities/Factory.h>
#include <simpla/utilities/Log.h>
#include <vector>
#include "gCurve.h"
#include "GeoEngine.h"
//#include "PointsOnCurve.h"
#include "Shell.h"
#include "gSurface.h"
namespace simpla {
namespace geometry {

IntersectionCurveSurface::IntersectionCurveSurface() = default;
IntersectionCurveSurface::~IntersectionCurveSurface() = default;
IntersectionCurveSurface::IntersectionCurveSurface(IntersectionCurveSurface const &) = default;
IntersectionCurveSurface::IntersectionCurveSurface(std::shared_ptr<const GeoObject> const &g, Real tolerance)
    : m_shape_(g), m_tolerance_(tolerance) {}

std::shared_ptr<IntersectionCurveSurface> IntersectionCurveSurface::Create(std::string const &key) {
    return Factory<IntersectionCurveSurface>::Create(key.empty() ? GEO_ENGINE->GetRegisterName() : key);
}
// size_type IntersectionCurveSurface::Intersect(std::shared_ptr<const gCurve> const &curve, std::vector<Real> *p) {
//    return const_cast<this_type const *>(this)->Intersect(curve, p);
//}
// size_type IntersectionCurveSurface::Intersect(std::shared_ptr<const gCurve> const &curve, std::vector<Real> *p) const
// {
//    if (curve == nullptr) { return 0; }
//    ASSERT(p != nullptr);
//    size_type count = 0;
//    if (auto points = std::dynamic_pointer_cast<PointsOnCurve>(m_shape_->GetIntersection((curve), m_tolerance_))) {
//        for (auto const &v : points->data()) { p->push_back(v); }
//        count = points->data().size();
//    }
//    return count;
//}

}  //    namespace geometry{
}  // namespace simpla{