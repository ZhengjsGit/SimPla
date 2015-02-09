/**
 * @file field_shared_ptr.h
 *
 *  Created on: @date{ 2015-1-30}
 *      @author: salmon
 */

#ifndef CORE_FIELD_FIELD_SHARED_PTR_H_
#define CORE_FIELD_FIELD_SHARED_PTR_H_

#include <cstdbool>
#include <memory>
#include <string>

#include "../application/sp_object.h"
#include "../gtl/expression_template.h"
#include "field_expression.h"

namespace simpla
{
template<typename ...> struct _Field;

/**
 * @ingroup field
 * @{
 */

/**
 *  Simple Field
 */
template<typename TM, typename TV, typename ...Others>
struct _Field<TM, std::shared_ptr<TV>, Others...> : public SpObject
{

	typedef TM mesh_type;

	typedef typename mesh_type::id_type id_type;
	typedef typename mesh_type::coordinates_type coordinates_type;

	typedef std::shared_ptr<TV> container_type;
	typedef TV value_type;

	typedef _Field<mesh_type, container_type, Others...> this_type;

private:

	mesh_type mesh_;

	std::shared_ptr<TV> data_;

public:

	_Field(mesh_type const & d) :
			mesh_(d), data_(nullptr)
	{
	}
	_Field(this_type const & that) :
			mesh_(that.mesh_), data_(that.data_)
	{
	}
	~_Field()
	{
	}

	std::string get_type_as_string() const
	{
		return "Field<" + mesh_.get_type_as_string() + ">";
	}
	mesh_type const & mesh() const
	{
		return mesh_;
	}

	template<typename TU> using clone_field_type=
	_Field<TM,std::shared_ptr<TU>,Others... >;

	template<typename TU>
	clone_field_type<TU> clone() const
	{
		return clone_field_type<TU>(mesh_);
	}

	void clear()
	{
		allocate();
		*this = 0;
	}

	/** @name range concept
	 * @{
	 */

	template<typename ...Args>
	_Field(this_type & that, Args && ...args) :
			mesh_(that.mesh_, std::forward<Args>(args)...), data_(that.data_)
	{
	}
	bool empty() const
	{
		return mesh_.empty();
	}
	bool is_divisible() const
	{
		return mesh_.is_divisible();
	}

	/**@}*/

	/**
	 * @name assignment
	 * @{
	 */

	inline _Field<AssignmentExpression<_impl::_assign, this_type, this_type>> operator =(
			this_type const &that)
	{
		allocate();
		return std::move(
				_Field<
						AssignmentExpression<_impl::_assign, this_type,
								this_type>>(*this, that));
	}

	template<typename TR>
	inline _Field<AssignmentExpression<_impl::_assign, this_type, TR>> operator =(
			TR const &that)
	{
		allocate();
		return std::move(
				_Field<AssignmentExpression<_impl::_assign, this_type, TR>>(
						*this, that));
	}

	template<typename TFun> void pull_back(TFun const &fun)
	{
		allocate();
		mesh_.pull_back(*this, fun);
	}

	/** @} */

	/** @name access
	 *  @{*/

	typedef typename mesh_type::template field_value_type<value_type> field_value_type;

	field_value_type gather(coordinates_type const& x) const
	{
		return std::move(mesh_.gather(*this, x));
	}

	template<typename ...Args>
	void scatter(Args && ... args)
	{
		mesh_.scatter(*this, std::forward<Args>(args)...);
	}

	/**@}*/

//	DataSet dump_data() const
//	{
//		return DataSet();
//	}
private:
	void allocate()
	{
		if (data_ == nullptr)
		{
			data_ = sp_make_shared_array<value_type>(mesh_.max_hash());
		}
	}

public:
	value_type & operator[](id_type const & s)
	{
		return data_.get()[mesh_.hash(s)];
	}
	value_type const & operator[](id_type const & s) const
	{
		return data_.get()[mesh_.hash(s)];
	}

}
;
namespace _impl
{
class is_sequence_container;
template<typename TContainer> struct field_selector;
template<typename TV>
struct field_selector<std::shared_ptr<TV>>
{
	typedef is_sequence_container type;
};

}  // namespace _impl
/**@} */
}
// namespace simpla

#endif /* CORE_FIELD_FIELD_SHARED_PTR_H_ */