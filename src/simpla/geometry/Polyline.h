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
    explicit Polyline( Axis  const &axis) : Polyline() { Curve::SetAxis(axis); }

   public:
    ~Polyline() override;

    bool IsClosed() const override;
    bool IsPeriodic() const override;

    Real GetPeriod() const override;
    Real GetMinParameter() const override;
    Real GetMaxParameter() const override;

    void SetClosed(bool);
    void SetPeriod(Real);

    point_type Value(Real u) const override;

    void PushBack(std::shared_ptr<Curve> const &, Real length = SP_SNaN);
    void PushFront(std::shared_ptr<Curve> const &, Real length = SP_SNaN);
    void Foreach(std::function<void(std::shared_ptr<Curve> const &)> const &);
    void Foreach(std::function<void(std::shared_ptr<const Curve> const &)> const &) const;

   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;
};

}  // namespace geometry
}  // namespace simpla
#endif  // SIMPLA_POLYLINE_H
