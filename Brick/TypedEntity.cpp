#include <Brick/TypedEntity.hpp>

namespace brick
{
    void TypedEntity::assignEntity(const Entity & _e)
    {
        static_cast<Entity *>(this)->operator=(_e);
    }
}
