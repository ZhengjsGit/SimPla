//
// Created by salmon on 17-3-21.
//

#ifndef SIMPLA_CONFIGURABLE_H
#define SIMPLA_CONFIGURABLE_H

#include <memory>
#include <string>
#include "DataTable.h"
namespace simpla {
namespace data {

/**
 * Properties
 */
class Properties {
   public:
    Properties() = default;
    virtual ~Properties() = default;

    Properties(Properties const& other) : m_db_(data::DataTable::New()){};
    Properties(Properties&& other) noexcept : m_db_(other.m_db_){};
    Properties& operator=(Properties const& other) {
        Properties(other).swap(*this);
        return *this;
    };
    Properties& operator=(Properties&& other) noexcept {
        Properties(other).swap(*this);
        return *this;
    }

    virtual void swap(Properties& other) { std::swap(m_db_, other.m_db_); }

    const data::DataTable& db() const { return *m_db_; }
    data::DataTable& db() { return *m_db_; }
    template <typename U>
    U GetProperty(std::string const& uri) const {
        return m_db_->template GetValue<U>(uri);
    }
    template <typename U>
    U GetProperty(std::string const& uri, U const& default_value) const {
        return m_db_->template GetValue<U>(uri, default_value);
    }
    template <typename U>
    void SetProperty(std::string const& uri, U const& value) {
        m_db_->SetValue(uri, value);
    }
    bool CheckProperty(std::string const& uri) const { return m_db_->Check(uri, true); }

   private:
    std::shared_ptr<data::DataTable> m_db_ = nullptr;
};
}
}
#endif  // SIMPLA_CONFIGURABLE_H
