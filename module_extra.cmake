if(NOT TARGET realsense)
    find_package(realsense REQUIRED)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC ${REALSENSE_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} ${REALSENSE_LIBS})

if(WIN32)
    # Copy realsense DLL to build directory on Windows (plus into packaged app)
    copy_realsense_dll()
endif()

if(NAP_BUILD_CONTEXT MATCHES "framework_release")
    if(UNIX)
        # Install realsense lib into packaged app
        install(FILES $<TARGET_FILE:realsenselib> DESTINATION lib)
    endif()
endif()