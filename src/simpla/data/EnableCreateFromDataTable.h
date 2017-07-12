//
// Created by salmon on 17-4-12.
//

#ifndef SIMPLA_ENABLECREATEFROMDATATABLE_H
#define SIMPLA_ENABLECREATEFROMDATATABLE_H

#include <simpla/utilities/SingletonHolder.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include "DataTable.h"
namespace simpla {
namespace data {
class DataTable;
template <typename TObj, typename... Args>
class EnableCreateFromDataTable : public data::Serializable {
    typedef EnableCreateFromDataTable<TObj> this_type;

   public:
    EnableCreateFromDataTable() = default;
    virtual ~EnableCreateFromDataTable() = default;
    EnableCreateFromDataTable(this_type const &other) = delete;
    EnableCreateFromDataTable(this_type &&other) = delete;
    this_type &operator=(this_type const &other) = delete;
    this_type &operator=(this_type &&other) = delete;

    virtual std::string GetRegisterName() const { return TObj::RegisterName(); }

    std::shared_ptr<DataTable> Serialize() const override {
        auto res = std::make_shared<data::DataTable>();
        res->SetValue("Type", GetRegisterName());
        return res;
    }

    void Deserialize(const std::shared_ptr<DataTable> &t) override{};

    struct ObjectFactory {
        std::map<std::string, std::function<TObj *(Args const &...)>> m_factory_;
    };
    static bool HasCreator(std::string const &k) {
        auto const &f = SingletonHolder<ObjectFactory>::instance().m_factory_;
        return f.find(k) != f.end();
    }
    static std::string ShowDescription(std::string const &k = "") {
        auto const &f = SingletonHolder<ObjectFactory>::instance().m_factory_;
        std::string res;
        if (k != "") {
            auto it = f.find(k);
            if (it != f.end()) { res = it->first; }
        }
        if (res == "") {
            std::ostringstream os;
            os << std::endl << "Registered " << TObj::RegisterName() << " Creator:" << std::endl;
            for (auto const &item : f) { os << " " << item.first << std::endl; }
            res = os.str();
        }
        return res;
    };
    static bool RegisterCreator(std::string const &k, std::function<TObj *(Args const &...)> const &fun) noexcept {
        return SingletonHolder<ObjectFactory>::instance().m_factory_.emplace(k, fun).second;
    };
    template <typename U>
    static bool RegisterCreator(std::string const &k_hint = "") noexcept {
        return RegisterCreator(k_hint != "" ? k_hint : U::RegisterName(),
                               [](Args const &... args) { return new U(args...); });
    };
    template <typename... U>
    static std::shared_ptr<TObj> Create(std::string const &k, U const &... args) {
        if (k == "") { return nullptr; }
        if (k.find("://") != std::string::npos) { return Create(std::make_shared<data::DataTable>(k), args...); }
        auto const &f = SingletonHolder<ObjectFactory>::instance().m_factory_;
        std::shared_ptr<TObj> res = nullptr;
        auto it = f.find(k);

        if (it != f.end()) {
            res.reset(it->second(args...));
            LOGGER << TObj::RegisterName() << "::" << it->first << "  is created!" << std::endl;
        } else {
            std::ostringstream os;
            os << "Can not find Creator " << k << std::endl;
            os << std::endl << "Register " << TObj::RegisterName() << " Creator:" << std::endl;
            for (auto const &item : f) { os << item.first << std::endl; }
            WARNING << os.str();
        }
        return res;
    }

   private:
    template <typename... U>
    static std::shared_ptr<TObj> _CreateIfNotAbstract(std::integral_constant<bool, true>, U &&... args) {
        return std::make_shared<TObj>(std::forward<U>(args)...);
    }
    template <typename... U>
    static std::shared_ptr<TObj> _CreateIfNotAbstract(std::integral_constant<bool, false>, U &&... args) {
        return nullptr;
    }

   public:
    template <typename... U>
    static std::shared_ptr<TObj> Create(std::shared_ptr<DataEntity> const &cfg, U &&... args) {
        std::shared_ptr<TObj> res = nullptr;
        std::string s_type = "";
        if (cfg == nullptr) {
        } else if (cfg->value_type_info() == typeid(std::string)) {
            s_type = data::DataCastTraits<std::string>::Get(cfg);
        } else if (cfg->isTable()) {
            auto t = std::dynamic_pointer_cast<data::DataTable>(cfg);
            s_type = t->GetValue<std::string>("Type", "");
        }

        if (s_type != "") {
            res = Create(s_type, args...);
        } else {
            res = _CreateIfNotAbstract(std::integral_constant<bool, !std::is_abstract<TObj>::value>(),
                                       std::forward<U>(args)...);
        }

        if (res != nullptr && cfg != nullptr && cfg->isTable()) {
            res->Deserialize(std::dynamic_pointer_cast<data::DataTable>(cfg));
        }

        return res;
    }
};

#define DECLARE_REGISTER_NAME(_REGISTER_NAME_)                                           \
   public:                                                                               \
    std::string GetRegisterName() const override { return RegisterName(); }              \
    static std::string RegisterName() { return std::string(__STRING(_REGISTER_NAME_)); } \
    static bool is_registered;

#define REGISTER_CREATOR(_CLASS_NAME_) bool _CLASS_NAME_::is_registered = _CLASS_NAME_::RegisterCreator<_CLASS_NAME_>();

}  // namespace data{
}  // namespace simpla{
#endif  // SIMPLA_ENABLECREATEFROMDATATABLE_H
