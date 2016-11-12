#ifndef BRICK_SHAREDENTITY_HPP
#define BRICK_SHAREDENTITY_HPP

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

            Size count() const
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

        ~SharedEntityT()
        {
            invalidate();
        }

        void invalidate()
        {
            if (m_refCount)
            {
                m_refCount->decrement();
                if (m_refCount->count() == 0)
                {
                    
                }
            }
        }


    private:

        using EntityBase::invalidate;
        using EntityBase::destroy;
        RefCounter * m_refCount;
    };
}

#endif //BRICK_SHAREDENTITY_HPP
