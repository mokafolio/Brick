#ifndef BRICK_SHAREDENTITY_HPP
#define BRICK_SHAREDENTITY_HPP

#include <Brick/TypedEntity.hpp>

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

    template<class EntityBase, class RefCounter = detail::SimpleRefCounter>
    class STICK_API SharedEntityT : public EntityBase
    {
    public:

        SharedEntityT() :
            m_refCount(nullptr)
        {

        }

        SharedEntityT(const SharedEntityT & _other) :
            m_refCount(_other.m_refCount),
            EntityBase(_other)
        {
            if (m_refCount)
                m_refCount->increment();
        }

        SharedEntityT(SharedEntityT && _other) :
            m_refCount(std::move(_other.m_refCount)),
            EntityBase(std::move(_other))
        {
            _other.m_refCount = nullptr;
        }

        ~SharedEntityT()
        {
            invalidate();
        }

        SharedEntityT & operator = (const SharedEntityT & _other)
        {
            m_refCount = _other.m_refCount;
            if (m_refCount)
                m_refCount->increment();
            EntityBase::operator = (_other);
            return *this;
        }

        SharedEntityT & operator = (SharedEntityT && _other)
        {
            m_refCount = std::move(_other.m_refCount);
            _other.m_refCount = nullptr;
            EntityBase::operator = (std::move(_other));
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
                    EntityBase::destroy();
                }
                m_refCount = nullptr;
            }
            EntityBase::invalidate();
        }

        stick::Size referenceCount() const
        {
            return m_refCount ? m_refCount->count() : 0;
        }

        void initialize(const EntityBase & _e, stick::Allocator & _alloc = stick::defaultAllocator())
        {
            invalidate();
            m_refCount = _alloc.create<RefCounter>();
            EntityBase::assignEntity(_e);
        }

    private:

        using EntityBase::invalidate;
        using EntityBase::destroy;
        RefCounter * m_refCount;
    };

    template<class EntityBase>
    SharedEntityT<EntityBase> createSharedEntity(Hub & _hub, stick::Allocator & _alloc = stick::defaultAllocator())
    {
         SharedEntityT<EntityBase> ret;
         ret.initialize(createEntity<EntityBase>(_hub), _alloc);
         return ret;
    }
}

#endif //BRICK_SHAREDENTITY_HPP
