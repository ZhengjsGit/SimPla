/**
 * @file  fdm.h
 *
 *  Created on: 2014年9月23日
 *      Author: salmon
 */

#ifndef FDM_H_
#define FDM_H_

#include <complex>
#include <cstddef>
#include <type_traits>

#include "../../gtl/expression_template.h"
#include "../../gtl/ntuple.h"
#include "../../gtl/primitives.h"
#include "../calculus.h"
#include "../mesh.h"

namespace simpla
{

template<typename ... > class _Field;
template<typename ...>class field_traits;

/** @ingroup diff_scheme
 *  @brief   FdMesh
 */
struct FiniteDiffMethod
{
public:

	typedef FiniteDiffMethod this_type;
	typedef Real scalar_type;

	FiniteDiffMethod()
	{
	}

	FiniteDiffMethod(this_type const & r) = default;

	~FiniteDiffMethod() = default;

	this_type & operator=(this_type const &) = default;
	//***************************************************************************************************
	// General algebra
	//***************************************************************************************************

	template<typename geometry_type, typename ...Others>
	static Real policy_calculate(geometry_type const & geo, Real v, Others &&... s)
	{
		return v;
	}

	template<typename geometry_type, typename ...Others>
	static int policy_calculate(geometry_type const & geo, int v, Others &&... s)
	{
		return v;
	}

	template<typename geometry_type, typename ...Others>
	static std::complex<Real> policy_calculate(geometry_type const & geo,
			std::complex<Real> v, Others &&... s)
	{
		return v;
	}

	template<typename geometry_type, typename T, size_t ...N, typename ...Others>
	static nTuple<T, N...> const& policy_calculate(geometry_type const & geo,
			nTuple<T, N...> const& v, Others &&... s)
	{
		return v;
	}

	template<typename geometry_type, typename ...T, typename ...Others>
	static inline typename nTuple_traits<nTuple<Expression<T...>>> ::primary_type
	policy_calculate(geometry_type const & geo,nTuple<Expression<T...>> const & v, Others &&... s)
	{
		typename nTuple_traits<nTuple<Expression<T...>>> ::primary_type res;
		res=v;
		return std::move(res);
	}

	template<typename geometry_type,typename TM,typename TV, typename ... Others,typename ... Args>
	static inline TV
	policy_calculate(geometry_type const & geo,_Field<TM, TV, Others...> const &f, Args && ... s)
	{
		return get_value(f,std::forward<Args>(s)...);
	}

	template<typename geometry_type,typename TOP, typename TL, typename TR, typename ...Others>
	static inline typename field_traits< _Field<Expression<TOP, TL, TR>>>::value_type
	policy_calculate(geometry_type const & geo,_Field<Expression<TOP, TL, TR>> const &f, Others &&... s)
	{
		return f.op_(policy_calculate( geo,f.lhs,std::forward<Others>(s)...),
				policy_calculate(geo,f.rhs,std::forward<Others>(s)...));
	}

	template<typename geometry_type,typename TOP, typename TL, typename ...Others>
	static inline typename field_traits< _Field<Expression<TOP, TL,std::nullptr_t>>>::value_type
	policy_calculate(geometry_type const & geo,_Field<Expression<TOP, TL,std::nullptr_t>> const &f, Others &&... s)
	{
		return f.op_(policy_calculate(geo,f.lhs,std::forward<Others>(s)...) );
	}

	//***************************************************************************************************
	// Exterior algebra
	//***************************************************************************************************

	template<typename geometry_type,typename T>
	static inline typename field_traits<_Field<_impl::ExteriorDerivative<VERTEX,T> >>::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::ExteriorDerivative<VERTEX,T> > const & f, typename geometry_type::id_type s)
	{
		constexpr typename geometry_type::id_type D = geometry_type::delta_index(s);

		return (policy_calculate(geo,f.lhs, s + D) * geo.volume(s + D)
				- policy_calculate(geo,f.lhs, s - D) * geo.volume(s - D)) * geo.inv_volume(s);
	}

