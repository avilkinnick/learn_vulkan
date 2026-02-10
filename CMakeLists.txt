cmake_minimum_required(VERSION 3.23)

project(learn_vulkan LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Vulkan COMPONENTS volk REQUIRED)

add_executable(learn_vulkan)

target_sources(learn_vulkan
    PRIVATE
        src/main.c
)

target_link_libraries(learn_vulkan
    PRIVATE
        Vulkan::Vulkan
        Vulkan::volk
)

if(MSVC)
    target_compile_options(learn_vulkan PRIVATE /W3)
else()
    target_compile_options(learn_vulkan PRIVATE -Wall -Wextra -Wpedantic)
endif()
