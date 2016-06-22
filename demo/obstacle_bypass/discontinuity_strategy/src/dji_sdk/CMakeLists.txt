cmake_minimum_required(VERSION 2.8.3)
project(dji_sdk)

### GUIDANCE ###

set(CMAKE_CXX_FLAGS "-std=c++11 -g -Wall -I/home/dji/opencv_build/include -I/home/dji/opencv_src/modules/video/include -I/home/dji/opencv_src/modules/imgproc/include -I/home/dji/opencv_src/modules/highgui/include")

# add_definitions(-g -Wall)

set(CMAKE_SHARED_LINKER_FLAGS "-Wl,-rpath,./ -lpthread -lrt -L./ -L/usr/local/lib/ -L/home/dji/opencv_build/lib -lDJI_guidance -lusb-1.0 -lopencv_core -lopencv_highgui")

### ONBOARD ###

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
##  roscpp
  std_msgs
  cv_bridge
)

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################


## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
##generate_messages(
##   DEPENDENCIES
##   std_msgs
## )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS include
  LIBRARIES dji_sdk
##  CATKIN_DEPENDS roscpp std_msgs rospy
  CATKIN_DEPENDS std_msgs
  DEPENDS system_lib
)
 
###########
## Build ##  
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
# include_directories(include)

file(GLOB DJI_SDK_LIB_SOURCES src/sdk_lib/*.cpp)

##SET_SOURCE_FILES_PROPERTIES(${DJI_SDK_LIB_SOURCES} PROPERTIES LANGUAGE CXX)




include_directories(
  ${catkin_INCLUDE_DIRS}
  "${PROJECT_SOURCE_DIR}/include"
)

## Declare a cpp library
# add_library(dji_sdk
#   src/${PROJECT_NAME}/dji_sdk.cpp
# ) 

## Declare a cpp executable
add_executable(dji_sdk_node src/dji_sdk_node.cpp ${DJI_SDK_LIB_SOURCES})

## Add cmake target dependencies of the executable/library
## as an example, message headers may need to be generated before nodes
## add_dependencies(dji_sdk_node dji_sdk_generate_messages_cpp)

## Specify libraries to link a library or executable target against
target_link_libraries(dji_sdk_node
   ${catkin_LIBRARIES}
   ${PROJECT_SOURCE_DIR}/include/libDJI_guidance.so
   ${PROJECT_SOURCE_DIR}/include/DJI_guidance.h
   ${PROJECT_SOURCE_DIR}/include/DJI_utility.h
)
link_directories(/usr/local/lib)
