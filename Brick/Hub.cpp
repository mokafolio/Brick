#include <Brick/Hub.hpp>
#include <Brick/Entity.hpp>

namespace brick
{
    using namespace stick;

    stick::Size Hub::s_nextComponentID = 0;

    Hub::Hub(Allocator & _allocator) :
        m_alloc(&_allocator),
        m_componentStorage(_allocator),
        m_componentBitsets(_allocator),
        m_freeList(_allocator),
        m_handleVersions(_allocator),
        m_nextEntityID(0)
    {

    }

    Hub::~Hub()
    {

    }

    Entity Hub::createEntity()
    {
        if (!m_freeList.count())
        {
            return createNextEntity();
        }
        else
        {
            EntityID id = m_freeList.last();
            m_freeList.removeLast();
            return Entity(this, id, m_handleVersions[id]);
        }
    }

    Entity Hub::createNextEntity()
    {
        EntityID id = m_nextEntityID++;
        for (auto & ptr : m_componentStorage)
        {
            if (ptr)
                ptr->resize(id + 1);
        }
        m_componentBitsets.append(ComponentBitset(0));
        m_handleVersions.append(0);
        return Entity(this, id, 0);
    }

    bool Hub::isValid(EntityID _id, Size _version) const
    {
        return _id < m_handleVersions.count() && m_handleVersions[_id] == _version;
    }

    void Hub::destroyEntity(const Entity & _entity)
    {
        m_freeList.append(_entity.m_id);
        //reset all the components of this entity
        for (auto & ptr : m_componentStorage)
        {
            if (ptr)
                ptr->resetComponent(_entity.m_id);
        }
        m_componentBitsets[_entity.m_id].reset();
        m_handleVersions[_entity.m_id]++;
    }

    Size Hub::entityCount() const
    {
        return m_nextEntityID - m_freeList.count();
    }

    Entity Hub::clone(EntityID _id)
    {
        Entity ret = createEntity();
        cloneComponents(_id, ret.m_id);
        return ret;
    }

    void Hub::cloneComponents(EntityID _from, EntityID _to)
    {
        for (stick::Size i = 0; i < m_componentStorage.count(); ++i)
        {
            auto & ptr = m_componentStorage[i];
            if (ptr)
            {
                ptr->cloneComponent(_from, _to);
                m_componentBitsets[_to][i] = true;
            }
        }
    }

    stick::Allocator & Hub::allocator() const
    {
        return m_componentStorage.allocator();
    }
}
