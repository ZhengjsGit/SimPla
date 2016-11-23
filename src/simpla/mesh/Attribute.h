//
// Created by salmon on 16-10-20.
//

#ifndef SIMPLA_ATTRIBUTE_H
#define SIMPLA_ATTRIBUTE_H

#include <simpla/SIMPLA_config.h>
#include <simpla/concept/Object.h>
#include <simpla/concept/Serializable.h>
#include <simpla/concept/Printable.h>
#include <simpla/concept/Configurable.h>
#include <simpla/toolbox/design_pattern/Observer.h>

#include "MeshBlock.h"
#include "DataBlock.h"

namespace simpla { namespace mesh
{

/**
 *  AttributeBase IS-A container of data blocks
 *  Define of attribute
 *  *  is printable
 *  *  is serializable
 *  *  is unware of mesh block
 *  *  is unware of mesh atlas
 *  *  has a value type
 *  *  has a MeshEntityType (entity_type)
 *  *  has n data block (insert, erase,has,at)
 *  *
 */
class AttributeBase :
        public Object,
        public concept::Printable,
        public concept::Serializable,
        public concept::Configurable
{


public:

    SP_OBJECT_HEAD(AttributeBase, Object)

    AttributeBase();

    AttributeBase(std::string const &s, std::string const &config_str = "");

    AttributeBase(AttributeBase const &) = delete;

    AttributeBase(AttributeBase &&) = delete;

    virtual ~AttributeBase();

    virtual MeshEntityType entity_type() const =0;

    virtual std::type_info const &value_type_info() const =0;

    virtual size_t dof() const { return 1; };

    virtual std::ostream &print(std::ostream &os, int indent = 1) const;

    virtual std::string name() const { return m_name_; };

    virtual void load(const data::DataBase &);

    virtual void save(data::DataBase *) const;

    virtual bool has(const id_type &) const;

    virtual void erase(const id_type &);

    virtual std::shared_ptr<DataBlock> const &at(const id_type &m) const;

    virtual std::shared_ptr<DataBlock> &at(const id_type &m);

    virtual std::shared_ptr<DataBlock> &get(id_type const &, std::shared_ptr<DataBlock> const &default_v = nullptr);

    virtual std::shared_ptr<DataBlock> insert_or_assign(const id_type &m,
                                                        const std::shared_ptr<DataBlock> &p);

    virtual std::shared_ptr<DataBlock> insert_or_assign(const std::shared_ptr<MeshBlock> &m,
                                                        const std::shared_ptr<DataBlock> &p = nullptr);

    virtual void register_data_block_factory(std::type_index idx,
                                             const std::function<std::shared_ptr<DataBlock>(
                                                     std::shared_ptr<MeshBlock> const &, void *)> &f);

private:
    std::string m_name_;
    struct pimpl_s;
    std::unique_ptr<pimpl_s> m_pimpl_;
};

template<typename TV, MeshEntityType IFORM, size_type IDOF>
class Attribute : public AttributeBase
{
public:
    static constexpr mesh::MeshEntityType iform = IFORM;
    static constexpr size_type DOF = IDOF;

    template<typename ...Args>
    Attribute(Args &&...args):AttributeBase(std::forward<Args>(args)...) {}

    virtual ~Attribute() {}

    virtual MeshEntityType entity_type() const { return IFORM; };

    virtual std::type_info const &value_type_info() const { return typeid(typename traits::value_type<TV>::type); };

    virtual size_type dof() const { return DOF; };

};

struct AttributeViewBase;

struct AttributeHolder;


struct AttributeViewBase : public design_pattern::Observer<void(id_type const &)>
{
    typedef design_pattern::Observer<void(id_type const &)> base_type;

    AttributeViewBase(std::shared_ptr<AttributeBase> const &attr = nullptr) : m_attr_(attr) {};

    AttributeViewBase(AttributeHolder *holder, std::shared_ptr<AttributeBase> const &attr)
            : m_attr_(attr) { connect(holder); }

    virtual ~AttributeViewBase() {}

    AttributeViewBase(AttributeViewBase const &other) = delete;

    AttributeViewBase(AttributeViewBase &&other) = delete;

    void notify(id_type const &m) { move_to(m); };

    id_type mesh_id() const { return m_id_; }

    std::shared_ptr<AttributeBase> &attribute() { return m_attr_; }

    std::shared_ptr<AttributeBase> const &attribute() const { return m_attr_; }

    DataBlock *data_block() { return m_data_holder_.get(); };

    DataBlock const *data_block() const { return m_data_holder_.get(); };

    template<typename U>
    U const *get_data() const
    {
        auto const *d = data_block();
        ASSERT(d != nullptr);
        ASSERT(d->is_a(typeid(U)));
        return static_cast<U const *>(d);
    }

    template<typename U>
    U *get_data()
    {
        auto *d = data_block();
        ASSERT(d != nullptr);
        ASSERT(d->is_a(typeid(U)));
        return static_cast<U *>(d);
    }

