/**
 * @file cube.h
 *
 *  Created on: 2015-6-7
 *      Author: salmon
 */

#ifndef CORE_GEOMETRY_CUBE_H_
#define CORE_GEOMETRY_CUBE_H_

#include <simpla/SIMPLA_config.h>
#include <simpla/toolbox/sp_def.h>
#include "GeoObject.h"

namespace simpla {
namespace geometry {

struct Cube : public GeoObject {
    SP_OBJECT_HEAD(Cube, GeoObject)

    box_type m_bound_box_;

    Cube(std::initializer_list<std::initializer_list<Real>> const &v)
        : m_bound_box_(point_type(*v.begin()), point_type(*(v.begin() + 1))) {}

    template <typename V, typename U>
    Cube(V const *l, U const *h) : m_bound_box_(box_type({l[0], l[1], l[2]}, {h[0], h[1], h[2]})){};
    Cube(box_type const &b) : m_bound_box_(b) {}

    virtual ~Cube() {}

    virtual std::shared_ptr<data::DataTable> Serialize() const {
        auto p = std::make_shared<data::DataTable>();
        p->SetValue<std::string>("Type", "Cube");
        p->SetValue("Box", m_bound_box_);
        return p;
    };
    virtual void Deserialize(std::shared_ptr<data::DataTable> const &d) { m_bound_box_ = d->GetValue<box_type>("Box"); }

    virtual box_type const &GetBoundBox() const { return m_bound_box_; };
    virtual point_type const &lower() const { return std::get<0>(m_bound_box_); }
    virtual point_type const &upper() const { return std::get<1>(m_bound_box_); }

    virtual Real distance(point_type const &x) const { return 0; }
    /**
     * @brief >0 out ,=0 surface ,<0 in
     * @param x
     * @return
     */
    virtual int isInside(point_type const &x) const { return 0; }
};

// namespace traits
//{
//
// template<typename > struct facet;
// template<typename > struct number_of_points;
//
// template<typename CS>
// struct facet<model::Primitive<1, CS, tags::Cube>>
//{
//	typedef model::Primitive<0, CS> value_type_info;
//};
//
// template<typename CS>
// struct facet<model::Primitive<2, CS, tags::Cube>>
//{
//	typedef model::Primitive<1, CS> value_type_info;
//};
//
// template<size_t N, typename CS>
// struct number_of_points<model::Primitive<N, CS, tags::Cube>>
//{
//	static constexpr size_t value = number_of_points<
//			typename facet<model::Primitive<N, CS, tags::Cube> >::value_type_info>::value
//			* 2;
//};
//
//} // namespace traits
// template<typename CS>
// typename traits::length_type<CS>::value_type_info distance(
//		model::Primitive<0, CS> const & p,
//		model::Primitive<1, CS> const & line_segment)
//{
//
//}
// template<typename CS>
// typename traits::length_type<CS>::value_type_info distance(
//		model::Primitive<0, CS> const & p,
//		model::Primitive<2, CS, tags::Cube> const & rect)
//{
//
//}
// template<typename CS>
// typename traits::length_type<CS>::value_type_info length(
//		model::Primitive<2, CS> const & rect)
//{
//}
// template<typename CS>
// typename traits::area_type<CS>::value_type_info area(
//		model::Primitive<2, CS, tags::Cube> const & rect)
//{
//}
// template<typename CS>
// typename traits::volume_type<CS>::value_type_info volume(
//		model::Primitive<3, CS, tags::Cube> const & poly)
//{
//}
}  // namespace geometry
}  // namespace simpla

#endif /* CORE_GEOMETRY_CUBE_H_ */