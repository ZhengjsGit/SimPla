//
// Created by salmon on 17-5-29.
//
#include "Chart.h"
#include "Cube.h"

namespace simpla {
namespace geometry {
Chart::Chart() {}

Chart::~Chart() = default;

std::shared_ptr<data::DataNode> Chart::Serialize() const {
    auto tdb = base_type::Serialize();
    if (tdb != nullptr) {
        tdb->SetValue("Type", ClassName());
        tdb->SetValue("Level", GetLevel());
        tdb->SetValue("Origin", GetOrigin());
        tdb->SetValue("Scale", GetScale());
        tdb->SetValue("Rotation", GetRotation());
    }
    return tdb;
}
void Chart::Deserialize(std::shared_ptr<data::DataNode> const &tdb) {
    if (tdb != nullptr) {
        m_origin_ = tdb->GetValue<point_type>("Origin", m_origin_);
        m_scale_ = tdb->GetValue<point_type>("Scale", m_scale_);
        m_rotation_ = tdb->GetValue<point_type>("Rotation", m_rotation_);
    }
};

void Chart::SetOrigin(point_type const &x) { m_origin_ = x; }
point_type const &Chart::GetOrigin() const { return m_origin_; }

void Chart::SetScale(point_type const &x) { m_scale_ = x; }
point_type const &Chart::GetScale() const { return m_scale_; }

void Chart::SetRotation(point_type const &x) { m_rotation_ = x; }
point_type const &Chart::GetRotation() const { return m_rotation_; }

point_type Chart::GetCellWidth(int level) const {
    point_type res = m_scale_;
    if (m_level_ < level) {
        res /= static_cast<Real>(1 << (level - m_level_));
    } else if (m_level_ > level) {
        res *= static_cast<Real>(1 << (m_level_ - level));
    }

    return res;
}

void Chart::SetLevel(int level) {
    m_scale_ = GetCellWidth(level);
    m_level_ = level;
};
int Chart::GetLevel() const { return m_level_; }
int Chart::GetNDIMS() const { return 3; }

std::shared_ptr<GeoObject> Chart::GetBoundingShape(box_type const &b) const { return Cube::New(MapToBase(b)); }
std::shared_ptr<GeoObject> Chart::GetBoundingShape(index_box_type const &b) const {
    return Cube::New(std::make_tuple(global_coordinates(std::get<0>(b)), global_coordinates(std::get<0>(b))));
};
}  // namespace geometry {
}  // namespace simpla {