	template<typename geometry_type,typename T>
	static inline typename field_traits<_Field<_impl::ExteriorDerivative< EDGE,T> >>::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::ExteriorDerivative<EDGE,T> > const & expr, typename geometry_type::id_type s)
	{

		typename geometry_type::id_type X = geometry_type::delta_index(geometry_type::dual(s));
		typename geometry_type::id_type Y = geometry_type::roate(X);
		typename geometry_type::id_type Z = geometry_type::inverse_roate(X);

		return (
				(
						policy_calculate(geo,expr.lhs, s + Y) * geo.volume(s + Y) //
						- policy_calculate(geo,expr.lhs, s - Y) * geo.volume(s - Y) ) - (
						policy_calculate(geo,expr.lhs, s + Z) * geo.volume(s + Z)//
						- policy_calculate(geo,expr.lhs, s - Z) * geo.volume(s - Z)//
				)

		) * geo.inv_volume(s);

	}

	template<typename geometry_type,typename T>
	static constexpr inline typename field_traits<_Field<_impl::ExteriorDerivative< FACE,T> >>::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::ExteriorDerivative<FACE,T> > const & expr, typename geometry_type::id_type s)
	{
		return (
				policy_calculate(geo,expr.lhs, s + geometry_type::_DI) * geo.volume(s + geometry_type::_DI)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DI) * geo.volume(s - geometry_type::_DI)
				+ policy_calculate(geo,expr.lhs, s + geometry_type::_DJ) * geo.volume(s + geometry_type::_DJ)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DJ) * geo.volume(s - geometry_type::_DJ)
				+ policy_calculate(geo,expr.lhs, s + geometry_type::_DK) * geo.volume(s + geometry_type::_DK)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DK) * geo.volume(s - geometry_type::_DK)

		) * geo.inv_volume(s)

		;
	}
//
////	template<typename geometry_type,typename TM, size_t IL, typename TL> void policy_calculate(
////			_impl::ExteriorDerivative, _Field<Domain<TM, IL>, TL> const & f,
////					typename geometry_type::id_type   s)  = delete;
////
////	template<typename geometry_type,typename TM, size_t IL, typename TL> void policy_calculate(
////			_impl::CodifferentialDerivative,
////			_Field<TL...> const & f, 		typename geometry_type::id_type   s)  = delete;

	template<typename geometry_type,typename T >
	static constexpr inline typename field_traits<_Field< _impl::CodifferentialDerivative< EDGE, T> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::CodifferentialDerivative< EDGE, T>> const & expr,
			typename geometry_type::id_type s)
	{
		return
		-(
				policy_calculate(geo,expr.lhs, s + geometry_type::_DI) * geo.dual_volume(s + geometry_type::_DI)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DI) * geo.dual_volume(s - geometry_type::_DI)
				+ policy_calculate(geo,expr.lhs, s + geometry_type::_DJ) * geo.dual_volume(s + geometry_type::_DJ)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DJ) * geo.dual_volume(s - geometry_type::_DJ)
				+ policy_calculate(geo,expr.lhs, s + geometry_type::_DK) * geo.dual_volume(s + geometry_type::_DK)
				- policy_calculate(geo,expr.lhs, s - geometry_type::_DK) * geo.dual_volume(s - geometry_type::_DK)

		) * geo.inv_dual_volume(s)

		;

	}

	template<typename geometry_type,typename T >
	static inline typename field_traits<_Field< _impl::CodifferentialDerivative< FACE, T> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::CodifferentialDerivative< FACE, T>> const & expr,
			typename geometry_type::id_type s)
	{

		typename geometry_type::id_type X = geometry_type::delta_index(s);
		typename geometry_type::id_type Y = geometry_type::roate(X);
		typename geometry_type::id_type Z = geometry_type::inverse_roate(X);

		return

		-(
				(policy_calculate(geo,expr.lhs, s + Y) * (geo.dual_volume(s + Y))
						- policy_calculate(geo,expr.lhs, s - Y) * (geo.dual_volume(s - Y)))

				- (policy_calculate(geo,expr.lhs, s + Z) * (geo.dual_volume(s + Z))
						- policy_calculate(geo,expr.lhs, s - Z) * (geo.dual_volume(s - Z)))

		) * geo.inv_dual_volume(s)

		;
	}

	template<typename geometry_type,typename T >
	static inline typename field_traits<_Field< _impl::CodifferentialDerivative< VOLUME, T> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::CodifferentialDerivative< VOLUME, T>> const & expr,
			typename geometry_type::id_type s)
	{
		typename geometry_type::id_type D = geometry_type::delta_index(geometry_type::dual(s));
		return

		-(

				policy_calculate(geo,expr.lhs, s + D) * (geo.dual_volume(s + D)) //
				- policy_calculate(geo,expr.lhs, s - D) * (geo.dual_volume(s - D))

		) * geo.inv_dual_volume(s)

		;
	}

