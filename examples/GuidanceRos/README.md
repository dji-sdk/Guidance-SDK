# GudianceSDKROS
The official ROS package of Guidance SDK for 32/64 bit Ubuntu and XU3.

- We write the CMakeLists.txt file so that it automatically detects your operating system and choose the appropriate library file.
- We suppose the users are using USB for Guidance SDK on ROS. To use UART for Guidance SDK, plese reference [uart_example](https://github.com/dji-sdk/GuidanceSDK/tree/master/examples/uart_example).

# How to use
1. Setup USB devide rules so that no root privilege is required when using Guidance SDK via USB.
		
		sudo sh -c 'echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"18d1\", ATTR{idProduct}==\"d009\", MODE=\"0666\"" > /etc/udev/rules.d/51-guidance.rules'
2. Clone the repo to the catkin workspace source directory `catkin_ws/src` and then 
	
		cd ~/catkin_ws
		catkin_make
		rosrun GuidanceRos GuidanceNode
		rosrun GuidanceRos GuidanceNodeTest

# Documentation
To reduce the size of this package, we omit all documents. 

- For getting started, please refer to [Developer Guide](https://github.com/dji-sdk/GuidanceSDK/blob/master/doc/DeveloperGuide/DeveloperGuide.md).
- For detailed API documentation, please refer to [Guidance_SDK_API](https://github.com/dji-sdk/GuidanceSDK/blob/master/doc/Guidance_SDK_API.md).

