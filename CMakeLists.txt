project(mod_naprealsense)

find_package(realsense REQUIRED)

# add all cpp files to SOURCES
file(GLOB SOURCES src/*.cpp)
file(GLOB HEADERS src/*.h)
source_group("headers" FILES ${HEADERS})
source_group("sources" FILES ${SOURCES})

set(INCLUDES src ${REALSENSE_INCLUDE_DIR})

# Get our NAP modules dependencies from module.json
module_json_to_cmake()

# compile shared lib as target
add_library(${PROJECT_NAME} SHARED ${SOURCES} ${HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER Modules)

# Remove lib prefix on Unix libraries
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")

# add include dirs
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${REALSENSE_LIBRARIES_DEBUG} optimized ${REALSENSE_LIBRARIES_RELEASE})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

# Package into platform release
if(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/realsense/lib)
endif()

# Package into platform release
package_module()
