/**
 * @file object.cpp
 * @author salmon
 * @date 2015-12-16.
 */
#include <iomanip>
#include <ostream>
#include "Object.h"

namespace simpla { namespace base
{
Object::Object() : m_click_(0) { };

Object::Object(Object &&other) : m_click_(other.m_click_) { };

Object::Object(Object const &) : m_click_(0) { };

Object &Object::operator=(Object const &other)
{
    Object(other).swap(*this);
    return *this;
};

Object::~Object() { }

void Object::swap(Object &other) { std::swap(m_click_, other.m_click_); };

bool Object::is_a(std::type_info const &info) const { return typeid(Object) == info; }

std::string Object::get_class_name() const { return "GeoObject"; }

bool Object::is_same(Object const &other) const { return this == &other; }


std::ostream &Object::print(std::ostream &os, int indent) const
{
    os << std::setw(indent) << this->get_class_name() << "= {";
    os << std::setw(indent) << "}," << std::endl;

    return os;
}

}}//namespace simpla { namespace base

