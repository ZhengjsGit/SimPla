//
// Created by salmon on 16-11-19.
//

#ifndef SIMPLA_GEOMETRY_H
#define SIMPLA_GEOMETRY_H

#include <simpla/toolbox/design_pattern/Observer.h>
#include <simpla/mesh/MeshBlock.h>
#include <simpla/mesh/Attribute.h>
#include "CoordinateFrame.h"

namespace simpla { namespace mesh
{


/**
 *  Define:
 *   A bundle is a triple $(E, p, B)$ where $E$, $B$ are sets and $p:E→B$ a map
 *   - $E$ is called the total space
 *   - $B$ is the base space of the bundle
 *   - $p$ is the projection
 *
 */
struct ChartBase
{
    ChartBase();

    virtual ~ChartBase();

    virtual bool is_a(std::type_info const &info) const;

    virtual void initialize();

    virtual void deploy();


    virtual void move_to(std::shared_ptr<MeshBlock> const &m);

    /**
     * @return current MeshBlock
     */
    virtual CoordinateFrame *coordinate_frame() =0;

    /**
     * @return current MeshBlock
     */
    virtual CoordinateFrame const *coordinate_frame() const =0;

    template<typename U>
    U const *mesh_as() const
    {
        ASSERT(coordinate_frame() != nullptr);
        ASSERT(coordinate_frame()->is_a(typeid(U)));
        return static_cast<U const *>(coordinate_frame());
    }

    /**
     * @param attributes
     */
    virtual std::shared_ptr<AttributeViewBase>
    connect(std::shared_ptr<AttributeViewBase> const &attr, std::string const &key = "");


    std::map<std::string, std::shared_ptr<AttributeViewBase> > &attributes();

    std::map<std::string, std::shared_ptr<AttributeViewBase> > const &attributes() const;

private:

    std::map<std::string, std::shared_ptr<AttributeViewBase> > m_attr_views_;
};

template<typename TG>
class Chart : public ChartBase
{
public:
    typedef TG coord_frame_type;

    Chart() : m_mesh_(this) {}

    template<typename ...Args>
    Chart(Args &&...args):m_mesh_(this, std::forward<Args>(args)...) {}

    virtual ~Chart() {}

    /**
     * @return current MeshBlock
     */
    coord_frame_type *coordinate_frame() { return &m_mesh_; };

    /**
     * @return current MeshBlock
     */
    coord_frame_type const *coordinate_frame() const { return &m_mesh_; };

    coord_frame_type const *mesh() const { return &m_mesh_; };

private:
    TG m_mesh_{this};
};

}}//namespace simpla { namespace mesh

#endif //SIMPLA_GEOMETRY_H
