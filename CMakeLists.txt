cmake_minimum_required (VERSION 3.10)
project (mtsFBGSensor VERSION 0.0.0)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

include_directories (include)

# Variable Setting for ease of use
set (REQUIRED_CISST_LIBRARIES 
    cisstCommon
    cisstVector
    cisstOSAbstraction
    cisstMultiTask
    cisstParameterTypes
)

set(mtsFBGSensor_SOURCE_FILES
    # hyperion interrogator library
    code/hCommLibrary.cpp
    code/hLibrary.cpp
    code/HyperionInterrogator.cpp

    # cisst MultiTask FBGSensor
    code/mtsFBGSensor.cpp
    code/Interrogator.cpp
)

set(mtsFBGSensor_HEADER_FILES
    # hyperion interrogator library
    include/hyperion/hCommLibrary.h
    include/hyperion/hLibrary.h
    include/hyperion/HyperionInterrogator.h
    
    # cisst MultiTask FBGSensor
    include/mtsFBGSensor/mtsFBGSensor.h
    include/mtsFBGSensor/Interrogator.h
)

find_package (cisst 1.2.0 REQUIRED ${REQUIRED_CISST_LIBRARIES})

if (cisst_FOUND_AS_REQUIRED)

    # load cisst configuration
    include (${CISST_USE_FILE})

    if (NOT CISST_HAS_JSON)
        message( FATAL_ERROR "mtsFBGSensor requires cisst to be built with JSON" )
    endif ()

    # catkin/ROS paths
    cisst_set_output_path ()
    

    # add all config files for this component
    cisst_add_config_files (mtsFBGSensor)
    
    add_library (mtsFBGSensor 
        ${mtsFBGSensor_HEADER_FILES}
        ${mtsFBGSensor_SOURCE_FILES}
    )

    set_target_properties (mtsFBGSensor PROPERTIES
                            VERSION ${mtsFBGSensor_VERSION}
                            FOLDER "mtsFBGSensor`"
    )
    target_link_libraries (mtsFBGSensor
        ${mtsFBGSensor_INCLUDE_DIR}                  
    )
    cisst_target_link_libraries (mtsFBGSensor ${REQUIRED_CISST_LIBRARIES})

    # Install target for headers and library
    install (
        DIRECTORY "${mtsFBGSensor_SOURCE_DIR}/include/mtsFBGSensor"
        DESTINATION include
        COMPONENT mtsFBGSensor-dev
        PATTERN .svn EXCLUDE
    )

    install (
        TARGETS mtsFBGSensor 
        COMPONENT mtsFBGSensor
            RUNTIME DESTINATION bin
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib
    )

else (cisst_FOUND_AS_REQUIRED)
    message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
endif (cisst_FOUND_AS_REQUIRED)