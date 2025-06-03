cmake_minimum_required(VERSION 3.10)
project(EnhancedBouncingBall)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB SOURCES "src/*.cpp")
list(APPEND SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/texture.cpp)


get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
string(REPLACE " " "_" EXECUTABLE_NAME ${PROJECT_NAME})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

if(APPLE)
    # Uncomment and set GLFW_DIR if needed on Apple Silicon
    # set(GLFW_DIR "/opt/homebrew/Cellar/glfw/3.4")
endif()
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)


add_executable(${EXECUTABLE_NAME} ${SOURCES})

target_include_directories(${EXECUTABLE_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIR}
)

target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
    ${OPENGL_LIBRARIES}
    glfw
)

if(APPLE)
    target_link_libraries(${EXECUTABLE_NAME} PRIVATE 
        "-framework OpenGL" 
        "-framework Cocoa" 
        "-framework IOKit" 
        "-framework CoreVideo"
    )
endif()

file(GLOB SHADER_FILES "*.glsl")
foreach(SHADER_FILE ${SHADER_FILES})
    configure_file(${SHADER_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SHADER_FILE} COPYONLY)
endforeach()

message(STATUS "Building ${EXECUTABLE_NAME} with sources: ${SOURCES}")
message(STATUS "Shader files: ${SHADER_FILES}")
