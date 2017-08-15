//
// Created by salmon on 16-10-6.
//

#ifndef SIMPLA_DATATABLE_H_
#define SIMPLA_DATATABLE_H_
#include "simpla/SIMPLA_config.h"

#include <memory>
#include "DataArray.h"
#include "DataEntity.h"
#include "DataTraits.h"
#include "simpla/utilities/ObjectHead.h"
namespace simpla {
namespace data {

class DataBase;
class KeyValue;

class KeyValue : public std::pair<std::string, std::shared_ptr<DataEntity>> {
    typedef std::pair<std::string, std::shared_ptr<DataEntity>> base_type;

   public:
    explicit KeyValue(std::string const& k, std::shared_ptr<DataEntity> const& p = nullptr) : base_type(k, p) {}
    KeyValue(KeyValue const& other) : base_type(other) {}
    KeyValue(KeyValue&& other) : base_type(other) {}
    ~KeyValue() = default;

    KeyValue& operator=(KeyValue const& other) {
        //        base_type::operator=(other);
        return *this;
    }

    template <typename U>
    KeyValue& operator=(U const& u) {
        second = make_data_entity(u);
        return *this;
    }
    template <typename U>
    KeyValue& operator=(std::initializer_list<U> const& u) {
        second = make_data_entity(u);
        return *this;
    }
    template <typename U>
    KeyValue& operator=(std::initializer_list<std::initializer_list<U>> const& u) {
        second = make_data_entity(u);
        return *this;
    }
    template <typename U>
    KeyValue& operator=(std::initializer_list<std::initializer_list<std::initializer_list<U>>> const& u) {
        second = make_data_entity(u);
        return *this;
    }
};

inline KeyValue operator"" _(const char* c, std::size_t n) { return KeyValue{std::string(c), make_data_entity(true)}; }
/** @ingroup data */
/**
 * @brief  a @ref DataEntity tree, a key-value table of @ref DataEntity, which is similar as Group
 * in HDF5,  all node/table are DataEntity.
 * @design_pattern
 *  - Proxy for DataBackend
 */
class DataTable : public DataEntity {
    SP_OBJECT_HEAD(DataTable, DataEntity);
    std::shared_ptr<DataBase> m_database_;

   protected:
    explicit DataTable(std::shared_ptr<DataEntity> const& parent = nullptr,
                       std::shared_ptr<DataBase> const& db = nullptr);

   public:
    SP_DEFAULT_CONSTRUCT(DataTable);
    ~DataTable() override = default;
    //******************************************************************************************************************
    /** Interface DataBackend */
    std::shared_ptr<DataBase> database() const { return m_database_; }

    static std::shared_ptr<DataTable> New(std::string const& uri);
    static std::shared_ptr<DataTable> New(std::shared_ptr<DataBase> const& db = nullptr);

    int Flush();
    //******************************************************************************************************************
    /** Interface DataEntity */
    //    std::ostream& Serialize(std::ostream& os, int indent) const override;
    //    std::istream& Deserialize(std::istream& is) override;

    bool has(std::string const& uri) const { return Get(uri) != nullptr; }

    bool isNull(std::string const& url = "") const;
    size_type Count(std::string const& uri = "") const;
    std::shared_ptr<DataEntity> Get(std::string const& uri) const;
    int Set(std::string const& uri, const std::shared_ptr<DataEntity>& v);
    int Add(std::string const& uri, const std::shared_ptr<DataEntity>& v);
    int Delete(std::string const& uri);
    int Foreach(std::function<int(std::string const&, std::shared_ptr<DataEntity>)> const& f) const;

    void Set(DataTable const& other) { SetTable(other); }
    void SetTable(DataTable const& other);
    DataTable& GetTable(std::string const& uri);
    const DataTable& GetTable(std::string const& uri) const;

    /** Interface DataBackend End */

