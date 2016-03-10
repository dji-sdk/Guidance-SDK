
# Guidance SDK Reference

*In case of any mistake or bug, please report to us using Github issue or DJI forum or email. You are welcome to send your pull request helping us fix issue. However, all pull requests related to document must follow the [document style](https://github.com/dji-sdk/Guidance-SDK/issues/3)*.

## Background

This document gives detailed explanation on the SDK structure and API functions. We assume that you have

- a Guidance system,
- a computer with OpenCV installed,

and you are:

- familiar with Linux programming,
- or familiar with Windows programming and Microsoft Visual Studio.


## Introduction

This section introduces the structure of the Guidance SDK. The SDK is divided into three layers:

![](Images/Guidance_SDK_API3987.png)

- **Application:** This layer processes data from the HAL layer. It is written by developers.
- **HAL:** Hardware Abstraction Layer. This layer packs/parses the data to/from the Driver layer. It is implemented by the sample code (for UART) or SDK library (for USB), e.g. _libDJI\_guidance.so_.
- **Driver:** This layer receives data from the Guidance system through USB/UART. It is implemented by OS or 3rd party libraries, e.g. _libusb_.

### Interface

The Guidance SDK supports two communication protocols: USB & UART.

#### 1. USB

The supported data types are Velocity Data, Obstacle Distance Data, IMU Data, Ultrasonic Data, Greyscale Image, and Depth Image.

There are two ways to subscribe the data through USB.

1. Guidance Assistant Software

	User can use Guidance assistant software to subscribe the data in "DIY->API->USB" tab.

	- Connect Guidance with PC using USB cable, power on the Guidance
	- Choose the "Enable" check box
	- Choose the data according your requirement

	**Notes:** The available bandwidth is subject to the selection of image data and the output frequency. The selection of subscribed image data and output frequency will be saved and take effect when the Guidance system is turned off and on again.

	![](Images/Guidance_SDK_API5146.png)

2. Guidance API

	User can subscribe the data using Guidance API. Identity these API functions that are named with "select".

	**Notes:** If user subscribes the image data and output frequency using Guidance API functions, it will only temporarily override the data selection that is made in the Guidance Assistant software when the Guidance system is still powered on. However, the data selection that is made through the Guidance API will not permanently change the data subsections options stored in the Guidance system, unless you de-select the "Enable" option in the "USB" tab.

#### 2. UART

The output data types of UART are Velocity Data, Obstacle Distance Data, IMU Data, and Ultrasonic Data. Image data are not output via UART due to the bandwidth limit.

**Note:** Guidance UART only supports **115200** baud rate.

1. Subscribe Data

	You may only use Guidance assistant software to subscribe UART data. Enable this selection from "DIY->API->UART" page. Same as USB, the configuration will be saved in Guidance Core, unless you de-select the "Enable" option in the "UART" tab.

	![](Images/Guidance_SDK_API6086.png)

2. Protocol Description

	Protocol Frame Format:

| SOF | LEN | VER | RES | SEQ | CRC16 | DATA | CRC32 |
| --- | --- | --- | --- | --- | --- | --- | --- |

Protocol Frame Explanation:

| Field | Byte Index | Size（bit） | Description |
| --- | --- | --- | --- |
| SOF | 0 | 8 | Frame start number, fixed to be 0xAA |
| LEN | 1 | 10 | Frame length, maximum length is 1023 bytes |
| VER | 1 | 6 | Version of the protocol |
| RES | 5 | 40 | Reserved bits, fixed to be 0 |
| SEQ | 8 | 16 | Frame sequence number |
| CRC16 | 10 | 16 | Frame header CRC16 checksum |
| DATA | 12 | --① | Frame data, maximum length  1007 bytes |
| CRC32 | --② | 32 | Frame CRC32 checksum |

① Frame data size can vary, 1007 is the maximum length.

② The index of this field depends on the length of the data field.

Data Field Format:

| COMMAND SET | COMMAND ID | COMMAND DATA |
| --- | --- | --- |

Data Field Explanation:

| Data Field | Byte Index | Size（byte） | Description |
| --- | --- | --- | --- |
| COMMAND SET | 0 | 1 | Always 0x00 |
| COMMAND ID | 1 | 1 | e\_image: 0x00; e\_imu: 0x01; e\_ultrasonic: 0x02; e\_velocity: 0x03; e\_obstacle\_distance: 0x04 |
| COMMAND DATA | 2 | -- | Data body |

### Data Types

Each of the supported data types is described below.

- [**Error Code**](#e_sdk_err_code) enumerates possible error codes. When error occurs, usually an error code will be given, and the developer can reference this enum to find the error type. 
- [**Velocity Data:**](#velocity) velocity in body frame. The unit is **millimeter/second** and the frequency is 10 Hz.
- [**Obstacle Distance Data:**](#obstacle_distance) obstacle distance from five Guidance Sensors. The unit is **centimeter** and the frequency is 20 Hz.
- [**IMU Data:**](#imu) IMU data including accelerometer (in unit of acceleration of gravity **g**) and gyroscope (in quaternion format) data. The frequency is 20 Hz.
- [**Ultrasonic Data:**](#ultrasonic_data) Outputs ultrasonic data from five Guidance Sensors, including obstacle distance (in unit of **meter**) and reliability of the data. The frequency is 20 Hz.
- [**Greyscale Image:**](#image_data) Outputs Greyscale images for five directions. The image size is 320\*240 bytes for individual sensor. The default frequency is 20 Hz and can be scaled down using API functions.
- [**Depth Image:**](#image_data) Outputs depth images for five directions. The image size is 320\*240\*2 bytes for each direction. The default frequency is 20 Hz and can be scaled down using API functions.  
- [**Disparity Image:**](#image_data) Outputs disparity images for five directions. This data is useful when developers want to further refine the disaprity images using functions like speckle filter. The image size is 320\*240\*2 bytes for each direction. The default frequency is 20 Hz and can be scaled down using API functions.  

## Data Structures


###   e_sdk_err_code

**Description:**  Define error code of SDK.

~~~ cpp
enum e_sdk_err_code
{
	e_timeout = -7,			// time out
	e_libusb_io_err = -1,	// libusb IO error
	e_OK = 0,				// Succeed with no error
	e_load_libusb_err=1,	// Load libusb library error
	e_sdk_not_inited=2,		// SDK software is not ready
	e_hardware_not_ready=3, // Guidance hardware is not ready
	e_disparity_not_allowed=4,		// Disparity or depth image is not allowed
	e_image_frequency_not_allowed=5,  // Image frequency must be one of the enum type e_image_data_frequecy
	e_config_not_ready=6,			// Config is not ready
	e_online_flag_not_ready=7,	// Online flag is not ready
	e_stereo_cali_not_ready = 8,// Stereo calibration parameters are not ready
	e_max_sdk_err = 100			// maximum number of possible SDK errors
};
~~~ 

**Explanation:** 

1. `e_timeout`: time out during USB transfer.
2. `e_libusb_io_err`: IO error returned by libusb library. This can be caused by physical connection problem of USB.
3. `e_OK`: Succeed with no error.
4. `e_load_libusb_err`: Load libusb library error. This is caused by the inappropriate libusb library.
5. `e_sdk_not_inited`: SDK software is not ready.
6. `e_hardware_not_ready`: Guidance hardware is not ready.
7. `e_disparity_not_allowed`: If your Guidance is working in standard mode with obstacle sensing function activated, disparity or depth image is not allowed to select. The reason is, obstacle sensing has its own way to select disparity images. 
8. `e_image_frequency_not_allowed`: Image frequency must be one of the enum type `e_image_data_frequecy`.
9. `e_config_not_ready`: Configuration data is not ready. When Guidance is powered on, it takes several seconds (sometimes longer) to initiate, including loading configuration data (including other data) into memory, and sending to application layer (i.e. the SDK software). If the users start SDK application before configuration data is ready, this error will be thrown. Configuration data includes: Guidance working mode, Guidance Sensor online status, stereo calibration parameters, and so on.
10. `e_online_flag_not_ready`: Online flag is not ready. Guidance system allows users to use any number of sensors, from 1 to 5. We use an array of online status to indicate which sensor are online. If users subscribe data from a sensor that is not online, no data will be returned.
11. `e_stereo_cali_not_ready`: Stereo calibration parameters are not ready. The calibration parameters are useful for 3D applications. As the images are already rectified, no distortion coefficients are provided, but only coordinates of the principal point `cu, cv`, focal length `focal`, and baseline `baseline`.  

###   e_vbus_index

**Description:**  Define logical direction of vbus, i.e. the direction of selected Guidance Sensor. Note that they are only defined by the VBUS ports on Guidance Core, not by the Guidance Sensors. 

The comment of each element indicates the default direction when Guidance is installed on Matrice 100. However the developers can install Guidance in any manner on any device, thus the directions might also be different.

~~~ cpp
enum e_vbus_index
{
	e_vbus1 = 1,	// front on M100 
	e_vbus2 = 2,	// right on M100 
	e_vbus3 = 3,	// rear on M100 
	e_vbus4 = 4,	// left on M100 
	e_vbus5 = 0	    // down on M100 
};
~~~ 

###   e_image_data_frequecy

**Description:**  Define frequency of image data. The supported frequencies are: 5Hz, 10Hz, 20Hz. With more images selected, smaller frequency should be selected.

~~~ cpp
enum e_image_data_frequecy
{
	e_frequecy_5 =  0,	// frequecy of image data: 5Hz 
	e_frequecy_10 = 1,	// frequecy of image data: 10Hz 
	e_frequecy_20 = 2	// frequecy of image data: 20Hz 
};
~~~ 

###   e_guidance_event

**Description:**  Define event type of callback

~~~ cpp
enum e_guidance_event
{
	e_image = 0,	   	      	// called back when image comes 
	e_imu,	       	      	    // called back when imu comes 
	e_ultrasonic,        	    // called back when ultrasonic comes 
	e_velocity,	    	        // called back when velocity data comes 
	e_obstacle_distance,	    // called back when obstacle data comes 
	e_motion,               	// called back when global position comes
	e_event_num
};
~~~ 

###   image_data

**Description:**  Define image data structure. For each direction of stereo camera pair, the depth image aligns with the left greyscale image.

~~~ cpp
typedef struct _image_data
{
	unsigned int     frame_index;	                              // frame index 
	unsigned int     time_stamp;	                              // time stamp of image captured in ms 
	char             *m_greyscale_image_left[CAMERA_PAIR_NUM];	  // greyscale image of left camera 
	char             *m_greyscale_image_right[CAMERA_PAIR_NUM];   // greyscale image of right camera 
	char             *m_depth_image[CAMERA_PAIR_NUM];	          // depth image in meters 
	char             *m_disparity_image[CAMERA_PAIR_NUM];         // disparity image in pixels 
}image_data;
~~~ 

###   ultrasonic_data

**Description:**  Define ultrasonic data structure. `ultrasonic` is the distance between Guidance Sensor and the nearest object detected by ultrasonic sensor. The Unit is `mm`. `reliability` is the reliability of this distance measurement, with 1 meaning reliable and 0 unreliable. **Note:** Due to noise in the distance measurement, it is recommended to filter the data before use.

~~~ cpp
typedef struct _ultrasonic_data
{
	unsigned int     frame_index;	                  // correspondent frame index 
	unsigned int     time_stamp;	                  // time stamp of correspondent image captured in ms 
	short            ultrasonic[CAMERA_PAIR_NUM];	  // distance in mm. -1 means invalid. 
	unsigned short   reliability[CAMERA_PAIR_NUM];	  // reliability of the distance data 
}ultrasonic_data;
~~~ 

###   velocity

**Description:**  Define velocity in body frame coordinate. Unit is `mm/s`.

~~~ cpp
typedef struct _velocity
{
	unsigned int     frame_index;	          // correspondent frame index 
	unsigned int     time_stamp;	          // time stamp of correspondent image captured in ms 
	short            vx;	                  // velocity of x in mm/s 
	short            vy;	                  // velocity of y in mm/s 
	short            vz;	                  // velocity of z in mm/s 
}velocity;
~~~ 

###   obstacle_distance

**Description:**  Define obstacle distance calculated by fusing vision and ultrasonic sensors. Unit is `cm`.

~~~ cpp
typedef struct _obstacle_distance
{
	unsigned int     frame_index;	             // correspondent frame index 
	unsigned int     time_stamp;	             // time stamp of correspondent image captured in ms 
	unsigned short   distance[CAMERA_PAIR_NUM];  // distance of obstacle in cm 
}obstacle_distance;
~~~ 

###   imu

**Description:**  Define IMU data structure. Unit of acceleration is `m/s^2`.

~~~ cpp
typedef struct _imu
{
	unsigned int     frame_index;	          // correspondent frame index 
	unsigned int     time_stamp;	          // time stamp of correspondent image captured in ms 
	float            acc_x;	                  // acceleration of x in unit of m/s^2 
	float            acc_y;	                  // acceleration of y in unit of m/s^2 
	float            acc_z;	                  // acceleration of z in unit of m/s^2
	float            q[4];	                  // quaternion: [w,x,y,z] 
}imu;
~~~ 

###   stereo_cali

**Description:**  Calibration parameters of cameras. All values will be zero if the corresponding sensor is not online.

~~~ cpp
typedef struct _stereo_cali
{
	float cu;			// x position of focal center in units of pixels 
	float cv;			// y position of focal center in units of pixels 
	float focal;		// focal length in units of pixels 
	float baseline;		// baseline of stereo cameras in units of meters 
	_stereo_cali() { }
	_stereo_cali(float _cu, float _cv, float _focal, float _baseline)
	{
		cu = _cu, cv = _cv;
		focal = _focal, baseline = _baseline;
	}
}stereo_cali;
~~~ 

###   exposure_param

**Description:**  Parameters of camera exposure. When m_expo_time = m_expected_brightness=0, return to default AEC. 

~~~ cpp
typedef struct _exposure_param
{
	float	      m_step;		// adjustment step for auto exposure control (AEC). Default is 10.
	float		  m_exposure_time;	// constant exposure time in mini-seconds. Range is 0.1~20. Default is 7.25.
	unsigned int  m_expected_brightness;// constant expected brightness for AEC. Range is 50~200. Default is 85.
	unsigned int  m_is_auto_exposure;	// 1: auto exposure; 0: constant exposure
	int           m_camera_pair_index;	// index of Guidance Sensor
	_exposure_param(){
		m_step = 10;
		m_exposure_time = 7.68;
		m_expected_brightness = 85;
		m_is_auto_exposure = 1;
		m_camera_pair_index = 1;
	}
}exposure_param;
~~~ 

### motion

**Description:**  Define global motion data. Unit is `m` for position and `m/s` for velocity.

~~~ cpp
typedef struct _motion
{
	unsigned int     frame_index;
	unsigned int     time_stamp;

	int		         corresponding_imu_index;

	float		     q0;
	float		     q1;
	float		     q2;
	float		     q3;
	int			     attitude_status;  // 0:invalid; 1:valid

	float		     position_in_global_x;  // position in global frame: x 
	float		     position_in_global_y;  // position in global frame: y 
	float		     position_in_global_z;  // position in global frame: z 
	int			     position_status; // lower 3 bits are confidence. 0:invalid; 1:valid

	float		     velocity_in_global_x;  // velocity in global frame: x 
	float		     velocity_in_global_y;  // velocity in global frame: y 
	float		     velocity_in_global_z;  // velocity in global frame: z 
	int			     velocity_status; // lower 3 bits are confidence. 0:invalid; 1:valid

	float		     reserve_float[8];
	int			     reserve_int[4];

	float   	     uncertainty_location[3];// uncertainty of position
	float   	     uncertainty_velocity[3];// uncertainty of velocity
} motion;
~~~

## API

### Overview

The Guidance API provides configuration and control methods for Guidance with C interface. Here is an overview of the key methods available in this API.

Please reference the protocol of Section 2.1.2 and also the example code of `uart_example` when using UART transfer type.

- initialization
	- [reset_config](#reset_config)
	- [init_transfer](#init_transfer)

- subscribe data
	- [select_imu](#select_imu)
	- [select_ultrasonic](#select_ultrasonic)
	- [select_velocity](#select_velocity)
	- [select_obstacle_distance ](#select_obstacle_distance )
	- [set_image_frequecy](#set_image_frequecy)
	- [select_depth_image](#select_depth_image)
	- [select_disparity_image](#select_disparity_image)
	- [select_greyscale_image](#select_greyscale_image)
	- [select_motion](#select_motion)

- set callback and exposure
	- [set_sdk_event_handler](#set_sdk_event_handler)
	- [set_exposure_param](#set_exposure_param)

- get data 
	- [get_online_status](#get_online_status)
	- [get_stereo_cali](#get_stereo_cali)
	- [get_device_type](#get_device_type) 
	- [get_image_size](#get_image_size)

- transfer control
	- [start_transfer](#start_transfer)
	- [stop_transfer](#stop_transfer)
	- [release_transfer](#release_transfer)
	- [wait_for_board_ready](#wait_for_board_ready)

### Method

####  user_call_back

- **Description:**  Callback function prototype. The developer must write his/her own callback function in this form. In order to achieve best performance, it is suggested not performing any time-consuming processing in the callback function, but only copying the data out. Otherwise the transfer frequency might be slowed down.  
- **Parameters:**  `event_type` use it to identify the data:image,imu,ultrasonic,velocity or obstacle distance
- **Parameters:**  `data_len` length of the input data
- **Parameters:**  `data` data read from Guidance.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
typedef int (*user_call_back)( int event_type, int data_len, char *data );
~~~

####  reset_config

- **Description:**   Clear subscribed configure, if you want to subscribe data different from last time.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int reset_config( void );
~~~ 

####  init_transfer

- **Description:**  Initialize Guidance and create data transfer thread.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int init_transfer( void );
~~~ 

####  select_imu

- **Description:**   Subscribe IMU data. In standard mode, IMU data can only be output when Guidance is connected to DJI N1 flight controller. While in DIY mode, IMU data can always be output without connecting to a flight controller.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API void select_imu( void );
~~~ 

####  select_ultrasonic

- **Description:**   Subscribe ultrasonic data.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API void select_ultrasonic( void );
~~~ 

####  select_velocity

- **Description:**   subscribe velocity data, i.e. velocity of Guidance in body coordinate system.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API void select_velocity( void );
~~~ 

####  select_obstacle_distance

- **Description:**   Subscribe obstacle distance.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API void select_obstacle_distance( void );
~~~ 

####  select_greyscale_image

- **Description:**   Subscribe rectified greyscale image.
- **Parameters:**  `camera_pair_index` index of camera pair selected
- **Parameters:**  `is_left` whether the image data selected is left
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left );
~~~ 

####  select_depth_image

- **Description:**   Subscribe depth image.
- **Parameters:**  `camera_pair_index` index of camera pair selected
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int select_depth_image( e_vbus_index camera_pair_index );
~~~ 

####  select_disparity_image

- **Description:**   Subscribe disparity image, which can be filtered with functions such as filterSpeckles.
- **Parameters:**  `camera_pair_index` index of camera pair selected
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int select_disparity_image( e_vbus_index camera_pair_index );
~~~ 


####  select_motion

- **Description:**   Subscribe global motion data, i.e. velocity and position of Guidance in global coordinate system.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API void select_motion( void );
~~~ 

####  set_image_frequecy

- **Description:**  Set frequecy of image transfer. **Note**: The bandwidth of USB is limited. If you subscribe too many images (greyscale image or depth image), the frequency should be set relatively small, otherwise the SDK cannot guarantee the continuity of image transfer.
- **Parameters:**  `frequecy` frequecy of image transfer
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int set_image_frequecy( e_image_data_frequecy frequecy );
~~~ 

####  start_transfer

- **Description:**  Inform Guidance to start data transfer. 
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int start_transfer( void );
~~~ 

####  stop_transfer

- **Description:**  Inform Guidance to stop data transfer.   
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int stop_transfer( void );
~~~ 

####  release_transfer

- **Description:**  Release the data transfer thread. 
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int release_transfer( void );
~~~ 

####  set_sdk_event_handler

- **Description:**   Set callback function handler. When data from Guidance comes, it will be called by data transfer thread.
- **Parameters:**  `handler` function pointer to callback function.
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int set_sdk_event_handler( user_call_back handler );
~~~ 

####  get_stereo_cali

- **Description:**  Get stereo calibration parameters.
- **Parameters:**  `stereo_cali_pram` Array of calibration parameters for all sensors. 
- **Return:**  `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int get_stereo_cali( stereo_cali stereo_cali_pram[CAMERA_PAIR_NUM]);
~~~ 

####  get_online_status

- **Description:**  Get the online status of Guidance sensors.
- **Parameters:**  `online_status` Array of online status for all sensors.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int get_online_status(int online_status[CAMERA_PAIR_NUM]);
~~~ 

####  get_device_type

- **Description:**  Get the type of devices. Currently only support one type of device: Guidance. 
- **Parameters:**  `device_type` Device type.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int get_device_type(e_device_type* device_type);
~~~ 

####  get_image_size

- **Description:**  Get the size of image data.
- **Parameters:**  `width` Image width.
- **Parameters:**  `height` Image height.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API	int get_image_size(int* width, int* height);
~~~ 

####  wait_for_board_ready

- **Description:**  Wait for board ready signal. This function waits 20 seconds for Guidance board to get started. If 20 seconds pass and the board is still not ready, return a timeout error code. The users usually do not need to use this function, as it is already called in init_transfer.
- **Return:**  `error code`. Zero if succeed, otherwise e_timout.

~~~ cpp
SDK_API int wait_for_board_ready();
~~~ 

####  set_exposure_param

- **Description:**  Set exposure mode and parameters.
- **Parameters:**  `param` pointer of exposure parameter struct.
- **Return:**   `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int set_exposure_param( exposure_param *param );
~~~ 
