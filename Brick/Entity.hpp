#ifndef BRICK_ENTITY_HPP
#define BRICK_ENTITY_HPP

#include <Brick/EntityID.hpp>
#include <Stick/Maybe.hpp>

#include <functional>

namespace brick
{
    class Hub;

    class Entity
    {
        friend class Hub;

    public:

        using ApplyFunction = std::function<void(Entity)>;


        Entity();

        Entity(const Entity &) = default;

        Entity(Entity &&) = default;

        ~Entity();

        Entity & operator = (const Entity &) = default;

        Entity & operator = (Entity &&) = default;

        explicit operator bool() const;

        void invalidate();

        void destroy();

        bool isValid() const;

        template<class T>
        bool hasComponent() const;

        template<class T, class...Args>
        typename T::ValueType & ensureComponent(Args..._args);

        template<class T, class...Args>
        void set(Args..._args);

        template<class T, class...Args>
        void setAndApply(ApplyFunction _fn, Args..._args)
        {
            set<T>(_args...);
            _fn(*this);
        }

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

        template<class T>
        const typename T::ValueType & getOrDefault(const typename T::ValueType & _default = typename T::ValueType()) const;

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

        void assignEntity(const Entity & _e);

        EntityID id() const
        {
            return m_id;
        }

        stick::Size version() const
        {
            return m_version;
        }

        Hub * hub() const
        {
            return m_hub;
        }

    private:

        explicit Entity(Hub * _hub, EntityID _id, stick::Size _version);

        //@TODO: Make id and version 32 bit so the handle does not get tooo huuuuge?
        // moka, 06/23/2017
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

    template<class T, class...Args>
    typename T::ValueType & Entity::ensureComponent(Args..._args)
    {
        if(!hasComponent<T>())
        {
            set<T>(std::forward<Args>(_args)...);
        }
        return get<T>();
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
    const typename T::ValueType & Entity::getOrDefault(const typename T::ValueType & _default) const
    {
        auto m = maybe<T>();
        if (m) return *m;
        return _default;
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
