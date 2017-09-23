//
// Created by salmon on 17-8-13.
//
#include <sys/stat.h>
#include <fstream>

#include <Xdmf.hpp>
#include <XdmfAttributeCenter.hpp>
#include <XdmfDomain.hpp>
#include <XdmfGridCollection.hpp>
#include <XdmfWriter.hpp>

#include <simpla/parallel/MPIComm.h>
#include <XdmfInformation.hpp>

#include "../DataNode.h"
#include "DataNodeMemory.h"

namespace simpla {
namespace data {
int XDMFDump(std::string const& url, std::shared_ptr<DataNode> const& v);

struct DataNodeXDMF : public DataNodeMemory {
    SP_DATA_NODE_HEAD(DataNodeXDMF, DataNodeMemory)

    int Connect(std::string const& authority, std::string const& path, std::string const& query,
                std::string const& fragment) override;
    int Disconnect() override { return Flush(); }
    bool isValid() const override { return true; }
    int Flush() override { return XDMFDump(m_file_, shared_from_this()); }

   private:
    std::string m_file_;
};
REGISTER_CREATOR(DataNodeXDMF, xmf);
DataNodeXDMF::DataNodeXDMF(DataNode::eNodeType etype) : base_type(etype) {}
DataNodeXDMF::~DataNodeXDMF() = default;

int DataNodeXDMF::Connect(std::string const& authority, std::string const& path, std::string const& query,
                          std::string const& fragment) {
    m_file_ = path;

#ifdef MPI_FOUND
    if (GLOBAL_COMM.size() > 1) {
        auto pos = path.rfind('.');
        auto prefix = (pos != std::string::npos) ? path.substr(0, pos) : path;
        auto ext = (pos != std::string::npos) ? path.substr(pos + 1) : path;
        int digital = static_cast<int>(std::floor(std::log(static_cast<double>(GLOBAL_COMM.size())))) + 1;
        if (GLOBAL_COMM.rank() == 0) {
            std::ofstream summary(prefix + ".summary.xmf");
            summary << R"(<?xml version="1.0" encoding="utf-8"?>)" << std::endl;
            summary << R"(<Xdmf Version="2.0" xmlns:xi="http://www.w3.org/2001/XInclude">)" << std::endl;
            for (int i = 0, ie = GLOBAL_COMM.size(); i < ie; ++i) {
                summary << "  <xi:include href=\"" << prefix << "." << std::setfill('0') << std::setw(digital) << i
                        << ".xmf\"/>" << std::endl;
            }
            summary << "</Xdmf>";
        }
        std::ostringstream os;
        os << prefix << "." << std::setfill('0') << std::setw(digital) << GLOBAL_COMM.rank() << ".xmf";
        m_file_ = os.str();
    }
#endif
    return SP_SUCCESS;
}
template <typename U>
struct XdmfType {};
template <>
struct XdmfType<double> {
    static auto type() { return XdmfArrayType::Float64(); }
};

template <>
struct XdmfType<float> {
    static auto type() { return XdmfArrayType::Float32(); }
};
template <>
struct XdmfType<int> {
    static auto type() { return XdmfArrayType::Int32(); }
};
template <>
struct XdmfType<long> {
    static auto type() { return XdmfArrayType::Int64(); }
};
template <>
struct XdmfType<unsigned int> {
    static auto type() { return XdmfArrayType::UInt32(); }
};
int XDMFWriteArray(XdmfArray* dst, std::shared_ptr<DataNode> const& data) {
    if (data == nullptr) { return 0; }
    if (data->type() == DataNode::DN_TABLE) {
    } else if (data->type() == DataNode::DN_ARRAY) {
        auto dof = static_cast<unsigned int>(data->size());
        nTuple<index_type, 3> lo{0, 0, 0}, hi{1, 1, 1}, dims{1, 1, 1};
        if (auto blk = std::dynamic_pointer_cast<ArrayBase>(data->Get(0)->GetEntity())) {
            blk->GetIndexBox(&lo[0], &hi[0]);
        } else {
            RUNTIME_ERROR << *data;
        }

        std::vector<unsigned int> dimensions{static_cast<unsigned int>(hi[0] - lo[0]),
                                             static_cast<unsigned int>(hi[1] - lo[1]),
                                             static_cast<unsigned int>(hi[2] - lo[2]), dof};
        if (auto block = std::dynamic_pointer_cast<Array<double>>(data->GetEntity(0))) {
            dst->initialize(XdmfType<double>::type(), dimensions);
        } else if (auto block = std::dynamic_pointer_cast<Array<float>>(data->GetEntity(0))) {
            dst->initialize(XdmfType<float>::type(), dimensions);
        } else if (auto block = std::dynamic_pointer_cast<Array<int>>(data->GetEntity(0))) {
            dst->initialize(XdmfType<int>::type(), dimensions);
        } else if (auto block = std::dynamic_pointer_cast<Array<long>>(data->GetEntity(0))) {
            dst->initialize(XdmfType<long>::type(), dimensions);
        } else if (auto block = std::dynamic_pointer_cast<Array<unsigned int>>(data->GetEntity(0))) {
            dst->initialize(XdmfType<unsigned int>::type(), dimensions);
        } else {
            UNIMPLEMENTED;
        }

        for (unsigned int i = 0; i < dof; ++i) {
            if (auto block = std::dynamic_pointer_cast<Array<double>>(data->GetEntity(i))) {
                dst->insert(i, block->get(), static_cast<unsigned int>(block->size()), dof, 1);
            } else if (auto block = std::dynamic_pointer_cast<Array<float>>(data->GetEntity(i))) {
                dst->insert(i, block->get(), static_cast<unsigned int>(block->size()), dof, 1);
            } else if (auto block = std::dynamic_pointer_cast<Array<int>>(data->GetEntity(i))) {
                dst->insert(i, block->get(), static_cast<unsigned int>(block->size()), dof, 1);
            } else if (auto block = std::dynamic_pointer_cast<Array<long>>(data->GetEntity(i))) {
                dst->insert(i, block->get(), static_cast<unsigned int>(block->size()), dof, 1);
            } else if (auto block = std::dynamic_pointer_cast<Array<unsigned int>>(data->GetEntity(i))) {
                dst->insert(i, block->get(), static_cast<unsigned int>(block->size()), dof, 1);
            } else {
                UNIMPLEMENTED;
            }
        }
    } else if (auto blk = std::dynamic_pointer_cast<ArrayBase>(data->GetEntity())) {
        nTuple<index_type, 3> lo{0, 0, 0}, hi{1, 1, 1};
        blk->GetIndexBox(&lo[0], &hi[0]);
        std::vector<unsigned int> dimensions{static_cast<unsigned int>(hi[0] - lo[0]),
                                             static_cast<unsigned int>(hi[1] - lo[1]),
                                             static_cast<unsigned int>(hi[2] - lo[2])};

        if (auto block = std::dynamic_pointer_cast<Array<double>>(blk)) {
            dst->initialize(XdmfType<double>::type(), dimensions);
            dst->insert(0, block->get(), static_cast<unsigned int>(block->size()), 1, 1);
        } else if (auto block = std::dynamic_pointer_cast<Array<float>>(blk)) {
            dst->initialize(XdmfType<float>::type(), dimensions);
            dst->insert(0, block->get(), static_cast<unsigned int>(block->size()), 1, 1);
        } else if (auto block = std::dynamic_pointer_cast<Array<int>>(blk)) {
            dst->initialize(XdmfType<int>::type(), dimensions);
            dst->insert(0, block->get(), static_cast<unsigned int>(block->size()), 1, 1);
        } else if (auto block = std::dynamic_pointer_cast<Array<long>>(blk)) {
            dst->initialize(XdmfType<long>::type(), dimensions);
            dst->insert(0, block->get(), static_cast<unsigned int>(block->size()), 1, 1);
        } else if (auto block = std::dynamic_pointer_cast<Array<unsigned int>>(blk)) {
            dst->initialize(XdmfType<unsigned int>::type(), dimensions);
            dst->insert(0, block->get(), static_cast<unsigned int>(block->size()), 1, 1);
        } else {
            UNIMPLEMENTED;
        }
    }

    //            (auto p = std::dynamic_pointer_cast<Array<double>>(data->GetEntity())) {
    //        XDMFWriteArrayT(dst, p);
    //    } else if (auto p = std::dynamic_pointer_cast<Array<float>>(data->GetEntity())) {
    //        XDMFWriteArrayT(dst, p);
    //    } else if (auto p = std::dynamic_pointer_cast<Array<int>>(data->GetEntity())) {
    //        XDMFWriteArrayT(dst, p);
    //    } else if (auto p = std::dynamic_pointer_cast<Array<long>>(data->GetEntity())) {
    //        XDMFWriteArrayT(dst, p);
    //    } else if (auto p = std::dynamic_pointer_cast<Array<unsigned int>>(data->GetEntity())) {
    //        XDMFWriteArrayT(dst, p);
    //    }
    return 1;
}
auto XDMFAttributeInsertOne(std::shared_ptr<data::DataNode> const& attr_desc,
                            std::shared_ptr<data::DataNode> const& data) {
    auto attr = XdmfAttribute::New();
    attr->setName(attr_desc->GetValue<std::string>("Name"));
    auto iform = attr_desc->GetValue<int>("IFORM");
    switch (iform) {
        case NODE:
            attr->setCenter(XdmfAttributeCenter::Node());
            break;
        case CELL:
            attr->setCenter(XdmfAttributeCenter::Cell());
            break;
        case EDGE:
            attr->setCenter(XdmfAttributeCenter::Edge());
            break;
        case FACE:
            attr->setCenter(XdmfAttributeCenter::Face());
            break;
        default:
            UNIMPLEMENTED;
            break;
    }
    size_type rank = 0;
    size_type extents[MAX_NDIMS_OF_ARRAY];
    if (auto p = attr_desc->Get("DOF")) {
        if (auto dof = std::dynamic_pointer_cast<DataLightT<int>>(p->GetEntity())) {
            switch (dof->value()) {
                case 1:
                    attr->setType(XdmfAttributeType::Scalar());
                    break;
                case 3:
                    attr->setType(XdmfAttributeType::Vector());
                    break;
                case 6:
                    attr->setType(XdmfAttributeType::Tensor6());
                    break;
                case 9:
                    attr->setType(XdmfAttributeType::Tensor());
                    break;
                default:
                    attr->setType(XdmfAttributeType::Matrix());
                    break;
            }
            rank = dof->value() > 1 ? 1 : 0;
            extents[0] = static_cast<size_type>(dof->value());
        } else if (auto dof = std::dynamic_pointer_cast<DataLightT<int*>>(p->GetEntity())) {
            attr->setType(XdmfAttributeType::Matrix());
            rank = dof->extents(extents);
        }
    } else {
        attr->setType(XdmfAttributeType::Scalar());
    }

    if (iform == NODE || iform == CELL) {
        XDMFWriteArray(attr.get(), data);
    } else {
        //        VERBOSE << std::setw(20) << "Write XDMF : "
        //                << "Ignore EDGE/FACE center attribute \"" << s_name << "\".";
    }
    return attr;
}

boost::shared_ptr<XdmfCurvilinearGrid> XDMFCurvilinearGridNew(std::shared_ptr<DataNode> const& chart,
                                                              std::shared_ptr<data::DataNode> const& blk,
                                                              std::shared_ptr<data::DataNode> const& coord) {
    auto lo = blk->GetValue<nTuple<index_type, 3>>("LowIndex");
    auto hi = blk->GetValue<nTuple<index_type, 3>>("HighIndex");

    VERBOSE << lo << "~" << hi;
    nTuple<unsigned int, 3> dims{0, 0, 0};
    dims = hi - lo + 1;
    unsigned int num = dims[0] * dims[1] * dims[2];
    auto grid = XdmfCurvilinearGrid::New(dims[0], dims[1], dims[2]);
    auto geo = XdmfGeometry::New();
    geo->setType(XdmfGeometryType::XYZ());
    nTuple<Real, 3> origin = chart->GetValue<nTuple<Real, 3>>("Origin");
    origin += lo * chart->GetValue<nTuple<Real, 3>>("Scale");
    geo->setOrigin(origin[0], origin[1], origin[2]);
    XDMFWriteArray(geo.get(), coord);
    grid->setGeometry(geo);

    auto level_info = XdmfInformation::New();
    level_info->setKey("Level");
    level_info->setValue(std::to_string(blk->GetValue<int>("Level", 0)));
    grid->insert(level_info);
    grid->setName(std::to_string(blk->GetValue<id_type>("GUID")));
    return grid;
}
boost::shared_ptr<XdmfRegularGrid> XDMFRegularGridNew(std::shared_ptr<DataNode> const& chart,
                                                      std::shared_ptr<data::DataNode> const& blk) {
    auto x0 = chart->GetValue<nTuple<Real, 3>>("Origin");
    auto dx = chart->GetValue<nTuple<Real, 3>>("Scale");
    auto lo = blk->GetValue<index_tuple>("LowIndex");
    auto hi = blk->GetValue<index_tuple>("HighIndex");

    nTuple<unsigned int, 3> dims{1, 1, 1};
    dims = hi - lo + 1;
    nTuple<Real, 3> origin{0, 0, 0};
    origin = lo * dx + x0;
    auto grid = XdmfRegularGrid::New(dx[0], dx[1], dx[2], dims[0], dims[1], dims[2], origin[0], origin[1], origin[2]);
    auto level_info = XdmfInformation::New();
    level_info->setKey("Level");
    level_info->setValue(std::to_string(blk->GetValue<int>("Level", 0)));
    grid->insert(level_info);
    grid->setName(std::to_string(blk->GetValue<id_type>("GUID")));
    return grid;
}
int XDMFDump(std::string const& url, std::shared_ptr<DataNode> const& obj) {
    if (obj == nullptr || url.empty()) { return SP_FAILED; }
    VERBOSE << std::setw(20) << "Write XDMF : " << url;

    int success = SP_FAILED;
    auto grid_collection = XdmfGridCollection::New();
    grid_collection->setType(XdmfGridCollectionType::Spatial());

    if (auto time = obj->Get("Time")) {
        if (auto t = std::dynamic_pointer_cast<DataLightT<Real>>(time->GetEntity())) {
            grid_collection->setTime(XdmfTime::New(t->value()));
        }
    }

    auto attrs = obj->Get("Attributes");

    auto patches = obj->Get("Patches");

    ASSERT(attrs != nullptr && patches != nullptr);

    if (auto atlas = obj->Get("Atlas")) {
        auto chart = atlas->Get("Chart");
        auto blks = atlas->Get("Blocks");

        blks->Foreach([&](std::string const& k, std::shared_ptr<data::DataNode> const& blk) {
            if (auto patch = patches->Get(k)) {
                if (patch->Get("_COORDINATES_") != nullptr) {
                    auto g = XDMFCurvilinearGridNew(chart, blk, patch->Get("_COORDINATES_"));
                    patch->Foreach([&](std::string const& s, std::shared_ptr<data::DataNode> const& d) {
                        g->insert(XDMFAttributeInsertOne(attrs->Get(s), d));
                    });
                    grid_collection->insert(g);

                } else {
                    auto g = XDMFRegularGridNew(chart, blk);
                    patch->Foreach([&](std::string const& s, std::shared_ptr<data::DataNode> const& d) {
                        g->insert(XDMFAttributeInsertOne(attrs->Get(s), d));
                    });
                    grid_collection->insert(g);
                }
            }
            return 1;
        });
    }
    auto domain = XdmfDomain::New();
    domain->insert(grid_collection);
    if (auto writer = XdmfWriter::New(url)) {
        domain->accept(writer);
        success = SP_SUCCESS;
    }
    return success;
}

}  // namespace data{
}  // namespace simpla