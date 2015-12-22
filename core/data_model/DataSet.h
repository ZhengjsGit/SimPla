/**
 * @file data_set.h
 *
 *  Created on: 2014-11-10
 *      Author: salmon
 */

#ifndef CORE_DATASET_DATASET_H_
#define CORE_DATASET_DATASET_H_

#include <list>
#include <stddef.h>
#include <algorithm>
#include <memory>
#include <type_traits>
#include <tbb/concurrent_vector.h>

#include "../gtl/check_concept.h"
#include "../gtl/type_traits.h"
#include "../base/Object.h"
#include "DataSpace.h"
#include "DataType.h"

namespace simpla { namespace data_model
{
/**
 * @addtogroup data_model Dataset
 * @brief This section describes the interface of data set.
 *
 * @ref data_model  is a group of classes used to exchange data between different libraries
 * and program languages in the memory. For example, we can transfer an array
 * of particle structure in memory to hdf5 library, and write it to disk.
 */

/**
 * @ingroup data_model
 *
 * @brief Describe structure of data in the memory.
 *
 * A data_set is composed of a pointer to raw data , a description
 * of element data type (DataType), a description of memory layout of
 * data set (data_space),and a container of meta data (Properties).
 */

struct DataSet : public base::Object
{
    SP_OBJECT_HEAD(DataSet, base::Object);

    std::shared_ptr<void> data;

    DataType data_type;

    DataSpace data_space;

    DataSpace memory_space;


    DataSet() : data(nullptr) { }

    DataSet(DataSet const &other) :
            data(other.data),
            data_type(other.data_type),
            data_space(other.data_space),
            memory_space(other.memory_space)
    {
    }

    DataSet(DataSet &&other) :
            data(other.data),
            data_type(other.data_type),
            data_space(other.data_space),
            memory_space(other.memory_space)
    {
    }

    virtual ~DataSet() { }

    void swap(DataSet &other)
    {
        std::swap(data, other.data);
        std::swap(data_type, other.data_type);
        std::swap(data_space, other.data_space);
        std::swap(memory_space, other.memory_space);
    }

    bool operator==(DataSet const &other) const { return is_equal(other.data.get()); }

    bool is_valid() const
    {
        return (data != nullptr)
               && data_type.is_valid()
               && data_space.is_valid()
               && memory_space.is_valid()
               && (data_space.num_of_elements() == memory_space.num_of_elements());
    }

    virtual bool empty() const { return data == nullptr; }

    virtual std::ostream &print(std::ostream &os) const;

    bool is_same(void const *other) const;

    bool is_equal(void const *other) const;

    template<typename T> T &get_value(size_t s) { return reinterpret_cast<T *>( data.get())[s]; }

    template<typename T> T const &get_value(size_t s) const { return reinterpret_cast<T *>( data.get())[s]; }


    template<typename TV> TV *pointer() { return reinterpret_cast<TV *>(data.get()); }

    template<typename TV> TV const *pointer() const { return reinterpret_cast<TV *>(data.get()); }


    template<typename ...Args>
    static DataSet create(Args &&...args);
}; //class data_set




//
namespace _impl
{
HAS_MEMBER_FUNCTION(data_set)

template<typename T>
auto create_data_set(T const &f)
-> typename std::enable_if<has_member_function_data_set<T>::value, DataSet>::type { return f.data_set(); }

DataSet create_data_set(DataType const &dtype, std::shared_ptr<void> const &p, size_t rank,
                        size_t const *dims = nullptr);

DataSet create_data_set(DataType const &dtype);

template<typename T>
DataSet create_data_set()
{
    return create_data_set(DataType::create<T>());
}

template<typename T>
DataSet create_data_set(T const *p, size_t rank, size_t const *dims)
{
    return create_data_set(DataType::create<T>(),
                           std::shared_ptr<void>(reinterpret_cast<void *>(const_cast<T *>(p)), tags::do_nothing()),
                           rank, dims);
}

template<typename T>
DataSet create_data_set(std::shared_ptr<T> &p, size_t rank, size_t const *dims)
{
    return create_data_set(DataType::create<T>(),
                           std::shared_ptr<void>(reinterpret_cast<void *>(p.get()), tags::do_nothing()),
                           rank, dims);
}


HAS_MEMBER(_tag)

template<typename T>
struct point_type_check : public std::integral_constant<int,
        (std::is_integral<T>::value) ? 0 :
        ((std::is_array<T>::value || traits::is_ntuple<T>::value) ? 1 :
         ((std::is_class<T>::value && has_member__tag<T>::value) ? 2 : -1)
        )>
{
};

template<typename TV>
size_t get_tag(std::integral_constant<int, -1>, const TV &v)
{
    UNIMPLEMENTED;
    return 0;
}

template<typename TV>
size_t get_tag(std::integral_constant<int, 0>, const TV &v) { return v; }


template<typename TV>
auto get_tag(std::integral_constant<int, 2>, const TV &v) -> decltype(v.tag) { return v.tag; }

template<typename T>
auto select_tag(DataSpace &sp, std::shared_ptr<T> const &p,
                size_t num) -> typename std::enable_if<has_member__tag<T>::value, void>::type
{

    size_t ndims = std::get<0>(sp.shape());

    std::vector<size_t> tags(num * ndims);
//
//    parallel::parallel_for(
//            parallel::blocked_range<size_t>(0, num),
//            [&](parallel::blocked_range <size_t> &r)
//            {
//                for (auto i = r.begin(), ie = r.end(); i != ie; ++i)
    for (size_t i = 0; i < num; ++i)
    {
        for (size_t j = 0; j < ndims; ++j)
        {
            tags[i * ndims + j] = reinterpret_cast<size_t *>(&p.get()[i]._tag)[j];
        }
    }
//            }
//
//    );

    sp.select_points(num, &tags[0]);

}

template<typename T>
auto select_tag(DataSpace &sp, std::shared_ptr<T> const &p,
                size_t) -> typename std::enable_if<!has_member__tag<T>::value, void>::type
{
}

template<typename T> DataSet
create_data_set(std::shared_ptr<T> const &p, size_t num)
{
    auto ds = create_data_set(DataType::create<T>(),
                              std::shared_ptr<void>(reinterpret_cast<void *>(p.get()), tags::do_nothing()),
                              num);
    select_tag(ds.data_space, p, num);

    return std::move(ds);
}

template<typename T>
DataSet create_data_set(std::vector<T> const &p)
{
    return create_data_set(&p[0], p.size());
}
/**@}*/
} // namespace traits

template<typename ...Args>
DataSet DataSet::create(Args &&...args)
{
    return _impl::create_data_set(std::forward<Args>(args)...);
}


}} //namespace simpla { namespace data_model
#endif /* CORE_DATASET_DATASET_H_ */
