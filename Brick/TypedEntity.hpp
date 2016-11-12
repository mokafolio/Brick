#ifndef BRICK_TYPEDENTITY_HPP
#define BRICK_TYPEDENTITY_HPP

#include <Brick/Entity.hpp>
#include <Brick/Component.hpp>
#include <Stick/TypeInfo.hpp>

namespace brick
{
    namespace detail
    {
        using EntityTypeHolder = Component<ComponentName("EntityTypeHolder"), stick::TypeID>;
    }

    class STICK_API TypedEntity : public Entity
    {
    public:

        virtual ~TypedEntity() {};

        virtual stick::TypeID entityType() const = 0;
    };

    template<class T>
    class STICK_API TypedEntityT : public TypedEntity
    {
    public:

        stick::TypeID entityType() const
        {
            if (hasComponent<detail::EntityTypeHolder>())
                return get<detail::EntityTypeHolder>();
            return 0;
        }
    };

    template<class T>
    T entityCast(const Entity & _e)
    {
        if (_e.hasComponent<detail::EntityTypeHolder>() &&
                _e.get<detail::EntityTypeHolder>() == stick::TypeInfoT<T>::typeID())
        {
            T ret;
            ret.assignEntity(_e);
            return ret;
        }
        return T();
    }

    template<class T>
    T reinterpretEntity(const Entity & _e)
    {
        T ret;
        ret.assignEntity(_e);
        return ret;
    }

    template<class T>
    T createEntity(Hub & _h)
    {
        T ret;
        ret.assignEntity(_h.createEntity());
        ret.template set<detail::EntityTypeHolder>(stick::TypeInfoT<T>::typeID());
        return ret;
    }
}

#endif //BRICK_TYPEDENTITY_HPP
