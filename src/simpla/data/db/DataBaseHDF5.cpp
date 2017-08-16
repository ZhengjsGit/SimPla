//
// Created by salmon on 17-3-10.
//
#include "DataBaseHDF5.h"
#include <sys/stat.h>
#include <regex>
#include "../DataArray.h"
#include "../DataBlock.h"
#include "../DataEntity.h"
#include "../DataTable.h"
#include "DataUtility.h"
#include "simpla/parallel/MPIComm.h"
extern "C" {
#include <hdf5.h>
#include <hdf5_hl.h>
}

#include <H5FDmpio.h>
namespace simpla {
namespace data {
REGISTER_CREATOR(DataBaseHDF5, hdf5);

#define H5_ERROR(_FUN_)                                                               \
    if ((_FUN_) < 0) {                                                                \
        H5Eprint(H5E_DEFAULT, stderr);                                                \
        RUNTIME_ERROR << "\e[1;32m"                                                   \
                      << "HDF5 Error:" << __STRING(_FUN_) << "\e[1;37m" << std::endl; \
    }

void HDF5Closer(void* ptr) {
    hid_t f = *reinterpret_cast<hid_t*>(ptr);
    if (f != -1) { H5Fclose(f); }
}
struct DataBaseHDF5::pimpl_s {
    std::shared_ptr<const hid_t> m_f_id_;
    hid_t m_g_id_ = -1;

    static std::pair<hid_t, std::string> HDf5GetTable(DataBaseHDF5 const* self, hid_t root, std::string const& uri,
                                                      bool return_if_not_exist = false);
    static std::shared_ptr<DataEntity> HDF5Get(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key = ".");

    static std::shared_ptr<DataEntity> HDF5AttrCast(hid_t attr_id);

    static std::pair<std::string, std::shared_ptr<DataEntity>> HDF5GetAttrByIndex(DataBaseHDF5 const* self,
                                                                                  hid_t loc_id, int i);

    static int HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataEntity> const&, bool overwrite = true);
    static int HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataTable> const&, bool overwrite = true);
    static int HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataArray> const&, bool overwrite = true);
    static int HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataBlock> const&, bool overwrite = true);

    static int HDF5Add(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataBlock> const&);
    static int HDF5Add(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                       std::shared_ptr<DataEntity> const&);
};

std::pair<hid_t, std::string> DataBaseHDF5::pimpl_s::HDf5GetTable(DataBaseHDF5 const* self, hid_t root,
                                                                  std::string const& uri, bool return_if_not_exist) {
    ASSERT(root != -1);
    return HierarchicalTableForeach(
        root, uri, [&](hid_t g, std::string const& k) { return H5Lexists(g, k.c_str(), H5P_DEFAULT) != 0; },
        [&](hid_t g, std::string const& k) { return H5Gopen(g, k.c_str(), H5P_DEFAULT); },
        [&](hid_t g, std::string const& k) { return H5Gcreate(g, k.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); });
};

template <typename U>
std::shared_ptr<DataEntity> read_attr(hid_t attr_id, hid_t d_type) {
    U res;
    H5Aread(attr_id, d_type, &res);
    return make_data_entity(res);
}

