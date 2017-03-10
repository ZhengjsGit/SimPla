//
// Created by salmon on 17-3-6.
//

#ifndef SIMPLA_DATATABLEFACTROY_H
#define SIMPLA_DATATABLEFACTROY_H

#include <simpla/concept/Printable.h>
#include <simpla/design_pattern/Factory.h>
#include <simpla/design_pattern/SingletonHolder.h>
#include <map>
#include <memory>
#include <string>
namespace simpla {
namespace data {
class DataBackend;

class DataBackendFactory : public design_pattern::Factory<std::string, DataBackend, std::string const &>,
                           public concept::Printable {
    typedef design_pattern::Factory<id_type, DataBackend, std::string const &> base_type;

   public:
    DataBackendFactory();
    virtual ~DataBackendFactory();
    std::ostream &Print(std::ostream &os, int indent = 0) const;
    DataBackend *Create(std::string const &uri);
    void RegisterDefault();


   private:
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};

// static DataBackendFactory g_DataBackendFactory;g_DataBackendFactory

#define GLOBAL_DATA_BACKEND_FACTORY SingletonHolder<DataBackendFactory>::instance()

//#define REGISTER_DATA_BACKEND_CREATOR(__type, __ext)
//    static const bool __type##_registered = GLOBAL_DATA_BACKEND_FACTORY.template Register<__type>(__STRING(__ext));

}  // namespace data{
}  // namespace simpla{
#endif  // SIMPLA_DATATABLEFACTROY_H