/**
 * @file MeshAttribute.h
 * @author salmon
 * @date 2016-05-19.
 */

#ifndef SIMPLA_MESHATTRIBUTE_H
#define SIMPLA_MESHATTRIBUTE_H

#include "../base/Object.h"
#include "Mesh.h"
#include "MeshEntity.h"
#include "MeshAtlas.h"
#include "../gtl/Log.h"
#include "../gtl/MemoryPool.h"

namespace simpla { namespace mesh
{
namespace tags
{
struct DENSE;
struct SPARSE;
struct BIT;
}


/**
 *  PlaceHolder class of MeshAttributeBase
 */
struct MeshAttributeBase : public base::Object
{
    SP_OBJECT_HEAD(MeshAttributeBase, base::Object)

public:

    MeshAttributeBase(MeshAtlas const &m) : m_atlas_(m) { }

    virtual ~MeshAttributeBase() { }

    MeshAttributeBase(MeshAttributeBase const &other) = delete;

    MeshAttributeBase(MeshAttributeBase &&other) = delete;

    MeshAttributeBase &operator=(MeshAttributeBase const &) = delete;

    void swap(MeshAttributeBase &other) = delete;

    virtual MeshEntityType entity_type() const = 0;

    virtual void apply(Visitor &vistor) { vistor.visit(*this); };

    virtual void apply(Visitor &vistor) const { vistor.visit(*this); };


    /** register MeshBlockId to attribute data collection.  */
    virtual bool add(MeshBlockId const &) = 0;

    /** remove MeshBlockId from attribute data collection.  */
    virtual void remove(MeshBlockId const &) = 0;

    MeshAtlas const &mesh_atlas() const { return m_atlas_; }

    struct View
    {
        View() { }

        View(MeshBlockId const &m_id) : m_id_(m_id) { }

        View(View const &other) : m_id_(other.m_id_) { }

        View(View &&other) : m_id_(other.m_id_) { }

        ~View() { };

        View &operator=(View const &other) = delete;

        virtual bool is_a(std::type_info const &t_info) const = 0;

        template<typename T> inline bool is_a() const { return (std::is_base_of<View, T>::value && is_a(typeid(T))); }

        virtual void swap(View &other) { std::swap(m_id_, other.m_id_); }

        virtual MeshEntityType entity_type() const = 0;

        virtual MeshEntityRange const &range() const = 0;

        virtual MeshEntityRange &range() = 0;

        MeshBlockId const &block_id() const { return m_id_; }

    private:
        MeshBlockId m_id_;

    };