std::pair<std::string, std::shared_ptr<DataEntity>> DataBaseHDF5::pimpl_s::HDF5GetAttrByIndex(DataBaseHDF5 const* self,
                                                                                              hid_t loc_id, int i) {
    ssize_t num = H5Aget_name_by_idx(loc_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, nullptr, 0, H5P_DEFAULT);
    char buffer[num + 1];
    H5_ERROR(H5Aget_name_by_idx(loc_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, buffer, static_cast<size_t>(num + 1),
                                H5P_DEFAULT));
    hid_t a_id = H5Aopen_by_idx(loc_id, ".", H5_INDEX_NAME, H5_ITER_INC, i, H5P_DEFAULT, H5P_DEFAULT);
    auto v = pimpl_s::HDF5AttrCast(a_id);
    H5Aclose(a_id);
    return std::make_pair(std::string(buffer), v);
}
template <template <typename> class TFun, typename... Args>
void H5TypeDispatch(hid_t d_type, Args&&... args) {
    H5T_class_t type_class = H5Tget_class(d_type);

    if ((type_class == H5T_INTEGER || type_class == H5T_FLOAT)) {
        if (H5Tequal(d_type, H5T_NATIVE_CHAR) > 0) {
            TFun<char>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_SHORT) > 0) {
            TFun<short>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_INT) > 0) {
            TFun<int>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_LONG) > 0) {
            TFun<double>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_LLONG) > 0) {
            TFun<long long>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_UCHAR) > 0) {
            TFun<unsigned char>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_USHORT) > 0) {
            TFun<unsigned short>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_UINT) > 0) {
            TFun<unsigned int>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_ULONG) > 0) {
            TFun<unsigned long>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_ULLONG) > 0) {
            TFun<unsigned long long>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_FLOAT) > 0) {
            TFun<float>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_DOUBLE) > 0) {
            TFun<double>(std::forward<Args>(args)...);
        } else if (H5Tequal(d_type, H5T_NATIVE_LDOUBLE) > 0) {
            TFun<long double>(std::forward<Args>(args)...);
        }
    } else if (type_class == H5T_ARRAY) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_STRING) {
        TFun<std::string>(std::forward<Args>(args)...);
    } else if (type_class == H5T_TIME) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_BITFIELD) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_REFERENCE) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_ENUM) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_VLEN) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_NO_CLASS) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_OPAQUE) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_COMPOUND) {
        UNIMPLEMENTED;
    }
}

std::shared_ptr<DataEntity> DataBaseHDF5::pimpl_s::HDF5AttrCast(hid_t attr_id) {
    std::shared_ptr<DataEntity> res = nullptr;
    hid_t d_type = H5Aget_type(attr_id);
    hid_t d_space = H5Aget_space(attr_id);

    H5T_class_t type_class = H5Tget_class(d_type);

    if ((type_class == H5T_INTEGER || type_class == H5T_FLOAT)) {
        if (H5Tequal(d_type, H5T_NATIVE_CHAR) > 0) {
            res = read_attr<char>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_SHORT) > 0) {
            res = read_attr<short>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_INT) > 0) {
            res = read_attr<int>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_LONG) > 0) {
            res = read_attr<double>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_LLONG) > 0) {
            res = read_attr<long long>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_UCHAR) > 0) {
            res = read_attr<unsigned char>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_USHORT) > 0) {
            res = read_attr<unsigned short>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_UINT) > 0) {
            res = read_attr<unsigned int>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_ULONG) > 0) {
            res = read_attr<unsigned long>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_ULLONG) > 0) {
            res = read_attr<unsigned long long>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_FLOAT) > 0) {
            res = read_attr<float>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_DOUBLE) > 0) {
            res = read_attr<double>(attr_id, d_type);
        } else if (H5Tequal(d_type, H5T_NATIVE_LDOUBLE) > 0) {
            res = read_attr<long double>(attr_id, d_type);
        }
    } else if (type_class == H5T_ARRAY) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_STRING) {
        size_t sdims = H5Tget_size(d_type);
        ++sdims;
        char buffer[sdims];
        hid_t m_type = H5Tcopy(H5T_C_S1);
        H5Tset_size(m_type, sdims);
        H5Aread(attr_id, m_type, buffer);
        res = make_data_entity<std::string>(std::string(buffer));
        H5Tclose(m_type);
    } else if (type_class == H5T_TIME) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_BITFIELD) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_REFERENCE) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_ENUM) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_VLEN) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_NO_CLASS) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_OPAQUE) {
        UNIMPLEMENTED;
    } else if (type_class == H5T_COMPOUND) {
        UNIMPLEMENTED;
    }

    H5Tclose(d_type);
    H5Sclose(d_space);
    return res;
}
template <typename U>
std::shared_ptr<DataBlock> HDF5ReadDataSet(DataBaseHDF5 const* self, hid_t loc_id, std::string const& name) {
    UNIMPLEMENTED;
    return nullptr;
}
std::shared_ptr<DataEntity> DataBaseHDF5::pimpl_s::HDF5Get(DataBaseHDF5 const* self, hid_t loc_id,
                                                           std::string const& key) {
    if (H5Lexists(loc_id, key.c_str(), H5P_DEFAULT) == 0) { return nullptr; }
    std::shared_ptr<DataEntity> res = nullptr;
    if (H5Oexists_by_name(loc_id, key.c_str(), H5P_DEFAULT) != 0) {
        H5O_info_t g_info;
        H5_ERROR(H5Oget_info_by_name(loc_id, key.c_str(), &g_info, H5P_DEFAULT));
        switch (g_info.type) {
            case H5O_TYPE_GROUP: {
                auto t_backend = DataBaseHDF5::New();
                t_backend->m_pimpl_->m_f_id_ = self->m_pimpl_->m_f_id_;
                t_backend->m_pimpl_->m_g_id_ = H5Gopen(loc_id, key.c_str(), H5P_DEFAULT);
                res = DataTable::New(t_backend);
                break;
            }
            case H5O_TYPE_DATASET: {
                hid_t d_id = H5Dopen(loc_id, key.c_str(), H5P_DEFAULT);

                break;
            }
            default:
                break;
        }
    } else if (H5Aexists(loc_id, key.c_str()) != 0) {
        hid_t a_id = H5Aopen(loc_id, key.c_str(), H5P_DEFAULT);
        auto v = pimpl_s::HDF5AttrCast(a_id);
        H5Aclose(a_id);
    }
    //    else {
    //        RUNTIME_ERROR << "Object does not exist! [" << key << "]" << std::endl;
    //    }
    return res;
}

