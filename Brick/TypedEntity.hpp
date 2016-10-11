#ifndef BRICK_TYPEDENTITY_HPP
#define BRICK_TYPEDENTITY_HPP

#include <Brick/Entity.hpp>
#include <Stick/TypeInfo.hpp>

namespace brick
{
    class STICK_API TypedEntity : public Entity
    {
    public:

        void assignEntity(const Entity & _e);

        virtual stick::TypeID entityType() const = 0;
    };

    template<class T>
    class STICK_API TypedEntityT : public TypedEntity
    {
    public:

        stick::TypeID entityType() const
        {
            return stick::TypeInfoT<T>::typeID();
        }
    };

    template<class T>
    T entityCast(const TypedEntity & _e)
    {
        if(_e.entityType() == stick::TypeInfoT<T>::typeID())
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
        return ret;
    }
}

#endif //BRICK_TYPEDENTITY_HPP
