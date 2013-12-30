/*
 * field.h
 *
 *  Created on: 2013-7-19
 *      Author: salmon
 */

#ifndef FIELD_H_
#define FIELD_H_

#include "primitives.h"
#include "../utilities/log.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include <utility>
#include <mutex>
namespace simpla
{
template<typename TG, typename TValue> struct Field;

/***
 *
 * @brief Field
 *
 * @ingroup Field Expression
 *
 */

template<typename TG, typename TValue>
struct Field
{
	std::mutex write_lock_;
public:

	typedef TG geometry_type;

	typedef typename geometry_type::mesh_type mesh_type;

	enum
	{
		IForm = geometry_type::IForm
	};

	typedef TValue value_type;

	typedef Field<geometry_type, value_type> this_type;

	typedef typename mesh_type::template Container<value_type> base_type;

	static const int NUM_OF_DIMS = mesh_type::NUM_OF_DIMS;

	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef typename mesh_type::index_type index_type;

	typedef typename geometry_type::template field_value_type<value_type> field_value_type;

	typedef typename mesh_type::template Container<value_type> container_type;

private:
	container_type data_;
	index_type num_of_eles_;
public:

	mesh_type const &mesh;

	Field(mesh_type const &pmesh)
			: mesh(pmesh), data_(nullptr), num_of_eles_(0)
	{
	}

	Field(mesh_type const &pmesh, value_type d_value)
			: mesh(pmesh), data_(nullptr), num_of_eles_(0)
	{
		*this = d_value;
	}

	/**
	 *  Copy/clone Construct only copy mesh reference, but do not copy/move data, which is designed to
	 *  initializie stl containers, such as std::vector
	 *    \code
	 *       Field<...> default_value(mesh);
	 *       std::vector<Field<...> > v(4,default_value);
	 *    \endcode
	 *  the element in v have same mesh reference.
	 *
	 * @param rhs
	 */

	Field(this_type const & rhs)
			: mesh(rhs.mesh), data_(nullptr), num_of_eles_(rhs.num_of_eles_)
	{
	}

	/// Move Construct copy mesh, and move data,
	Field(this_type &&rhs)
			: mesh(rhs.mesh), data_(rhs.data_), num_of_eles_(rhs.num_of_eles_)
	{
	}

	virtual ~Field()
	{
	}

	void swap(this_type & rhs)
	{
		base_type::swap(rhs);
		std::swap(rhs.num_of_eles_, num_of_eles_);
	}

	container_type & data()
	{
		return data_;
	}

	const container_type & data() const
	{
		return data_;
	}
	index_type size() const
	{
		return (data_ == nullptr) ? 0 : mesh.GetNumOfElements(IForm);
	}
	bool empty() const
	{
		return size() <= 0;
	}

	typedef value_type* iterator;

	iterator begin()
	{
		return &mesh.get_value(data_, 0);
	}
	iterator end()
	{
		return &mesh.get_value(data_, size());
	}

	const iterator begin() const
	{
		return &mesh.get_value(data_, 0);
	}
	const iterator end() const
	{
		return &mesh.get_value(data_, size());
	}

	inline std::vector<size_t> GetShape() const
	{
		return std::move(mesh.GetShape(IForm));
	}

	inline value_type & get(index_type s)
	{
		ASSERT(s < num_of_eles_)
		return mesh.get_value(data_, s);
	}
	inline value_type const & get(index_type s) const
	{
		return mesh.get_value(data_, s);
	}

	template<typename ... TI>
	inline value_type & get(TI ...s)
	{
		index_type ts = mesh.GetComponentIndex(IForm, s...);
		ASSERT(ts < num_of_eles_);
		ASSERT(ts >= 0);
		return mesh.get_value(data_, ts);
	}
	template<typename ...TI>
	inline value_type const & get(TI ...s) const
	{
		return mesh.get_value(data_, mesh.GetComponentIndex(IForm, s...));
	}

	inline value_type & operator[](index_type s)
	{
		return get(s);
	}
	inline value_type const & operator[](index_type s) const
	{
		return get(s);
	}

	void Init()
	{
		Update();
	}
	void Update()
	{
		if (data_ == nullptr)
		{
			data_ = mesh.template MakeContainer<IForm, value_type>();
			num_of_eles_ = mesh.GetNumOfElements(IForm);
		}

	}

	template<typename TD>
	void Fill(TD default_value)
	{
		Init();
		for (index_type s = 0; s < num_of_eles_; ++s)
		{
			mesh.get_value(data_, s) = default_value;
		}

	}

	inline this_type &
	operator =(this_type const & rhs)
	{
		Init();
		mesh.AssignContainer(IForm, this, rhs);
		return (*this);
	}

	template<typename TR> inline this_type &
	operator =(TR const & rhs)
	{
		Init();
		mesh.AssignContainer(IForm, this, rhs);
		return (*this);
	}

#define DECL_SELF_ASSIGN( _OP_ )                                                                   \
	template<typename TR> inline this_type &                                                       \
	operator _OP_(TR const & rhs)                                                                  \
	{   Init();                                                                                    \
		mesh.ForEach( [](value_type &l,typename FieldTraits<TR>::value_type const & r)             \
	            {	l _OP_ r;},	 this,std::forward<TR const &>(rhs) );     return (*this);}

	DECL_SELF_ASSIGN (+=)

DECL_SELF_ASSIGN	(-=)

	DECL_SELF_ASSIGN (*=)

	DECL_SELF_ASSIGN (/=)
#undef DECL_SELF_ASSIGN

	inline field_value_type mean(coordinates_type const &x) const
	{
		return std::move(Gather(x));
	}

	inline field_value_type operator()(coordinates_type const &x) const
	{
		return std::move(Gather(x));
	}

	inline field_value_type operator()(index_type s,Real const *r) const
	{
		return std::move(Gather(s,r));
	}

	inline field_value_type Gather(coordinates_type const &x) const
	{

		coordinates_type r;

		r=x;

		index_type s = mesh.SearchCell(x, &r[0]);

		return std::move(Gather(s, &r[0]));

	}

	inline field_value_type Gather(index_type const & s, Real const *x) const
	{

		field_value_type res;

		mesh.Gather(Int2Type<IForm>(),s,x,data_.get(),&res);

		return std::move(res);

	}

	template<typename TV>
	inline void Collect(TV const & v, coordinates_type const &x)
	{
		coordinates_type r;

		r=x;

		index_type s = mesh.SearchCell(x, &r[0]);

		Collect(v, s, &r[0]);

	}
	template<typename TV>
	inline void Collect(TV const & v, index_type s, Real * r)
	{

		write_lock_.lock();

		mesh.Scatter(Int2Type<IForm>(),s,r,v,data_.get());

		write_lock_.unlock();

	}

	inline void Collect(index_type num,index_type const * points,value_type const * cache)
	{
		if(num==0)
		WARNING<< "Cache is empty!";

		write_lock_.lock();
		for (int i=0; i<num;++i)
		{
			mesh.get_value(data_, points[i])+=cache[i];
		}
		write_lock_.unlock();
	}
};

}
// namespace simpla

#endif /* FIELD_H_ */
