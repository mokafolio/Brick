add_executable (BrickTests BrickTests.cpp)
target_link_libraries(BrickTests Brick ${BRICKDEPS})
add_custom_target(check COMMAND BrickTests)
