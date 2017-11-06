#ifndef BRICK_HUB_HPP
#define BRICK_HUB_HPP

#include <Stick/DynamicArray.hpp>
#include <Stick/UniquePtr.hpp>
#include <Stick/Maybe.hpp>
#include <Brick/EntityID.hpp>

#include <type_traits>
#include <algorithm>
#include <bitset>

namespace brick
{
    class Entity;

    //@TODO: Add some way to reserve memory/storage for a certain number of entities/components?
    class Hub
    {
        friend class Entity;

        typedef stick::DynamicArray<stick::Size> FreeList;
        typedef stick::DynamicArray<stick::Size> HandleVersionArray;
        typedef std::bitset<64> ComponentBitset;
        typedef stick::DynamicArray<ComponentBitset> ComponentBitsetArray;

        struct FreeListAccessor
        {
            FreeListAccessor()
            {}

            FreeListAccessor(Hub * _hub) :
                m_hub(_hub)
            {}

            FreeList & freeList()
            {
                return m_hub->m_freeList;
            }

            Hub * m_hub;
        };

        struct FreeListAccessorConst
        {
            FreeListAccessorConst()
            {}

            FreeListAccessorConst(const Hub * _hub) :
                m_list(_hub->m_freeList)
            {}

            FreeList & freeList()
            {
                return m_list;
            }

            FreeList m_list;
        };

    public:

        template<bool IsConst, bool All = true>
        class EntityIterator
        {
        public:

            typedef typename std::conditional<IsConst, const Hub *, Hub *>::type HubPtr;
            typedef typename std::conditional<IsConst, const Entity, Entity>::type EntityType;
            typedef typename std::conditional<IsConst, FreeListAccessorConst, FreeListAccessor>::type FreeListAccessorType;


            EntityIterator();

            EntityIterator(HubPtr _hub, stick::Size _current, const ComponentBitset & _mask = ComponentBitset(0));

            bool operator == (const EntityIterator & _other) const;

            bool operator != (const EntityIterator & _other) const;

            EntityIterator & operator--();

            EntityIterator operator--(int);

            EntityIterator & operator-=(stick::Size _i);

            EntityIterator operator-(stick::Size _i) const;

            EntityIterator & operator++();

            EntityIterator operator++(int);

            EntityIterator & operator+=(stick::Size _i);

            EntityIterator operator+(stick::Size _i) const;

            void increment();

            void decrement();

            inline EntityType operator * () const;

        private:

            bool isValidEntity();

            HubPtr m_hub;
            stick::Size m_current;
            stick::Size m_freeListIndex;
            ComponentBitset m_mask;
            FreeListAccessorType m_freeListAccessor;
        };

        typedef EntityIterator<false, true> Iter;
        typedef EntityIterator<true, true> ConstIter;

        template<class...C>
        class TypedEntityRange
        {
        public:

            typedef EntityIterator<false, false> Iter;
            typedef EntityIterator<true, false> ConstIter;

            TypedEntityRange(Hub * _hub) :
                m_hub(_hub)
            {

            }

            Iter begin()
            {
                return Iter(m_hub, 0, m_hub->template componentMask<C...>());
            }

            ConstIter begin() const
            {
                return ConstIter(m_hub, 0, m_hub->template componentMask<C...>());
            }

            Iter end()
            {
                return Iter(m_hub, m_hub->m_nextEntityID, m_hub->template componentMask<C...>());
            }

            ConstIter end() const
            {
                return ConstIter(m_hub, m_hub->m_nextEntityID, m_hub->template componentMask<C...>());
            }

        private:

            Hub * m_hub;
        };


        Hub(stick::Allocator & _allocator = stick::defaultAllocator());

        ~Hub();

        Entity createEntity();

        template<class...Components>
        void reserve(stick::Size _count);

        Iter begin()
        {
            return Iter(this, 0);
        }

        Iter end()
        {
            return Iter(this, m_nextEntityID);
        }

        ConstIter begin() const
        {
            return ConstIter(this, 0);
        }

        ConstIter end() const
        {
            return ConstIter(this, m_nextEntityID);
        }

        template<class...C>
        TypedEntityRange<C...> view()
        {
            return TypedEntityRange<C...>(this);
        }

        template<class...C>
        const TypedEntityRange<C...> view() const
        {
            return TypedEntityRange<C...>(this);
        }

        stick::Size entityCount() const;

        stick::Allocator & allocator() const;

    private:

        Entity createNextEntity();

        void destroyEntity(const Entity & _entity);

        bool isValid(EntityID _id, stick::Size _version) const;

        template<class Component>
        bool cloneComponentImpl(EntityID _from, EntityID _to);

