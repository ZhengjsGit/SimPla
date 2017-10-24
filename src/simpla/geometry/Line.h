//
// Created by salmon on 17-10-21.
//

#ifndef SIMPLA_LINE_H
#define SIMPLA_LINE_H

#include <simpla/SIMPLA_config.h>
#include <simpla/utilities/SPDefines.h>
#include <memory>
#include "Curve.h"
namespace simpla {
namespace geometry {
struct Line : public Curve {
    SP_GEO_OBJECT_HEAD(Line, Curve);

   protected:
    Line() = default;
    Line(Line const &) = default;
    explicit Line(Axis const &axis, Real alpha0 = SP_SNaN, Real alpha1 = SP_SNaN) : Curve(axis) {
        SetParameterRange(std::isnan(alpha0) ? GetMinParameter() : alpha0,
                          std::isnan(alpha1) ? GetMaxParameter() : alpha1);
    };
    explicit Line(point_type const &p0, point_type const &p1) : Curve(Axis{p0, p1 - p0}) { SetParameterRange(0, 1); };
    explicit Line(vector_type const &v) : Curve(Axis{point_type{0, 0, 0}, v}) { SetParameterRange(0, 1); };

   public:
    ~Line() override = default;

    bool IsClosed() const override { return false; };
    bool IsPeriodic() const override { return false; };
    Real GetPeriod() const override { return SP_INFINITY; };
    Real GetMinParameter() const override { return -SP_INFINITY; }
    Real GetMaxParameter() const override { return SP_INFINITY; }

    point_type Value(Real u) const override { return m_axis_.Coordinates(u); }
};

}  // namespace geometry
}  // namespace simpla
#endif  // SIMPLA_LINE_H