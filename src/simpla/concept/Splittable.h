//
// Created by salmon on 16-12-21.
//

#ifndef SIMPLA_SPLITTABLE_H
#define SIMPLA_SPLITTABLE_H

namespace simpla { namespace concept
{
/** @ingroup concept */
namespace tags
{
struct split {};


//! Type enables transmission of splitting proportion from partitioners to range objects
/**
 * In order to make use of such facility Range objects must implement
 * splitting constructor with this type passed and initialize static
 * constant boolean field 'is_splittable_in_proportion' with the value
 * of 'true'
 */
class proportional_split
{
public:
    proportional_split(size_t _left = 1, size_t _right = 1) : my_left(_left), my_right(_right) {}

    proportional_split(proportional_split const &) = delete;

    ~proportional_split() {}

    size_t left() const { return my_left; }

    size_t right() const { return my_right; }

    // used when range does not support proportional split
    operator split() const { return split(); }

private:
    size_t my_left, my_right;
};
}//namespace tags

/** @ingroup concept */
/**
 * @brief a type whose instances can be split into two pieces;     *
 * @details
 * ## Summary
 *
 *  @ref tbb::splittable
 *
 *  @ref [ https://software.intel.com/zh-cn/node/506141 ]
 *
 * ## Requirements
 *  Class \c R implementing the concept of Splittable must define:
 *	Pseudo-Signature                              | Semantics
 * ---------------------------------------------- |----------
 *  \code R::R( const R& );              \endcode |Copy constructor
 *  \code R::~R();                       \endcode |Destructor
 *  \code bool R::is_divisible() const;  \endcode |True if range can be partitioned into two subranges
 *  \code bool R::empty() const;         \endcode |True if range is empty
 *  \code R::R( R& r, split );           \endcode |Split range \c r into two subranges.
 *
 * ## Description
 * > _from TBB_
 * >
 * >   A type is splittable if it has a splitting constructor that allows
 * >  an instance to be split into two pieces. The splitting constructor
 * >  takes as arguments a reference to the original object, and a dummy
 * >   argument of type split, which is defined by the library. The dummy
 * >   argument distinguishes the splitting constructor from a copy constructor.
 * >   After the constructor runs, x and the newly constructed object should
 * >   represent the two pieces of the original x. The library uses splitting
 * >    constructors in two contexts:
 * >    - Partitioning a entity_id_range into two subranges that can be processed concurrently.
 * >    - Forking a body (function object) into two bodies that can run concurrently.
 *
 * - Split  @ref Container  into two part, that can be accessed concurrently
 * - if X::left() and X::right() exists, construct proportion with the
 * ratio specified by left() and right(). (alter from tbb::proportional_split)
 *
 */
struct Splittable
{
    Splittable(Splittable &, tags::split) {}

    ~Splittable() {}

    virtual bool is_divisible()=0;

    virtual bool empty()=0;

};
}}//namespace simpla { namespace concept

#endif //SIMPLA_SPLITTABLE_H