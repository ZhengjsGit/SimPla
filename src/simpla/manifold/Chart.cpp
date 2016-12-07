//
// Created by salmon on 16-11-24.
//
#include "Chart.h"

#include <simpla/mesh/Attribute.h>
#include <simpla/mesh/MeshBlock.h>


namespace simpla
{
namespace mesh
{


Chart::Chart() {}

Chart::~Chart() {}

std::ostream &Chart::print(std::ostream &os, int indent) const
{
    os << std::setw(indent + 1) << " " << "Mesh = { ";
    os << "Type = \"" << get_class_name() << "\",";
    if (m_mesh_block_ != nullptr)
    {
        os << std::endl;
        os << std::setw(indent + 1) << " " << " Block = {";
        m_mesh_block_->print(os, indent + 1);
        os << std::setw(indent + 1) << " " << "},";
    }
    os << std::setw(indent + 1) << " " << "}," << std::endl;

    os << std::setw(indent + 1) << " " << "Attribute= { ";

    for (auto const &item:attributes())
    {
        os << "\"" << item->attribute()->db.get_value("name", std::string("unnamed")) << "\" , ";
    }

    os << std::setw(indent + 1) << " " << "} , " << std::endl;
};


bool Chart::is_a(std::type_info const &info) const { return typeid(Chart) == info; }


AttributeView *
Chart::connect(AttributeView *attr)
{
    m_attr_views_.insert(attr);
    return attr;
}

void Chart::disconnect(AttributeView *attr) { m_attr_views_.erase(attr); }

void Chart::initialize(Real data_time, Real dt)
{
    pre_process();
}

void Chart::finalize(Real data_time, Real dt)
{
    post_process();
}

void Chart::pre_process()
{
    ASSERT(m_mesh_block_ != nullptr);
    for (auto &item:m_attr_views_)
    {
        item->move_to(m_mesh_block_);
        item->pre_process();
    }
}

void Chart::post_process()
{
    for (auto &item:m_attr_views_) { item->post_process(); }
    m_mesh_block_.reset();
}

void Chart::move_to(std::shared_ptr<MeshBlock> const &m)
{
    post_process();
    m_mesh_block_ = m;
    pre_process();
};


std::set<AttributeView *> &Chart::attributes() { return m_attr_views_; };

std::set<AttributeView *> const &Chart::attributes() const { return m_attr_views_; };


}
}//namespace simpla {namespace mesh