hid_t GetHDF5DataType(std::type_info const& t_info) {
    hid_t v_type = H5T_NO_CLASS;

    if (t_info == typeid(int)) {
        v_type = H5T_NATIVE_INT;
    } else if (t_info == typeid(long)) {
        v_type = H5T_NATIVE_LONG;
    } else if (t_info == typeid(unsigned long)) {
        v_type = H5T_NATIVE_ULONG;
    } else if (t_info == typeid(float)) {
        v_type = H5T_NATIVE_FLOAT;
    } else if (t_info == typeid(double)) {
        v_type = H5T_NATIVE_DOUBLE;
    } else if (t_info == typeid(std::complex<double>)) {
        H5_ERROR(v_type = H5Tcreate(H5T_COMPOUND, sizeof(std::complex<double>)));
        H5_ERROR(H5Tinsert(v_type, "r", 0, H5T_NATIVE_DOUBLE));
        H5_ERROR(H5Tinsert(v_type, "i", sizeof(double), H5T_NATIVE_DOUBLE));

    }
    // TODO:
    //   else if (d_type->isArray()) {
    //        auto const& t_array = d_type->cast_as<DataArray>();
    //        hsize_t dims[t_array.rank()];
    //        for (int i = 0; i < t_array.rank(); ++i) { dims[i] = t_array.dimensions()[i]; }
    //        hid_t res2 = res;
    //        H5_ERROR(res2 = H5Tarray_create(res, t_array.rank(), dims));
    //        if (H5Tcommitted(res) > 0) H5_ERROR(H5Tclose(res));
    //        res = res2;
    //    } else if (d_type->isTable()) {
    //        H5_ERROR(v_type = H5Tcreate(H5T_COMPOUND, d_type.size_in_byte()));
    //
    //        for (auto const& item : d_type.members()) {
    //            hid_t t_member = convert_data_type_sp_to_h5(std::get<0>(item), true);
    //
    //            H5_ERROR(H5Tinsert(res, std::get<1>(item).c_str(), std::get<2>(item), t_member));
    //            if (H5Tcommitted(t_member) > 0) H5_ERROR(H5Tclose(t_member));
    //        }
    //    }
    else {
        RUNTIME_ERROR << "Unknown m_data type:" << t_info.name();
    }

    return (v_type);
}

