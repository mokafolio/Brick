#ifndef BRICK_COMPONENT_HPP
#define BRICK_COMPONENT_HPP

#include <Stick/String.hpp>
#include <type_traits>

namespace brick
{
    template<class N, class T>
    class Component
    {
    public:

        using ValueType = T;

        static const stick::String & name()
        {
            return N::name();
        }
    };

    namespace detail
    {
        template<char... CHARS>
        struct ComponentNameHolder
        {
            static const stick::String & name()
            {
                static const stick::String str = stick::String::concat(CHARS...);
                return str;
            }
        };

        template< typename, char ... >
        struct ComponentNameBuilder;

        template< typename T >
        struct ComponentNameBuilder< T >
        {
            using Type = T;
        };

        template<stick::Size N, stick::Size M>
        constexpr char charAt( const char(&c)[ M ] ) noexcept
        {
            static_assert( M < 100, "Component name too long" );
            return ( N < M ) ? c[ N ] : 0;
        }

        template< template< char ... > class S, char ... Hs, char C, char ... Cs >
        struct ComponentNameBuilder< S< Hs ... >, C, Cs ... >
            : std::conditional< C == '\0',
              ComponentNameBuilder< S< Hs ... > >,
              ComponentNameBuilder< S< Hs ..., C >, Cs ... > >::type
        { };
    }
}


#define BRICK_COMPONENT_NAME_10(n,x) \
   brick::detail::charAt< n##0 >( x ), \
   brick::detail::charAt< n##1 >( x ), \
   brick::detail::charAt< n##2 >( x ), \
   brick::detail::charAt< n##3 >( x ), \
   brick::detail::charAt< n##4 >( x ), \
   brick::detail::charAt< n##5 >( x ), \
   brick::detail::charAt< n##6 >( x ), \
   brick::detail::charAt< n##7 >( x ), \
   brick::detail::charAt< n##8 >( x ), \
   brick::detail::charAt< n##9 >( x )

#define BRICK_COMPONENT_NAME_100(x) \
   BRICK_COMPONENT_NAME_10(,x), \
   BRICK_COMPONENT_NAME_10(1,x), \
   BRICK_COMPONENT_NAME_10(2,x), \
   BRICK_COMPONENT_NAME_10(3,x), \
   BRICK_COMPONENT_NAME_10(4,x), \
   BRICK_COMPONENT_NAME_10(5,x), \
   BRICK_COMPONENT_NAME_10(6,x), \
   BRICK_COMPONENT_NAME_10(7,x), \
   BRICK_COMPONENT_NAME_10(8,x), \
   BRICK_COMPONENT_NAME_10(9,x)

#define ComponentName(x) \
   brick::detail::ComponentNameBuilder< brick::detail::ComponentNameHolder<>, BRICK_COMPONENT_NAME_100(x) >::Type

#endif //BRICK_COMPONENT_HPP
