cmake_minimum_required(VERSION 3.15) 
cmake_policy(SET CMP0048 NEW)
PROJECT(ProcGensu VERSION 0.0.2 LANGUAGES CXX C)

find_package(OpenGL REQUIRED)

set(CMAKE_BUILD_TYPE RelWithDebInfo)

set(GLFW_SOURCE_DIR "external/glfw")
set(GLM_SOURCE_DIR "external/glm")
set(GLEW_SOURCE_DIR "external/glew")


# Building only the GLFW lib
set(BUILD_SHARED_LIBS OFF CACHE BOOL "")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "")
set(GLFW_BUILD_TESTS OFF CACHE BOOL "")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "")
set(GLFW_INSTALL OFF CACHE BOOL "")

add_subdirectory(${GLFW_SOURCE_DIR} ${GLEW_SOURCE_DIR})
include_directories(${GLFW_SOURCE_DIR}/include
                    ${GLFW_SOURCE_DIR}/deps
                    ${GLM_SOURCE_DIR}
                    ${GLEW_SOURCE_DIR}/include
                    src/)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_library(glew STATIC
  ${GLEW_SOURCE_DIR}/src/glew.c
  ${GLEW_SOURCE_DIR}/include
)

target_link_libraries(glew glfw ${GLEW_LIBRARY} ${OPENGL_LIBRARIES})

add_definitions(
  -DGLEW_STATIC
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

FILE(GLOB SRCFILES src/*.cpp)
FILE(GLOB HEADERFILES src/*.h)
set(APP_NAME "dual-contouring")
if (APPLE)
    if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)
        add_executable(${APP_NAME} MACOSX_BUNDLE src/main.cpp icon.icns src/utils.cpp src/utils.h)
        set_source_files_properties("icon.icns" PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icon.icns)
    else ()

        add_executable(${APP_NAME} MACOSX_BUNDLE ${SRCFILES})
    endif ()

    set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules/app.plist.in)

    set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME})
    set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0")
    set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_LONG_VERSION_STRING "1.0")
    set_target_properties(${APP_NAME} PROPERTIES MACOSX_BUNDLE_INFO_STRING "Dual contouring OpenGL stuff")
else()
    add_executable(${APP_NAME} ${SRCFILES} ${HEADERFILES})
endif()

set_property(TARGET ${APP_NAME} PROPERTY CXX_STANDARD 17)
set_property(TARGET ${APP_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
#set_property(TARGET ${APP_NAME} PROPERTY WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")

target_link_libraries(${APP_NAME} glfw ${GLFW_LIBRARIES} glew)


