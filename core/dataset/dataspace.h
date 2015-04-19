/**
 * @file dataspace.h
 *
 *  Created on: 2014年11月10日
 *  @author: salmon
 */

#ifndef CORE_DATASET_DATASPACE_H_
#define CORE_DATASET_DATASPACE_H_

#include <stddef.h>
#include <memory>
#include <tuple>
#include <vector>

#include "../gtl/ntuple.h"
#include "../gtl/primitives.h"
#include "../gtl/properties.h"

namespace simpla
{

struct DataSet;
/**
 * @ingroup data_interface
 * @brief  Define the size and  shape of data set in memory/file
 *  Ref. http://www.hdfgroup.org/HDF5/doc/UG/UG_frame12Dataspaces.html
 */
class DataSpace
{
public:
	Properties properties;

	// Creates a null dataspace
	DataSpace();

	DataSpace(int rank, long const * dims);

	// Copy constructor: makes a copy of the original DataSpace object.
	DataSpace(const DataSpace& other);
//	DataSpace(DataSpace&& other);
	// Destructor: properly terminates access to this dataspace.
	~DataSpace();

	void swap(DataSpace &);

	// Assignment operator
	DataSpace& operator=(const DataSpace& rhs)
	{
		DataSpace(rhs).swap(*this);
		return *this;
	}

	static DataSpace create_simple(int rank, const long * dims);

	DataSpace & convert_to_local(long const * gw = nullptr);

	DataSpace & select_hyperslab(long const *offset, long const * stride,
			long const * count, long const * block = nullptr);

	bool is_valid() const;

	bool is_distributed() const
	{
		OBSOLETE;
		return true;
	}
	;

	bool is_simple() const
	{
		/// TODO support  complex selection of data space
		/// @ref http://www.hdfgroup.org/HDF5/doc/UG/UG_frame12Dataspaces.html
		return is_valid() && (!is_distributed());
	}

	/**
	 * @return <ndims,dimensions,start,count,stride,block>
	 */
	std::tuple<int, long const *, long const *, long const *, long const *,
			long const *> shape() const;

	std::tuple<int, long const *, long const *, long const *, long const *,
			long const *> global_shape() const;

private:
	struct pimpl_s;
	std::unique_ptr<pimpl_s> pimpl_;

};
/**
 * @ingroup data_interface
 * create dataspace
 * @param args
 * @return
 */
template<typename ... Args>
DataSpace make_dataspace(Args && ... args)
{
	return DataSpace(std::forward<Args>(args)...);
}

/**@}  */

}  // namespace simpla

#endif /* CORE_DATASET_DATASPACE_H_ */
