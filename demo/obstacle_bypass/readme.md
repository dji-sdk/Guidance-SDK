# Obstacle Bypass

This demo project performs a fully autonomous flight, with online obstacle avoidance algorithms, using image output from **Guidance SDK** to detect obstacles and implement avoidance strategy via control input to **Onboard SDK**. This project is build on **ROS**. It contains three different strategies, each working independently.

## How To Run

1. Install and configure ROS.
2. Put the directory of the desired strategy in catkin workspace.
3. Build the project: `catkin_make`
4. Execute project: `sudo ./devel/lib/dji_sdk/dji_sdk_node`
5. Press Enter to end autonomous flight

## Output Data

Images from the flight are displayed in real time with observed keypoints in blue, detected obstacles in red, and corresponding suggested maneuvers in green.