        template<class Component>
        bool reserveComponentImpl(stick::Size _count);


        template<class T, class ... Args>
        void setComponent(EntityID _id, Args ..._args)
        {
            using ValueType = typename T::ValueType;
            stick::Size cid = componentID<T>();

            if (m_componentStorage.count() <= cid)
            {
                m_componentStorage.resize(cid + 1);
            }
            auto & storage = m_componentStorage[cid];
            if (!storage)
            {
                createStorageForComponentID<ValueType>(cid);
            }
            (*storage).componentArray<ValueType>()[_id] = (ValueType) {std::forward<Args>(_args)...};
            m_componentBitsets[_id][cid] = true;
        }

        template<class VT>
        void createStorageForComponentID(stick::Size _cid)
        {
            ComponentStorage * storage = m_alloc->create<ComponentStorageT<VT>>(*m_alloc);
            storage->resize(m_nextEntityID);
            m_componentStorage[_cid] = stick::UniquePtr<ComponentStorage>(storage, *m_alloc);
        }

        template<class T>
        void removeComponent(EntityID _id)
        {
            stick::Size cid = componentID<T>();
            if (m_componentStorage.count() > cid)
            {
                m_componentStorage[cid]->componentArray<typename T::ValueType>()[_id].reset();
                m_componentBitsets[_id][cid] = false;
            }
        }

        template<class T>
        stick::Maybe<typename T::ValueType &> component(EntityID _id)
        {
            using ValueType = typename T::ValueType;
            stick::Size cid = componentID<T>();
            if (m_componentStorage.count() <= cid)
                return stick::Maybe<ValueType &>();
            auto & storage = m_componentStorage[cid];
            if (!storage)
                return stick::Maybe<ValueType &>();

            //this is kind of ugly right now as we need to do the lookup
            //twice to ensure we return a valid reference
            if ((*storage).componentArray<ValueType>()[_id])
                return *(*storage).componentArray<ValueType>()[_id];

            return stick::Maybe<ValueType &>();
        }

        template<class T>
        stick::Maybe<const typename T::ValueType &> component(EntityID _id) const
        {
            using ValueType = typename T::ValueType;
            stick::Size cid = componentID<T>();
            if (m_componentStorage.count() <= cid)
                return stick::Maybe<const ValueType &>();
            auto & storage = m_componentStorage[cid];
            if (!storage)
                return stick::Maybe<const ValueType &>();

            //this is kind of ugly right now as we need to do the lookup
            //twice to ensure we return a valid reference
            if ((*storage).componentArray<ValueType>()[_id])
                return *(*storage).componentArray<ValueType>()[_id];

            return stick::Maybe<const ValueType &>();
        }

        template<class T>
        bool hasComponent(EntityID _id)
        {
            stick::Size cid = componentID<T>();
            return m_componentBitsets[_id][cid];
        }

        template <class C>
        bool contains(stick::Size _componentID) const
        {
            return componentID<C>() == _componentID;
        }

        template <class C1, class C2, class ... Components>
        bool contains(stick::Size _componentID) const
        {
            return contains<C1>(_componentID) || contains<C2, Components ...>(_componentID);
        }


        Entity clone(EntityID _id);

        template<class ... Components>
        Entity cloneWithout(EntityID _id);

        template<class ... Components>
        Entity cloneWith(EntityID _id);

        void cloneComponents(EntityID _from, EntityID _to);

        template<class ... Components>
        void cloneComponents(EntityID _from, EntityID _to);

        template<class ... Components>
        void cloneComponentsWithout(EntityID _from, EntityID _to);

        template<class T>
        stick::Size componentID() const
        {
            //TODO: find a solution that does not rely on
            //static to make sure component ids are hub specific
            //and thread safe.
            static stick::Size id = s_nextComponentID++;
            return id;
        }

        template <class C>
        ComponentBitset componentMask() const
        {
            ComponentBitset mask;
            mask.set(componentID<C>());
            return mask;
        }

        template <class C1, class C2, class ... Components>
        ComponentBitset componentMask() const
        {
            return componentMask<C1>() | componentMask<C2, Components ...>();
        }

        struct ComponentStorage
        {
            ComponentStorage(void * _array) :
                arrayPtr(_array)
            {

            }

            virtual ~ComponentStorage()
            {
            }

            template<class T>
            stick::DynamicArray<stick::Maybe<T>> & componentArray()
            {
                return *reinterpret_cast<stick::DynamicArray<stick::Maybe<T>>*>(arrayPtr);
            }

            virtual void cloneComponent(stick::Size _from, stick::Size _to) = 0;

            virtual void resize(stick::Size _s) = 0;

