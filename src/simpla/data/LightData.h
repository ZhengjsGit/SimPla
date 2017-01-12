/***
 *
 * @file  LightData.h
 *    base on LightData.h
 *
 * Created by salmon on 17-1-6.
 */
#ifndef SIMPLA_LIGHTDATA_H
#define SIMPLA_LIGHTDATA_H

#include <simpla/algebra/nTuple.h>
#include <simpla/toolbox/FancyStream.h>
#include <stddef.h>
#include <algorithm>
#include <cstdbool>
#include <iostream>
#include <memory>
#include <string>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include "DataEntity.h"

namespace simpla {
namespace data {

namespace _impl {

template <typename T>
auto _to_bool(T const& v) ->
    typename std::enable_if<std::is_convertible<T, bool>::value, bool>::type {
    return static_cast<bool>(v);
};

template <typename T>
auto _to_bool(T const& v) ->
    typename std::enable_if<!std::is_convertible<T, bool>::value, bool>::type {
    return false;
};

template <typename T>
auto _to_integer(T const& v) ->
    typename std::enable_if<std::is_convertible<T, int>::value, int>::type {
    return static_cast<int>(v);
};

template <typename T>
auto _to_integer(T const& v) ->
    typename std::enable_if<!std::is_convertible<T, int>::value, int>::type {
    return 0;
};

template <typename T>
auto _to_floating_point(T const& v) ->
    typename std::enable_if<std::is_convertible<T, double>::value, double>::type {
    return static_cast<double>(v);
};

template <typename T>
auto _to_floating_point(T const& v) ->
    typename std::enable_if<!std::is_convertible<T, double>::value, double>::type {
    return 0;
};

inline std::string _to_string(std::string const& v) { return (v); };

template <typename T>
std::string _to_string(T const& v) {
    return "";
};

template <typename T>
auto get_bool(T* v, bool other) -> typename std::enable_if<std::is_integral<T>::value, bool>::type {
    *v = other;
    return true;
}

template <typename T>
auto get_bool(T* v, bool) -> typename std::enable_if<!std::is_integral<T>::value, bool>::type {
    return false;
}

template <typename T>
auto get_integer(T* v, int other) ->
    typename std::enable_if<std::is_integral<T>::value, bool>::type {
    *v = other;
    return true;
}

template <typename T>
auto get_integer(T* v, int) -> typename std::enable_if<!std::is_integral<T>::value, bool>::type {
    return false;
}

template <typename T>
auto get_floating_point(T* v, double other) ->
    typename std::enable_if<std::is_floating_point<T>::value, bool>::type {
    *v = other;
    return true;
}

template <typename T>
auto get_floating_point(T* v, double) ->
    typename std::enable_if<!std::is_floating_point<T>::value, bool>::type {
    return false;
}

template <typename T>
bool get_string(T* v, std::string const&) {
    return false;
}

inline bool get_string(std::string* v, std::string const& other) {
    *v = other;
    return true;
}

}  // namespace _impl

/**
 * @brief small data, which can be passed  between modules easily, such as short string, sigle
 * double or n-tuple
 *  @ingroup  Data
 *   PlaceHolder on http://www.cnblogs.com/qicosmos/p/3420095.html
 *   alt. <boost/any.hpp>
 *
 *   This an implement of 'any' with m_data type description/serialization information
 */
struct LightData : public DataEntity {
   private:
    struct PlaceHolder;

    template <typename>
    struct Holder;

    SP_OBJECT_HEAD(LightData, DataEntity);

   public:
    LightData() : m_data_(nullptr) {}

    template <typename ValueType>
    explicit LightData(const ValueType& value)
        : m_data_(new Holder<typename std::remove_cv<typename std::decay<ValueType>::type>::type>(
              value)) {}

    LightData(const LightData& other)
        : m_data_(other.m_data_ == nullptr ? other.m_data_->clone() : nullptr) {}

    // Move constructor
    LightData(LightData&& other) : m_data_(nullptr) { other.m_data_.swap(m_data_); }

    LightData(PlaceHolder* p) : m_data_(p) {}

    // Perfect forwarding of ValueType
    template <typename ValueType>
    explicit LightData(ValueType&& value,
                       typename std::enable_if<!(std::is_same<LightData&, ValueType>::value ||
                                                 std::is_const<ValueType>::value)>::type* = 0
                       // disable if entity find type `LightData&`
                       // disable if entity find type `const ValueType&&`
                       )
        : m_data_(
              new Holder<typename std::decay<ValueType>::type>(static_cast<ValueType&&>(value))) {}

    virtual ~LightData() {}

    LightData& swap(LightData& other) {
        std::swap(m_data_, other.m_data_);
        return *this;
    }

    LightData& operator=(const LightData& rhs) {
        LightData(rhs).swap(*this);
        return *this;
    }

    // move assignement
    LightData& operator=(LightData&& rhs) {
        rhs.swap(*this);
        LightData().swap(rhs);
        return *this;
    }

