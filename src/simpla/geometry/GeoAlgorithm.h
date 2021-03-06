/**
 * @file GeoAlgorithm.h
 *
 *  created on: 2014-4-21
 *      Author: salmon
 */

#ifndef GEOMETRY_ALGORITHM_H_
#define GEOMETRY_ALGORITHM_H_

#include "simpla/SIMPLA_config.h"

#include <cstddef>
#include <tuple>

#include <simpla/algebra/nTuple.ext.h>
#include <simpla/algebra/nTuple.h>
#include <simpla/utilities/Log.h>
#include <simpla/utilities/SPDefines.h>
#include <simpla/utilities/integer_sequence.h>
#include <simpla/utilities/utility.h>
#include <vector>
namespace simpla {
namespace geometry {
using namespace simpla::utility;

template <typename U, typename V>
bool CheckBoxOverlapped(std::tuple<simpla::nTuple<U, 3>, simpla::nTuple<V, 3>> const& b0,
                        std::tuple<simpla::nTuple<U, 3>, simpla::nTuple<V, 3>> const& b1) {
    return !((std::get<1>(b0)[0] <= std::get<0>(b1)[0] || std::get<0>(b0)[0] >= std::get<1>(b1)[0]) ||
             (std::get<1>(b0)[1] <= std::get<0>(b1)[1] || std::get<0>(b0)[1] >= std::get<1>(b1)[1]) ||
             (std::get<1>(b0)[2] <= std::get<0>(b1)[2] || std::get<0>(b0)[2] >= std::get<1>(b1)[2]));
}
template <typename U>
auto CalBoxOverlappedArea(std::tuple<simpla::nTuple<U, 3>, simpla::nTuple<U, 3>> const& b0,
                          std::tuple<simpla::nTuple<U, 3>, simpla::nTuple<U, 3>> const& b1) {
    return std::max(
               0.0,
               std::min(std::get<1>(b0)[0], std::get<1>(b1)[0]) - std::max(std::get<0>(b0)[0], std::get<0>(b1)[0])) *
           std::max(
               0.0,
               std::min(std::get<1>(b0)[1], std::get<1>(b1)[1]) - std::max(std::get<0>(b0)[1], std::get<0>(b1)[1])) *
           std::max(
               0.0,
               std::min(std::get<1>(b0)[2], std::get<1>(b1)[2]) - std::max(std::get<0>(b0)[2], std::get<0>(b1)[2]));
}
bool CheckPointInBox(nTuple<Real, 2> const& p, nTuple<Real, 2> const& bMin, nTuple<Real, 2> const& bMax);
bool CheckPointInBox(nTuple<Real, 2> const& p, std::tuple<nTuple<Real, 2>, nTuple<Real, 2>> const& box);
bool CheckPointInBox(point_type const& p, point_type const& bMin, point_type const& bMax);
bool CheckPointInBox(point_type const& p, std::tuple<point_type, point_type> const& box);
bool CheckPointOnPlane(point_type const& p, point_type const& o, vector_type const& normal,
                       Real tolerance = SP_GEO_DEFAULT_TOLERANCE);

bool CheckIntersectionCubeSphere(point_type const& bMin, point_type const& bMax, point_type const& C, Real r);
int GetIntersectionLineSphere(point_type const& p0, point_type const& p1, point_type const& c, Real r, Real tolerance,
                              std::vector<Real>& res);
/**
 *   (b0 & b1).volume
 * @tparam U
 * @tparam V
 * @tparam N
 * @param b0
 * @param b1
 * @return
 */
template <typename U, typename V, int N>
Real OverlapVolume(std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> const& b0,
                   std::tuple<simpla::nTuple<V, N>, simpla::nTuple<V, N>> const& b1) {
    Real area = 1;
    for (int i = 0; i < N; ++i) {
        Real l = std::min(std::get<1>(b0)[i], std::get<1>(b1)[i]) - std::max(std::get<0>(b0)[i], std::get<0>(b1)[i]);
        area *= (l > 0) ? l : 0;
    }
    return area;
};

template <typename U, int N>
Real Volume(std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> const& b0) {
    Real volume = 1;
    for (int i = 0; i < N; ++i) { volume *= std::get<1>(b0)[i] - std::get<0>(b0)[i]; }
    return volume;
};

template <typename U, int N>
std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> BoundBox(
    std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> const& l,
    std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> const& r) {
    std::tuple<simpla::nTuple<U, N>, simpla::nTuple<U, N>> res;
    for (int i = 0; i < N; ++i) {
        std::get<0>(res)[i] = std::min(std::get<0>(l)[i], std::get<0>(r)[i]);
        std::get<1>(res)[i] = std::max(std::get<1>(l)[i], std::get<1>(r)[i]);
    }
    return std::move(res);
};
///**
// * @ingroup geometry
// * @{
// * @defgroup geo_algorithm Geometric Algorithm
// * @{
// */
//
// template <typename T, typename T1>
// bool in_box(T1 const& p0, T1 const& p1, simpla::nTuple<T, 2> const& x0) {
//    return (x0[0] >= p0[0]) && (x0[1] >= p0[1]) && (x0[0] < p1[0]) && (x0[1] < p1[1]);
//}
//
// template <typename T0, typename T1>
// bool in_box(std::tuple<T1, T1> const& b, T0 const& x0) {
//    return (x0[0] >= std::get<0>(b)[0]) && (x0[1] >= std::get<0>(b)[1]) && (x0[2] >= std::get<0>(b)[2]) &&
//           (x0[0] < std::get<1>(b)[0]) && (x0[1] < std::get<1>(b)[1]) && (x0[2] < std::get<1>(b)[2]);
//}
//
// template <typename T0, typename T1>
// std::tuple<Real, point_type, point_type> DistanceToBox(std::tuple<T1, T1> const& b, T0 const& x0) {
//    UNIMPLEMENTED;
//    return std::make_tuple(SP_SNaN, x0, point_type{SP_SNaN, SP_SNaN, SP_SNaN});
//}
//
// template <typename T0, typename... Args>
// std::tuple<Real, point_type, point_type> DistanceToBox(T0 const& b, Args&&... args) {
//    UNIMPLEMENTED;
//    return std::make_tuple(SP_SNaN, point_type{SP_SNaN, SP_SNaN, SP_SNaN}, point_type{SP_SNaN, SP_SNaN, SP_SNaN});
//}
//
// template <typename T0, typename T1, typename T2, typename T3>
// std::tuple<Real, Vec3> DistanceToPlane(T0 const& x0, T1 const& p0, T2 const& p1, T3 const& p2) {
//    Vec3 n;
//
//    n = cross(p1 - p0, p2 - p1);
//
//    n /= dot(n, n);
//
//    return std::make_tuple(dot(p0 - x0, n), std::move(n));
//}

template <typename TS, int N>
void extent_box(simpla::nTuple<TS, N>* x0, simpla::nTuple<TS, N>* x1, const TS* x) {
    for (int i = 0; i < N; ++i) {
        (*x0)[i] = std::min(x[i], (*x0)[i]);
        (*x1)[i] = std::max(x[i], (*x1)[i]);
    }
}

template <typename TS, int N, typename T1, typename... Others>
void extent_box(simpla::nTuple<TS, N>* x0, simpla::nTuple<TS, N>* x1, T1 const& y0, Others&&... others) {
    extent_box(x0, x1, y0);
    extent_box(x0, x1, std::forward<Others>(others)...);
}

template <typename T0, typename T1, typename TP>
auto bound_box(T0 const& p0, T1 const& p1) -> std::tuple<decltype(*p0), decltype(*p0)> {
    typedef decltype(*p0) point_type;

    std::tuple<point_type, point_type> res{*p0, *p1};

    for (auto it = p0; it != p1; ++it) { extent_box(*it, &std::get<0>(res), &std::get<1>(res)); }

    return std::move(res);
};

/**
 *
 *
 *     x  o
 *       /|
 *      / | d
 *     o--o---------------o
 *  p0   s                      p1
 *
 *
 *
 * @param x coordinates of point
 * @param ib begin iterator of polygon
 * @param ie end iterator of polygon
 * @return   <d,s,p0,p1>
 *
 * @param v
 * @param l0,l1 P= P0*(1-s)+ P1*s
 * @param tolerance
 * @return tuple( distance, u )
 */
template <typename T0, typename T1, typename T2>
std::tuple<Real, Real> NearestPointPointToLine(T0 const& v, T1 const& l0, T2 const& l1,
                                               Real tolerance = SP_GEO_DEFAULT_TOLERANCE) {
    Real u = dot(v - l0, l1 - l0) / dot(l1 - l0, l1 - l0);
    auto d = v - l0 - u * (l1 - l0);
    Real d2 = dot(d, d);
    return std::make_tuple(d2, u);
}
/**
 *
 * @param P0,P1 [P0,P1) line segment one P= P0*(1-s)+ P1*s
 * @param Q0,Q1 [Q0,Q1) line segment two Q= Q0*(1-t)+t*Q1
 * @param flag 0: line to line
 *             1: line segment to line segment
 * @return < dist2,s,t>
 *         if  parallel then  s,t=nan
 *         if  intersected than dist2=0  else dist2=d^2
 *
 */
template <typename T0, typename T1, typename T2, typename T3>
std::tuple<Real, Real, Real> NearestPointLineToLine(T0 const& P0, T1 const& P1, T2 const& Q0, T3 const& Q1,
                                                    Real tolerance = SP_GEO_DEFAULT_TOLERANCE) {
    Real s = SP_SNaN;
    Real t = SP_SNaN;
    Real dist2 = SP_SNaN;

    auto u = P1 - P0;
    auto v = Q1 - Q0;
    auto w0 = P0 - Q0;

    // @ref http://geomalgorithms.com/a07-_distance.html
    Real a = dot(u, u);
    Real b = dot(u, v);
    Real c = dot(v, v);
    Real d = dot(u, w0);
    Real e = dot(v, w0);

    if (std::abs(a * c - b * b) < tolerance) {
        // two lines are parallel
        s = 0;

        dist2 = std::get<0>(NearestPointPointToLine(P0, Q0, Q1));
    } else {
        s = (b * e - c * d) / (a * c - b * b);

        t = (a * e - b * d) / (a * c - b * b);

        //		auto w = w0
        //				+ ((b * e - c * d) * u - (a * e - b * d) * v) / (a * c - b *
        // b);
        //		dist = dot(w, w);
    }

    return std::make_tuple(dist2, s, t);
}

/**
 *
 * @param P0
 * @param Q0 , Q1 , Q2  Plane Q=Q0+u*(Q1-Q0)+v*(Q2-Q0)
 * @param tolerance
 * @return <dist2 ,u,v >
 *           if  parallel then  s,u,v=nan
 *           if  intersected than dist2=0
 *
 */
template <typename T0, typename T1, typename T2, typename T3>
std::tuple<Real, Real, Real> NearestPointPointToPlane(T0 const& P0, T1 const& Q0, T2 const& Q1, T3 const& Q2,
                                                      Real tolerance = SP_GEO_DEFAULT_TOLERANCE) {
    Real dist2, u, v;
    UNIMPLEMENTED;
    return std::make_tuple(dist2, u, v);
}

/**
 *
 * @param l0,l1    Line  L=l0+s*(l1-l0)
 * @param p0,p1,p2 Plane P=p0+u*(p1-p0)+v*(p2-p0)
 *
 *  @return <dist2 ,s,u,v >
 *           if  parallel then  s,u,v=nan
 *           if  intersected than dist2=0
 *          nearest point on plane
 *          nearest point on line segment P= (1-s)*P0+s*P1
 *          dist = |P  Q|. |(Q1-Q0) x (Q2-Q0)|
 *
 */
template <typename T0, typename T1, typename T2, typename T3, typename T4>
std::tuple<Real, Real, Real, Real> GetIntersectionLineToPlane(T0 const& l0, T1 const& l1, T2 const& p0, T3 const& p1,
                                                              T4 const& p2, Real tolerance = SP_GEO_DEFAULT_TOLERANCE) {
    Real dist2, s, u, v;

    UNIMPLEMENTED;
    return std::make_tuple(dist2, s, u, v);
}
template <typename T0, typename T1, typename T2, typename T3, typename T4, typename T5>
std::tuple<Real, Real, Real, Real, Real> NearestPointTriangleToPlane(T0 const& l0, T1 const& l1, T2 const& l2,
                                                                     T3 const& p0, T4 const& p1, T5 const& p2,
                                                                     Real tolerance = SP_GEO_DEFAULT_TOLERANCE) {
    Real dist2, u0, v0, u1, v1;

    UNIMPLEMENTED;
    return std::make_tuple(dist2, u0, v0, u1, v1);
}
//
// template <typename TI, typename TX>
// std::tuple<Real, Real, TI, TI> NearestPointLineToPolylines(TX const& x, TI const& ib, TI const& ie,
//                                                           Vec3 normal_vec = Vec3({0, 0, 1})) {
//    auto it = make_cycle_iterator(ib, ie);
//
//    Real min_dist2 = std::numeric_limits<Real>::max();
//
//    Real res_s = 0;
//
//    TI res_p0, res_p1;
//
//    TI p0, p1;
//
//    Real dist;
//
//    p1 = it;
//
//    while (it != ie) {
//        p0 = p1;
//
//        ++it;
//
//        Vec3 u, v, d;
//
//        u = x - *p0;
//        v = *p1 - *p0;
//
//        Real v2 = dot(v, v);
//
//        Real s = dot(u, v) / v2;
//
//        if (s < 0) {
//            s = 0;
//        } else if (s > 1) {
//            s = 1;
//        }
//
//        d = u - v * s;
//
//        Real dist2 = dot(d, d);
//
//        if (min_dist2 > dist2 || (min_dist2 == dist2 && s == 0)) {
//            res_p0 = p0;
//            res_p1 = p1;
//            res_s = s;
//            min_dist2 = dist2;
//        }
//    }
//
//    return std::forward_as_tuple(std::sqrt(min_dist2), res_s, res_p0, res_p1);
//}
// template <typename T0, typename T1, typename T2>
// Real GetIntersectionionLineToPolygons(T0 const& p0, T1 const& p1, T2 const& polygon) {
//    auto it = polygon.begin();
//
//    auto q0 = *it;
//    auto q1 = *(++it);
//    auto q2 = *(++it);
//
//    Vec3 n;
//    n = cross(q2 - q1, q1 - q0);
//    n /= std::sqrt(static_cast<Real>(dot(n, n)));
//
//    it = polygon.begin();
//
//    while (it != polygon.end()) {
//        auto q0 = *it;
//        auto q1 = *(++it);
//        q0 -= dot(q0, n) * n;
//        q1 -= dot(q1, n) * n;
//    }
//}
// namespace mesh_intersection
//{
// enum
//{
//	HALF_OPEN, CLOSED
//};
//
// template<typename T0, typename T1, typename T2>
// Real to_polygons(T0 const & p0, T1 const & p1, T2 const & polygen,
//		std::vector<coordinate_tuple> *res)
//{
//
//}
//}  // namespace box_intersection

/**
 *
 *
 *     x' o
 *    v+ /
 *      /
 *     o------------------o
 *  p0  \                      p1
 *    v- \
 *        o
 *        x
 *
 *
 */
/**
 *  reflect a vector `v` if `inner_prodcut(v,normal)>0`
 * @param v
 * @param normal
 * @param reflect_if_out
 * @return
 */
template <typename T0, typename T1>
Vec3 reflect(T0 const& v, T1 const& normal) {
    return v - (2 * dot(v, normal) / dot(normal, normal)) * normal;
}

/**
 *
 * @param v
 * @param p0
 * @param p1
 * @param p2
 * @return
 */
template <typename T0, typename T1, typename T2, typename T3>
inline Vec3 reflect_vector_by_plane(T0 const& v, T1 const& p0, T2 const& p1, T3 const& p2) {
    return reflect(v, cross(p1 - p0, p2 - p0));
}

/**
 * reflect a point `x0` if `x0` at the out side of plane `(p0p1p2)`
 * @param x0
 * @param p0
 * @param p1
 * @param p2
 * @return
 */
template <typename T0, typename T1, typename T2, typename T3>
inline Vec3 reflect_point_by_plane(T0 const& x0, T1 const& p0, T2 const& p1, T3 const& p2) {
    return p0 + reflect(x0 - p0, cross(p1 - p0, p2 - p0));
}

// template<typename TC>
// std::tuple<Real, Real> intersection_line_and_triangle(TC const& l0,
//		TC const & l1, TC const & p0, TC const & p1, TC const & p2)
//{
//	Real s, t;
//
//	auto u = p1 - p0;
//	auto v = p2 - p0;
//
//	// @ref http://geomalgorithms.com/a07-_distance.html
//	Real a = dot(x1 - x0, x1 - x0);
//	Real b = dot(x1 - x0, y1 - y0);
//	Real c = dot(y1 - y0, y1 - y0);
//	Real d = dot(y0 - x0, x1 - x0);
//	Real e = dot(y0 - x0, y1 - y0);
//
//	if (std::abs(a * c - b * b) < EPSILON)
//	{
//		//two lines are parallel
//		s = 0;
//		t = d / b;
//	}
//	else
//	{
//		s = (b * e - c * d) / (a * c - b * b);
//		t = (a * e - b * d) / (a * c - b * b);
//	}
//	return std::make_tuple(s, t);
//}

template <typename TS, int NDIMS, typename TV>
bool box_intersection(nTuple<TS, NDIMS> const& l_b, nTuple<TS, NDIMS> const& l_e, TV* r_b, TV* r_e) {
    bool has_overlap = false;

    nTuple<TS, NDIMS> r_start = *r_b;
    nTuple<TS, NDIMS> r_count = *r_e - *r_b;

    nTuple<TS, NDIMS> l_count = l_e - l_b;

    for (int i = 0; i < NDIMS; ++i) {
        if (r_start[i] + r_count[i] <= l_b[i] || r_start[i] >= l_b[i] + l_count[i]) return false;

        TS start = std::max(l_b[i], r_start[i]);
        TS end = std::min(l_b[i] + l_count[i], r_start[i] + r_count[i]);

        if (end > start) {
            r_start[i] = start;
            r_count[i] = end - start;

            has_overlap = true;
        }
    }

    if (has_overlap) {
        for (int i = 0; i < NDIMS; ++i) {
            (*r_b)[i] = r_start[i];
            (*r_e)[i] = r_start[i] + r_count[i];
        }
    }
    return has_overlap;
}

template <typename TL, typename TR>
bool box_intersection(TL const& l, TR* r) {
    return box_intersection(std::get<0>(l), std::get<1>(l), &std::get<0>(*r), &std::get<1>(*r));
};
//
// template<size_t DIM, typename TR, typename TRange>
// bool PointInRectangle(nTuple<TR, DIM> const &x, TRange const & entity_id_range)
//{
//	bool res = true;
//
//	auto min = std::Serialize<0>(entity_id_range);
//
//	auto max = std::Serialize<1>(entity_id_range);
//
//	for (size_t i = 0; i < DIM; ++i)
//	{
//		res = res && (x[i] >= min[i] && x[i] <= max[i]);
//	}
//	return res;
//}
//
// template<typename TI>
// bool clipping(int m_ndims_, TI const * l_begin, TI const * l_end, TI * r_begin,
//		TI * r_end)
//{
//	bool has_overlap = false;
//
//	for (int i = 0; i < m_ndims_; ++i)
//	{
//		if (r_end[i] <= l_begin[i] || r_begin[i] >= l_end[i])
//			return false;
//
//		auto begin = std::max(l_begin[i], r_begin[i]);
//		auto end = std::min(l_end[i], r_end[i]);
//
//		if (end > begin)
//		{
//			r_begin[i] = begin;
//			r_end[i] = end;
//
//			has_overlap = true;
//		}
//	}
//
//	return has_overlap;
//}
// template<typename TS, size_t NDIMS>
// bool clipping(nTuple<TS, NDIMS> l_begin, nTuple<TS, NDIMS> l_end,
//		nTuple<TS, NDIMS> *pr_begin, nTuple<TS, NDIMS> *pr_end)
//{
//	return clipping(NDIMS, &l_begin[0], &l_end[0], &(*pr_begin)[0],
//			&(*pr_end)[0]);
//}

// inline nTuple<Real,3> Distance(nTuple<2, nTuple<Real,3>> p, nTuple<Real,3> const &x)
//{
//	nTuple<Real,3> u, v;
//	v = p[1] - p[0];
//	u = Cross(Cross(x - p[0], v), v) / _DOT3(v, v);
//	return std::Move(u);
//}
// template<typename TDict, typename TModel, typename TSurface>
// void createSurface(TDict const & dict, TModel const & geometry, TSurface * surf)
//{
//	if (dict["Width"].is_number())
//	{
//		createSurface(geometry, dict["Width"].template as<Real>(), surf);
//	}
//	else
//	{
//		WARNING << "illegal configuation!";
//	}
//}
// template<typename TModel, typename TSurface>
// void createSurface(TModel const & geometry, Real width, TSurface * surf)
//{
////
//////	typedef typename TSurface::plane_type plane_type;
////
//////	auto extent = geometry.get_extents();
//////	auto topology_dims = geometry.get_dimensions();
//////	auto xmin = extent.first;
//////	auto xmax = extent.second;
//////	auto d = geometry.get_dx();
//////	nTuple<Real,3> x0 = { 0, 0, 0 };
//////	nTuple<Real,3> x1 = { d[0], 0, 0 };
//////	nTuple<Real,3> x2 = { 0, d[1], 0 };
//////	nTuple<Real,3> x3 = { 0, 0, d[2] };
//////
//////	for (auto s : geometry.select(VERTEX))
//////	{
//////		auto x = geometry.get_coordinates(s);
//////
//////		if (x[0] < xmin[0] + width)
//////		{
//////			surf->insert(s, plane_type( { x0, x1, x2 }));
//////			continue;
//////		}
//////		else if (x[0] > xmax[0] - width)
//////		{
//////			surf->insert(s, plane_type( { x0, x2, x1 }));
//////			continue;
//////		}
//////
//////		if (x[1] < xmin[1] + width)
//////		{
//////			surf->insert(s, plane_type( { x0, x1, x2 }));
//////			continue;
//////		}
//////		else if (x[1] > xmax[1] + width)
//////		{
//////			surf->insert(s, plane_type( { x0, x1, x2 }));
//////			continue;
//////		}
//////
//////		if (x[2] < xmin[2] + width)
//////		{
//////			surf->insert(s, plane_type( { x0, x1, x2 }));
//////			continue;
//////		}
//////		else if (x[2] > xmax[2] - width)
//////		{
//////			surf->insert(s, plane_type( { x0, x1, x2 }));
//////			continue;
//////		}
//////
//////	}
//}
//
///**
// * decompose a N-dimensional block entity_id_range [b,e) into 'num_part' parts,
// * and return the 'proc_num'th part [ob,oe)
// * @_fdtd_param b minus index of block entity_id_range
// * @_fdtd_param e maxim index of block entity_id_range
// * @_fdtd_param num_part
// * @_fdtd_param part_num
// * @_fdtd_param decompose_method select decompose algorithm (UNIMPLEMENTED)
// * @return  the 'proc_num'th part [ob,oe)
// * if 'num_part==0' return [b,e)
// */
// template<typename TI, size_t N>
// std::tuple<nTuple<TI, N>, nTuple<TI, N>> block_decompose(
//		nTuple<TI, N> const & b, nTuple<TI, N> const & e, int num_part = 0,
//		int part_num = 0, size_t decompose_method = 0UL)
//{
//	if (num_part == 0)
//	{
//		return std::forward_as_tuple(b, e);
//	}
//
//	nTuple<TI, N> ob, oe;
//
//	TI length = 0;
//	int dim_num = 0;
//	for (int i = 0; i < N; ++i)
//	{
//		if (e[i] - b[i] > length)
//		{
//			length = e[i] - b[i];
//			dim_num = i;
//		}
//	}
//
//	ob = b;
//	oe = e;
//	ob[dim_num] = b[dim_num] + (length * part_num) / num_part;
//	oe[dim_num] = b[dim_num] + (length * (part_num + 1)) / num_part;
//
//	return std::forward_as_tuple(ob, oe);
//}

/** @}
 * @}
 **/

}  // namespace geometry
}  // namespace simpla

#endif /* GEOMETRY_ALGORITHM_H_ */
