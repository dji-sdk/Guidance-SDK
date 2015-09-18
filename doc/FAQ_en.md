# FAQ

## What information is provided by Guidance SDK? 
Guidance SDK includes :

-	**demo**: a visual tracking project by using Guidance SDK
-	**doc**: API details
-	**examples**: examples for USB, UART and ROS
-	**include**: Header file of Guidance SDK 
-	**lib**: Library files for Windows
-	**so**: Library files for Linux

Guidance API gives the method for users to get `IMU data`, `Velocity`, `Obstacle distance`,  `grey-scale image`, `depth image` and `ultrasonic data` with `USB` and `UART` interface.


## Where can I apply the Guidance SDK?
One can use Guidance SDK on Guidance Core, which is separated from other hardware platforms. 
Since Guidance SDK provides the Library files for different operating systems, one can develop applications in Linux and Windows platforms, 32 bit and 64 bit versions. 


## How to use SDK to get data from Guidance Core?
You need to set up your development environment first referring to `Developer Guide` provided, which includes driver installation and project property setting. 
Then connect Guidance Core to PC and power it on, run the demo and examples to see what happens.


## Why can't I see Guidance device in "Device Manager"?
Firstly, make sure Guidance Core is powered on. If you connect Guidance Core to PC by USB, then check if `Guidance Assistant` software has been successfully installed. 
If you connect Guidance Core to PC by UART, then check if `USB_to_RS232` driver has been installed successfully. 


## Why don't demo and examples of SDK work?
Make sure the USB or UART driver has been installed successfully, you can check it with `Guidance Assistant` software by using USB connected to PC, or check in `Device Manager` if using UART. 

Then set the `DIY` Mode in Guidance Assistant software, and activate `USB` or `UART` interface if it's already activated. 

If errors about OpenCV occur, most likely it is because the environment variable `OPENCVROOT` is not properly added.


## Can I get image data using UART?
No. We don't transfer image data via UART because of the limited bandwidth of UART.


## Can I select all grey-scale images and depth images at the same time? 
No. Due to the bandwidth limit of USB, you cannot select the all image data at the same time.


## Do I have to select grey-scale image too if I only want depth image?
You do not have to select grey-scale image too if you only want depth image. Although the grey-scale image and the depth image are in the same struct, it is OK to select only depth image. Only remember that the pointer to grey-scale image is not valid in this case.


## Can I get the data from USB and UART at the same time?
Yes, you can get data from both of them at the same time, but it is not recommended as USB gives all types of data.


## Where can I get help when problems happen?
You could refer to **`Developer Guide`** provided in the SDK package first, which helps a lot at the beginning of using Guidance SDK. 

If the problem is still there, you are welcome to raise questions in our forum: [http://forum.dji.com/](http://forum.dji.com/forum.php?lang=en)


## How can I build my own project using Guidance SDK?
You can build your own project with Guidance SDK by modifying provided demo and example projects, or build a new project following the step-by-step tutorial `How to build a visual tracking project` in **`Developer Guide`**.


## What is the frequency limit of data transfer using USB?
The upper frequency limit of image data is 20hz, and can be scaled down if you select more image data. The other data such as IMU data, Velocity and so on, are fixed 20Hz.


## What are the supported baud rates using UART?
Currently we only support baud rate **115200** when using UART. 


## Can I get the camera intrinsic parameters using Guidance SDK?
Currently we do not have this function. The next version of Guidance SDK will open the interface to get the camera intrinsic parameters.