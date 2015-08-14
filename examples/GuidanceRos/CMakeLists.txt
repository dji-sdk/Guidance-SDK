cmake_minimum_required(VERSION 2.8.3)
project(GuidanceRos)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  cv_bridge
  sensor_msgs
)
find_package(OpenCV REQUIRED)


###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  ${catkin_INCLUDE_DIRS}
)

include_directories(${PROJECT_SOURCE_DIR}/include)
link_directories(${PROJECT_SOURCE_DIR}/lib)
aux_source_directory(${PROJECT_SOURCE_DIR}/src PROJECT_SOURCE)
# uncomment the following line if you wish to generate executable in bin
#set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


add_executable(GuidanceNode ${PROJECT_SOURCE_DIR}/src/GuidanceNode.cpp ${PROJECT_SOURCE_DIR}/src/DJI_utility.cpp)
add_executable(GuidanceNodeTest ${PROJECT_SOURCE_DIR}/src/GuidanceNodeTest.cpp)

target_link_libraries(GuidanceNode DJI_guidance ${catkin_LIBRARIES} ${OpenCV_LIBS})
target_link_libraries(GuidanceNodeTest ${catkin_LIBRARIES} ${OpenCV_LIBS})
