
# Guidance SDK Documentation

*In case of any mistake or bug, please report to us using Github issue or DJI forum or email. You are welcome to send your pull request helping us fix issue. However, all pull requests related to document must follow the [document style](https://github.com/dji-sdk/Guidance-SDK/issues/3)*

## Background

This guide assumes that you have

- a Guidance system,

- a computer with OpenCV installed,

and you are:

- familiar with Linux programming,

- or familiar with Windows programming and Microsoft Visual Studio.


## Introduction

This section introduces the structure of the Guidance SDK. The SDK is divided into three layers:

![](Images/Guidance_SDK_API3987.png)

- **Application:** This layer processes data from the HAL layer. It is written by developers.
- **HAL:** This layer packs/parses the data to/from the Driver layer. It is implemented by the sample code (for UART) or SDK library (for USB), e.g. _libDJI\_guidance.so_.
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

- **Velocity Data:** velocity in body frame. The unit is **millimeter/second** and the frequency is 10 Hz.

- **Obstacle Distance Data:** obstacle distance from five Guidance Sensors. The unit is **centimeter** and the frequency is 20 Hz.

- **IMU Data:** IMU data including accelerometer (in unit of m/s^2) and gyroscope (in quaternion format) data. The frequency is 20 Hz.

- **Ultrasonic Data:** Outputs ultrasonic data from five Guidance Sensors, including obstacle distance (in unit of meter) and reliability of the data. The frequency is 20 Hz.

- **Greyscale Image:** Outputs Greyscale images for five directions. The image size is 320\*240 bytes for individual sensor. The default frequency is 20 Hz and can be scaled down using API functions.

- **Depth Image:** Outputs depth images for five directions. The image size is 320\*240\*2 bytes for each direction. The default frequency is 20 Hz and can be scaled down using API functions.
  
  **Notes:** In order to achieve best performance, it is suggested not performing any time-consuming processing in the callback function, but only copying the data out. Otherwise the transfer frequency might be slowed down. 


## Data Structures

### e\_sdk\_err\_code

**Description:** Define error code of SDK.

~~~ cpp
enum e_sdk_err_code
{
    e_sdk_no_err = 0,					// Succeed with no error.
    e_load_libusb_err,					// Load libusb library error.
    e_sdk_not_inited,					// SDK software is not ready 
    e_guidance_hardware_not_ready,		// Guidance hardware is not ready 
    e_disparity_not_allowed,			// If work type is standard, disparity is not allowed to be selected 
    e_image_frequency_not_allowed,		// Image frequency must be one of the enum type e_image_data_frequecy 
    e_config_not_ready,					// Get config including the work type flag, before you can select data 
    e_online_flag_not_ready,			// Online flag is not ready 
    e_max_sdk_err = 100					// maximum number of errors
};
~~~


### e\_vbus\_index

**Description:** Define logical direction of vbus, i.e. the Guidance Sensor selected.


~~~ cpp
enum e_vbus_index
{
    e_vbus1 = 1,        // logic direction of vbus
    e_vbus2 = 2,        // logic direction of vbus
    e_vbus3 = 3,        // logic direction of vbus
    e_vbus4 = 4,        // logic direction of vbus
    e_vbus5 = 0         // logic direction of vbus
};
~~~

### e\_image\_data\_frequecy

**Description:** Define frequency of image data.

~~~ cpp
enum e_image_data_frequecy
{
    e_frequecy_5  = 0,  // frequecy of image data 
    e_frequecy_10 = 1,  // frequecy of image data 
    e_frequecy_20 = 2   // frequecy of image data 
};
~~~

### user\_callback

- **Description:** Call back function prototype.
- **Parameters:** `event_type` use it to identify the data type: image, imu, ultrasonic, velocity or obstacle distance; `data_len` length of the input data; `data` input data read from GUIDANCE
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
typedef int (*user_call_back)( int event_type, int data_len, char *data );
~~~


### e\_guidance\_event

**Description:** Define event type of callback.

~~~ cpp
enum e_guidance_event
{
    e_image = 0,            // called back when image comes 
    e_imu,                  // called back when imu comes 
    e_ultrasonic,           // called back when ultrasonic comes 
    e_velocity,             // called back when velocity data comes 
    e_obstacle_distance,    // called back when obstacle data comes 
    e_event_num
};
~~~

### image\_data

**Description:** Define image data structure. The center of depth image coincides with the corresponding left greyscale image's.

~~~ cpp
typedef struct _image_data
{
    unsigned int     frame_index;   // frame index 
    unsigned int     time_stamp;    // time stamp of image captured in ms 
    char      *m_greyscale_image_left[CAMERA_PAIR_NUM];   // greyscale image of left camera 
    char      *m_greyscale_image_right[CAMERA_PAIR_NUM];  // greyscale image of right camera 
    char      *m_depth_image[CAMERA_PAIR_NUM];   // depth image in meters
}image_data;
~~~

### ultrasonic\_data

**Description:** Define ultrasonic data structure. Unit is `mm`.

~~~ cpp
typedef struct _ultrasonic_data
{
    unsigned int     frame_index;    // corresponse frame index 
    unsigned int     time_stamp;     // time stamp of corresponse image captured in ms 
    unsigned short   ultrasonic[CAMERA_PAIR_NUM];    // distance in mm
    unsigned short   reliability[CAMERA_PAIR_NUM];   // reliability of the distance data 
}ultrasonic_data;
~~~

### velocity

**Description:** Define velocity structure. Unit is `mm/s`.

~~~ cpp
typedef struct _velocity
{
    unsigned int     frame_index;        // corresponse frame index 
    unsigned int     time_stamp;         // time stamp of corresponse image captured in ms 
    short            vx;                 // velocity of x in mm/s
    short            vy;                 // velocity of y in mm/s 
    short            vz;                 // velocity of z in mm/s 
}velocity;
~~~


### obstacle\_distance

**Description:** Define obstacle distance structure. Unit is `cm`.

~~~ cpp
typedef struct _obstacle_distance
{
    unsigned int     frame_index;       // corresponse frame index 
    unsigned int     time_stamp;        // time stamp of corresponse image captured in ms 
    unsigned short   distance[CAMERA_PAIR_NUM];     // distance of obstacle in cm
}obstacle_distance;
~~~

### imu

**Description:** Define imu structure. Unit of acceleration is `m/s^2`.

~~~ cpp
typedef struct _imu
{
    unsigned int     frame_index;             // corresponse frame index 
    unsigned int     time_stamp;              // time stamp of corresponse image captured in ms 
    float            acc_x;                   // acceleration of x m/s^2
    float            acc_y;                   // acceleration of y m/s^2
    float            acc_z;                   // acceleration of z m/s^2
    float            q[4];                    // quaternion: [w,x,y,z]
}imu;
~~~


## API

### Overview

The GUIDANCE API provides configuration and control methods for GUIDANCE with C interface.


Here is an overview of the key methods available in this API:

~~~ cpp
// Guidance initialization
SDK_API int reset_config( void );
SDK_API int init_transfer( void );

// Guidance subscribe data
SDK_API void select_imu( void );    
SDK_API void select_ultrasonic( void );
SDK_API void select_velocity( void );   
SDK_API void select_obstacle_distance( void ); 
SDK_API int set_image_frequecy( e_image_data_frequecy frequecy ); 
SDK_API int select_depth_image( e_vbus_index camera_pair_index ); 
SDK_API int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left ); 

