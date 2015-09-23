# Guidance SDK开发者指南

*如发现任何错误，请通过Github issue或开发者论坛或邮件反馈给我们。欢迎提交pull request来帮助我们修正问题，关于文档的修改需要符合[格式标准](https://github.com/dji-sdk/Guidance-SDK/issues/3)*。

## SDK包下载

Guidance SDK软件包在GitHub上更新和维护。

- [Guidance-SDK](https://github.com/dji-sdk/Guidance-SDK)：完整包。包括头文件、所有平台的库文件、所有文档。
- [Guidance-SDK-ROS](https://github.com/dji-sdk/GuidanceSDK-ROS)：ROS包。包括头文件、Ubuntu及Odroid XU3的库文件。

## 环境配置

### Windows

#### 安装Guidance Assistant软件

首先，在Windows平台下安装Guidance Assistant软件。软件在安装过程中会自动安装Guidance所需的USB驱动，在软件安装引导结束后需重启系统使之生效。系统重新启动后，将Guidance通过USB连至电脑并上电，进入**“计算机管理-设备管理器”**中查看，如果在设备列表中出现“DJI, Inc. - Guidance”，意味着软件安装成功。

![Alt text](./Images/1.png)

#### 安装UART驱动
如果要通过UART使用Guidance SDK，需要安装USB转RS232的驱动。使用过程中请注意，Guidance SDK中examples默认读取的串口号为**“COM5”**。如果您的计算机上的串口号与之不同，请修改代码中的设置或更改设备串口号。

#### 通过Guidance Assistant软件激活并订阅数据
>*假设用户已经在Windows上正确安装了Guidance Assistant软件。*
 
- 首先，将Guidance连接至电脑并上电启动；接着等待Guidance上的绿灯开始闪烁后，打开Guidance Assistant软件，可以看到软件**“查看”**页面的左下角绿灯亮起，说明连接正常；此时将工作模式设置为**“自定义模式”**。

![Alt text](./Images/2.png)

- 然后转到“自定义”页面，在“接口参数”选项卡中根据需要激活USB与UART，并通过勾选相应的选项来订阅图像数据及图像输出频率；图像数据及频率设置也可通过Guidance SDK中相应的API进行设置。

![Alt text](./Images/3.png)

![Alt text](./Images/4.png)

- 关闭Guidance Assistant软件并重启Guidance使配置生效。

#### 推荐使用Visual Studio
Guidance SDK中提供了Demo与examples供参考使用,其中包含了Visual Studio 2010的完整配置文件与Visual studio 2013的部分配置文件。分别是Windows7系统下测试通过的32位与64位、debug与release相关的opencv与SDK配置文件，形如**“use\_Guidance\_\*.prop”**, **“use\_opencv\_\*.prop”**，用户可以根据需要将它们复制并添加到自己的工程中，省去配置的麻烦。

#### 安装OpenCV
Guidance SDK分别在Opencv2.4.8、OpenCV2.4.9及OpenCV2.4.11上进行了测试，Demo及examples中的例程默认使用的是OpenCV2.4.11版本。用户可以从OpenCV官方网站[http://opencv.org/](http://opencv.org/)上下载合适版本的OpenCV并安装到电脑上。使用时请注意，在Demo及examples的配置文件中，由于使用了 **OPENCVROOT** 的环境变量，因此用户在配置OpenCV环境的过程中需要新建一个名为 **OPENCVROOT** 的系统环境变量，其值为OpenCV的安装目录。
>*假设用户已经成功在电脑上安装了OpenCV2.4.11*

- 安装目录为**D:/OPENCV/opencv2411/build[sources]**，如下图所示，进入**“高级系统设置-环境变量-系统变量”**，新建一个变量，变量名为**OPENCVROOT**，值为 D:\OPENCV\opencv2411\

![Alt text](./Images/5.png)

- 接着在系统变量的PATH变量尾部添加OpenCV的库目录，注意不同目录间用分号隔开: 
**D:/OPENCV/opencv2411/build/x64/vc10/bin;**
**D:/OPENCV/opencv2411/build/x86/vc10/bin;**   
- 如果用户使用Visual Studio作为开发环境，接下来就只需将相应的 **“use\_Guidance\_\*.prop”** 与 **“use\_opencv\_\*.prop”** 配置文件添加到工程中去即可；或者也可直接复制例程，在例程的基础上进行开发。

#### 正确放置DJI\_guidance.dll
>*务必记得将DJI\_guidance.dll拷贝至exe所在的目录，否则会显示无法找到DJI\_guidance.dll的错误。*

![Alt text](./Images/222.png)

### Linux

#### 安装libusb驱动

从[http://www.libusb.org/](http://www.libusb.org/)下载并解压**“libusb-1.0.9.tar.bz2”**，并按照指导正确安装libusb驱动；不推荐通过`apt-get install` 安装libusb驱动；
例如:
解压**libusb-1.0.9.tar.bz2**并利用`cd`命令跳转到解压目录下；接着运行以下指令:

	>> ./configure
	>> make
	>> make install


#### 安装UART驱动

Linux默认包含了UART驱动，不需要另外安装。

#### 通过Guidance Assistant软件激活并订阅数据

参考上一篇**通过Guidance Assistant软件激活并订阅数据**。

#### 将g++更新到最新版本

请确保使用的是最新版本的g++；如果更新后仍提示无法找到g++指令，请检查系统时间是否与真实世界不同步！ 
例如:

	>> sudo apt-get install g++

	
#### 安装OpenCV

用户可以直接通过apt-get install来安装OpenCV: 

	>> sudo apt-get install libopencv-dev


或者，用户可以上GitHub下载**Install-OpenCV-master**压缩包并解压到本地，这是由jayrambhia提供的一个Linux下各OpenCV版本的安装脚本，利用这个脚本用户可以非常方便地将OpenCV安装到Linux； 
例如:


	>> cd PATH/TO/Install-OpenCV-master/Ubuntu
	>> ./dependencies.sh 
	>> cd 2.4
	>> ./opencv2_4_9.sh


#### 正确放置DJI\_guidance.so

将相应的Guidance共享库**libDJI\_guidance.so**拷贝至**/usr/local/lib**.
例如: 

	>> sudo cp libDJI_guidance.so /usr/local/lib


#### 通过sudo指令运行程序

- 使用Guidance的USB接口时，需要使用`sudo`指令，以取得对USB接口的root权限。否则会出现*permission denied*的错误。
例如:

		>> sudo ./guidance_example

- 为了避免每次都要使用sudo来运行Guidance SDK程序的麻烦，可以在`/etc/udev/rules.d/`文件夹中创建一个规则文件`51-guidance.rules`，内容如下：
		
		SUBSYSTEM=="usb", ATTR{idVendor}=="fff0", ATTR{idProduct}=="d009", MODE="0666"

	然后再拨插USB线，就可以以普通用户的权限来运行Guidance SDK程序了。
 
## 常见错误

> *参考开发者指南给出的方法进行开发环境的搭建，能够避免大部分的错误。*


### 连接错误

***问题描述:***

“usb error” 或者类似的报错，例如:
 
![Alt text](./Images/6.png)


***解决方法:***

- 首先确定Guidance已经连接到电脑上，并且Guidance上的绿灯开始进入闪烁状态；
- 检查USB驱动是否正确安装，Windows下的检查方法连接上Guidance后进入设备管理器中查看；或者使用DJIGuidance软件进行查看，如果**“查看”**页面左下角的绿灯亮起，说明USB驱动已经正确安装；否则请参考开发者指南安装Guidance Assistant软件；Linux下用户需严格遵循开发者指南中给出的安装方法进行安装，通过Guidance SDK中相关的USB例程进行测试；
- 如果在Windows下使用UART获取数据，请检查串口号是否为**“COM5**”；否则请在查看串口端号后，在examples中进行相应更改；
- 成功连接上Guidance Assistant软件后，进入**“自定义”**界面的**“接口参数”**选项卡，查看是否已经激活USB或UART选项；如果没有激活，请勾选相应的选项并关闭软件与Guidance，Guidance重启后配置生效；
- 如果数据传输非正常退出，也就是线程在运行到**stop_transfer()**与**release_transfer()**之前就退出，也会导致此类错误，这种情况下需要重启Guidance； 
- 在Linux下请务必使用`sudo`命令启动程序；
- 请确认在Guidance上的绿灯开始闪烁后运行程序；
- 请确认在使用SDK时已关闭“Guidance Assistant”软件；
- 在虚拟机上运行可能会导致问题；


### 编译错误

#### 共享库加载错误

***错误描述:***

> Error while loading shared libraries : libDJI_guidance.so: cannot open shared object file : No such file or directory.

	
***解决方法:***

- 请确认将系统所对应的**DJI\_guidance.so**复制到了**/usr/local/lib**目录下；若仍存在这种错误，请将其再拷贝一份至out程序运行目录下；
- 请确认该so文件与你电脑的系统版本是对应的；

#### 无法打开DJI\_guidance.dll文件

***错误描述:***

> Cannot open DJI_guidance.dll file:No such file or directory.

    
***解决方法:***

请确认将系统所对应的**DJI\_guidance.dll**分别复制到了lib文件夹与*.exe程序运行目录下；以及其版本是否与可执行程序一致。


#### OpenCV相关错误

***错误描述:***

> 无法打开与OpenCV有关的**\*.h**与**\*.dll**等文件；

***解决办法:***

请确认OpenCV是否正确安装，且版本与程序要求的一致；另外，若运行Guidance SDK中给出的Demo与examples，记得要在系统变量中添加**OPENCVROOT**变量；


### 数据传输错误

***错误描述:***

数据传输错误一般指得到了不正确的数据，例如图像显示错误:

![Alt text](./Images/7.png)

***解决办法:***

- 首先检查你的代码，或者通过Guidance SDK中给出的例程进行检验；
- 确认不是代码的问题后，请重启Guidance，并在Guidance上的绿灯开始闪烁后再运行程序；这种情况一般是由于非正常退出导致的，例如线程在运行到**stop\_transfer()**与 **release\_transfer()**之前就已经退出。

 
 