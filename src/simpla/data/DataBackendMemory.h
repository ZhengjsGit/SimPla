//
// Created by salmon on 17-3-6.
//

#ifndef SIMPLA_DATABACKENDMEMORY_H
#define SIMPLA_DATABACKENDMEMORY_H

#include <ostream>
#include <typeindex>
#include "DataBackend.h"
#include "DataEntity.h"
#include "DataTable.h"
namespace simpla {
namespace data {
class DataBackendMemory : public DataBackend {
   public:
    DataBackendMemory(std::string const& url = "", std::string const& status = "");
    virtual ~DataBackendMemory();
    virtual std::type_info const& type() const { return typeid(DataBackendMemory); };
    virtual std::ostream& Print(std::ostream& os, int indent = 0) const;
    virtual bool empty() const;

    virtual void Open(std::string const& url, std::string const& status = "");
    virtual void Parse(std::string const& str);
    virtual void Close();
    virtual void Flush();
    virtual void Clear();
    virtual void Reset();
    virtual std::shared_ptr<DataTable> CreateTable(std::string const& url);
    virtual std::shared_ptr<DataEntity> Set(std::string const& k, std::shared_ptr<DataEntity> const& v);
    virtual std::shared_ptr<DataEntity> Get(std::string const& url);
    virtual std::shared_ptr<DataEntity> Get(std::string const& url) const;
    virtual bool Erase(std::string const& url);

   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};  // class DataBackend {
}  // namespace data {
}  // namespace simpla{
#endif  // SIMPLA_DATABACKENDMEMORY_H
