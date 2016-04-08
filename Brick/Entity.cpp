#include <Brick/Entity.hpp>

namespace brick
{
    using namespace stick;

    Entity::Entity() :
        m_hub(nullptr),
        m_id(-1)
    {

    }

    Entity::Entity(Hub * _hub, EntityID _id, Size _version) :
        m_hub(_hub),
        m_id(_id),
        m_version(_version)
    {

    }

    Entity::~Entity()
    {

    }

    void Entity::invalidate()
    {
        m_hub = nullptr;
    }

    void Entity::destroy()
    {
        STICK_ASSERT(m_hub);
        m_hub->destroyEntity(*this);
        invalidate();
    }

    bool Entity::isValid() const
    {
        if (!m_hub)
            return false;
        return m_hub->isValid(m_id, m_version);
    }

    bool Entity::operator == (const Entity & _other) const
    {
        return m_id == _other.m_id && m_version == _other.m_version;
    }

    bool Entity::operator != (const Entity & _other) const
    {
        return !(*this == _other);
    }

    Entity Entity::clone() const
    {
        STICK_ASSERT(isValid());
        return m_hub->clone(m_id);
    }

    void Entity::cloneComponents(const Entity & _from)
    {
        STICK_ASSERT(isValid() && _from.isValid());
        return m_hub->cloneComponents(_from.m_id, m_id);
    }
}
