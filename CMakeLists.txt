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
target_include_directories(${PROJECT_NAME} PUBLIC ${INCLUDES})
target_compile_definitions(${PROJECT_NAME} PRIVATE NAP_SHARED_LIBRARY)
target_compile_definitions(${PROJECT_NAME} PRIVATE MODULE_NAME=${PROJECT_NAME})

# link with external libs
target_link_libraries(${PROJECT_NAME} napcore ${DEPENDENT_NAP_MODULES} debug ${REALSENSE_LIBRARIES_DEBUG} optimized ${REALSENSE_LIBRARIES_RELEASE})

# Deploy module.json as MODULENAME.json alongside module post-build
copy_module_json_to_bin()

if(WIN32)
    copy_realsense_dll()
endif()

# Package into platform release
if(UNIX)
    set(LINUX_EXTRA_RPATH ../../../../thirdparty/librealsense/bin)
endif()
package_module()

# Package libartnet into platform release
# Install docs
install(FILES ${REALSENSE_DIR}/LICENSE DESTINATION thirdparty/librealsense)
#install(FILES ${THIRDPARTY_DIR}/librealsense/source/COPYING DESTINATION thirdparty/librealsense)
#install(FILES ${THIRDPARTY_DIR}/librealsense/source/README DESTINATION thirdparty/librealsense)

# install includes
install(DIRECTORY ${REALSENSE_INCLUDE_DIR}/ DESTINATION thirdparty/librealsense/include)

# install libraries
if(UNIX AND NOT APPLE)
    install(FILES ${REALSENSE_LIBRARIES_RELEASE} DESTINATION thirdparty/librealsense/bin)
else()
    install(FILES $<TARGET_FILE:realsense> DESTINATION thirdparty/librealsense/bin)
endif()

# set realsense install name
if(WIN32)
    install(FILES $<TARGET_FILE_DIR:realsense>/realsense2.lib DESTINATION thirdparty/librealsense/bin)
endif()

