cmake_minimum_required (VERSION 2.8.11)
project (Brick)
set(CMAKE_CXX_FLAGS "-std=c++11 -fno-exceptions -w")

option(BuildSubmodules "BuildSubmodules" OFF)
option(AddTests "AddTests" ON)

if(BuildSubmodules)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Stick)
else()
    include_directories(${CMAKE_CURRENT_SOURCE_DIR} /usr/local/include ${CMAKE_INSTALL_PREFIX}/include)
endif()

link_directories(/usr/local/lib)

set (BRICKDEPS Stick pthread)

set (BRICKINC 
Brick/Component.hpp
Brick/Entity.hpp
Brick/EntityID.hpp
Brick/Hub.hpp
Brick/SharedEntity.hpp
Brick/TypedEntity.hpp
)

set (BRICKSRC
Brick/Entity.cpp
Brick/Hub.cpp
)

if(BuildSubmodules)
    set(PrevAddTests ${AddTests})
    set(AddTests OFF)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Submodules/Stick)
    set(AddTests ${PrevAddTests})
endif()

add_library(Brick SHARED ${BRICKSRC})
add_library(BrickStatic STATIC ${BRICKSRC})
target_link_libraries(Brick ${BRICKDEPS})
target_link_libraries(BrickStatic ${BRICKDEPS})
install(TARGETS Brick BrickStatic DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(FILES ${BRICKINC} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/Brick)
if(AddTests)
    add_subdirectory(Tests)
endif()