hid_t OpenGroup(hid_t loc_id, std::string const& key, bool overwrite) {
    bool is_exist = H5Lexists(loc_id, key.c_str(), H5P_DEFAULT) != 0;
    //    H5Oexists_by_name(loc_id, key.c_str(), H5P_DEFAULT) != 0;
    H5O_info_t g_info;
    if (is_exist) { H5_ERROR(H5Oget_info_by_name(loc_id, key.c_str(), &g_info, H5P_DEFAULT)); }
    if (is_exist && !overwrite) { return -1; }

    //    if (overwrite && is_exist && g_info.type != H5O_TYPE_GROUP) {
    //        H5Ldelete(loc_id, key.c_str(), H5P_DEFAULT);
    //        is_exist = false;
    //    }

    hid_t gid;
    if (!is_exist) {
        gid = H5Gcreate(loc_id, key.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    } else {
        gid = H5Gopen(loc_id, key.c_str(), H5P_DEFAULT);
    }
    return gid;
}
int DataBaseHDF5::pimpl_s::HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                                   std::shared_ptr<DataTable> const& src, bool overwrite) {
    hid_t gid = OpenGroup(loc_id, key, overwrite);

    src->Foreach(
        [&](std::string const& k, std::shared_ptr<data::DataEntity> v) { return HDF5Set(self, gid, k, v, overwrite); });
    H5_ERROR(H5Gclose(gid));
    return 1;
};
int DataBaseHDF5::pimpl_s::HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                                   std::shared_ptr<DataArray> const& t, bool overwrite) {
    hid_t gid = OpenGroup(loc_id, key, overwrite);

    auto num = t->Count();
    for (int i = 0; i < num; ++i) { HDF5Set(self, gid, std::to_string(i), t->Get(i), overwrite); }

    H5_ERROR(H5Gclose(gid));
    return 1;
}
int DataBaseHDF5::pimpl_s::HDF5Set(DataBaseHDF5 const* self, hid_t loc_id, std::string const& key,
                                   std::shared_ptr<DataBlock> const& src, bool overwrite) {
    if (src->isNull()) {
        LOGGER << "Write Empty DataBlock" << key << std::endl;
        return 0;
    }
    bool is_exist = H5Lexists(loc_id, key.c_str(), H5P_DEFAULT) != 0;
    //            H5Oexists_by_name(loc_id, key.c_str(), H5P_DEFAULT) != 0;
    H5O_info_t g_info;
    if (is_exist) { H5_ERROR(H5Oget_info_by_name(loc_id, key.c_str(), &g_info, H5P_DEFAULT)); }
    if (is_exist && !overwrite) { return 0; }

    if (overwrite && is_exist && g_info.type != H5O_TYPE_DATASET) {
        H5Ldelete(loc_id, key.c_str(), H5P_DEFAULT);
        is_exist = false;
    }

    index_type const* inner_lower = src->GetInnerLowerIndex(0);
    index_type const* inner_upper = src->GetInnerUpperIndex(0);
    index_type const* outer_lower = src->GetOuterLowerIndex(0);
    index_type const* outer_upper = src->GetOuterUpperIndex(0);
    const int ndims = src->GetNDIMS();
    hsize_t m_shape[ndims];
    hsize_t m_start[ndims];
    hsize_t m_count[ndims];
    hsize_t m_stride[ndims];
    hsize_t m_block[ndims];
    for (int i = 0; i < ndims; ++i) {
        m_shape[i] = static_cast<hsize_t>(outer_upper[i] - outer_lower[i]);
        m_start[i] = static_cast<hsize_t>(inner_lower[i] - outer_lower[i]);
        m_count[i] = static_cast<hsize_t>(inner_upper[i] - inner_lower[i]);
        m_stride[i] = static_cast<hsize_t>(1);
        m_block[i] = static_cast<hsize_t>(1);
    }
    hid_t m_space = H5Screate_simple(ndims, &m_shape[0], nullptr);
    H5_ERROR(H5Sselect_hyperslab(m_space, H5S_SELECT_SET, &m_start[0], &m_stride[0], &m_count[0], &m_block[0]));
    hid_t f_space = H5Screate_simple(ndims, &m_count[0], nullptr);
    hid_t dset;
    hid_t d_type = GetHDF5DataType(src->value_type_info());
    H5_ERROR(dset = H5Dcreate(loc_id, key.c_str(), d_type, f_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT));
    H5_ERROR(H5Dwrite(dset, d_type, m_space, f_space, H5P_DEFAULT, src->GetPointer(0)));

    H5_ERROR(H5Dclose(dset));
    if (m_space != H5S_ALL) H5_ERROR(H5Sclose(m_space));
    if (f_space != H5S_ALL) H5_ERROR(H5Sclose(f_space));

    return 1;
}
int DataBaseHDF5::pimpl_s::HDF5Add(DataBaseHDF5 const* self, hid_t loc_id, std::string const& name,
                                   std::shared_ptr<DataBlock> const&) {
    UNSUPPORTED;
    return 0;
};

