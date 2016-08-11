#ifndef BRICK_ENTITY_HPP
#define BRICK_ENTITY_HPP

#include <Brick/EntityID.hpp>
#include <Stick/Maybe.hpp>

namespace brick
{
    class Hub;

    class Entity
    {
        friend class Hub;

    public:

        Entity();

        ~Entity();

        void invalidate();

        void destroy();

        bool isValid() const;

        template<class T>
        bool hasComponent() const;

        template<class T, class ...Args>
        void set(Args..._args);

        template<class T>
        void removeComponent();

        template<class T>
        stick::Maybe<typename T::ValueType &> maybe();

        template<class T>
        stick::Maybe<const typename T::ValueType &> maybe() const;

        template<class T>
        typename T::ValueType & get();

        template<class T>
        const typename T::ValueType & get() const;

        bool operator == (const Entity & _other) const;

        bool operator != (const Entity & _other) const;

        Entity clone() const;

        template<class ...Components>
        Entity cloneWithout() const;

        template<class ...Components>
        Entity cloneWith() const;

        void cloneComponents(const Entity & _from);

        template<class ...Components>
        void cloneComponents(const Entity & _from);

        template<class ...Components>
        void cloneComponentsWithout(const Entity & _from);

        void swap(Entity & _other)
        {
            std::swap(m_id, _other.m_id);
            std::swap(m_version, _other.m_version);
            std::swap(m_hub, _other.m_hub);
        }

    private:

        explicit Entity(Hub * _hub, EntityID _id, stick::Size _version);

        EntityID m_id;
        stick::Size m_version;
        Hub * m_hub;
    };
}

#include <Brick/Hub.hpp>

namespace brick
{
    template<class T, class ...Args>
    void Entity::set(Args..._args)
    {
        STICK_ASSERT(isValid());
        m_hub->setComponent<T>(m_id, std::forward<Args>(_args)...);
    }

    template<class T>
    void Entity::removeComponent()
    {
        STICK_ASSERT(isValid());
        m_hub->removeComponent<T>(m_id);
    }

    template<class T>
    bool Entity::hasComponent() const
    {
        STICK_ASSERT(isValid());
        return m_hub->hasComponent<T>(m_id);
    }

    template<class T>
    stick::Maybe<typename T::ValueType &> Entity::maybe()
    {
        STICK_ASSERT(isValid());
        return m_hub->component<T>(m_id);
    }

    template<class T>
    stick::Maybe<const typename T::ValueType &> Entity::maybe() const
    {
        STICK_ASSERT(isValid());
        return const_cast<const Hub *>(m_hub)->component<T>(m_id);
    }

    template<class T>
    typename T::ValueType & Entity::get()
    {
        return maybe<T>().value();
    }

    template<class T>
    const typename T::ValueType & Entity::get() const
    {
        return maybe<T>().value();
    }

    template<class ...Args>
    Entity Entity::cloneWithout() const
    {
        STICK_ASSERT(isValid());
        return m_hub->cloneWithout<Args...>(m_id);
    }

    template<class ...Args>
    Entity Entity::cloneWith() const
    {
        STICK_ASSERT(isValid());
        return m_hub->cloneWith<Args...>(m_id);
    }

    template<class ...Components>
    void Entity::cloneComponents(const Entity & _from)
    {
        STICK_ASSERT(isValid() && _from.isValid());
        return m_hub->cloneComponents<Components...>(_from.m_id, m_id);
    }

    template<class ...Components>
    void Entity::cloneComponentsWithout(const Entity & _from)
    {
        STICK_ASSERT(isValid() && _from.isValid());
        return m_hub->cloneComponentsWithout<Components...>(_from.m_id, m_id);
    }
}

#endif //BRICK_ENTITY_HPP
