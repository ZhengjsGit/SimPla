//
// Created by salmon on 17-11-17.
//

#include "Serializable.h"
#include "Configurable.h"
#include "DataEntry.h"
namespace simpla {
namespace data {
Serializable::Serializable() = default;
Serializable::Serializable(Serializable const &) = default;
Serializable::~Serializable() = default;
void Serializable::Deserialize(std::shared_ptr<const DataEntry> const &cfg) {
    if (auto *config = dynamic_cast<Configurable *>(this)) { config->db()->Set(cfg); }
}
std::shared_ptr<DataEntry> Serializable::Serialize() const {
    auto res = DataEntry::Create(DataEntry::DN_TABLE);
    res->SetValue("_TYPE_", FancyTypeName());
    if (auto const *config = dynamic_cast<const Configurable *>(this)) { res->Set(config->db()); }
    return res;
};

std::ostream &operator<<(std::ostream &os, Serializable const &obj) {
    os << *obj.Serialize();
    return os;
}
std::istream &operator>>(std::istream &is, Serializable &obj) {
    auto db = DataEntry::Create(DataEntry::DN_TABLE);
    is >> *db;
    obj.Deserialize(db);
    return is;
}
}  // namespace geometry
}  // namespace simpla