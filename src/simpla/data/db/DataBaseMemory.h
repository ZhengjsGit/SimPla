//
// Created by salmon on 17-3-6.
//

#ifndef SIMPLA_DATABACKENDMEMORY_H
#define SIMPLA_DATABACKENDMEMORY_H

#include <ostream>
#include <typeindex>

#include "../DataBase.h"

namespace simpla {
namespace data {
class DataBaseMemory : public DataBase {
    SP_DATABASE_DECLARE_MEMBERS(DataBaseMemory)
};  // class DataBase {
}  // namespace data {
}  // namespace simpla{
#endif  // SIMPLA_DATABACKENDMEMORY_H