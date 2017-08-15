//
// Created by salmon on 16-10-28.
//

#ifndef SIMPLA_LUADATABASE_H
#define SIMPLA_LUADATABASE_H
#include <memory>
#include <ostream>
#include <string>
#include "simpla/SIMPLA_config.h"
#include "simpla/data/DataBase.h"

namespace simpla {
namespace data {

class DataEntity;

class DataBaseLua : public DataBase {
    SP_DATABASE_DECLARE_MEMBERS(DataBaseLua)

   private:
    struct pimpl_s;
    pimpl_s *m_pimpl_ = nullptr;
};
}  // { namespace data {
}  // namespace simpla
#endif  // SIMPLA_LUADATABASE_H