            virtual void reserve(stick::Size _s) = 0;

            virtual void resetComponent(stick::Size _index) = 0;

            void * arrayPtr;
        };

        template<class T>
        struct IsCopyConstructible
        {
            static constexpr bool Value = std::is_copy_constructible<T>::value;
        };

        template<class T>
        struct IsCopyConstructible<stick::DynamicArray<T> >
        {
            static constexpr bool Value = std::is_copy_constructible<T>::value;
        };

        template<class T, class Enable = void>
        struct ComponentStorageT : public ComponentStorage
        {
            typedef stick::Maybe<T> MaybeType;
            typedef stick::DynamicArray<MaybeType> DynamicArrayType;

            ComponentStorageT(stick::Allocator & _alloc) :
                ComponentStorage(_alloc.create<DynamicArrayType>(_alloc)),
                m_alloc(&_alloc)
            {
            }

            ~ComponentStorageT()
            {
                m_alloc->destroy(&componentArray<T>());
            }

            void cloneComponent(stick::Size _from, stick::Size _to)
            {
                auto & ca = componentArray<T>();
                if (ca[_from])
                {
                    ca[_to] = *ca[_from];
                }
            }

            void resize(stick::Size _s)
            {
                componentArray<T>().resize(_s);
            }

            void reserve(stick::Size _s)
            {
                componentArray<T>().reserve(_s);
            }

            void resetComponent(stick::Size _index)
            {
                // reset the mabye!
                STICK_ASSERT(_index < componentArray<T>().count());
                componentArray<T>()[_index].reset();
            }

            stick::Allocator * m_alloc;
        };

        template<class T>
        struct ComponentStorageT < T, typename std::enable_if < !IsCopyConstructible<T>::Value >::type > : public ComponentStorage
        {
            typedef stick::Maybe<T> MaybeType;
            typedef stick::DynamicArray<MaybeType> DynamicArrayType;

            ComponentStorageT(stick::Allocator & _alloc) :
                ComponentStorage(_alloc.create<DynamicArrayType>(_alloc)),
                m_alloc(&_alloc)
            {
            }

            ~ComponentStorageT()
            {
                m_alloc->destroy(&componentArray<T>());
            }

            //@TODO: BIG N FAT
            void cloneComponent(stick::Size _from, stick::Size _to)
            {
                //Warning? Fail?
            }

            void resize(stick::Size _s)
            {
                componentArray<T>().resize(_s);
            }

            void reserve(stick::Size _s)
            {
                componentArray<T>().reserve(_s);
            }

            void resetComponent(stick::Size _index)
            {
                // reset the mabye!
                STICK_ASSERT(_index < componentArray<T>().count());
                componentArray<T>()[_index].reset();
            }

            stick::Allocator * m_alloc;
        };

        stick::Allocator * m_alloc;
        stick::DynamicArray<stick::UniquePtr<ComponentStorage>> m_componentStorage;
        ComponentBitsetArray m_componentBitsets;
        FreeList m_freeList;
        HandleVersionArray m_handleVersions;
        EntityID m_nextEntityID;
        static stick::Size s_nextComponentID;
    };
}

#include <Brick/Entity.hpp>

