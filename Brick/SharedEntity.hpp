#ifndef BRICK_SHAREDENTITY_HPP
#define BRICK_SHAREDENTITY_HPP

#include <Brick/Entity.hpp>

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

        SharedEntity() :
            m_refCount(nullptr)
        {

        }

        SharedEntity(const SharedEntity & _other) :
            m_refCount(_other.m_refCount),
            Entity(_other)
        {
            if (m_refCount)
                m_refCount->increment();
        }

        SharedEntity(SharedEntity && _other) :
            m_refCount(std::move(_other.m_refCount)),
            Entity(std::move(_other))
        {
            _other.m_refCount = nullptr;
        }

        ~SharedEntity()
        {
            invalidate();
        }

        SharedEntity & operator = (const SharedEntity & _other)
        {
            m_refCount = _other.m_refCount;
            if (m_refCount)
                m_refCount->increment();
            Entity::operator = (_other);
            return *this;
        }

        SharedEntity & operator = (SharedEntity && _other)
        {
            m_refCount = std::move(_other.m_refCount);
            _other.m_refCount = nullptr;
            Entity::operator = (std::move(_other));
            return *this;
        }

        void invalidate()
        {
            if (m_refCount)
            {
                m_refCount->decrement();
                if (m_refCount->count() == 0)
                {
                    stick::destroy(m_refCount);
                    Entity::destroy();
                }
                m_refCount = nullptr;
            }
            Entity::invalidate();
        }

        stick::Size referenceCount() const
        {
            return m_refCount ? m_refCount->count() : 0;
        }

        void assignEntity(const Entity & _e, stick::Allocator & _alloc = stick::defaultAllocator())
        {
            invalidate();
            m_refCount = _alloc.create<RefCounter>();
            Entity::assignEntity(_e);
        }

    private:

        //using Entity::assignEntity;
        using Entity::invalidate;
        using Entity::destroy;

        RefCounter * m_refCount;
    };
}

#endif //BRICK_SHAREDENTITY_HPP
