cmake_minimum_required(VERSION 4.0.1)
project(MainApp)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



file(GLOB IMGUI_SRC
    "${CMAKE_SOURCE_DIR}/External/imgui/imgui.cpp"
    "${CMAKE_SOURCE_DIR}/External/imgui/imgui_draw.cpp"
    "${CMAKE_SOURCE_DIR}/External/imgui/imgui_tables.cpp"
    "${CMAKE_SOURCE_DIR}/External/imgui/imgui_widgets.cpp"
    "${CMAKE_SOURCE_DIR}/External/imgui/imgui_demo.cpp"  
)

file(GLOB IMGUI_BACKENDS
    "${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/External/imgui/backends/imgui_impl_vulkan.cpp"
)

include_directories(
    C:/msys64/mingw64/include
    C:/msys64/mingw64/include/freetype2
    C:/VulkanSDK/1.4.321.1/Include
    C:/msys64/mingw64/include/glm
    ${CMAKE_SOURCE_DIR}/External
    ${CMAKE_SOURCE_DIR}/Include
    ${CMAKE_SOURCE_DIR}/Include/Vulkan
    ${CMAKE_SOURCE_DIR}/External/imgui
    ${CMAKE_SOURCE_DIR}/External/imgui/backends

)

link_directories(C:/msys64/mingw64/lib C:/VulkanSDK/1.4.321.1/Lib)

file(GLOB ENGINE_SRC "${CMAKE_SOURCE_DIR}/Engine/*.cpp")


add_library(VulkanEngine SHARED
    ${ENGINE_SRC}
    ${IMGUI_SRC}
    ${IMGUI_BACKENDS}
)

target_link_libraries(VulkanEngine
    glfw3
    vulkan-1
    freetype
    LinearMath
    ws2_32
)

target_compile_definitions(VulkanEngine PRIVATE BUILD_ENGINE_DLL)

file(GLOB SOURCES "${CMAKE_SOURCE_DIR}/Test/*.cpp")

add_executable(main ${SOURCES})

target_link_libraries(main PRIVATE VulkanEngine)

add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/Assets $<TARGET_FILE_DIR:main>/Assets
)
add_custom_command(TARGET main POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:main>/shaders
)