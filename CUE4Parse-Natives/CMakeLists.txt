cmake_minimum_required(VERSION 3.10)
project(CUE4Parse-Natives CXX)

set(CMAKE_CXX_STANDARD 17)

# 设置运行时库为动态链接，确保与 .NET 8.0 兼容
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/ACL/external/acl/includes")
    set(WITH_ACL 1)
endif()

if (EXISTS "${PROJECT_SOURCE_DIR}/Oodle/external/oodle2/core")
    set(WITH_Oodle 1)
endif()

# ACL
if (WITH_ACL)
        add_compile_definitions(WITH_ACL)
        add_compile_definitions(WIN_EXPORT)
        include_directories("${PROJECT_SOURCE_DIR}/ACL/external/acl/includes")
        include_directories("${PROJECT_SOURCE_DIR}/ACL/external/acl/external/rtm/includes")
        set(SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/ACL/ACL.cpp)
endif()

# Oodle
if (WITH_Oodle)
        add_compile_definitions(OODLE_BUILDING_DLL)
        add_compile_definitions(WITH_Oodle)
        add_subdirectory("${PROJECT_SOURCE_DIR}/Oodle/")
endif()

set(SOURCES ${SOURCES} ${PROJECT_SOURCE_DIR}/Features.cpp)
include_directories("${PROJECT_SOURCE_DIR}/common")

add_library("${PROJECT_NAME}" SHARED ${SOURCES})
set_target_properties("${PROJECT_NAME}" PROPERTIES PREFIX "")

# Oodle
if (WITH_Oodle)
    target_link_libraries("${PROJECT_NAME}" PRIVATE oodle2)

    # Ensure all Oodle symbols are exported with the shared library
    if(WIN32)
        target_link_options("${PROJECT_NAME}" PRIVATE "/WHOLEARCHIVE:oodle2")
    elseif(APPLE)
        target_link_options("${PROJECT_NAME}" PRIVATE "-Wl,-force_load,$<TARGET_FILE:oodle2>")
    else() # Linux and other platforms
        target_link_options("${PROJECT_NAME}" PRIVATE "-Wl,--whole-archive" "$<TARGET_FILE:oodle2>" "-Wl,--no-whole-archive")
    endif()
endif()

if (WIN32)
    add_compile_definitions(WIN_EXPORT)
endif()

# 简化的复制命令 - 只复制 DLL，不处理 PDB
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/"
    COMMENT "Copying ${PROJECT_NAME} to bin/$<CONFIG>/"
)

# 添加 install 目标以兼容 CUE4Parse.csproj
install(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION "${PROJECT_SOURCE_DIR}/bin/$<CONFIG>")

# 只在Debug配置下复制PDB文件，并且只有当PDB文件存在时
if (MSVC)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Checking for PDB file in $<CONFIG> configuration"
        COMMAND $<$<CONFIG:Debug>:${CMAKE_COMMAND}> $<$<CONFIG:Debug>:-E> $<$<CONFIG:Debug>:copy_if_different> $<$<CONFIG:Debug>:$<TARGET_PDB_FILE:${PROJECT_NAME}>> $<$<CONFIG:Debug>:"${PROJECT_SOURCE_DIR}/bin/$<CONFIG>/">
        COMMENT "Copying PDB file to bin/$<CONFIG>/ (Debug only)"
    )
endif()
