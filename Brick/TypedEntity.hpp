#ifndef BRICK_TYPEDENTITY_HPP
#define BRICK_TYPEDENTITY_HPP

#include <Brick/SharedEntity.hpp>
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

        stick::TypeID entityType()
        {
            if (hasComponent<detail::EntityTypeHolder>())
                return get<detail::EntityTypeHolder>();
            return 0;
        }
    };

    template<class RefCounter = detail::SimpleRefCounter>
    class STICK_API SharedTypedEntityT : public SharedEntity<RefCounter>
    {
    public:
        virtual ~SharedTypedEntityT() {};

        stick::TypeID entityType()
        {
            if (SharedEntity<RefCounter>::template hasComponent<detail::EntityTypeHolder>())
                return SharedEntity<RefCounter>::template get<detail::EntityTypeHolder>();
            return 0;
        }
    };

    using SharedTypedEntity = SharedTypedEntityT<>;

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

    template<class T, class...Args>
    T createEntity(Hub & _h, Args..._args)
    {
        T ret;
        ret.assignEntity(_h.createEntity(), _args...);
        ret.template set<detail::EntityTypeHolder>(stick::TypeInfoT<T>::typeID());
        return ret;
    }
}

#endif //BRICK_TYPEDENTITY_HPP