int DataBaseHDF5::pimpl_s::HDF5Set(DataBaseHDF5 const* self, hid_t g_id, std::string const& key,
                                   std::shared_ptr<DataEntity> const& src, bool overwrite) {
    bool is_exist = H5Lexists(g_id, key.c_str(), H5P_DEFAULT) != 0;
    is_exist = is_exist || H5Aexists(g_id, key.c_str()) != 0;

    if (is_exist && !overwrite) { return 0; }

    if (std::dynamic_pointer_cast<DataTable>(src) != nullptr) {
        HDF5Set(self, g_id, key, std::dynamic_pointer_cast<DataTable>(src), overwrite);
    } else if (std::dynamic_pointer_cast<DataBlock>(src) != nullptr) {
        HDF5Set(self, g_id, key, std::dynamic_pointer_cast<DataBlock>(src), overwrite);
    } else if (std::dynamic_pointer_cast<DataEntityWrapper<std::string>>(src) != nullptr) {
        std::string const& s_str = std::dynamic_pointer_cast<DataEntityWrapper<std::string>>(src)->value();
        hid_t m_type = H5Tcopy(H5T_C_S1);
        H5Tset_size(m_type, s_str.size());
        H5Tset_strpad(m_type, H5T_STR_NULLTERM);
        hid_t m_space = H5Screate(H5S_SCALAR);
        hid_t a_id = H5Acreate(g_id, key.c_str(), m_type, m_space, H5P_DEFAULT, H5P_DEFAULT);
        H5Awrite(a_id, m_type, s_str.c_str());
        H5Tclose(m_type);
        H5Aclose(a_id);
    } else if (std::dynamic_pointer_cast<data::DataArray>(src) != nullptr) {
        HDF5Set(self, g_id, key, std::dynamic_pointer_cast<DataArray>(src), overwrite);
    } else {
        hid_t d_type = -1;
        hid_t d_space;
        char* data = nullptr;
        if (dynamic_cast<DataArray*>(src.get()) != nullptr) {
            hsize_t s = std::dynamic_pointer_cast<DataArray>(src)->Count();
            d_space = H5Screate_simple(1, &s, nullptr);
        } else {
            d_space = H5Screate(H5S_SCALAR);
        }

        if (false) {}
#define DEC_TYPE(_T_, _H5_T_)                                                                                 \
    else if (src->value_type_info() == typeid(_T_)) {                                                         \
        d_type = _H5_T_;                                                                                      \
        if (std::dynamic_pointer_cast<DataArrayWrapper<_T_>>(src) != nullptr) {                               \
            data = reinterpret_cast<char*>(&std::dynamic_pointer_cast<DataArrayWrapper<_T_>>(src)->get()[0]); \
        } else {                                                                                              \
            data = new char[sizeof(_T_)];                                                                     \
            *reinterpret_cast<_T_*>(data) = std::dynamic_pointer_cast<DataEntityWrapper<_T_>>(src)->value();  \
        }                                                                                                     \
    }

        //        DEC_TYPE(bool, H5T_NATIVE_HBOOL)
        DEC_TYPE(float, H5T_NATIVE_FLOAT)
        DEC_TYPE(double, H5T_NATIVE_DOUBLE)
        DEC_TYPE(int, H5T_NATIVE_INT)
        DEC_TYPE(long, H5T_NATIVE_LONG)
        DEC_TYPE(unsigned int, H5T_NATIVE_UINT)
        DEC_TYPE(unsigned long, H5T_NATIVE_ULONG)
#undef DEC_TYPE

        if (d_type != -1) {
            hid_t a_id;
            if (H5Aexists(g_id, key.c_str())) { H5Adelete(g_id, key.c_str()); }
            a_id = H5Acreate(g_id, key.c_str(), d_type, d_space, H5P_DEFAULT, H5P_DEFAULT);
            H5Awrite(a_id, d_type, data);
            H5Aclose(a_id);
        }
        if (std::dynamic_pointer_cast<DataArray>(src) == nullptr) { delete data; }
    }

    return 1;
}