////***************************************************************************************************
//
////! Form<IR> ^ Form<IR> => Form<IR+IL>

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<VERTEX,VERTEX,TL,TR> > >::value_type
	calculate(geometry_type const & geo,_Field<_impl::Wedge<VERTEX,VERTEX,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		return (policy_calculate(expr.lhs, s) * policy_calculate(expr.rhs, s));
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<VERTEX,EDGE,TL,TR> > >::value_type
	policy_calculate(geometry_type& geo,_Field<_impl::Wedge<VERTEX,EDGE,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto X = geometry_type::delta_index(s);

		return (policy_calculate(expr.lhs, s - X) + policy_calculate(expr.lhs, s + X)) * 0.5
		* policy_calculate(expr.rhs, s);
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<VERTEX,FACE,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<VERTEX,FACE,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto X = geometry_type::delta_index(geometry_type::dual(s));
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);

		return (

				policy_calculate(geo,expr.lhs, (s - Y) - Z) +
				policy_calculate(geo,expr.lhs, (s - Y) + Z) +
				policy_calculate(geo,expr.lhs, (s + Y) - Z) +
				policy_calculate(geo,expr.lhs, (s + Y) + Z)

		) * 0.25 * policy_calculate(geo,expr.rhs, s);
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<VERTEX,VOLUME,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<VERTEX,VOLUME,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{

		auto const & l =expr.lhs;
		auto const & r =expr.rhs;

		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return (

				policy_calculate(geo,l, ((s - X) - Y) - Z) +

				policy_calculate(geo,l, ((s - X) - Y) + Z) +

				policy_calculate(geo,l, ((s - X) + Y) - Z) +

				policy_calculate(geo,l, ((s - X) + Y) + Z) +

				policy_calculate(geo,l, ((s + X) - Y) - Z) +

				policy_calculate(geo,l, ((s + X) - Y) + Z) +

				policy_calculate(geo,l, ((s + X) + Y) - Z) +

				policy_calculate(geo,l, ((s + X) + Y) + Z)

		) * 0.125 * policy_calculate(geo,r, s);
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<EDGE,VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<EDGE,VERTEX,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{

		auto const & l =expr.lhs;
		auto const & r =expr.rhs;

		auto X = geometry_type::delta_index(s);
		return policy_calculate(geo,l, s) * (policy_calculate(geo,r, s - X) + policy_calculate(geo,r, s + X))
		* 0.5;
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<EDGE,EDGE,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<EDGE,EDGE,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & l =expr.lhs;
		auto const & r =expr.rhs;

		auto Y = geometry_type::delta_index(geometry_type::roate(geometry_type::dual(s)));
		auto Z = geometry_type::delta_index(geometry_type::inverse_roate(geometry_type::dual(s)));

		return ((policy_calculate(geo,l, s - Y) + policy_calculate(geo,l, s + Y))
				* (policy_calculate(geo,l, s - Z) + policy_calculate(geo,l, s + Z)) * 0.25);
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<EDGE,FACE,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<EDGE,FACE,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & l =expr.lhs;
		auto const & r =expr.rhs;
		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return

		(

				(policy_calculate(geo,l, (s - Y) - Z)
						+ policy_calculate(geo,l, (s - Y) + Z)
						+ policy_calculate(geo,l, (s + Y) - Z)
						+ policy_calculate(geo,l, (s + Y) + Z))
				* (policy_calculate(geo,r, s - X) + policy_calculate(geo,r, s + X))
				+

				(policy_calculate(geo,l, (s - Z) - X)
						+ policy_calculate(geo,l, (s - Z) + X)
						+ policy_calculate(geo,l, (s + Z) - X)
						+ policy_calculate(geo,l, (s + Z) + X))
				* (policy_calculate(geo,r, s - Y)
						+ policy_calculate(geo,r, s + Y))
				+

				(policy_calculate(geo,l, (s - X) - Y)
						+ policy_calculate(geo,l, (s - X) + Y)
						+ policy_calculate(geo,l, (s + X) - Y)
						+ policy_calculate(geo,l, (s + X) + Y))
				* (policy_calculate(geo,r, s - Z)
						+ policy_calculate(geo,r, s + Z))

		) * 0.125;
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<FACE,VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<FACE,VERTEX,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & l =expr.lhs;
		auto const & r =expr.rhs;
		auto Y = geometry_type::delta_index(geometry_type::roate(geometry_type::dual(s)));
		auto Z = geometry_type::delta_index(geometry_type::inverse_roate(geometry_type::dual(s)));

		return policy_calculate(geo,l, s)
		* (policy_calculate(geo,r, (s - Y) - Z) + policy_calculate(geo,r, (s - Y) + Z)
				+ policy_calculate(geo,r, (s + Y) - Z)
				+ policy_calculate(geo,r, (s + Y) + Z)) * 0.25;
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<FACE,EDGE,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<FACE,EDGE,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & l =expr.lhs;
		auto const & r =expr.rhs;
		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return

		(

				(policy_calculate(geo,r, (s - Y) - Z)
						+ policy_calculate(geo,r, (s - Y) + Z)
						+ policy_calculate(geo,r, (s + Y) - Z)
						+ policy_calculate(geo,r, (s + Y) + Z))
				* (policy_calculate(geo,l, s - X) + policy_calculate(geo,l, s + X))

				+ (policy_calculate(geo,r, (s - Z) - X)
						+ policy_calculate(geo,r, (s - Z) + X)
						+ policy_calculate(geo,r, (s + Z) - X)
						+ policy_calculate(geo,r, (s + Z) + X))
				* (policy_calculate(geo,l, s - Y)
						+ policy_calculate(geo,l, s + Y))

				+ (policy_calculate(geo,r, (s - X) - Y)
						+ policy_calculate(geo,r, (s - X) + Y)
						+ policy_calculate(geo,r, (s + X) - Y)
						+ policy_calculate(geo,r, (s + X) + Y))
				* (policy_calculate(geo,l, s - Z)
						+ policy_calculate(geo,l, s + Z))

		) * 0.125;
	}

	template<typename geometry_type,typename TL,typename TR>
	static inline typename field_traits<_Field<_impl::Wedge<VOLUME,VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::Wedge<VOLUME,VERTEX,TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & l =expr.lhs;
		auto const & r =expr.rhs;
		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return

		policy_calculate(geo,l, s) * (

				policy_calculate(geo,r, ((s - X) - Y) - Z) + //
				policy_calculate(geo,r, ((s - X) - Y) + Z) +//
				policy_calculate(geo,r, ((s - X) + Y) - Z) +//
				policy_calculate(geo,r, ((s - X) + Y) + Z) +//
				policy_calculate(geo,r, ((s + X) - Y) - Z) +//
				policy_calculate(geo,r, ((s + X) - Y) + Z) +//
				policy_calculate(geo,r, ((s + X) + Y) - Z) +//
				policy_calculate(geo,r, ((s + X) + Y) + Z)//

		) * 0.125;
	}
//
////***************************************************************************************************

	template<typename geometry_type,typename T >
	static inline typename field_traits<_Field< _impl::HodgeStar< VOLUME, T> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::HodgeStar< VOLUME, T>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;
//		auto X = geo.DI(0,s);
//		auto Y = geo.DI(1,s);
//		auto Z =geo.DI(2,s);
//
//		return
//
//		(
//
//		policy_calculate(geo,f,((s + X) - Y) - Z)*geo.inv_volume(((s + X) - Y) - Z) +
//
//		policy_calculate(geo,f,((s + X) - Y) + Z)*geo.inv_volume(((s + X) - Y) + Z) +
//
//		policy_calculate(geo,f,((s + X) + Y) - Z)*geo.inv_volume(((s + X) + Y) - Z) +
//
//		policy_calculate(geo,f,((s + X) + Y) + Z)*geo.inv_volume(((s + X) + Y) + Z) +
//
//		policy_calculate(geo,f,((s - X) - Y) - Z)*geo.inv_volume(((s - X) - Y) - Z) +
//
//		policy_calculate(geo,f,((s - X) - Y) + Z)*geo.inv_volume(((s - X) - Y) + Z) +
//
//		policy_calculate(geo,f,((s - X) + Y) - Z)*geo.inv_volume(((s - X) + Y) - Z) +
//
//		policy_calculate(geo,f,((s - X) + Y) + Z)*geo.inv_volume(((s - X) + Y) + Z)
//
//		) * 0.125 * geo.volume(s);

		return policy_calculate(geo,f, s) /** geo._impl::HodgeStarVolumeScale(s)*/;
	}

