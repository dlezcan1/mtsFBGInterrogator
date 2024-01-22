cmake_minimum_required (VERSION 3.10)
project (mts_fbg_sensor VERSION 0.0.0)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

include_directories (include)

# Variable Setting for ease of use
set (REQUIRED_CISST_LIBRARIES 
    cisstCommon
    cisstVector
    cisstOSAbstraction
    cisstNumerical
    cisstMultiTask
    cisstParameterTypes
)

set( catkin_REQUIRED_COMPONENTS 
  roscpp
  rospy
  std_msgs
  geometry_msgs
  cisst_msgs
  cisst_ros_bridge
)

set(mtsFBGSensor_SOURCE_FILES
    # hyperion interrogator library
    code/hCommLibrary.cpp
    code/hLibrary.cpp
    code/HyperionInterrogator.cpp

    # cisst MultiTask FBGTools
    code/GreenDualTool.cpp
    code/CannulationTool.cpp
    code/ThreeDOFTool.cpp
    code/mtsFBGTool.cpp

    # filter/math library
    code/SensorOneEuroFilter.cpp
    code/BernsteinPolynomial.cpp

    # cisst MultiTask FBGSensor
    code/mtsFBGSensor.cpp
    code/Interrogator.cpp
)

set(mtsFBGSensor_HEADER_FILES
    # hyperion interrogator library
    include/mtsFBGSensor/hyperion/hCommLibrary.h
    include/mtsFBGSensor/hyperion/hLibrary.h
    include/mtsFBGSensor/hyperion/HyperionInterrogator.h

    # cisst MultiTask FBGTools
    include/mtsFBGSensor/mtsFBGTool/FBGToolFactory.h
    include/mtsFBGSensor/mtsFBGTool/FBGToolInterface.h
    include/mtsFBGSensor/mtsFBGTool/GreenDualTool.h
    include/mtsFBGSensor/mtsFBGTool/CannulationTool.h
    include/mtsFBGSensor/mtsFBGTool/ThreeDOFTool.h
    include/mtsFBGSensor/mtsFBGTool/mtsFBGTool.h

    # filter/math library
    include/mtsFBGSensor/SensorFilters/SensorOneEuroFilter.h
    include/mtsFBGSensor/mtsFBGTool/UtilMath/BernsteinPolynomial.h

    # cisst MultiTask FBGSensor
    include/mtsFBGSensor/mtsFBGSensor/mtsFBGSensor.h
    include/mtsFBGSensor/mtsFBGSensor/Interrogator.h
)

# find packages
find_package (cisst 1.2.0 REQUIRED ${REQUIRED_CISST_LIBRARIES})
find_package(catkin REQUIRED COMPONENTS
  ${catkin_REQUIRED_COMPONENTS}
)

if (NOT cisst_FOUND_AS_REQUIRED)
    message ("Information: code in ${CMAKE_CURRENT_SOURCE_DIR} will not be compiled, it requires ${REQUIRED_CISST_LIBRARIES}")
    return ()
endif ()

# load cisst configuration
include (${CISST_USE_FILE})

if (NOT CISST_HAS_JSON)
    message( FATAL_ERROR "mtsFBGSensor requires cisst to be built with JSON" )
endif ()

# catkin/ROS paths
cisst_set_output_path ()
message (${catkin_INCLUDE_DIRS})
include_directories (${catkin_INCLUDE_DIRS})
catkin_package(
    INCLUDE_DIRS include
    CATKIN_DEPENDS ${catkin_REQUIRED_COMPONENTS}
)

# add all config files for this component
cisst_add_config_files (mtsFBGSensor)

add_library (mtsFBGSensor
    ${mtsFBGSensor_SOURCE_FILES}
)
set_target_properties (mtsFBGSensor PROPERTIES
    VERSION ${mts_fbg_sensor_VERSION}
    FOLDER "mtsFBGSensor"
    PUBLIC_HEADER "${mtsFBGSensor_HEADER_FILES}"
)
target_include_directories(mtsFBGSensor PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/include
    )
cisst_target_link_libraries (mtsFBGSensor ${REQUIRED_CISST_LIBRARIES})

# executable
add_executable(fbg_force_tool code/main.cpp)
target_link_libraries(fbg_force_tool mtsFBGSensor ${catkin_LIBRARIES})
cisst_target_link_libraries(fbg_force_tool ${REQUIRED_CISST_LIBRARIES})

# Install target for headers and library
install (
    DIRECTORY "${mts_fbg_sensor_SOURCE_DIR}/include"
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

install(TARGETS mtsFBGSensor # fbg_force_tool
    LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
    RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
    PUBLIC_HEADER DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}
)

install (DIRECTORY include/
    DESTINATION ${CATKIN_GLOBAL_INCLUDE_DESTINATION}
    PATTERN .svn EXCLUDE
)