    // Perfect forwarding of ValueType
    template <class ValueType>
    LightData& operator=(ValueType&& rhs) {
        LightData(static_cast<ValueType&&>(rhs)).swap(*this);
        return *this;
    }

    virtual bool empty() const { return m_data_ == nullptr; }

    virtual bool is_null() const { return m_data_ == nullptr; }

    virtual void clear() { LightData().swap(*this); }

    //    virtual void* data() { return m_data_->data(); }
    //
    //    virtual void const* data() const { return m_data_->data(); }

    const std::type_info& type() const { return m_data_ ? m_data_->type() : typeid(void); }

    template <typename U, typename... Args>
    static LightData create(Args&&... args) {
        return LightData(static_cast<PlaceHolder*>(new Holder<U>(std::forward<Args>(args)...)));
    };

    //----------------------------------------------------------------------------------------------
    // SimPla extent

    operator bool() const { return m_data_ != nullptr; }
    //
    //    void const *m_data() const { return m_attr_data_ != nullptr ? m_attr_data_->m_data() :
    //    nullptr; }
    //
    //    void *m_data() { return m_attr_data_ != nullptr ? m_attr_data_->m_data() : nullptr; }

    std::string string() const {
        std::ostringstream os;
        this->print(os, 0);
        return os.str();
    }

    template <class U>
    bool is_same() const {
        return m_data_ != nullptr && m_data_->is_same<U>();
    }

    template <typename U>
    bool equal(U const& u) const {
        UNIMPLEMENTED;
        return false;
        // return m_data_->equal(u);
    }

    virtual bool is_boolean() const { return m_data_ != nullptr && m_data_->is_bool(); }

    virtual bool is_integral() const { return m_data_ != nullptr && m_data_->is_integral(); }

    virtual bool is_floating_point() const {
        return m_data_ != nullptr && m_data_->is_floating_point();
    }

    virtual bool is_string() const { return m_data_ != nullptr && m_data_->is_string(); }

    template <class U>
    bool as(U* v) const {
        return m_data_ != nullptr && m_data_->as(v);
    }

    template <class U>
    operator U() const {
        return as<U>();
    }

    template <class U>
    U const& as() const {
        return LightData::get<U>();
    }

    template <class U>
    U as(U const& def_v) const {
        if (!empty() && this->template is_same<U>()) {
            return dynamic_cast<Holder<U>*>(m_data_)->m_value_;
        } else {
            U res;
            if (as(&res)) {
                return std::move(res);
            } else {
                return def_v;
            }
        }
    }

    template <class U>
    U as(U const& def_v) {
        if (!empty() && this->template is_same<U>()) {
            return dynamic_cast<Holder<U>*>(m_data_)->m_value_;
        } else {
            LightData(def_v).swap(*this);
            return def_v;
        }
    }

    template <class U>
    U const& get() const {
        if (!is_same<U>()) { THROW_EXCEPTION_BAD_CAST(typeid(U).name(), m_data_->type().name()); }

        return dynamic_cast<Holder<U>*>(m_data_.get())->m_value_;
    }

    template <class U>
    U& get() {
        if (!is_same<U>()) { THROW_EXCEPTION_BAD_CAST(typeid(U).name(), m_data_->type().name()); }
        return dynamic_cast<Holder<U>*>(m_data_)->m_value_;
    }

    virtual std::ostream& print(std::ostream& os, int indent = 1) const {
        if (m_data_ != nullptr) {
            m_data_->print(os, indent);
        } else {
            CHECK(m_data_ == nullptr);
        }
        return os;
    }

   private:
    std::unique_ptr<PlaceHolder> m_data_ = nullptr;

    PlaceHolder* clone() const { return (m_data_ != nullptr) ? m_data_->clone() : nullptr; }

    struct PlaceHolder {
        PlaceHolder() {}

        virtual ~PlaceHolder() {}

        virtual PlaceHolder* clone() const = 0;

        virtual const std::type_info& type() const = 0;
        //----------------------------------------------------------------------------------------------
        // SimPla extent

        virtual std::ostream& print(std::ostream& os, int indent = 1) const = 0;

        template <typename U>
        bool is_same() const {
            return typeid(U) == type();
        }

        //    virtual data_model::DataType data_type() const = 0;

        virtual bool is_bool() const = 0;

        virtual bool is_integral() const = 0;

        virtual bool is_floating_point() const = 0;

        virtual bool is_string() const = 0;

        virtual bool is_array() const = 0;

        virtual bool to_bool() const = 0;

        virtual int to_integer() const = 0;

        virtual double to_floating_point() const = 0;

        virtual std::string to_string() const = 0;

        virtual int size() const = 0;

        virtual std::shared_ptr<PlaceHolder> get(int) const = 0;

        //        virtual void* data() = 0;
        //
        //        virtual void const* data() const = 0;