int DataBaseHDF5::pimpl_s::HDF5Add(DataBaseHDF5 const* self, hid_t g_id, std::string const& key,
                                   std::shared_ptr<DataEntity> const& src) {
    int res = 0;
    if (dynamic_cast<DataTable const*>(src.get()) != nullptr) {
        res = HDF5Set(self, g_id, key, std::dynamic_pointer_cast<DataTable>(src), true);
    } else if (dynamic_cast<DataBlock const*>(src.get()) != nullptr) {
        res = HDF5Add(self, g_id, key, std::dynamic_pointer_cast<DataBlock>(src));
    } else {
        res = HDF5Set(self, g_id, key, src, true);
    }

    return res;
}
DataBaseHDF5::DataBaseHDF5() : m_pimpl_(new pimpl_s) {}
DataBaseHDF5::~DataBaseHDF5() { delete m_pimpl_; };

int DataBaseHDF5::Connect(std::string const& authority, std::string const& path, std::string const& query,
                          std::string const& fragment) {
    Disconnect();

    std::string filename = AutoIncreaseFileName(authority + "/" + path, ".h5");

    LOGGER << "Create HDF5 File: [" << filename << "]" << std::endl;

    mkdir(authority.c_str(), 0777);
    hid_t fid;
    H5_ERROR(fid = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT));
    m_pimpl_->m_f_id_ = std::shared_ptr<const hid_t>(new hid_t(fid), HDF5Closer);
    H5_ERROR(m_pimpl_->m_g_id_ = H5Gopen(*m_pimpl_->m_f_id_, "/", H5P_DEFAULT));
    return SP_SUCCESS;
};
int DataBaseHDF5::Disconnect() {
    if (m_pimpl_->m_g_id_ != -1) {
        H5Fclose(m_pimpl_->m_g_id_);
        m_pimpl_->m_g_id_ = -1;
    }
    m_pimpl_->m_f_id_.reset();
    return SP_SUCCESS;
};

int DataBaseHDF5::Flush() {
    H5_ERROR(H5Fflush(*m_pimpl_->m_f_id_, H5F_SCOPE_GLOBAL));
    return SP_SUCCESS;
}
bool DataBaseHDF5::isNull(std::string const& uri) const { return m_pimpl_->m_f_id_ == nullptr; }
size_type DataBaseHDF5::Count(std::string const& uri) const {
    UNIMPLEMENTED;
    return 0;
}

