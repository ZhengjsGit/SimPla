//
// Created by salmon on 16-11-2.
//

#ifndef SIMPLA_DATABLOCK_H
#define SIMPLA_DATABLOCK_H

#include <simpla/SIMPLA_config.h>
#include <simpla/concept/LifeControllable.h>
#include <simpla/concept/Printable.h>
#include <simpla/concept/Serializable.h>
#include <simpla/toolbox/FancyStream.h>

namespace simpla {
namespace mesh {
/**
 *  Base class of Data Blocks (pure virtual)
 */
class MeshBlock;

class DataBlock : public concept::Serializable, public concept::Printable, public concept::LifeControllable {
    SP_OBJECT_BASE(DataBlock);

   public:
    DataBlock() {}

    virtual ~DataBlock() {}

    virtual std::type_info const &value_type_info() const = 0;

    virtual int entity_type() const = 0;

    virtual int dof() const = 0;

    virtual void Clear() = 0;

    virtual void *raw_data() = 0;
    /**
     * concept::Serializable
     *    virtual void load(data::DataTable const &) =0;
     *    virtual void save(data::DataTable *) const =0;
     *
     * concept::Printable
     *    virtual std::ostream &print(std::ostream &os, int indent) const =0;
     *
     * concept::LifeControllable
     *    virtual bool is_deployed() const =0;
     *    virtual bool is_valid() const =0;
     *    virtual void deploy()=0;
     *    virtual void PreProcess() =0;
     *    virtual void PostProcess()=0;
     *    virtual void destroy()=0;
     */
};

template <typename...>
class DataBlockAdapter;

template <typename U>
class DataBlockAdapter<U> : public DataBlock, public U {
    SP_OBJECT_HEAD(DataBlockAdapter<U>, DataBlock);
    typedef algebra::traits::value_type_t<U> value_type;

   public:
    template <typename... Args>
    explicit DataBlockAdapter(Args &&... args) : U(std::forward<Args>(args)...) {}

    ~DataBlockAdapter() {}

    virtual std::type_info const &value_type_info() const { return typeid(algebra::traits::value_type_t<U>); };

    virtual int entity_type() const { return algebra::traits::iform<U>::value; }

    virtual int dof() const { return algebra::traits::dof<U>::value; }

    virtual void Load(data::DataTable const &d){/* Load(*this, d); */};

    virtual void Save(data::DataTable *d) const {/* Save(*this, d); */};

    virtual std::ostream &Print(std::ostream &os, int indent) const {
        os << " type = \'" << value_type_info().name() << "\' "
           << ", entity type = " << (entity_type()) << ", dof = " << (dof()) << ", data_block = {";
        U::Print(os, indent + 1);
        os << "}";
        return os;
    }
    void *raw_data() { return nullptr; };

    static std::shared_ptr<DataBlock> Create(MeshBlock const *m, void *p) {
        return std::dynamic_pointer_cast<DataBlock>(std::make_shared<DataBlockAdapter<U>>());
    }

    virtual void Clear() { U::Clear(); }
    //    virtual std::shared_ptr<DataBlock> clone(std::shared_ptr<MeshBlock> const &m, void *p = nullptr)
    //    {
    //        return create(m, static_cast<value_type *>(p));
    //    };
    //
    //
    //    static std::shared_ptr<DataBlock>
    //    create(std::shared_ptr<MeshBlock> const &m, value_type *p = nullptr)
    //    {
    //        index_type n_dof = DOF;
    //        int ndims = 3;
    //        if (IFORM == EDGE || IFORM == FACE)
    //        {
    //            n_dof *= 3;
    //            ++ndims;
    //        }
    //        auto b = m->outer_index_box();
    //        index_type lo[4] = {std::get<0>(b)[0], std::get<0>(b)[1], std::Get<0>(b)[2], 0};
    //        index_type hi[4] = {std::get<1>(b)[0], std::get<1>(b)[1], std::Get<0>(b)[2], n_dof};
    //        return std::dynamic_pointer_cast<DataBlock>(std::make_shared<this_type>(p, ndims, lo, hi));
    //    };

    /**
     * concept::Serializable
     *    virtual void load(data::DataTable const &) =0;
     *    virtual void save(data::DataTable *) const =0;
     *
     * concept::Printable
     *    virtual std::ostream &print(std::ostream &os, int indent) const =0;
     *
     * concept::LifeControllable
     *    virtual bool is_deployed() const =0;
     *    virtual bool is_valid() const =0;
     *    virtual void deploy()=0;
     *    virtual void PreProcess() =0;
     *    virtual void PostProcess()=0;
     *    virtual void destroy()=0;
     */
    virtual void Deploy(){
        //        U::Deploy();
    };