    template <typename U>
    bool Check(std::string const& uri, U const& u = true) const {
        auto p = Get(uri);
        return p->Check(u);
    }
    template <typename U>
    bool isA(std::string const& uri) const {
        auto p = Get(uri);
        return p != nullptr && p->isA<U>();
    }

    template <typename U>
    U GetValue(std::string const& uri) const {
        auto res = std::dynamic_pointer_cast<DataEntityWrapper<U>>(Get(uri));
        if (res == nullptr) { OUT_OF_RANGE << "Can not find entity [" << uri << "]" << std::endl; }
        return res->value();
    }

    template <typename U>
    U GetValue(std::string const& uri, U const& default_value) const {
        auto res = std::dynamic_pointer_cast<DataEntityWrapper<U>>(Get(uri));
        return res == nullptr ? default_value : res->value();
    }

    void SetValue(std::string const& uri, DataTable const& v) { GetTable(uri).SetTable(v); };

    void SetValue(KeyValue const& kv) { Set(kv.first, kv.second); }
    template <typename... Others>
    void SetValue(KeyValue const& kv, Others&&... others) {
        Set(kv.first, kv.second);
        SetValue(std::forward<Others>(others)...);
    }
    void SetValue(std::initializer_list<KeyValue> const& u) {
        for (auto const& item : u) { SetValue(item); }
    }

    template <typename U>
    void SetValue(std::string const& uri, U const& v) {
        Set(uri, make_data_entity(v));
    };

    template <typename U>
    void SetValue(std::string const& uri, std::initializer_list<U> const& u) {
        Set(uri, make_data_entity(u));
    };
    template <typename U>
    void SetValue(std::string const& uri, std::initializer_list<std::initializer_list<U>> const& u) {
        Set(uri, make_data_entity(u));
    };
    template <typename U>
    void SetValue(std::string const& uri,
                  std::initializer_list<std::initializer_list<std::initializer_list<U>>> const& u) {
        Set(uri, make_data_entity(u));
    };
    template <typename U>
    void AddValue(std::string const& uri, U const& v) {
        Add(uri, make_data_entity(v));
    };
    template <typename U>
    void AddValue(std::string const& uri, std::initializer_list<U> const& u) {
        Add(uri, make_data_entity(u));
    };
    template <typename U>
    void AddValue(std::string const& uri, std::initializer_list<std::initializer_list<U>> const& u) {
        Add(uri, make_data_entity(u));
    };
};

inline std::shared_ptr<DataEntity> make_data_entity(std::initializer_list<KeyValue> const& u) {
    auto res = DataTable::New();
    for (KeyValue const& v : u) { res->Set(v.first, v.second); }
    return std::dynamic_pointer_cast<DataEntity>(res);
}
template <typename... Others>
std::shared_ptr<DataEntity> make_data_entity(KeyValue const& first, Others&&... others) {
    auto res = DataTable::New();
    res->Set(first, std::forward<Others>(others)...);
    return res;
}
}  // namespace data

template <typename U, typename... Args>
std::shared_ptr<U> CreateObject(data::DataEntity const* dataEntity, Args&&... args) {
    std::shared_ptr<U> res = nullptr;
    if (dynamic_cast<data::DataEntityWrapper<std::string> const*>(dataEntity) != nullptr) {
        res = U::Create(dynamic_cast<data::DataEntityWrapper<std::string> const*>(dataEntity)->value(),
                        std::forward<Args>(args)...);
    } else if (dynamic_cast<data::DataTable const*>(dataEntity) != nullptr) {
        auto const* db = dynamic_cast<data::DataTable const*>(dataEntity);
        res = U::Create(db->GetValue<std::string>("Type", ""), std::forward<Args>(args)...);
        res->Deserialize(*db);
    } else {
        res = U::Create("", std::forward<Args>(args)...);
    }
    return res;
};

}  // namespace simpla

#endif  // SIMPLA_DATATABLE_H_