//	template<typename geometry_type,typename TM, typename TL, typename TR> void policy_calculate(
//			_impl::InteriorProduct, nTuple<TR, G::ndims> const & v,
//			_Field<Domain<TM, VERTEX>, TL> const & f,
//					typename geometry_type::id_type   s)  = delete;

	template<typename geometry_type,typename TL,typename TR >
	static inline typename field_traits<_Field< _impl::InteriorProduct<EDGE, VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::InteriorProduct<EDGE, VERTEX, TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;
		auto const & v =expr.rhs;

		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return

		(policy_calculate(geo,f, s + X) - policy_calculate(geo,f, s - X)) * 0.5 * v[0] //
		+ (policy_calculate(geo,f, s + Y) - policy_calculate(geo,f, s - Y)) * 0.5 * v[1]//
		+ (policy_calculate(geo,f, s + Z) - policy_calculate(geo,f, s - Z)) * 0.5 * v[2];
	}

	template<typename geometry_type,typename TL,typename TR >
	static inline typename field_traits<_Field< _impl::InteriorProduct<FACE, VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::InteriorProduct<FACE, VERTEX, TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;
		auto const & v =expr.rhs;

		size_t n = geometry_type::component_number(s);

		auto X = geometry_type::delta_index(s);
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);
		return

		(policy_calculate(geo,f, s + Y) + policy_calculate(geo,f, s - Y)) * 0.5 * v[(n + 2) % 3] -

		(policy_calculate(geo,f, s + Z) + policy_calculate(geo,f, s - Z)) * 0.5 * v[(n + 1) % 3];
	}

	template<typename geometry_type,typename TL,typename TR >
	static inline typename field_traits<_Field< _impl::InteriorProduct<VOLUME, VERTEX,TL,TR> > >::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::InteriorProduct<VOLUME, VERTEX, TL,TR>> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;
		auto const & v =expr.rhs;
		size_t n = geometry_type::component_number(geometry_type::dual(s));
		size_t D = geometry_type::delta_index(geometry_type::dual(s));

		return (policy_calculate(geo,f, s + D) - policy_calculate(geo,f, s - D)) * 0.5 * v[n];
	}

