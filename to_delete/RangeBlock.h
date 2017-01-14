/**
 * @file block_range.h
 * @author salmon
 * @date 2015-10-26.
 */

#ifndef SIMPLA_BLOCK_RANGE_H
#define SIMPLA_BLOCK_RANGE_H

#include <stddef.h>
#include <algorithm>
#include <cstdbool>
#include <functional>
#include <set>
#include <tuple>
#include <type_traits>

#include "simpla/mpl/type_traits.h"

namespace simpla { namespace toolbox
{

template<typename ...> struct Range;

template<typename T, int NDIMS> class IteratorBlock;

// PlaceHolder on Holder concept in TBB

template<typename T, int NDIMS>
class Range<IteratorBlock<T, NDIMS>>
{
public:

// types

    typedef size_t size_type;

    typedef IteratorBlock<T, NDIMS> iterator;

// constructors
    Range(const_iterator b, const_iterator e, size_type grain_size = 1) :
            m_begin_(b), m_end_(e), m_grain_size_(grain_size) {}

    Range(Range &r, tags::split) : m_begin_(r.m_begin_ + r.size() / 2), m_end_(r.m_end_),
                                   m_grain_size_(r.grainsize()) { r.m_end_ = m_begin_; };

    Range(Range &r, tags::proportional_split &proportion) :
            m_begin_(r.m_begin_ + r.size() * proportion.left() / (proportion.left() + proportion.right())),
            m_end_(r.m_end_), m_grain_size_(r.grainsize()) { r.m_end_ = m_begin_; };

// Proportional split is enabled
    static const bool is_splittable_in_proportion = true;

// capacity
    size_type size() const { return traits::distance(m_begin_, m_end_); };

    bool empty() const { return m_begin_ == m_end_; };

// access
    size_type grainsize() const { return m_grain_size_; }

    bool is_divisible() const { return size() > grainsize(); }

// iterators
    iterator begin() const { return m_begin_; }

    iterator end() const { return m_end_; }

private:

    nTuple <T, NDIMS> m_begin_, m_end_, m_start_, m_shape_;

    size_type m_grain_size_;




//        struct iterator : public std::iterator<
//                typename std::bidirectional_iterator_tag, mesh_id_type,
//                difference_type>
//        {
//        private:
//            mesh_id_type m_idx_min_, m_idx_max_, m_self_;
//        public:
//            iterator(mesh_id_type const &min, mesh_id_type const &max,
//                     mesh_id_type const &self) :
//                    m_idx_min_(min), m_idx_max_(max), m_self_(self)
//            {
//            }
//
//            iterator(mesh_id_type const &min, mesh_id_type const &max) :
//                    m_idx_min_(min), m_idx_max_(max), m_self_(min)
//            {
//            }
//
//            iterator(iterator const &other) :
//                    m_idx_min_(other.m_idx_min_), m_idx_max_(other.m_idx_max_), m_self_(
//                    other.m_self_)
//            {
//            }
//
//            ~iterator()
//            {
//
//            }
//
//            typedef iterator this_type;
//
//            bool operator==(this_type const &other) const
//            {
//                return m_self_ == other.m_self_;
//            }
//
//            bool operator!=(this_type const &other) const
//            {
//                return m_self_ != other.m_self_;
//            }
//
//            value_type const &operator*() const
//            {
//                return m_self_;
//            }
//
//        private:
//
//            index_type carray_(index_type *self, index_type min, index_type max,
//                               index_type id = 0)
//            {
//
//                auto div = std::div(
//                        static_cast<long>(*self + id * (_D << 1) + max
//                                          - min * 2), static_cast<long>(max - min));
//
//                *self = static_cast<mesh_id_type>(div.rem + min);
//
//                return div.quot - 1L;
//            }
//
//            index_type carray(mesh_id_type *self, mesh_id_type xmin, mesh_id_type xmax,
//                              index_type id = 0)
//            {
//                index_tuple idx, min, max;
//
//                idx = unpack(*self);
//                min = unpack(xmin);
//                max = unpack(xmax);
//
//                id = carray_(&idx[0], min[0], max[0], id);
//                id = carray_(&idx[1], min[1], max[1], id);
//                id = carray_(&idx[2], min[2], max[2], id);
//
//                *self = pack(idx) | (std::abs(id) << (FULL_DIGITS - 1));
//                return id;
//            }
//
//        public:
//            void next()
//            {
//                m_self_ = rotate(m_self_);
//                if (sub_index(m_self_) == 0)
//                {
//                    carray(&m_self_, m_idx_min_, m_idx_max_, 1);
//                }
//
//            }
//
//            void prev()
//            {
//                m_self_ = inverse_rotate(m_self_);
//                if (sub_index(m_self_) == 0)
//                {
//                    carray(&m_self_, m_idx_min_, m_idx_max_, -1);
//                }
//            }
//
//            this_type &operator++()
//            {
//                next();
//                return *this;
//            }
//
//            this_type &operator--()
//            {
//                prev();
//
//                return *this;
//            }
//
//            this_type operator++(int)
//            {
//                this_type res(*this);
//                ++(*this);
//                return std::move(res);
//            }
//
//            this_type operator--(int)
//            {
//                this_type res(*this);
//                --(*this);
//                return std::move(res);
//            }
//
//        };
//
//    };

};//Holder<IteratorBlock>
}}//namespace simpla{namespace toolbox{

#endif //SIMPLA_BLOCK_RANGE_H