# DJI Guidance SDK

　[English Version](#english-version)
　[中文版本](#中文版本)

## English Version
Official Guidance SDK package for accessing the rich categories of output data from Guidance via USB and UART, and configure your Guidance all by your demand.

### Document

　[Developer Guide](doc/Guides/Developer_Guide/en/DeveloperGuide_en.md)

　[Run Example](doc/Guides/RunExample/runExample.md)

　[Build a Visual Tracking Project](doc/Guides/Visual_Tracking_tutorial/visual_Tracking_tutorial_en.md)

　[API Documentation](doc/Guidance_SDK_API/Guidance_SDK_API_en.md)
 
　[FAQ](doc/FAQ_en.md)

### Structure

-	**demo**: demo applications using Guidance SDK
-	**doc**: documentations
-	**examples**: examples for USB and UART
-	**include**: Header file of Guidance SDK 
-	**lib**: Library files for Windows
	- 2010/x64: build with Visual Studio 2010 64 bit
	- 2010/x86: build with Visual Studio 2010 32 bit
	- 2013/x64: build with Visual Studio 2013 64 bit
	- 2013/x86: build with Visual Studio 2013 32 bit
-	**so**: Library files for Linux	
	- x64: build with `g++` on 64 bit Linux system
	- x86: build with `g++` on 32 bit Linux system
	- XU3: build with `g++` on XU3
	- arm: build with latest `arm-linux-gnueabi-g++` for embedded ARM systems.Please install the cross-compiling toolchain by `sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi`

Also notice that, to enable fast download for ROS users, we have a separate ROS repo with much smaller size: [Guidance-SDK-ROS](https://github.com/dji-sdk/Guidance-SDK-ROS).

### Usage

#### Windows 

Examples of USB and UART can be found in *examples/usb\_example*, *examples/uart\_example*,	including Visual Studio projects which is ready to compile. Remember to copy the corresponding DJI_guidance.dll file to the same directory where the output binary locates.  

#### Linux

Examples of USB and UART can be found in *examples/usb\_example*, *examples/uart\_example*,	including Makefile which is ready to compile. Remember to copy the corresponding libDJI_guidance.so file to the same directory where the output binary locates. 

Notice that, reading and writing Guidance USB port in Linux requires root authority. To save the trouble of typing `sudo` every time running Guidance SDK applications, it is suggested to add a rule to `/etc/udev/rules.d` directory, which can be found in **doc/51-guidance.rules**. Or typing from terminal the following line

	sudo sh -c 'echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"fff0\", ATTR{idProduct}==\"d009\", MODE=\"0666\"" > /etc/udev/rules.d/51-guidance.rules'


## 中文版本
Guidance SDK可以让开发者通过Guidance的USB口和串口来获取丰富的传感器数据，以及按照自己的需要来灵活配置Guidance。

### 文档
　[开发者指南](doc/Guides/Developer_Guide/cn/DeveloperGuide_cn.md)　

　[创建一个视觉跟踪工程](doc/Guides/Visual_Tracking_tutorial/visual_Tracking_tutorial_cn.md)

　[API文档](doc/Guidance_SDK_API/Guidance_SDK_API_cn.md)

　[FAQ](doc/FAQ_cn.md)

### 结构

-	**demo**: 用Guidance SDK实现的示例应用
-	**doc**: 文档
-	**examples**: USB和串口的简单示例程序
-	**include**: Guidance SDK头文件 
-	**lib**: Windows下的库文件
	- 2010/x64: 使用 Visual Studio 2010 64 bit 编译
	- 2010/x86: 使用 Visual Studio 2010 32 bit 编译
	- 2013/x64: 使用 Visual Studio 2013 64 bit 编译
	- 2013/x86: 使用 Visual Studio 2013 32 bit 编译
-	**so**: Linux下的库文件	
	- x64: 在64位Linux系统上用 `g++` 编译
	- x86: 在32位Linux系统上用 `g++` 编译
	- XU3: 在XU3上用 `g++` 编译 
	- arm: 使用最新的 `arm-linux-gnueabi-g++` 编译，供嵌入式ARM系统使用。使用时，请先安装交叉编译工具链：`sudo apt-get install gcc-arm-linux-gnueabi g++-arm-linux-gnueabi`

请注意，为了便于ROS用户的快速下载，我们还维护了一个独立的ROS包。它不包含任何文档和Windows下的库文件，因此具有更小的体积：[Guidance-SDK-ROS](https://github.com/dji-sdk/Guidance-SDK-ROS).

### 如何使用
#### Windows 

USB和串口的示例代码可以在 *examples/usb\_example* 和 *examples/uart\_example*中找到，其中包含了可直接编译运行的Visual Studio工程。需要将对应的dll文件复制到exe所在目录或Windows系统目录。

#### Linux

USB和串口的示例代码可以在同样的目录（*examples/usb\_example* 和 *examples/uart\_example*）中找到，其中包含了可直接编译的Makefile文件。

注意，在Linux下读写Guidance的USB口需要root权限。为了省去每次运行Guidance SDK程序都要输入`sudo`的麻烦，建设在 `/etc/udev/rules.d` 下添加一个规则文件，即 **doc/51-guidance.rules**. 或者也可以从终端输入下面的这行：

	sudo sh -c 'echo "SUBSYSTEM==\"usb\", ATTR{idVendor}==\"fff0\", ATTR{idProduct}==\"d009\", MODE=\"0666\"" > /etc/udev/rules.d/51-guidance.rules'