//**************************************************************************************************
// Non-standard operation

	template<typename geometry_type, size_t IL, typename T > static inline typename field_traits<T>::value_type
	policy_calculate(geometry_type const & geo,_Field<_impl::MapTo<IL, IL, T>> const & f, typename geometry_type::id_type s)
	{
		return policy_calculate(geo,f,s);
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo<EDGE, VERTEX, T>>>::value_type
	map_to(geometry_type const & geo ,_Field<_impl::MapTo<EDGE, VERTEX, T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;

		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return nTuple<
		typename std::remove_reference<decltype(policy_calculate(geo,f,s))>::type, 3>(
				{

					(policy_calculate(geo,f, s - X) + policy_calculate(geo,f, s + X)) * 0.5, //
					(policy_calculate(geo,f, s - Y) + policy_calculate(geo,f, s + Y)) * 0.5,//
					(policy_calculate(geo,f, s - Z) + policy_calculate(geo,f, s + Z)) * 0.5

				});
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo< VERTEX, EDGE,T>>>::value_type
	map_to(geometry_type const & geo ,_Field<_impl::MapTo< VERTEX,EDGE, T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;
		auto n = geometry_type::component_number(s);
		auto D = geometry_type::delta_index(s);

		return ((policy_calculate(geo,f, s - D)[n] + policy_calculate(geo,f, s + D)[n]) * 0.5);
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo< FACE, VERTEX,T>>>::value_type
	map_to(geometry_type const & geo ,_Field<_impl::MapTo< FACE,VERTEX, T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;
		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return nTuple<
		typename std::remove_reference<decltype(policy_calculate(geo,f,s))>::type, 3>(
				{	(

							policy_calculate(geo,f, (s - Y) - Z) +

							policy_calculate(geo,f, (s - Y) + Z) +

							policy_calculate(geo,f, (s + Y) - Z) +

							policy_calculate(geo,f, (s + Y) + Z)

					) * 0.25,

					(

							policy_calculate(geo,f, (s - Z) - X) +

							policy_calculate(geo,f, (s - Z) + X) +

							policy_calculate(geo,f, (s + Z) - X) +

							policy_calculate(geo,f, (s + Z) + X)

					) * 0.25,

					(

							policy_calculate(geo,f, (s - X) - Y) +

							policy_calculate(geo,f, (s - X) + Y) +

							policy_calculate(geo,f, (s + X) - Y) +

							policy_calculate(geo,f, (s + X) + Y)

					) * 0.25

				});
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo< VERTEX,FACE, T>>>::value_type
	map_to(geometry_type const & geo,_Field<_impl::MapTo< VERTEX, FACE, T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;

		auto n = geometry_type::component_number(geometry_type::dual(s));
		auto X = geometry_type::delta_index(geometry_type::dual(s));
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);

		return (

				(

						policy_calculate(geo,f, (s - Y) - Z)[n] +

						policy_calculate(geo,f, (s - Y) + Z)[n] +

						policy_calculate(geo,f, (s + Y) - Z)[n] +

						policy_calculate(geo,f, (s + Y) + Z)[n]

				) * 0.25

		);
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo< FACE,VOLUME, T>>>::value_type
	map_to(geometry_type const & geo,_Field<_impl::MapTo< FACE,VOLUME,T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;

		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return nTuple<
		typename std::remove_reference<decltype(policy_calculate(geo,f,s))>::type,
		3>(
				{

					(policy_calculate(geo,f, s - X) + policy_calculate(geo,f, s + X)) * 0.5, //
					(policy_calculate(geo,f, s - Y) + policy_calculate(geo,f, s + Y)) * 0.5,//
					(policy_calculate(geo,f, s - Z) + policy_calculate(geo,f, s + Z)) * 0.5

				});
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo<VOLUME, FACE, T>>>::value_type
	map_to(geometry_type const & geo,_Field<_impl::MapTo< VOLUME,FACE,T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;

		auto n = geometry_type::component_number(geometry_type::dual(s));
		auto D = geometry_type::delta_index(geometry_type::dual(s));

		return ((policy_calculate(geo,f, s - D)[n] + policy_calculate(geo,f, s + D)[n]) * 0.5);
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo<EDGE, VOLUME, T>>>::value_type
	map_to(geometry_type const & geo,_Field<_impl::MapTo<EDGE, VOLUME,T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;

		auto X = geo.DI(0, s);
		auto Y = geo.DI(1, s);
		auto Z = geo.DI(2, s);

		return nTuple<
		typename std::remove_reference<decltype(policy_calculate(geo,f,s))>::type,
		3>(
				{	(

							policy_calculate(geo,f, (s - Y) - Z) +

							policy_calculate(geo,f, (s - Y) + Z) +

							policy_calculate(geo,f, (s + Y) - Z) +

							policy_calculate(geo,f, (s + Y) + Z)

					) * 0.25,

					(

							policy_calculate(geo,f, (s - Z) - X) +

							policy_calculate(geo,f, (s - Z) + X) +

							policy_calculate(geo,f, (s + Z) - X) +

							policy_calculate(geo,f, (s + Z) + X)

					) * 0.25,

					(

							policy_calculate(geo,f, (s - X) - Y) +

							policy_calculate(geo,f, (s - X) + Y) +

							policy_calculate(geo,f, (s + X) - Y) +

							policy_calculate(geo,f, (s + X) + Y)

					) * 0.25,

				});
	}

	template<typename geometry_type, typename T>
	typename field_traits<_Field<_impl::MapTo< VOLUME,EDGE, T>>>::value_type
	map_to(geometry_type const & geo,_Field<_impl::MapTo< VOLUME,EDGE,T>> const & expr, typename geometry_type::id_type s)
	{
		auto const & f= expr.lhs;
		auto n = geometry_type::component_number(geometry_type::dual(s));
		auto X = geometry_type::delta_index(geometry_type::dual(s));
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);
		return (

				(

						policy_calculate(geo,f, (s - Y) - Z)[n] +

						policy_calculate(geo,f, (s - Y) + Z)[n] +

						policy_calculate(geo,f, (s + Y) - Z)[n] +

						policy_calculate(geo,f, (s + Y) + Z)[n]

				) * 0.25

		);
	}

	// For curl_pdx

	template<typename geometry_type,size_t N , typename T> static inline
	typename field_traits<_Field< _impl::PartialExteriorDerivative< N,EDGE , T > > >::value_type
	policy_calculate(geometry_type const & geo,_Field< _impl::PartialExteriorDerivative< N,EDGE , T >> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;

		auto X = geometry_type::delta_index(geometry_type::dual(s));
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);

		Y = (geometry_type::component_number(Y) == N) ? Y : 0UL;
		Z = (geometry_type::component_number(Z) == N) ? Z : 0UL;

		return (policy_calculate(geo,f, s + Y) - policy_calculate(geo,f, s - Y))
		- (policy_calculate(geo,f, s + Z) - policy_calculate(geo,f, s - Z));
	}

	template<typename geometry_type,size_t N , typename T> static inline
	typename field_traits<_Field< _impl::PartialCodifferentialDerivative< N,FACE , T >> >::value_type
	policy_calculate(geometry_type const & geo,_Field< _impl::PartialCodifferentialDerivative< N,FACE , T >> const & expr,
			typename geometry_type::id_type s)
	{
		auto const & f =expr.lhs;

		auto X = geometry_type::delta_index(s);
		auto Y = geometry_type::roate(X);
		auto Z = geometry_type::inverse_roate(X);

		Y = (geometry_type::component_number(Y) == N) ? Y : 0UL;
		Z = (geometry_type::component_number(Z) == N) ? Z : 0UL;

		return (

				policy_calculate(geo,f, s + Y) * (geo.dual_volume(s + Y))      //
				- policy_calculate(geo,f, s - Y) * (geo.dual_volume(s - Y))//
				- policy_calculate(geo,f, s + Z) * (geo.dual_volume(s + Z))//
				+ policy_calculate(geo,f, s - Z) * (geo.dual_volume(s - Z))//

		) * geo.inv_dual_volume(s);
	}

};

}
// namespace simpla

#endif /* FDM_H_ */
