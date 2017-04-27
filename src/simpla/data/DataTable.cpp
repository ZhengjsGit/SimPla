//
// Created by salmon on 16-11-9.
//
#include "DataTable.h"
#include <simpla/SIMPLA_config.h>
#include <simpla/utilities/Log.h>
#include <simpla/utilities/SingletonHolder.h>
#include <iomanip>
#include <regex>
#include <string>
#include "DataBackend.h"
#include "DataBackendMemory.h"
#include "DataEntity.h"
#include "KeyValue.h"
namespace simpla {
namespace data {
DataTable::DataTable() : m_backend_(DataBackend::Create("mem://", "")) { ASSERT(m_backend_ != nullptr); };
DataTable::DataTable(std::string const& uri, std::string const& param) : m_backend_(DataBackend::Create(uri, param)) {
    ASSERT(m_backend_ != nullptr);
};
DataTable::DataTable(std::shared_ptr<DataBackend> const& p) : m_backend_(p) { ASSERT(m_backend_ != nullptr); };
DataTable::DataTable(const DataTable& other) : m_backend_(other.m_backend_->Duplicate()) {
    ASSERT(m_backend_ != nullptr);
}
DataTable::DataTable(std::initializer_list<KeyValue> const& l) : DataTable() {
    for (auto const& item : l) { Set(item.first, item.second); }
}

DataTable::DataTable(DataTable&& other) noexcept : m_backend_(other.m_backend_) { ASSERT(m_backend_ != nullptr); }

DataTable::~DataTable(){};
void DataTable::swap(DataTable& other) {
    std::swap(m_backend_, other.m_backend_);
    ASSERT(m_backend_ != nullptr);
};

//******************************************************************************************************************

void DataTable::Flush() { m_backend_->Flush(); }

std::shared_ptr<DataEntity> DataTable::Duplicate() const {
    return std::dynamic_pointer_cast<DataEntity>(std::make_shared<DataTable>(m_backend_->Duplicate()));
}

bool DataTable::isNull() const { return m_backend_ == nullptr; }
size_type DataTable::size() const { return m_backend_->size(); }
std::shared_ptr<DataEntity> DataTable::Get(std::string const& path) const { return m_backend_->Get(path); };
void DataTable::Set(std::string const& uri, std::shared_ptr<DataEntity> const& v, bool overwrite) {
    //    return m_backend_->Push(uri, v, overwrite);
    //    auto tab_res = pimpl_s::get_table((this), uri, overwrite);
    //    if (tab_res.second == "") { return 0; }
    //    auto res = tab_res.first->m_pimpl_->m_table_.emplace(tab_res.second, nullptr);
    bool success = false;
    auto res = Get(uri);
    if (res != nullptr && !overwrite) { return; }
    if (v == nullptr) {
    } else if (v->isTable()) {
        if (!overwrite && res != nullptr && !res->isTable()) {
            return;
        } else if (res == nullptr || !res->isTable()) {
            res = std::make_shared<DataTable>();
        }
        auto& dest_table = res->cast_as<DataTable>();
        auto const& src_table = v->cast_as<DataTable>();
        src_table.Foreach(
            [&](std::string const& k, std::shared_ptr<DataEntity> const& v) { dest_table.Set(k, v, overwrite); });
        success = true;
    } else if (v->isArray() && v->cast_as<DataArray>().isA(typeid(DataEntityWrapper<void*>))) {
        auto dest_array = std::make_shared<DataEntityWrapper<void*>>();
        auto const& src_array = v->cast_as<DataArray>();
        for (size_type i = 0, ie = src_array.size(); i < ie; ++i) { dest_array->Add(src_array.Get(i)); }
        res = dest_array;
        success = true;
    } else if (res == nullptr || overwrite) {
        res = v;
        success = true;
    }
    if (success) { m_backend_->Set(uri, res, overwrite); }
};
void DataTable::Add(std::string const& uri, std::shared_ptr<DataEntity> const& v) { m_backend_->Add(uri, v); };
//******************************************************************************************************************
void DataTable::Link(std::shared_ptr<DataEntity> const& other) { Link("", other); };

DataTable& DataTable::Link(std::string const& uri, DataTable const& other) {
    ASSERT(other.m_backend_ != nullptr);
    if (uri == "") {
        m_backend_ = other.m_backend_;
        return *this;
    } else {
        m_backend_->Set(uri, std::make_shared<DataTable>(other.m_backend_), true);
        return Get(uri)->cast_as<DataTable>();
    }
}

DataTable& DataTable::Link(std::string const& uri, std::shared_ptr<DataEntity> const& other) {
    if (other == nullptr || !other->isTable()) { RUNTIME_ERROR << "link array or entity to table" << std::endl; }
    return Link(uri, other->cast_as<DataTable>());
}

void DataTable::Set(std::string const& uri, DataEntity const& p, bool overwrite) {
    Set(uri, p.Duplicate(), overwrite);
};
void DataTable::Add(std::string const& uri, DataEntity const& p) { Add(uri, p.Duplicate()); };

std::shared_ptr<DataTable> DataTable::GetTable(std::string const& uri) const {
    auto p = Get(uri);
    if (p == nullptr) {
        return nullptr;
    } else if (p->isTable()) {
        return std::dynamic_pointer_cast<DataTable>(p);
    } else {
        RUNTIME_ERROR << uri << " is not a table!" << std::endl;
        return nullptr;
    }
}

void DataTable::Delete(std::string const& uri) { m_backend_->Delete(uri); };
void DataTable::Set(std::shared_ptr<DataTable> const& other, bool overwrite) {
    if (other != nullptr) Set(*other, overwrite);
}
void DataTable::Set(DataTable const& other, bool overwrite) {
    other.Foreach([&](std::string const& k, std::shared_ptr<DataEntity> v) { Set(k, v, overwrite); });
}
void DataTable::SetValue(KeyValue const& item) { Set(item.first, item.second, true); }

void DataTable::SetValue(std::initializer_list<KeyValue> const& other) {
    for (auto const& item : other) { Set(item.first, item.second, true); }
}

size_type DataTable::Foreach(std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const& f) const {
    return m_backend_->Foreach(f);
}
std::shared_ptr<DataTable> DataTable::Serialize() const {
    auto p = std::make_shared<DataTable>();
    p->Set(*this);
    return p;
}
void DataTable::Deserialize(const std::shared_ptr<DataTable>& other) { Set(*other); }

std::ostream& DataTable::Serialize(std::ostream& os, int indent) const {
    os << "{";

    m_backend_->Foreach([&](std::string const& k, std::shared_ptr<DataEntity> const& v) {
        os << std::endl
           << std::setw(indent + 1) << " "
           << "\"" << k << "\": ";
        v->Serialize(os, indent + 1);
        os << ",";
    });

    os << std::endl
       << std::setw(indent) << " "
       << "}";
    return os;
};
std::istream& DataTable::Deserialize(std::istream& is) { return is; }

}  // namespace data
}  // namespace simpla