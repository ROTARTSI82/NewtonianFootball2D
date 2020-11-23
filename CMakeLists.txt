cmake_minimum_required(VERSION 3.17)
project(Newtonian_Football_2D)

set(CMAKE_CXX_STANDARD 20)

add_executable(Newtonian_Football_2D src/main.cpp)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/sdl2)


# Add SDL2 library
find_package(SDL2 REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::Main)

# Add SDL2_image library
find_package(SDL2_image REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::Image)

# Add SDL2_ttf library
find_package(SDL2_ttf REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::TTF)

# Add SDL2_net library
find_package(SDL2_net REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::Net)

# Add SDL2_mixer library
find_package(SDL2_mixer REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::Mixer)

# Add SDL2_gfx library
find_package(SDL2_gfx REQUIRED)
target_link_libraries(Newtonian_Football_2D SDL2::GFX)

find_package(Bullet REQUIRED)
target_link_libraries(Newtonian_Football_2D ${BULLET_LIBRARIES})
target_include_directories(Newtonian_Football_2D PRIVATE ${BULLET_INCLUDE_DIRS})

target_include_directories(Newtonian_Football_2D PRIVATE ${SDL2_INCLUDE_DIRS})

target_include_directories(Newtonian_Football_2D PRIVATE src)
target_include_directories(Newtonian_Football_2D PRIVATE include)


add_subdirectory(dep/fmt)

target_link_libraries(Newtonian_Football_2D fmt)

target_include_directories(Newtonian_Football_2D PRIVATE dep/fmt/include)


add_subdirectory(dep/box2d)
target_link_libraries(Newtonian_Football_2D box2d)
target_include_directories(Newtonian_Football_2D PRIVATE dep/box2d/include)