    virtual MeshEntityType entity_type() const =0;

    virtual std::type_info const &value_type_info() const =0;

    virtual size_type dof() const =0;

    virtual bool is_a(std::type_info const &t_info) const =0;

    virtual void move_to(std::shared_ptr<MeshBlock> const &m, std::shared_ptr<DataBlock> const &d = nullptr);

    virtual void move_to(id_type const &id);

    virtual void deploy();

    virtual void clear();

    virtual void destroy();

    virtual void register_data_block_factory(std::shared_ptr<AttributeBase> const &attr) const =0;

    void connect(AttributeHolder *w);


private:
    id_type m_id_;
    std::shared_ptr<DataBlock> m_data_holder_;
    std::shared_ptr<AttributeBase> m_attr_;

};

struct AttributeHolder : public design_pattern::Observable<void(id_type const &)>
{
    typedef design_pattern::Observable<void(id_type const &)> base_type;

    AttributeHolder() {}

    virtual ~AttributeHolder() {}

    bool
    connect(AttributeViewBase *view)
    {
        ASSERT(view != nullptr);

        base_type::connect(static_cast<observer_type *>(view));

        m_attr_holders_[view->attribute()->name()] = view->attribute();

        return true;
    }

    virtual void disconnect(AttributeViewBase *view)
    {
        if (view->attribute() != nullptr) { m_attr_holders_.erase(view->attribute()->name()); }

        base_type::disconnect(view);
    }

    virtual void move_to(id_type const &m) { notify(m); }

    virtual void remove(std::string const &key) { UNIMPLEMENTED; }

    virtual void foreach(std::function<void(AttributeViewBase const &)> const &fun) const
    {
        base_type::foreach([&](observer_type const &obj) { fun(static_cast< AttributeViewBase const &>(obj)); });
    };

    virtual void foreach(std::function<void(AttributeViewBase &)> const &fun)
    {
        base_type::foreach([&](observer_type &obj) { fun(static_cast< AttributeViewBase &>(obj)); });
    };

    virtual std::shared_ptr<DataBlock> get_data(std::string const &key, id_type const &m_id)
    {
        return m_attr_holders_.at(key)->at(m_id);
    };

    virtual bool has(std::string const &key) const { return m_attr_holders_.find(key) != m_attr_holders_.end(); }

    virtual std::shared_ptr<AttributeBase> at(std::string const &key)
    {
        return m_attr_holders_.at(key);
    }

private:
    std::map<std::string, std::shared_ptr<AttributeBase>> m_attr_holders_;
};

/**
 * AttributeView: expose one block of attribute
 * -) is a view of Attribute
 * -) is unaware of the type of Mesh
 * -) has a pointer to a mesh block
 * -) has a pointer to a data block
 * -) has a shared pointer of attribute
 * -) can traverse on the Attribute
 * -) if there is no Atlas, AttributeView will hold the MeshBlock
 * -) if there is no AttributeHolder, AttributeView will hold the DataBlock and Attribute
 */
template<typename TV, MeshEntityType IFORM, size_type IDOF = 1>
class AttributeView : public AttributeViewBase
{

protected:

    typedef AttributeView this_type;
    typedef Attribute<TV, IFORM, IDOF> attribute_type;

public:

    static constexpr MeshEntityType iform = IFORM;

    static constexpr size_type DOF = IDOF;


    AttributeView(std::shared_ptr<attribute_type> const &attr) :
            AttributeViewBase(attr != nullptr ? attr : std::dynamic_pointer_cast<AttributeBase>(attr)) {}

    AttributeView(std::string const &key, std::string const &cfg = "") :
            AttributeViewBase(std::make_shared<attribute_type>(key, cfg)) {};

    AttributeView(AttributeHolder *holder, std::string const &key, std::string const &cfg = "") :
            AttributeViewBase(holder->has(key) ? holder->at(key) : std::make_shared<attribute_type>(key, cfg))
    {
        holder->connect(this);
    };

//    template<typename ...Args>
//    explicit AttributeView(Args &&...args) :
//            AttributeViewBase(std::make_shared<attribute_type>(std::forward<Args>(args)...)) {};

    virtual ~AttributeView() {}

    AttributeView(AttributeView const &other) = delete;

    AttributeView(AttributeView &&other) = delete;

    virtual void register_data_block_factory(std::shared_ptr<AttributeBase> const &attr) const
    {
        attr->register_data_block_factory(
                std::type_index(typeid(MeshBlock)),
                [&](const std::shared_ptr<MeshBlock> &m, void *p)
                {
                    ASSERT(p != nullptr);
                    return m->template create_data_block<TV, IFORM, DOF>(p);
                });
    }

    virtual MeshEntityType entity_type() const { return IFORM; };

    virtual std::type_info const &value_type_info() const { return typeid(TV); };

    virtual size_type dof() const { return DOF; };

    virtual bool is_a(std::type_info const &t_info) const { return t_info == typeid(this_type); }
};


}} //namespace data_block
#endif //SIMPLA_ATTRIBUTE_H

