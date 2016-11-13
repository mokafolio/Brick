#ifndef BRICK_SHAREDENTITY_HPP
#define BRICK_SHAREDENTITY_HPP

#include <Brick/Entity.hpp>
#include <Brick/Component.hpp>

namespace brick
{
    namespace detail
    {
        class SimpleRefCounter
        {
        public:

            SimpleRefCounter() :
                m_count(1)
            {

            }

            void increment()
            {
                ++m_count;
            }

            void decrement()
            {
                --m_count;
            }

            stick::Size count() const
            {
                return m_count;
            }

        private:

            stick::Size m_count;
        };
    }

    template<class RefCounter = detail::SimpleRefCounter>
    class STICK_API SharedEntity : public Entity
    {
    public:

        using RefCounterComponent = Component<ComponentName("RefCounter"), RefCounter>;

        SharedEntity()
        {

        }

        SharedEntity(const SharedEntity & _other) :
            Entity(_other)
        {
            auto refCount = maybe<RefCounterComponent>();
            if (refCount)
            {
                (*refCount).increment();
            }
        }

        SharedEntity(SharedEntity && _other) :
            Entity(std::move(_other))
        {
            _other.Entity::invalidate();
        }

        ~SharedEntity()
        {
            if (isValid())
                invalidate();
        }

        SharedEntity & operator = (const SharedEntity & _other)
        {
            invalidate();
            assignEntity(_other);
            return *this;
        }

        // SharedEntity & operator = (SharedEntity && _other)
        // {
        //     invalidate();
        //     _other.Entity::invalidate();
        //     return *this;
        // }

        void invalidate()
        {
            if (!isValid()) return;
            auto refCount = maybe<RefCounterComponent>();
            if (refCount)
            {
                (*refCount).decrement();
                if ((*refCount).count() == 0)
                {
                    Entity::destroy();
                }
                else
                {
                    Entity::invalidate();
                }
            }
        }

        stick::Size referenceCount() const
        {
            auto refCount = maybe<RefCounterComponent>();
            return refCount ? (*refCount).count() : 0;
        }

        void assignEntity(const Entity & _e)
        {
            invalidate();
            Entity::assignEntity(_e);
            auto refCount = maybe<RefCounterComponent>();
            if (!refCount)
            {
                set<RefCounterComponent>(RefCounter());
            }
            else
            {
                const_cast<RefCounter &>(*refCount).increment();
            }
        }

    private:

        using Entity::invalidate;
        using Entity::destroy;
    };
}

#endif //BRICK_SHAREDENTITY_HPP