std::shared_ptr<DataEntity> DataBaseHDF5::Get(std::string const& uri) const {
    auto res = pimpl_s::HDf5GetTable(this, m_pimpl_->m_g_id_, uri, true);
    return (res.first == -1) ? nullptr : pimpl_s::HDF5Get(this, res.first, res.second);
}
int DataBaseHDF5::Set(std::string const& uri, const std::shared_ptr<DataEntity>& src) {
    if (src == nullptr) { return 0; }
    ASSERT(m_pimpl_->m_g_id_ != -1);
    auto res = pimpl_s::HDf5GetTable(this, m_pimpl_->m_g_id_, uri, false);
    if (res.second.empty()) { return 0; }
    ASSERT(res.first != -1);
    pimpl_s::HDF5Set(this, m_pimpl_->m_g_id_, res.second, src);
    return 1;
}
int DataBaseHDF5::Add(std::string const& uri, const std::shared_ptr<DataEntity>& src) {
    if (src == nullptr) { return 0; }
    auto res = pimpl_s::HDf5GetTable(this, m_pimpl_->m_g_id_, uri, false);
    if (res.first == -1 || res.second.empty()) { return 0; }
    pimpl_s::HDF5Add(this, res.first, res.second, src);
    return 1;
}
int DataBaseHDF5::Delete(std::string const& uri) {
    auto res = pimpl_s::HDf5GetTable(this, m_pimpl_->m_g_id_, uri, false);
    if (res.first == -1 || res.second.empty()) { return 0; }
    int count = 0;
    if (H5Aexists(res.first, res.second.c_str())) {
        H5Adelete(res.first, res.second.c_str());
        ++count;
    } else if (H5Lexists(res.first, res.second.c_str(), H5P_DEFAULT) != 0) {
        H5_ERROR(H5Ldelete(res.first, res.second.c_str(), H5P_DEFAULT));
        ++count;
    }
    return count;
}

// size_type DataBaseHDF5::Foreach(
//    std::function<void(std::string const&, std::shared_ptr<DataEntity>)> const& fun) const {
//    auto res = get_table_from_h5(m_pack_->m_g_id_, uri, false);
//    if (res.first == -1 || res.second.empty()) { return 0; }
//    if (H5Aexists(res.first, res.second.c_str())) {
//        H5_ERROR(H5Adelete(res.first, res.second.c_str()));
//        return 1;
//    } else {
//        return 0;
//    }
//}
//
// struct attr_op {
//    std::function<void(std::string, std::shared_ptr<DataEntity>)> m_op_;
//};
// herr_t attr_info(hid_t location_id /*in*/, const char* attr_name /*in*/, const H5A_info_t* ainfo /*in*/,
//                 void* op_data /*in,out*/) {
//    auto const& op = *reinterpret_cast<attr_op*>(op_data);
//    op.m_op_(std::string(attr_name), DataBaseHDF5::pack_s::HDF5Get(location_id, std::string(attr_name)));
//}
int DataBaseHDF5::Foreach(std::function<int(std::string const&, std::shared_ptr<DataEntity>)> const& fun) const {
    if (m_pimpl_->m_g_id_ == -1) { return 0; };
    H5G_info_t g_info;
    H5_ERROR(H5Gget_info(m_pimpl_->m_g_id_, &g_info));

    for (hsize_t i = 0; i < g_info.nlinks; ++i) {
        ssize_t num = H5Lget_name_by_idx(m_pimpl_->m_g_id_, ".", H5_INDEX_NAME, H5_ITER_INC, i, NULL, 0, H5P_DEFAULT);
        char buffer[num + 1];
        H5Lget_name_by_idx(m_pimpl_->m_g_id_, ".", H5_INDEX_NAME, H5_ITER_INC, i, buffer, static_cast<size_t>(num + 1),
                           H5P_DEFAULT);
        std::string name(buffer);
        fun(name, pimpl_s::HDF5Get(this, m_pimpl_->m_g_id_, name));
    }
    H5O_info_t o_info;
    H5_ERROR(H5Oget_info(m_pimpl_->m_g_id_, &o_info));
    for (int i = 0; i < o_info.num_attrs; ++i) {
        auto res = pimpl_s::HDF5GetAttrByIndex(this, m_pimpl_->m_g_id_, i);
        fun(res.first, res.second);
    }
    return g_info.nlinks;
}

}  // namespace data{
}  // namespace simpla{