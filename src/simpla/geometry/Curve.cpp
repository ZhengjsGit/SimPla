//
// Created by salmon on 17-10-18.
//
#include "Curve.h"
namespace simpla {
namespace geometry {
std::shared_ptr<simpla::data::DataNode> Curve::Serialize() const { return base_type::Serialize(); };
void Curve::Deserialize(std::shared_ptr<simpla::data::DataNode> const &cfg) { base_type::Deserialize(cfg); };

PointsOnCurve::PointsOnCurve() = default;
PointsOnCurve::~PointsOnCurve() = default;
PointsOnCurve::PointsOnCurve(std::shared_ptr<const Curve> const &curve)
    : GeoObject(curve->GetAxis()), m_curve_(std::dynamic_pointer_cast<Curve>(curve->Copy())) {}
std::shared_ptr<data::DataNode> PointsOnCurve::Serialize() const { return base_type::Serialize(); };
void PointsOnCurve::Deserialize(std::shared_ptr<data::DataNode> const &cfg) { base_type::Deserialize(cfg); }
Axis const &PointsOnCurve::GetAxis() const { return m_curve_->GetAxis(); };
std::shared_ptr<const Curve> PointsOnCurve::GetBasisCurve() const { return m_curve_; }
void PointsOnCurve::SetBasisCurve(std::shared_ptr<const Curve> const &c) { m_curve_ = c; }

void PointsOnCurve::PutU(Real u) { m_data_.push_back(u); }
Real PointsOnCurve::GetU(size_type i) const { return m_data_[i]; }
point_type PointsOnCurve::GetPoint(size_type i) const { return m_curve_->Value(GetU(i)); }
size_type PointsOnCurve::size() const { return m_data_.size(); }
std::vector<Real> const &PointsOnCurve::data() const { return m_data_; }
std::vector<Real> &PointsOnCurve::data() { return m_data_; }
////
////Circle::Circle() = default;
////
////std::shared_ptr<simpla::data::DataNode> Circle::Serialize() const {
////    auto cfg = base_type::Serialize();
////    cfg->SetValue("Origin", m_origin_);
////    cfg->SetValue("Radius", m_radius_);
////    cfg->SetValue("Normal", m_normal_);
////    cfg->SetValue("R", m_r_);
////    cfg->SetValue("Radius", m_radius_);
////    return cfg;
////};
////void Circle::Deserialize(std::shared_ptr<simpla::data::DataNode> const& cfg) { base_type::Deserialize(cfg); };
////Arc::Arc() = default;
////Arc::~Arc() = default;
////std::shared_ptr<simpla::data::DataNode> Arc::Serialize() const {
////    auto cfg = base_type::Serialize();
////    cfg->SetValue("Origin", m_origin_);
////    cfg->SetValue("Radius", m_radius_);
////    cfg->SetValue("AngleBegin", m_angle_begin_);
////    cfg->SetValue("AngleEnd", m_angle_end_);
////    cfg->SetValue("XAxis", m_XAxis_);
////    cfg->SetValue("YAXis", m_YAxis_);
////    return cfg;
////};
////void Arc::Deserialize(std::shared_ptr<simpla::data::DataNode> const& cfg) {
////    base_type::Deserialize(cfg);
////    m_origin_ = cfg->GetValue("Origin", m_origin_);
////    m_radius_ = cfg->GetValue("Radius", m_radius_);
////    m_angle_begin_ = cfg->GetValue("AngleBegin", m_angle_begin_);
////    m_angle_end_ = cfg->GetValue("AngleEnd", m_angle_end_);
////    m_XAxis_ = cfg->GetValue("XAxis", m_XAxis_);
////    m_YAxis_ = cfg->GetValue("YAXis", m_YAxis_);
////};
// Line::Line() = default;
// Line::~Line() = default;
// std::shared_ptr<simpla::data::DataNode> Line::Serialize() const {
//    auto cfg = base_type::Serialize();
//    cfg->SetValue("Begin", m_p0_);
//    cfg->SetValue("End", m_p1_);
//    return cfg;
//};
// void Line::Deserialize(std::shared_ptr<simpla::data::DataNode> const& cfg) {
//    base_type::Deserialize(cfg);
//    m_p0_ = cfg->GetValue("Begin", m_p0_);
//    m_p1_ = cfg->GetValue("End", m_p1_);
//};
//
// AxeLine::AxeLine() = default;
// AxeLine::~AxeLine() = default;
// std::shared_ptr<simpla::data::DataNode> AxeLine::Serialize() const {
//    auto cfg = base_type::Serialize();
//    cfg->SetValue("Direction", m_dir_);
//    return cfg;
//};
// void AxeLine::Deserialize(std::shared_ptr<simpla::data::DataNode> const& cfg) {
//    base_type::Deserialize(cfg);
//    m_dir_ = cfg->GetValue("Direction", m_dir_);
//};
}  // namespace geometry
}  // namespace simpla