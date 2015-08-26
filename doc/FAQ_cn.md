# FAQ

## Guidance SDK 包含些什么内容？
Guidance SDK 包含有：

-	**demo**： 一个利用 Guidance API 实现的视觉跟踪应用程序
-	**doc**： SDK 整体说明文档
-	**examples**： USB、 UART 及 ROS 的使用例程
-	**include**： Guidance API 的头文件
-	**lib**： Windows 下的动态链接库文件
-	**so**： Linux 下的动态链接库文件

Guidance API 给用户提供了 Guidance 传感数据的读取接口，这些数据包括 `IMU data`， `Velocity`， `Obstacle distance`，  `grey-scale image`， `depth image` 及 `ultrasonic data`，并可通过 `USB` 与 `UART` 两种方式进行读取。


## 我可以在哪些平台上应用 Guidance SDK？
用户可以在 Guidance Core 硬件上单独使用 Guidance SDK，而与其他硬件平台独立开，也可连接到 `M100` 进行辅助视觉定位及避障。

用户可以在 32 位与 64 位的 Linux 及 Windows 操作系统上使用 Guidance SDK 进行 Guidance 的应用开发，这些操作系统平台所对应的动态链接库在 `lib` 与 `so` 中可以找到。


## 如何使用 Guidance SDK 从 Guidance Core 中获取传感数据？
首先你需要参考`开发者指南`中所述来配置开发环境，其中包括了驱动的安装及工程配置文件的设置。

成功完成相应的配置后，将 Guidance Core 连接至计算机并上电，运行 demo 或 examples 中的程序便可看到数据传输效果。


## 为何我无法在“资源管理器”中找到 Guidance 设备？
首先，请确认 Guidance Core 连接了计算机并已经上电；

如果您是通过 USB 的方式进行连接的，请检查 `DJI_Guidance` 软件是否安装成功；

如果您是通过 UART 方式进行连接的，请检查 `USB转RS232` 驱动是否安装成功。


## 为什么我无法运行 demo 与 examples 中的程序？
首先确认 UART 或 USB 的驱动安装成功了，在 Windows 平台下可以通过查看`设备管理器`进行检查。

接着打开 DJI_Guidance 软件，将模式设置为`自定义模式`，并在`接口参数`选项卡中根据需要打开 USB 与 UART 的数据订阅选项。

如果以上都没问题，错误仍然存在，还有可能是 OpenCV 的配置错误，或者是 dll 文件未放置正确；这些问题都可以在`开发者指南`中找到解决方案。


## 我能够通过 UART 的传输方式获取图像数据吗？
不能。由于 UART 传输的带宽限制，我们不提供 UART 下的图像传输方法。


## 我能够一次性订阅并传输所有的灰度图与深度图数据吗？
不能。由于带宽的限制，你不能同时选择所有通道的灰度图与深度图数据。


## 如果我只想订阅深度图数据，我是否也需要同时订阅同一方向上的灰度图？
你可以只订阅深度图数据，而不用同时订阅你不需要的灰度图数据。


## 我能够同时通过 USB 与 UART 获取数据吗？
是的，你可以同时通过这两种方式来读取数据，这意味着你将 USB 与 UART 同时连接至计算机并开启两个程序。但我们并不推荐这样做，用户可以单独通过 USB 方法获取所有的传感数据。


## 如果我在使用 Guidance SDK 中出现问题，应该去哪里寻求帮助？
首先你可以参考 SDK 包中的 **`开发者指南`** 来获得开发过程中可能出现的问题的解决方法。


如果在查阅了 **`开发者指南`** 后仍无法解决问题，你还可以登录我们的技术支持论坛[http://forum.dji.com/](http://forum.dji.com/forum.php?lang=cn)，将自己遇到的问题通过帖子的形式发表在论坛上，我们的工程师会为你及时地回复。


## 我怎么才能用 Guidance SDK 来开发我自己的 Guidance 项目?
你可以通过修改 Guidance SDk 中提供的 demo 与 examples 来适应你的项目， 或者你也可以通过参考 **`开发者指南`** 中的教程 `如何创建一个基于Guidance SDK的视觉跟踪程序` 来建立你自己的项目。


## 通过 USB 方式传输的数据频率最高能达到多少？
图像数据的最高频率为 20hz，其他如 IMU、速度等的数据频率固定为20hz。


## 通过 UART 方式进行数据传输，最高能到达多少波特率？
目前我们只支持 **115200** 波特率下 UART 方式的数据传输。 


## 我能够通过 Guidance SDK 获得 Guidance 摄像头的标定内参吗？
目前我们并不支持这项功能。在下一版我们会考虑提供相应方法，使用户能够取得摄像头的标定内参。