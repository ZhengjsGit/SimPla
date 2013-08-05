/*
 *  _fetl_impl::vector_calculus.h
 *
 *  Created on: 2012-3-1
 *      Author: salmon
 */

#ifndef VECTOR_CALCULUS_H_
#define VECTOR_CALCULUS_H_

#include "field.h"
#include "geometry.h"
#include "expression.h"
namespace simpla
{

#define DEF_OP_CLASS(_NAME_)                                                  \
template<typename > struct Op##_NAME_;                                        \
template<typename TGeometry, typename TL>                                     \
struct Field<TGeometry, Op##_NAME_<TL> > : public TGeometry                   \
{                                                                             \
	typename ConstReferenceTraits<TL>::type expr;                             \
	Field(TL const & l):TGeometry(l), expr(l){}                               \
	inline auto operator[](size_t s) const                                    \
	DECL_RET_TYPE((TGeometry::grid->_NAME_(expr, s)))                         \
};                                                                            \

DEF_OP_CLASS(Grad)
DEF_OP_CLASS(Diverge)
DEF_OP_CLASS(Curl)
DEF_OP_CLASS(HodgeStar)
DEF_OP_CLASS(ExtriorDerivative)
#undef DEF_OP_CLASS

template<typename TG, typename TR> inline auto //
Grad(Field<Geometry<TG, 0>, TR> const & f)
DECL_RET_TYPE ((Field<Geometry<TG, 1>, OpGrad<Field<Geometry<TG, 0>, TR> > >(f))
)

template<typename TG, typename TR> inline auto //
Diverge(Field<Geometry<TG, 1>, TR> const & f)
DECL_RET_TYPE ((Field<Geometry<TG, 0>, OpDiverge<Field<Geometry<TG, 1>, TR> > >(
						f))
)

template<typename TG, typename TR> inline auto //
Curl(Field<Geometry<TG, 1>, TR> const & f)
DECL_RET_TYPE ((Field<Geometry<TG, 2>, OpCurl<Field<Geometry<TG, 1>, TR> > >(f))
)
template<typename TG, typename TR> inline auto //
Curl(Field<Geometry<TG, 2>, TR> const & f)
DECL_RET_TYPE ((Field<Geometry<TG, 1>, OpCurl<Field<Geometry<TG, 2>, TR> > >(f))
)

template<int, typename > struct OpCurlPD;
template<typename TGeometry, int IPD, typename TL>
struct Field<TGeometry, OpCurlPD<IPD, TL> > : public TGeometry
{
	typename ConstReferenceTraits<TL>::type expr;
	Field(TL const & l) :
			TGeometry(l), expr(l)
	{
	}
	inline auto operator[](size_t s) const
	DECL_RET_TYPE((TGeometry::grid->CurlPD(Int2Type<IPD>(),expr, s)))

};
#define DECL_RET_TYPE_1_ARG(_TYPE_,_ARG_) ->_TYPE_{return (_TYPE_(_ARG_);}

template<int IPD, typename TG, typename TR> inline auto //
CurlPD(Int2Type<2>, Field<Geometry<TG, 2>, TR> const & f)
->Field<Geometry<TG, 1>,OpCurlPD<IPD,Field<Geometry<TG, 2>,TR > > >
{
	return (Field<Geometry<TG, 1>, OpCurlPD<IPD, Field<Geometry<TG, 2>, TR> > >(
			f));
}

template<typename TG, int IL, typename TL> inline  //
auto operator*(Field<Geometry<TG, IL>, TL> const & f)
DECL_RET_TYPE(
		(typename std::conditional<(IL > 0 && IL <= TG::NUM_OF_DIMS),
				Field<Geometry<TG, TG::NUM_OF_DIMS - IL>,
				OpHodgeStar<Field<Geometry<TG, IL>, TL> > >, Zero>::type(f)))

template<typename TG, int IL, typename TL> inline  //
auto d(Field<Geometry<TG, IL>, TL> const & f)
DECL_RET_TYPE(
		(typename std::conditional<(IL > 0 && IL+1 <= TG::NUM_OF_DIMS),
				Field<Geometry<TG, IL+1>,
				OpExtriorDerivative<Field<Geometry<TG, IL>, TL> > >
				, Zero>::type(f)) )

template<typename > struct OpNegate;
template<typename TGeometry, typename TL>
struct Field<TGeometry, OpNegate<TL> > : public TGeometry
{
	typename ConstReferenceTraits<TL>::type expr;
	Field(TL const & l) :
			TGeometry(l), expr(l)
	{
	}
	inline auto operator[](
			size_t s) const ->decltype(((TGeometry::grid->Negate(expr, s))))
	{
		return ((TGeometry::grid->Negate(expr, s)));
	}
};
template<typename TG, int IL, typename TL> inline  //
auto operator-(Field<Geometry<TG, IL>, TL> const & f)
DECL_RET_TYPE(
		( Field<Geometry<TG, IL>,
				OpNegate<Field<Geometry<TG, IL>, TL> > > (f)))

#define DEF_BIOP_CLASS(_NAME_)                                                  \
template<typename, typename > struct Op##_NAME_;                                    \
template<typename TGeometry, typename TL, typename TR>                              \
struct Field<TGeometry, Op##_NAME_<TL, TR> > : public TGeometry                     \
{                                                                                   \
	typename ConstReferenceTraits<TL>::type l_;                                     \
	typename ConstReferenceTraits<TR>::type r_;                                     \
                                                                                    \
	Field(TL const & l, TR const & r) :                                             \
			TGeometry(get_grid(l, r)), l_(l), r_(r)                                 \
	{                                                                               \
	}                                                                               \
	inline auto operator[](size_t s) const                                          \
	DECL_RET_TYPE((TGeometry::grid->_NAME_(l_,r_, s)))                              \
                                                                                    \
};

DEF_BIOP_CLASS(Wedge)
DEF_BIOP_CLASS(Plus)
DEF_BIOP_CLASS(Minus)
DEF_BIOP_CLASS(Multiplies)
DEF_BIOP_CLASS(Divides)
#undef DEF_BIOP_CLASS

template<typename TG, int IL, int IR, typename TL, typename TR> inline auto //
operator^(Field<Geometry<TG, IL>, TL> const & lhs,
		Field<Geometry<TG, IR>, TR> const & rhs)
		DECL_RET_TYPE(
				(typename std::conditional<(IL + IR >=0 &&
								IL+IR <= TG::NUM_OF_DIMS ),
						Field<Geometry<TG,IL+IR> ,
						OpWedge<Field<Geometry<TG, IL>, TL> ,
						Field<Geometry<TG, IR>, TR> > >,Zero>::type
						(lhs, rhs)))

template<typename TGeo, typename TL, typename TR> inline auto   //
operator+(Field<TGeo, TL> const & lhs, Field<TGeo, TR> const & rhs)
DECL_RET_TYPE(
		( Field<TGeo ,
				OpPlus<Field<TGeo, TL> , Field<TGeo, TR> > > (lhs, rhs)))

template<typename TGeo, typename TL, typename TR> inline auto   //
operator-(Field<TGeo, TL> const & lhs, Field<TGeo, TR> const & rhs)
DECL_RET_TYPE(
		( Field<TGeo ,
				OpMinus<Field<TGeo, TL> , Field<TGeo, TR> > > (lhs, rhs)))

template<typename TG, int IL, typename TL, typename TR> inline auto   //
operator*(Field<Geometry<TG, IL>, TL> const & lhs,
		Field<Geometry<TG, 0>, TR> const & rhs)
		DECL_RET_TYPE(
				(Field<Geometry<TG,IL >,
						OpMultiplies<Field<Geometry<TG, IL>, TL>,
						Field<Geometry<TG,0>, TR> > > (lhs, rhs))
		)

template<typename TG, int IR, typename TL, typename TR> inline auto   //
operator*(Field<Geometry<TG, 0>, TL> const & lhs,
		Field<Geometry<TG, IR>, TR> const & rhs)
		DECL_RET_TYPE(
				(Field<Geometry<TG,IR >,
						OpMultiplies<Field<Geometry<TG, 0>, TL>,
						Field<Geometry<TG,IR>, TR> > > (lhs, rhs))
		)

template<typename TG, typename TL, typename TR> inline auto   //
operator*(Field<TG, TL> const & lhs, TR const & rhs)
DECL_RET_TYPE(
		(Field<TG,OpMultiplies<Field<TG,TL>,TR > > (lhs, rhs)))

template<typename TG, typename TL, typename TR> inline auto   //
operator*(TL const & lhs, Field<TG, TR> const & rhs)
DECL_RET_TYPE(
		(Field<TG,OpMultiplies<TL,Field<TG,TR> > > (lhs, rhs)))

//template<typename TG, int IL, int IR, typename TL, typename TR> inline auto   //
//operator*(Field<Geometry<TG, IL>, TL> const & lhs,
//		Field<Geometry<TG, IR>, TR> const & rhs)
//		ENABLE_IF_DECL_RET_TYPE((IL+IR>0 && IL*IR==0),
//				(Field<Geometry<TG,IL >,
//						OpMultiplies<Field<Geometry<TG, IL>, TL>,
//						Field<Geometry<TG,IR>, TR> > > (lhs, rhs))
//		)
template<typename TG, int IL, typename TL, typename TR> inline auto   //
operator/(Field<Geometry<TG, IL>, TL> const & lhs,
		Field<Geometry<TG, 0>, TR> const & rhs)
		DECL_RET_TYPE(
				(Field<Geometry<TG,IL >,
						OpDivides<Field<Geometry<TG, IL>, TL>,
						Field<Geometry<TG, 0>, TR> > > (lhs, rhs))
		)
template<typename TG, typename TL, typename TR> inline auto   //
operator/(Field<Geometry<TG, 0>, TL> const & lhs,
		Field<Geometry<TG, 0>, TR> const & rhs)
		DECL_RET_TYPE(
				(Field<Geometry<TG,0 >,
						OpDivides<
						Field<Geometry<TG, 0>, TL>,
						Field<Geometry<TG, 0>, TR> > > (lhs, rhs))
		)
template<typename TG, int IL, typename TL, typename TR> inline auto   //
operator/(Field<Geometry<TG, IL>, TL> const & lhs, TR rhs)
DECL_RET_TYPE(
		(Field<Geometry<TG,IL >,
				OpDivides<Field<Geometry<TG, IL>, TL>, TR > > (lhs, rhs))
)

}
// namespace simpla
#endif /* VECTOR_CALCULUS_H_ */
