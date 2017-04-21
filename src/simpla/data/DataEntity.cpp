//
// Created by salmon on 16-6-6.
//
#include "DataEntity.h"
#include "DataBlock.h"
#include "DataTable.h"

namespace simpla {
namespace data {
//DataEntity::DataEntity() {}
//DataEntity::~DataEntity() {}
std::ostream& DataEntity::Serialize(std::ostream& os, int indent) const {
    if (isLight()) {
        os << "<Light Data:" << value_type_info().name() << ">";
    } else if (isBlock()) {
        os << "<Block:" << value_type_info().name() << "," << std::boolalpha << cast_as<DataBlock>().empty() << ">";
    } else if (isTable()) {
        os << "<Table:" << value_type_info().name() << ">";
    } else if (isArray()) {
        os << "<Array:" << value_type_info().name() << ">";
    }
    return os;
};
// DataEntity DataEntity::operator[](std::string const& url) {
//    if (m_data_ == nullptr) {
//        m_data_ = new DataTable;
//    } else if (!m_data_->isA(typeid(DataTable))) {
//        RUNTIME_ERROR << "Data entity is not indexable!" << std::endl;
//    }
//    return static_cast<DataTable*>(m_data_)->Insert(url).first;
//};
// DataEntity DataEntity::operator[](std::string const& url) const {
//    if (!m_data_->isA(typeid(DataTable))) { RUNTIME_ERROR << "Data entity is not indexable!" << std::endl; }
//    return (*static_cast<DataTable const*>(m_data_))[url];
//}
}  // namespace get_mesh{
}  // namespace simpla{