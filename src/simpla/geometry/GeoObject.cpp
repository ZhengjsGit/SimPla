//
// Created by salmon on 17-2-21.
//
#include "GeoObject.h"

#include <memory>

#include "BoxUtilities.h"
#include "Cube.h"
namespace simpla {
namespace geometry {
GeoObject::GeoObject() : SPObject(){};
GeoObject::~GeoObject(){};
GeoObject::GeoObject(GeoObject const &other) : SPObject(other), m_axis_(other.m_axis_){};
GeoObject::GeoObject(Axis const &axis) : SPObject(), m_axis_(axis){};
std::shared_ptr<data::DataNode> GeoObject::Serialize() const {
    auto res = base_type::Serialize();
    res->Set("Axis", m_axis_.Serialize());
    return res;
}
void GeoObject::Deserialize(std::shared_ptr<data::DataNode> const &cfg) {
    base_type::Deserialize(cfg);
    m_axis_.Deserialize(cfg->Get("Axis"));
}

box_type GeoObject::GetBoundingBox() const { return box_type{{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}}; }

Axis &GeoObject::GetAxis() { return m_axis_; }
Axis const &GeoObject::GetAxis() const { return m_axis_; }
void GeoObject::SetAxis(Axis const &a) { m_axis_ = a; }

void GeoObject::Mirror(const point_type &p) { m_axis_.Mirror(p); }
void GeoObject::Mirror(const Axis &a1) { m_axis_.Mirror(a1); }
void GeoObject::Rotate(const Axis &a1, Real angle) { m_axis_.Rotate(a1, angle); }
void GeoObject::Scale(Real s, int dir) { m_axis_.Scale(s, dir); }
void GeoObject::Translate(const vector_type &v) { m_axis_.Translate(v); }
void GeoObject::Move(const point_type &p) { m_axis_.Move(p); }

// std::shared_ptr<GeoObject> GeoObject::GetBoundary() const { return nullptr; }

// Real GeoObject::Measure() const {
//    auto b = GetBoundingBox();
//    return (std::get<1>(b)[0] - std::get<0>(b)[0]) * (std::get<1>(b)[1] - std::get<0>(b)[1]) *
//           (std::get<1>(b)[2] - std::get<0>(b)[2]);
//};
//
// bool GeoObject::CheckInside(point_type const &x, Real tolerance) const {
//    return geometry::isInSide(GetBoundingBox(), x);
//}
// std::shared_ptr<GeoObject> GeoObject::Intersection(std::shared_ptr<GeoObject> const &other) const {
//    return Cube::New(geometry::Overlap(GetBoundingBox(), other->GetBoundingBox()));
//}
// std::shared_ptr<GeoObject> GeoObject::Difference(std::shared_ptr<GeoObject> const &other) const {
//    UNIMPLEMENTED;
//    return nullptr;
//}
// std::shared_ptr<GeoObject> GeoObject::Union(std::shared_ptr<GeoObject> const &other) const {
//    return Cube::New(geometry::Union(GetBoundingBox(), other->GetBoundingBox()));
//}
// Real GeoObject::isOverlapped(box_type const &b) const { return Measure(Overlap(GetBoundingBox(), b)) / measure(); }
//
// Real GeoObject::CheckOverlap(GeoObject const &other) const { return isOverlapped(other.GetBoundingBox()); }
//
// bool GeoObject::CheckInside(const point_type &x) const { return CheckInSide(GetBoundingBox(), x); };
//
// std::tuple<Real, point_type, point_type> GeoObject::ClosestPoint(point_type const &x) const {
//    return std::tuple<Real, point_type, point_type>{0, x, x};
//}

}  // namespace geometry {
}  // namespace simpla {