# Obstacle Bypass

This demo project performs a fully autonomous flight, with online obstacle avoidance algorithms, using image output from **Guidance SDK** to detect obstacles and implement avoidance strategy via control input to **Onboard SDK**.

## How To Run

1. Enter the directory of the desired strategy in Terminal: `cd balance_strategy` or `cd discontinuity_strategy`
2. Build the project: `catkin_make`
3. Execute project: `sudo ./devel/lib/dji_sdk/dji_sdk_node`
4. Press Enter to end autonomous flight

## Output Data

Images from the flight are displayed in real time with observed keypoints in blue, detected obstacles in red, and corresponding suggested maneuvers in green.
