//
// Created by salmon on 17-10-23.
//

#ifndef SIMPLA_POLYLINE_H
#define SIMPLA_POLYLINE_H

#include "Curve.h"
namespace simpla {
namespace geometry {

struct Polyline : public Curve {
    SP_GEO_OBJECT_HEAD(Polyline, Curve);

   protected:
    Polyline();
    Polyline(Polyline const &);
    explicit Polyline(Axis const &axis) : Polyline() { Curve::SetAxis(axis); }

   public:
    ~Polyline() override;

    bool IsClosed() const override;
    bool IsPeriodic() const override;

    Real GetPeriod() const override;
    Real GetMinParameter() const override;
    Real GetMaxParameter() const override;

    point_type Value(Real u) const override;
    //    void Close(bool);
    //    void AddPoint(point_type const &xyz);  // add xyz
    //    point_type StartPoint(int n) const;
    //    point_type EndPoint(int n) const;

    int CheckOverlap(box_type const &) const override;
    std::shared_ptr<GeoObject> Intersection(std::shared_ptr<const GeoObject> const &, Real tolerance) const override;

   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;
};

}  // namespace geometry
}  // namespace simpla
#endif  // SIMPLA_POLYLINE_H