        //        template<typename U, size_type N>
        //        bool as(nTuple<U, N> *v) const
        //        {
        //            if (is_same<nTuple<U, N>>())
        //            {
        //                *v = dynamic_cast<Holder<nTuple<U, N>> const * > (this)->m_value_;
        //                return true;
        //            } else if (this->size() < N) { return false; }
        //
        //            for (int i = 0; i < N; ++i) { this->get(i)->as(&((*v)[i])); }
        //
        //            return true;
        //        }

        template <class U>
        bool as(U* v) const {
            bool success = true;
            if (is_same<U>()) {
                *v = dynamic_cast<Holder<U> const*>(this)->m_value_;
            } else if (_impl::get_integer(v, this->to_integer())) {
            } else if (_impl::get_floating_point(v, this->to_floating_point())) {
            } else if (_impl::get_string(v, this->to_string())) {
            } else {
                success = type_cast(v, *this);
            }

            return success;
        }
    };

    template <typename ValueType>
    struct Holder : PlaceHolder {
        ValueType m_value_;

       public:
        Holder(ValueType const& v) : m_value_(v) {}

        Holder(ValueType&& v) : m_value_(std::forward<ValueType>(v)) {}

        virtual ~Holder() {}

        Holder& operator=(const Holder&) = delete;

        virtual PlaceHolder* clone() const { return new Holder(m_value_); }

        virtual const std::type_info& type() const { return typeid(ValueType); }

        //----------------------------------------------------------------------------------------------
        // SimPla extent

        std::ostream& print(std::ostream& os, int indent = 1) const {
            if (std::is_same<ValueType, std::string>::value) {
                os << "\"" << m_value_ << "\"";
            } else {
                os << m_value_;
            }
            return os;
        }

        //        virtual void* data() { return &m_value_; };
        //
        //        virtual void const* data() const { return &m_value_; };

        //    data_model::DataType data_type() const { return data_model::DataType::template
        //    clone<T>(); }

        virtual bool is_bool() const { return std::is_same<ValueType, bool>::value; }

        virtual bool is_integral() const { return std::is_integral<ValueType>::value; }

        virtual bool is_floating_point() const { return std::is_floating_point<ValueType>::value; }

        virtual bool is_string() const { return std::is_same<ValueType, std::string>::value; }

        virtual bool is_array() const { return std::is_array<ValueType>::value; }

        virtual int to_integer() const { return _impl::_to_integer(m_value_); };

        virtual double to_floating_point() const { return _impl::_to_floating_point(m_value_); };

        virtual bool to_bool() const { return _impl::_to_bool(m_value_); };

        virtual std::string to_string() const { return _impl::_to_string(m_value_); };

       private:
        template <typename V>
        int _size_of(V const&) const {
            return 1;
        }

        //        template<typename V, int N, int ...M> int _size_of(nTuple<V, N, M...> const &)
        //        const { return N; }

        template <typename... V>
        int _size_of(std::tuple<V...> const&) const {
            return sizeof...(V);
        }

        template <typename V>
        std::shared_ptr<PlaceHolder> _index_of(V const& v, int n) const {
            return std::shared_ptr<PlaceHolder>(new Holder<V>(v));
        }

        //        template<typename V, int N>
        //        std::shared_ptr<PlaceHolder> _index_of(nTuple<V, N> const &v, int n) const
        //        {
        //            return std::shared_ptr<PlaceHolder>(new Holder<V>(v[n]));
        //        }

        template <typename T0, typename T1>
        std::shared_ptr<PlaceHolder> _index_of(std::tuple<T0, T1> const& v, int n) const {
            std::shared_ptr<PlaceHolder> res;
            switch (n) {
                case 0:
                    res = std::shared_ptr<PlaceHolder>(new Holder<T0>(std::get<0>(v)));
                case 1:
                    res = std::shared_ptr<PlaceHolder>(new Holder<T1>(std::get<1>(v)));
                default:
                    OUT_OF_RANGE << n << " >  2 " << std::endl;
            }

            return res;
        }

       public:
        virtual int size() const { return _size_of(m_value_); };

        virtual std::shared_ptr<PlaceHolder> get(int n) const { return _index_of(m_value_, n); }
    };

};  // class LightData

namespace traits {
template <typename U>
struct create_entity<U, std::enable_if_t<is_light<std::remove_cv_t<U>>::value>> {
    template <typename... Args>
    static std::shared_ptr<DataEntity> eval(Args&&... args) {
        return std::dynamic_pointer_cast<DataEntity>(std::make_shared<LightData>(
            LightData::template create<U>(std::forward<Args>(args)...)));
    }
};

}  // namespace traits;

std::shared_ptr<DataEntity> inline create_data_entity(char const* c) {
    return traits::create_entity<std::string>::eval(std::string(c));
};

}  // namespace data {
}  // namespace simpla{
#endif  // SIMPLA_LIGHTDATA_H