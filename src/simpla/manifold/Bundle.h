//
// Created by salmon on 16-11-24.
//

#ifndef SIMPLA_FIBERBUNDLE_H
#define SIMPLA_FIBERBUNDLE_H

#include <simpla/toolbox/Log.h>
#include <simpla/mesh/MeshCommon.h>
#include <simpla/mesh/DataBlock.h>
#include <simpla/mesh/Attribute.h>

#include "Chart.h"

namespace simpla { namespace mesh
{
class Chart;


template<typename TV, MeshEntityType IFORM, size_type DOF = 1>
class Bundle : public AttributeView
{
private:
    typedef Bundle<TV, IFORM, DOF> this_type;
    typedef Attribute <TV, IFORM, DOF> attribute_type;

    typedef AttributeView base_type;


public:
    typedef TV value_type;

    using base_type::preprocess;
    using base_type::move_to;

    Bundle() : m_chart_(nullptr) {}

    template<typename ...Args>
    explicit Bundle(Chart *c, Args &&...args) :
            base_type(std::make_shared<attribute_type>(std::forward<Args>(args)...)), m_chart_(nullptr) { connect(c); };

    template<typename ...Args>
    explicit Bundle(std::shared_ptr<Chart> const &c, Args &&...args) :
            Bundle(c.get(), std::forward<Args>(args)...) { connect(c.get()); };


    template<typename ...Args>
    explicit Bundle(std::string const &key, Args &&...args) :
            base_type(std::make_shared<attribute_type>(key, std::forward<Args>(args)...)),
            m_chart_(nullptr) {};

    Bundle(this_type const &other) = delete;

    Bundle(this_type &&other) = delete;

    void deep_copy(this_type const &other)
    {
        preprocess();
        if (m_data_ != nullptr) { m_data_->deep_copy(*other.m_data_); }
    }

    virtual MeshEntityType entity_type() const { return IFORM; };

    virtual std::type_info const &value_type_info() const { return typeid(TV); };

    virtual size_type dof() const { return DOF; };

    virtual bool is_a(std::type_info const &t_info) const
    {
        return t_info == typeid(this_type) || base_type::is_a(t_info);
    }

    bool is_connected() const { return m_chart_ != nullptr; }

    void connect(Chart *c)
    {
        if (c != nullptr && c != m_chart_)
        {
            if (m_chart_ != nullptr) { disconnect(); }

            m_chart_ = c;

            m_chart_->connect(this);

            // For scratch data block
            if (this->attribute() == nullptr && m_chart_ != nullptr)
            {
                this->move_to(m_chart_->mesh_block());
            }

        }
    }

    void disconnect()
    {
        if (m_chart_ != nullptr)
        {
            m_chart_->disconnect(this);
            m_chart_ = nullptr;
        }
    }

    virtual ~Bundle() { disconnect(); }


    template<typename U> U const *mesh_as() { return m_chart_->mesh_as<U>(); }


    Chart const *get_chart() const { return m_chart_; }

    void set_chart(Chart *c) { m_chart_ = c; }


    typedef mesh::DataBlockArray<TV, IFORM, DOF> default_data_block_type;

    virtual std::shared_ptr<mesh::DataBlock>
    create_data_block(std::shared_ptr<mesh::MeshBlock> const &m, value_type *p = nullptr) const
    {

        return std::dynamic_pointer_cast<DataBlock>(default_data_block_type::create(m, p));
    };

    virtual std::shared_ptr<DataBlock>
    create_data_block(std::shared_ptr<MeshBlock> const &m, void *p) const
    {
        ASSERT(m != nullptr);
        return create_data_block(m, static_cast<value_type *>(p));
    };

    virtual value_type &
    get(MeshEntityId s) { return m_data_->get(MeshEntityIdCoder::unpack_index4(s, DOF)); }

    virtual value_type const &
    get(MeshEntityId s) const { return m_data_->get(MeshEntityIdCoder::unpack_index4(s, DOF)); }


    virtual value_type &
    get(index_type i, index_type j, index_type k = 0, index_type l = 0) { return m_data_->get(i, j, k, l); }

    virtual value_type const &
    get(index_type i, index_type j, index_type k = 0, index_type l = 0) const { return m_data_->get(i, j, k, l); }

    virtual void preprocess()
    {
        if (is_valid()) { return; }
        else
        {
            ASSERT(m_chart_ != nullptr);
            base_type::move_to(m_chart_->mesh_block());
            base_type::preprocess();
        }
        m_data_ = base_type::template data_as<default_data_block_type>();
    }

    virtual void postprocess()
    {
        if (!is_valid()) { return; } else { base_type::postprocess(); }
        m_data_ = nullptr;

    }

private:
    Chart *m_chart_ = nullptr;
    default_data_block_type *m_data_ = nullptr;


};
}} //namespace simpla {

#endif //SIMPLA_FIBERBUNDLE_H