    virtual void PreProcess(){
        //        U::update();
    };

    virtual void PostProcess() {
        //        U::update();
        base_type::PostProcess();
    };

    virtual void Destroy() {
        //        U::Destroy();
        base_type::Destroy();
    };
};

// template<typename V, int IFORM = VERTEX, int DOF = 1, bool SLOW_FIRST = false>
// using DataBlockArray=
// DataBlockAdapter<
//        Array < V,
//        SIMPLA_MAXIMUM_DIMENSION + (((IFORM == VERTEX || IFORM == VOLUME) && DOF == 1) ? 0 : 1), SLOW_FIRST>>;
//
// template<typename TV, int IFORM, int DOF = 1>
// class DataBlockArray : public DataBlock, public data::DataEntityNDArray<TV>
//{
// public:
//    typedef DataBlockArray<TV, IFORM, DOF> block_array_type;
//    typedef data::DataEntityNDArray<TV> data_entity_type;
//    typedef TV value_type;
//
// SP_OBJECT_HEAD(block_array_type, DataBlock);
//
//    template<typename ...Args>
//    explicit DataBlockArray(Args &&...args) : DataBlock(), data_entity_type(std::forward<Args>(args)...) {}
//
//    virtual ~DataBlockArray() {}
//
//    virtual bool is_valid() { return data_entity_type::isValid(); };
//
//    virtual std::type_info const &value_type_info() const { return typeid(value_type); };
//
//    virtual int entity_type() const { return IFORM; }
//
//    virtual int dof() const { return DOF; }
//
//    virtual void Load(data::DataTable const &) { UNIMPLEMENTED; };
//
//    virtual void Save(data::DataTable *) const { UNIMPLEMENTED; };
//
//    virtual std::ostream &Print(std::ostream &os, int indent) const
//    {
//        os << " type = \'" << value_type_info().name() << "\' "
//           << ", entity type = " << static_cast<int>(entity_type())
//           << ", data_block = {";
//        data_entity_type::Print(os, indent + 1);
//        os << "}";
//        return os;
//    }
//
//    virtual std::shared_ptr<DataBlock> clone(std::shared_ptr<MeshBlock> const &m, void *p = nullptr)
//    {
//        return create(m, static_cast<value_type *>(p));
//    };
//
//
//    static std::shared_ptr<DataBlock>
//    create(std::shared_ptr<MeshBlock> const &m, value_type *p = nullptr)
//    {
//        index_type n_dof = DOF;
//        int ndims = 3;
//        if (IFORM == EDGE || IFORM == FACE)
//        {
//            n_dof *= 3;
//            ++ndims;
//        }
//        auto b = m->outer_index_box();
//        index_type lo[4] = {std::get<0>(b)[0], std::get<0>(b)[1], std::Get<0>(b)[2], 0};
//        index_type hi[4] = {std::get<1>(b)[0], std::Get<1>(b)[1], std::get<0>(b)[2], n_dof};
//        return std::dynamic_pointer_cast<DataBlock>(std::make_shared<this_type>(p, ndims, lo, hi));
//    };
//
//    virtual void Deploy()
//    {
//        base_type::Deploy();
//        data_entity_type::Deploy();
//    };
//
//    virtual void PreProcess() { data_entity_type::update(); };
//
//    virtual void update() { data_entity_type::update(); };
//
//    virtual void Destroy()
//    {
//        data_entity_type::Destroy();
//        base_type::Destroy();
//    };
//
//    virtual void clear() { data_entity_type::clear(); }
//
//    virtual void Sync(std::shared_ptr<DataBlock>, bool only_ghost = true) { UNIMPLEMENTED; };
//
//
//    template<typename ...Args>
//    value_type &get(Args &&...args) { return data_entity_type::Get(std::forward<Args>(args)...); }
//
//    template<typename ...Args>
//    value_type const &Get(Args &&...args) const { return data_entity_type::get(std::forward<Args>(args)...); }
//
//
//    EntityIdRange Range() const
//    {
//        EntityIdRange res;
//        index_tuple lower, upper;
//        lower = data_entity_type::index_lower();
//        upper = data_entity_type::index_upper();
//        res.append(MeshEntityIdCoder::make_range(lower, upper, entity_type()));
//        return res;
//    }
//
// private:
//    index_tuple m_ghost_width_{{0, 0, 0}};
//};
}
}  // namespace simpla { namespace mesh

#endif  // SIMPLA_DATABLOCK_H
