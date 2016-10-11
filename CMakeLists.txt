cmake_minimum_required (VERSION 2.8.11)
project (Brick)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions")

include_directories (${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include ${CMAKE_INSTALL_PREFIX}/include)
link_directories(/usr/local/lib)

set (BRICKDEPS Stick)

set (BRICKINC 
Brick/Component.hpp
Brick/Entity.hpp
Brick/EntityID.hpp
Brick/Hub.hpp
Brick/TypedEntity.hpp
)

set (BRICKSRC
Brick/Entity.cpp
Brick/Hub.cpp
Brick/TypedEntity.cpp
)

add_library (Brick SHARED ${BRICKSRC})
add_library (BrickStatic STATIC ${BRICKSRC})
target_link_libraries(Brick ${BRICKDEPS})
target_link_libraries(BrickStatic ${BRICKDEPS})
install (TARGETS Brick BrickStatic DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install (FILES ${BRICKINC} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Brick)
add_subdirectory (Tests)
