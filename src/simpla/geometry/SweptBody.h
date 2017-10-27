//
// Created by salmon on 17-10-24.
//

#ifndef SIMPLA_SWEPTBODY_H
#define SIMPLA_SWEPTBODY_H
#include <simpla/algebra/nTuple.ext.h>
#include <simpla/utilities/Constants.h>
#include "Body.h"
#include "Curve.h"
#include "Surface.h"
namespace simpla {
namespace geometry {

struct SweptBody : public Body {
    SP_GEO_OBJECT_HEAD(SweptBody, Body);

   protected:
    SweptBody();
    SweptBody(SweptBody const &other);
    explicit SweptBody(std::shared_ptr<const Surface> const &s, std::shared_ptr<const Curve> const &c);

   public:
    ~SweptBody() override;
    std::shared_ptr<const Surface> GetBasisSurface() const;
    void SetBasisSurface(std::shared_ptr<const Surface> const &c);
    std::shared_ptr<const Curve> GetShiftCurve() const;
    void SetShiftCurve(std::shared_ptr<const Curve> const &c);

    std::tuple<bool, bool, bool> IsClosed() const override;
    std::tuple<bool, bool, bool> IsPeriodic() const override;
    nTuple<Real, 3> GetPeriod() const override;
    nTuple<Real, 3> GetMinParameter() const override;
    nTuple<Real, 3> GetMaxParameter() const override;

    point_type Value(Real u, Real v, Real w) const override;
    int CheckOverlap(box_type const &) const override;
    std::shared_ptr<GeoObject> Intersection(std::shared_ptr<const GeoObject> const &, Real tolerance) const override;

   protected:
    std::shared_ptr<const Surface> m_basis_surface_;
    std::shared_ptr<const Curve> m_shift_curve_;
};

}  // namespace geometry
}  // namespace simpla
#endif  // SIMPLA_SWEPTBODY_H