namespace brick
{
    template<bool IC, bool A>
    Hub::EntityIterator<IC, A>::EntityIterator() :
        m_hub(nullptr),
        m_current(-1),
        m_freeListIndex(-1)
    {
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A>::EntityIterator(HubPtr _hub, stick::Size _current, const ComponentBitset & _mask) :
        m_hub(_hub),
        m_current(_current),
        m_freeListIndex(0),
        m_mask(_mask),
        m_freeListAccessor(_hub)
    {
        std::sort(m_freeListAccessor.freeList().begin(), m_freeListAccessor.freeList().end());
    }

    template<bool IC, bool A>
    bool Hub::EntityIterator<IC, A>::operator == (const EntityIterator & _other) const
    {
        return m_current == _other.m_current;
    }

    template<bool IC, bool A>
    bool Hub::EntityIterator<IC, A>::operator != (const EntityIterator & _other) const
    {
        return m_current != _other.m_current;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> & Hub::EntityIterator<IC, A>::operator--()
    {
        decrement();
        return *this;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> Hub::EntityIterator<IC, A>::operator--(int)
    {
        EntityIterator ret = *this;
        decrement();
        return ret;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> & Hub::EntityIterator<IC, A>::operator-=(stick::Size _i)
    {
        for (stick::Size i = 0; i <= _i; ++i)
            decrement();
        return *this;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> Hub::EntityIterator<IC, A>::operator-(stick::Size _i) const
    {
        EntityIterator ret = *this;
        for (stick::Size i = 0; i <= _i; ++i)
            ret.decrement();
        return ret;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> & Hub::EntityIterator<IC, A>::operator++()
    {
        increment();
        return *this;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> Hub::EntityIterator<IC, A>::operator++(int)
    {
        EntityIterator ret = *this;
        increment();
        return ret;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> & Hub::EntityIterator<IC, A>::operator+=(stick::Size _i)
    {
        for (stick::Size i = 0; i <= _i; ++i)
            increment();
        return *this;
    }

    template<bool IC, bool A>
    Hub::EntityIterator<IC, A> Hub::EntityIterator<IC, A>::operator+(stick::Size _i) const
    {
        EntityIterator ret = *this;
        for (stick::Size i = 0; i <= _i; ++i)
            ret.increment();
        return ret;
    }

    template<bool IC, bool A>
    void Hub::EntityIterator<IC, A>::increment()
    {
        STICK_ASSERT(m_hub);
        do
        {
            ++m_current;
        }
        while (m_current < m_hub->m_nextEntityID && !isValidEntity());
    }

    template<bool IC, bool A>
    void Hub::EntityIterator<IC, A>::decrement()
    {
        STICK_ASSERT(m_hub);
        do
        {
            --m_current;
        }
        while (m_current > 0 && !isValidEntity());
    }

    template<bool IC, bool A>
    typename Hub::EntityIterator<IC, A>::EntityType Hub::EntityIterator<IC, A>::operator * () const
    {
        return EntityType(const_cast<Hub *>(m_hub), m_current, m_hub->m_handleVersions[m_current]);
    }

    template<bool IC, bool A>
    bool Hub::EntityIterator<IC, A>::isValidEntity()
    {
        if (A)
        {
            if (m_freeListIndex < m_freeListAccessor.freeList().count() && m_freeListAccessor.freeList()[m_freeListIndex] == m_current)
            {
                m_freeListIndex++;
                return false;
            }
            return true;
        }
        else
        {
            return (m_hub->m_componentBitsets[m_current] & m_mask) == m_mask;
        }
    }

    template<class ... Components>
    Entity Hub::cloneWithout(EntityID _id)
    {
        Entity ret = createEntity();
        cloneComponentsWithout<Components...>(_id, ret.m_id);
        return ret;
    }

    template<class ... Components>
    Entity Hub::cloneWith(EntityID _id)
    {
        Entity ret = createEntity();
        cloneComponents<Components...>(_id, ret.m_id);
        return ret;
    }

    template<class Component>
    bool Hub::cloneComponentImpl(EntityID _from, EntityID _to)
    {
        stick::Size cid = componentID<Component>();
        if (cid < m_componentStorage.count())
        {
            auto & ptr = m_componentStorage[cid];
            if (ptr && m_componentBitsets[_from][cid])
            {
                ptr->cloneComponent(_from, _to);
                m_componentBitsets[_to][cid] = true;
                return true;
            }
        }
        return false;
    }

    template<class ... Components>
    void Hub::cloneComponents(EntityID _from, EntityID _to)
    {
        auto list = {cloneComponentImpl<Components>(_from, _to)...};
    }

    //@TODO: Here we are still iterating over all existing components...
    //maybe add a way to know which components are set on a certain entity
    //and only iterate over those instead?
    template<class ... Components>
    void Hub::cloneComponentsWithout(EntityID _from, EntityID _to)
    {
        for (stick::Size i = 0; i < m_componentStorage.count(); ++i)
        {
            auto & ptr = m_componentStorage[i];
            if (ptr && !contains<Components...>(i) && m_componentBitsets[_from][i])
            {
                ptr->cloneComponent(_from, _to);
                m_componentBitsets[_to][i] = true;
            }
        }
    }

    template<class Component>
    bool Hub::reserveComponentImpl(stick::Size _count)
    {
        using ValueType = typename Component::ValueType;
        stick::Size cid = componentID<Component>();
        if (m_componentStorage.count() <= cid)
        {
            m_componentStorage.resize(cid + 1);
        }
        auto & storage = m_componentStorage[cid];
        if (!storage)
        {
            createStorageForComponentID<ValueType>(cid);
        }
        (*storage).reserve(_count);
        return true;
    }

    template<class...Components>
    void Hub::reserve(stick::Size _count)
    {
        //first, we reserve _count entity handles
        stick::Size c = _count - m_freeList.count();
        for (stick::Size i = 0; i < c; ++i)
        {
            Entity e = createNextEntity();
            m_freeList.append(e.m_id);
        }

        //reserve the components
        auto list = {reserveComponentImpl<Components>(_count)...};
    }
}

#endif //BRICK_HUB_HPP