    virtual std::shared_ptr<View> view_(MeshBlockId const &id) = 0;


protected:
    MeshAtlas const &m_atlas_;
};

/**
 *  Data attached to mesh entities.
 *
 *  @comment similar to MOAB::Tag
 *
 **/
template<typename ...> class MeshAttribute;

template<typename TV, typename TM, size_t IEntityType>
class MeshAttribute<TV, TM, index_const<IEntityType>, tags::DENSE>
        : public MeshAttributeBase
{
private:
    typedef MeshAttribute<TV, TM, index_const<IEntityType>, tags::DENSE> this_type;

    typedef MeshAttribute<TV, TM, index_const<IEntityType>, tags::DENSE> mesh_attribute_type;


public:
    typedef TV value_type;
    typedef TM mesh_type;

    //*************************************************
    // Object Head

private:
    typedef MeshAttributeBase base_type;
public:
    virtual bool is_a(std::type_info const &info) const { return typeid(this_type) == info || base_type::is_a(info); }

    virtual std::string get_class_name() const { return class_name(); }

    static std::string class_name()
    {
        return std::string("MeshAttribute<") +
               traits::type_id<TV, TM, index_const<IEntityType>>::name() + ">";
    }
    //*************************************************
public:

    class View;

    MeshAttribute(MeshAtlas const &m) : MeshAttributeBase(m) { };

    virtual  ~MeshAttribute() { };

    MeshAttribute(MeshAttribute const &other) = delete;

    MeshAttribute(MeshAttribute &&other) = delete;

    MeshAttribute &operator=(MeshAttribute const &) = delete;

    void swap(MeshAttribute &other) = delete;

    virtual MeshEntityType entity_type() const { return static_cast<MeshEntityType >(IEntityType); }


    /** register MeshBlockId to attribute data collection.  */
    virtual bool add(MeshBlockId const &m_id)
    {
        bool success = false;
        auto it = m_data_collection_.find(m_id);
        if (it == m_data_collection_.end())
        {
            if (!m_atlas_.has(m_id))
            {
                RUNTIME_ERROR << "Mesh is not regitered! [" << hash_value(m_id) << "]" << std::endl;
            }

            size_t m_size = m_atlas_.template at<mesh_type>(m_id)->size(entity_type());

            // @NOTE  !!! HERE allocate memory !!!
            std::tie(std::ignore, success) =
                    m_data_collection_.emplace(
                            std::make_pair(m_id,
                                           sp_alloc_array<value_type>(m_size)));
        }
        return success;
    };

    /** remove MeshBlockId from attribute data collection.  */
    virtual void remove(MeshBlockId const &m_id) { m_data_collection_.erase(m_id); };

private:

    typedef std::map<MeshBlockId, std::shared_ptr<value_type> > data_collection_type;
    data_collection_type m_data_collection_;

public:

    struct View : public MeshAttributeBase::View
    {

        typedef typename MeshAttributeBase::View base_type;
        typedef mesh_attribute_type host_type;

        View() { }

        View(MeshBlockId const &id, mesh_type const *m = nullptr, value_type *d = nullptr)
                : base_type(id), m_mesh_(m), m_data_(d),
                  m_range_(m->range(static_cast<MeshEntityType>(IEntityType))) { }

        View(View const &other) : base_type(other), m_mesh_(other.m_mesh_), m_data_(other.m_data_),
                                  m_range_(other.m_range_) { }

        View(View &&other) : base_type(other), m_mesh_(other.m_mesh_), m_data_(other.m_data_),
                             m_range_(other.m_range_) { }

        virtual  ~View() { }

        virtual void swap(MeshAttributeBase::View &other)
        {
            base_type::swap(other);
            auto &o_view = static_cast<View &>(other);
            std::swap(m_mesh_, o_view.m_mesh_);
            std::swap(m_data_, o_view.m_data_);
            m_range_.swap(o_view.m_range_);
        }

        virtual bool is_a(std::type_info const &t_info) const { return t_info == typeid(View); }

        virtual MeshEntityType entity_type() const { return static_cast<MeshEntityType >(IEntityType); }

        virtual MeshEntityRange const &range() const { return m_range_; }

        virtual MeshEntityRange &range() { return m_range_; }

        mesh_type const &mesh() const { return *m_mesh_; }

        inline value_type &get(MeshEntityId const &s) { return m_data_[m_mesh_->hash(s)]; }

        inline value_type const &get(MeshEntityId const &s) const { return m_data_[m_mesh_->hash(s)]; }

        inline value_type &operator[](MeshEntityId const &s) { return get(s); }

        inline value_type const &operator[](MeshEntityId const &s) const { return get(s); }

    private:
        mesh_type const *m_mesh_;
        value_type *m_data_;
        MeshEntityRange m_range_;

    };


    View view(MeshBlockId const &id)
    {
        MeshBlockId m_id = (id.is_nil()) ? m_atlas_.root() : id;

        add(m_id);
        return View(m_id, m_atlas_.template at<mesh_type>(m_id), m_data_collection_[m_id].get());

    }

    virtual std::shared_ptr<MeshAttributeBase::View> view_(MeshBlockId const &id)
    {
        MeshBlockId m_id = (id.is_nil()) ? m_atlas_.root() : id;

        add(m_id);

        return std::dynamic_pointer_cast<MeshAttributeBase::View>(
                std::make_shared<View>(m_id, m_atlas_.template at<mesh_type>(m_id),
                                       m_data_collection_[m_id].get()));
    }

}; // class MeshAttribute

}}//namespace simpla{namespace mesh{

#endif //SIMPLA_MESHATTRIBUTE_H
