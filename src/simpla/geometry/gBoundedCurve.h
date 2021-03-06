//
// Created by salmon on 17-11-1.
//

#ifndef SIMPLA_BOUNDEDCURVE_H
#define SIMPLA_BOUNDEDCURVE_H

#include "gCurve.h"
namespace simpla {
namespace geometry {

class gBoundedCurve2D : public gCurve2D {
    SP_GEO_ENTITY_ABS_HEAD(gCurve2D, gBoundedCurve2D);

   public:
    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;

    virtual void Open();
    virtual void Close();
    virtual bool IsClosed() const override;
    virtual void AddPoint2D(Real x, Real y);
    virtual point2d_type GetPoint2D(index_type s) const;
    size_type size() const;

    void AddPoint(point_type const &p) { AddPoint2D(p[0], p[1]); }
    point_type GetPoint(index_type s) const {
        auto p = GetPoint2D(s);
        return point_type{p[0], p[1], 0};
    }

    point2d_type xy(Real u) const override;
    std::vector<point2d_type> &data() { return m_data_; }
    std::vector<point2d_type> const &data() const { return m_data_; }

   private:
    std::vector<point2d_type> m_data_;
};
class gBoundedCurve : public gCurve {
    SP_GEO_ENTITY_ABS_HEAD(gCurve, gBoundedCurve);

   public:
    void Deserialize(std::shared_ptr<const simpla::data::DataEntry> const &cfg) override;
    std::shared_ptr<simpla::data::DataEntry> Serialize() const override;

    void Open();
    void Close();
    bool IsClosed() const override;

    size_type size() const { return m_data_.size(); }
    virtual void AddPoint(Real x, Real y, Real z) { AddPoint(point_type{x, y, z}); }
    void AddPoint(point_type const &);
    point_type GetPoint(index_type s) const;
    std::vector<point_type> &data() { return m_data_; }
    std::vector<point_type> const &data() const { return m_data_; }
    point_type xyz(Real u) const override;

   private:
    std::vector<point_type> m_data_;
};
}  // namespace geometry
}  // namespace simpla

#endif  // SIMPLA_BOUNDEDCURVE_H