// Guidance set event
SDK_API int set_sdk_event_handler( user_call_back handler );

// Guidance transfer control
SDK_API int start_transfer( void );
SDK_API int stop_transfer( void );
SDK_API int release_transfer( void );

// Guidance get status 
SDK_API int get_online_status( int online_status[CAMERA_PAIR_NUM] );
~~~


### Method

#### reset\_config

- **Description:** Clear the subscribed configure, if you want to subscribe the different data from last time.
- **Parameters:** NULL
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int reset_config ( void );
~~~


#### init\_transfer

- **Description:** Initialize the GUIDANCE and connect it to PC.
- **Parameters:** NULL
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int init_transfer ( void );
~~~

#### select\_imu

- **Description:** Subscribe to imu.
- **Parameters:** NULL
- **Return:** NULL

~~~ cpp
SDK_API void select_imu ( void );
~~~

#### select\_ultrasonic

- **Description:** Subscribe to ultrasonic.
- **Parameters:** NULL
- **Return:** NULL

~~~ cpp
SDK_API void select_ultrasonic ( void );
~~~

#### select\_velocity

- **Description:** Subscribe to velocity data, i.e. velocity of GUIDANCE in body coordinate system.
- **Parameters:** NULL
- **Return:** NULL

~~~ cpp
SDK_API void select_velocity ( void );
~~~


#### select\_obstacle\_distance

- **Description:** Subscribe to obstacle distance, i.e. distance from obstacle.
- **Parameters:** NULL
- **Return:** NULL


~~~ cpp
SDK_API void select_obstacle_distance ( void );
~~~


#### set\_image\_frequecy

- **Description:** Set frequency of image transfer.
(**Note:** The bandwidth of USB is limited. If you subscribe too much images (greyscale image or depth image), the frequency should be relatively small, otherwise the SDK cannot guarantee the continuity of image transfer.)
- **Parameters:** `frequency` the frequency of image transfer
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int set_image_frequecy ( e_image_data_frequecy frequecy );
~~~

#### select\_depth\_image

- **Description:** Subscribe to depth image data.
- **Parameters:** `camera_pair_index` index of camera pair selected.
- **Return:** `NULL`


~~~ cpp
SDK_API void select_depth_image ( e_vbus_index camera_pair_index );
~~~

#### select\_greyscale\_image

- **Description:** Subscribe to rectified grey image data.
- **Parameters:** `camera_pair_index` index of camera pair selected; `is_left` whether the image data selected is left.
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int select_greyscale_image ( e_vbus_index camera_pair_index, bool is_left );
~~~

#### set\_sdk\_event\_handler

- **Description:** Set callback, when data from GUIDANCE comes, it will be called by transfer thread.
- **Parameters:** `handler` pointer to callback function.
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int set_sdk_event_handler ( user_call_back handler );
~~~

#### start\_transfer

- **Description:** Send message to GUIDANCE to start data transfer.
- **Parameters:** NULL .
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int start_transfer ( void );
~~~

#### stop\_transfer

- **Description:** Send message to GUIDANCE to stop data transfer.
- **Parameters:** NULL .
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int stop_transfer ( void );
~~~

#### release\_transfer

- **Description:** Release the data transfer thread.
- **Parameters:** NULL .
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int release_transfer ( void );
~~~

#### get\_online\_status

- **Description:** Get the online status of GUIDANCE sensors.
- **Parameters:** `online\_status[CAMERA\_PAIR\_NUM]` online status of GUIDANCE sensors
- **Return:** `error code`. Non-zero if error occurs.

~~~ cpp
SDK_API int get_online_status (int online_status[CAMERA_PAIR_NUM] );
~~~

**Notes** : These are only used for USB transfer type. Please reference the protocol of Section 2.1.2 and also the example code of `uart_example` when using UART transfer type.
