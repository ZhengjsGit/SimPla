//
// Created by salmon on 17-11-18.
//

#ifndef SIMPLA_SOLID_H
#define SIMPLA_SOLID_H

#include "GeoObject.h"
namespace simpla {
namespace geometry {
struct gBody;
struct Solid : public GeoObject {
    SP_GEO_OBJECT_HEAD(GeoObject, Solid);
    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;                            \
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;                                             \

   protected:
    explicit Solid(Axis const &axis, std::shared_ptr<const gBody> const &body = nullptr, Real u_min = 0, Real u_max = 1,
                   Real v_min = 0, Real v_max = 1, Real w_min = 0, Real w_max = 1);
    explicit Solid(Axis const &axis, std::shared_ptr<const gBody> const &body, point_type const &u_min,
                   point_type const &u_max);
    explicit Solid(Axis const &axis, std::shared_ptr<const gBody> const &body, box_type const &);

   public:
    void SetBody(std::shared_ptr<const gBody> const &s) { m_body_ = s; }
    std::shared_ptr<const gBody> GetBody() const { return m_body_; }
    void SetParameterRange(point_type const &umin, point_type const &umax) { m_range_ = std::tie(umin, umax); };
    box_type const &GetParameterRange() const { return m_range_; };

   private:
    std::shared_ptr<const gBody> m_body_ = nullptr;
    box_type m_range_{{0, 0, 0}, {1, 1, 1}};
};
}  // namespace geometry{
}  // namespace simpla{
#endif  // SIMPLA_SOLID_H