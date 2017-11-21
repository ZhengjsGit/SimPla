//
// Created by salmon on 17-11-17.
//

#ifndef SIMPLA_EDGE_H
#define SIMPLA_EDGE_H
#include <simpla/SIMPLA_config.h>
#include "GeoObject.h"

namespace simpla {
namespace geometry {
struct gCurve;
struct Edge : public GeoObject {
    SP_GEO_OBJECT_HEAD(GeoObject, Edge);

    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;


    explicit Edge(Axis const &axis, std::shared_ptr<const gCurve> const &curve = nullptr, Real l = 0, Real w = 1);
    explicit Edge(Axis const &axis, std::shared_ptr<const gCurve> const &curve, std::tuple<Real, Real> const &range);


    void SetCurve(std::shared_ptr<const gCurve> const &s);
    std::shared_ptr<const gCurve> GetCurve() const;
    void SetParameterRange(Real umin, Real umax);
    std::tuple<Real, Real> const &GetParameterRange() const;

   private:
    std::shared_ptr<const gCurve> m_curve_;
    std::tuple<Real, Real> m_range_{0, 1};
};
}  // namespace geometry{
}  // namespace simpla{

#endif  // SIMPLA_EDGE_H