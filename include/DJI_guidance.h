/** @file  DJI_guidance.h
* 
* @brief Define data struct & data type from DJI_guidance system.
*
*This file define data struct & data type from DJI_guidance system.
*
* @version 1.4.0
*
*************************************/

#ifndef __DJI_GUIDANCE_H__
#define __DJI_GUIDANCE_H__

#ifdef USBDLL
#define SDK_API _declspec(dllexport)
#else
#define SDK_API extern
#endif

#define CAMERA_PAIR_NUM      5

/**
* @enum  e_sdk_err_code
* @brief Define error code of SDK.
*/
enum e_sdk_err_code
{
	e_timeout = -7,			// time out
	e_libusb_io_err = -1,	// libusb IO error
	e_OK = 0,				// Succeed with no error
	e_load_libusb_err=1,	// Load libusb library error
	e_sdk_not_inited=2,		// SDK software is not ready
	e_hardware_not_ready=3, // Guidance hardware is not ready
	e_disparity_not_allowed=4,		// Disparity or depth image is not allowed to be selected
	e_image_frequency_not_allowed=5,  // Image frequency must be one of the enum type e_image_data_frequecy
	e_config_not_ready=6,			// Config is not ready
	e_online_flag_not_ready=7,	// Online flag is not ready
	e_stereo_cali_not_ready = 8,// Stereo calibration parameters are not ready
	e_max_sdk_err = 100			// maximum number of possible SDK errors
};

enum e_device_type
{
	Guidance = 0,	// Device type is Guidance
	GuidanceLite	// Possible future version
};


/**
* @enum  e_vbus_index
* @brief Define logical direction of vbus, i.e. the direction of selected Guidance Sensor. Note that they are only defined by the VBUS ports on Guidance Core, not by the Guidance Sensors. The comment of each element indicates the default direction when Guidance is installed on Matrice 100. However the developers can install Guidance in any manner on any device, thus the directions might also be different.
*/
enum e_vbus_index
{
	e_vbus1 = 1,	/**< front on M100 */
	e_vbus2 = 2,	/**< right on M100 */
	e_vbus3 = 3,	/**< rear on M100 */
	e_vbus4 = 4,	/**< left on M100 */
	e_vbus5 = 0	    /**< down on M100 */
};

/**
* @enum  e_image_data_frequecy
* @brief Define frequency of image data. The supported frequencies are: 5Hz, 10Hz, 20Hz. With more images selected, smaller frequency should be selected.
*/
enum e_image_data_frequecy
{
	e_frequecy_5 =  0,	/**< frequecy of image data: 5Hz */
	e_frequecy_10 = 1,	/**< frequecy of image data: 10Hz */
	e_frequecy_20 = 2	/**< frequecy of image data: 20Hz */
};

/**
* @enum  e_guidance_event
* @brief Define event type of callback
*/
enum e_guidance_event
{
	e_image = 0,	   	   /**< called back when image comes */
	e_imu,	       	       /**< called back when imu comes */
	e_ultrasonic,          /**< called back when ultrasonic comes */
	e_velocity,	    	   /**< called back when velocity data comes */
	e_obstacle_distance,   /**< called back when obstacle data comes */
	e_motion,              /**< called back when global position comes */
	e_event_num
};

/**
*@struct  image_data
*@brief Define image data structure. For each direction of stereo camera pair, the depth image aligns with the left greyscale image.
*/
typedef struct _image_data
{
	unsigned int     frame_index;	                                  /**< frame index */
	unsigned int     time_stamp;	                                  /**< time stamp of image captured in ms */
	char             *m_greyscale_image_left[CAMERA_PAIR_NUM];	      /**< greyscale image of left camera */
	char             *m_greyscale_image_right[CAMERA_PAIR_NUM];   	  /**< greyscale image of right camera */
	char             *m_depth_image[CAMERA_PAIR_NUM];	              /**< depth image in meters */
	char             *m_disparity_image[CAMERA_PAIR_NUM];             /**< disparity image in pixels */
}image_data;

/**
*@struct  ultrasonic_data
*@brief Define ultrasonic data structure. `ultrasonic` is the distance between Guidance Sensor and the nearest object detected by ultrasonic sensor. The Unit is `mm`. `reliability` is the reliability of this distance measurement, with 1 meaning reliable and 0 unreliable.
*/
typedef struct _ultrasonic_data
{
	unsigned int     frame_index;	                        /**< correspondent frame index */
	unsigned int     time_stamp;	                        /**< time stamp of correspondent image captured in ms */
	short            ultrasonic[CAMERA_PAIR_NUM];	        /**< distance in mm. -1 means invalid measurement. */
	unsigned short   reliability[CAMERA_PAIR_NUM];	        /**< reliability of the distance data */
}ultrasonic_data;

/**
*@struct  velocity
*@brief Define velocity in body frame coordinate. Unit is `mm/s`.
*/
typedef struct _velocity
{
	unsigned int     frame_index;	          /**< correspondent frame index */
	unsigned int     time_stamp;	          /**< time stamp of correspondent image captured in ms */
	short            vx;	                  /**< velocity of x in mm/s */
	short            vy;	                  /**< velocity of y in mm/s */
	short            vz;	                  /**< velocity of z in mm/s */
}velocity;

/**
*@struct  obstacle_distance
*@brief Define obstacle distance calculated by fusing vision and ultrasonic sensors. Unit is `cm`.
*/
typedef struct _obstacle_distance
{
	unsigned int     frame_index;	                /**< correspondent frame index */
	unsigned int     time_stamp;	                /**< time stamp of correspondent image captured in ms */
	unsigned short   distance[CAMERA_PAIR_NUM];     /**< distance of obstacle in cm */
}obstacle_distance;

/**
*@struct  imu
*@brief Define IMU data structure. Unit of acceleration is `m/s^2`.
*/
typedef struct _imu
{
	unsigned int     frame_index;	          /**< correspondent frame index */
	unsigned int     time_stamp;	          /**< time stamp of correspondent image captured in ms */
	float            acc_x;	                  /**< acceleration of x in unit of m/s^2 */
	float            acc_y;	                  /**< acceleration of y in unit of m/s^2 */
	float            acc_z;	                  /**< acceleration of z in unit of m/s^2 */
	float            q[4];	                  /**< quaternion: [w,x,y,z] */
}imu;

/**
* @struct  stereo_cali
* @brief Calibration parameters of cameras. All values will be zero if the corresponding sensor is not online.
*/
typedef struct _stereo_cali
{
	float cu;			/**< x position of focal center in units of pixels */
	float cv;			/**< y position of focal center in units of pixels */
	float focal;		/**< focal length in units of pixels */
	float baseline;		/**< baseline of stereo cameras in units of meters */
	_stereo_cali() { }
	_stereo_cali(float _cu, float _cv, float _focal, float _baseline)
	{
		cu = _cu, cv = _cv;
		focal = _focal, baseline = _baseline;
	}
}stereo_cali;

/**
*@struct  exposure_param
*@brief Parameters of camera exposure. When m_expo_time = m_expected_brightness=0, return to default AEC. 
* Otherwise set to constant exposure time or constant expected brightness.
*/
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


/**
*@struct  motion
*@brief Define motion 
*/
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


/**  
 *     @fn typedef int (*user_call_back)( int event_type, int data_len, char *data );
 *     @brief Callback function prototype. The developer must write his/her own callback function in this form. In order to achieve best performance, it is suggested not performing any time-consuming processing in the callback function, but only copying the data out. Otherwise the transfer frequency might be slowed down.  
 *     @param `event_type` use it to identify the data:image,imu,ultrasonic,velocity or obstacle distance
 *     @param `data_len` length of the input data
 *     @param `data` data read from Guidance.
 *     @return  `error code`. Non-zero if error occurs.
 */
typedef int (*user_call_back)( int event_type, int data_len, char *data );

/**  
*     @fn int reset_config();
*     @brief  Clear subscribed configure, if you want to subscribe data different from last time.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int reset_config( void );

/**  
 *     @fn int init_transfer( void );
 *     @brief Initialize Guidance and create data transfer thread.
 *     @return  `error code`. Non-zero if error occurs.
 */
SDK_API int init_transfer( void );


/**  
*     @fn void select_imu();
*     @brief  Subscribe IMU data.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API void select_imu( void );

/**  
*     @fn void select_ultrasonic();
*     @brief  Subscribe ultrasonic data.
*     @return  `error code`. Non-zero if error occurs.
*/
SDK_API void select_ultrasonic( void );

/**  
*     @fn void select_velocity();
*     @brief  Subscribe velocity data, i.e. velocity of Guidance in body coordinate system.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API void select_velocity( void );

/**  
*     @fn int select_motion();
*     @brief  Subscribe motion data, i.e. velocity and position of Guidance in global coordinate system.
*     @return  `error code`. Non-zero if error occurs.
*/
SDK_API void select_motion( void );

/**  
*     @fn void select_obstacle_distance();
*     @brief  Subscribe obstacle distance.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API void select_obstacle_distance( void );

/**  
*     @fn int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left );
*     @brief  Subscribe rectified greyscale image.
*     @param `camera_pair_index` index of camera pair selected
*     @param `is_left` whether the image data selected is left
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left );

/**  
*     @fn int select_depth_image( e_vbus_index camera_pair_index );
*     @brief  Subscribe depth image.
*     @param `camera_pair_index` index of camera pair selected
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int select_depth_image( e_vbus_index camera_pair_index );

/**  
*     @fn int select_disparity_image( e_vbus_index camera_pair_index );
*     @brief  Subscribe disparity image.
*     @param `camera_pair_index` index of camera pair selected
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int select_disparity_image( e_vbus_index camera_pair_index );

/**  
*     @fn int set_image_frequecy( e_image_data_frequecy frequecy );
*     @brief Set frequecy of image transfer. **Note**: The bandwidth of USB is limited. If you subscribe too many images (greyscale image or depth image), the frequency should be set relatively small, otherwise the SDK cannot guarantee the continuity of image transfer.
*     @param `frequecy` frequecy of image transfer
*     @return  `error code`. Non-zero if error occurs.
*/
SDK_API int set_image_frequecy( e_image_data_frequecy frequecy );

/**  
*     @fn int get_device_type(e_device_type& device_type);
*     @brief Get the device type. Currently we have two types: Guidance and GuidanceLite.
*     @param `device_type` device type.
*     @return  `error code`,if error occur,it will be non zero
*/
SDK_API int get_device_type(e_device_type* device_type);

/**  
 *     @fn int start_transfer();
 *     @brief Inform Guidance to start data transfer. 
 *     @return `error code`. Non-zero if error occurs.
 */
SDK_API int start_transfer( void );

/**  
*     @fn int stop_transfer();
*     @brief Inform Guidance to stop data transfer.   
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int stop_transfer( void );

/**  
*     @fn int release_transfer();
*     @brief Release the data transfer thread. 
*     @return  `error code`. Non-zero if error occurs.
*/
SDK_API int release_transfer( void );

/**  
*     @fn int set_sdk_event_handler( user_call_back handler );
*     @brief  Set callback function handler. When data from Guidance comes, it will be called by data transfer thread.
*     @param `handler` function pointer to callback function.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int set_sdk_event_handler( user_call_back handler );

/**  
*     @fn int get_stereo_cali( stereo_cali* stereo_cali_pram);
*     @brief Get stereo calibration parameters.
*	  @param `stereo_cali_pram` Array of calibration parameters for all sensors. 
*	Will be set after calling the function.
*     @return `error code`. Non-zero if error occurs.
*/
SDK_API int get_stereo_cali( stereo_cali stereo_cali_pram[CAMERA_PAIR_NUM]);

/**     
*	@fn int get_online_status(int online_status[CAMERA_PAIR_NUM]);
*	@brief Get the online status of Guidance sensors.
*	@param `online_status` Array of online status for all sensors.
*	@return  `error code`. Non-zero if error occurs.
*/
SDK_API int get_online_status(int online_status[CAMERA_PAIR_NUM]);

/**     
*	@fn int get_image_size(int* width, int* height);
*	@brief Get the image size.
*	@param `width` width of image.
*	@param `height` height of image.
*	@return  `error code`. Non-zero if error occurs.
*/
SDK_API	int get_image_size(int* width, int* height);

/**     
*	@fn int wait_for_board_ready();
*	@brief Wait for board ready signal. This function waits 20 seconds for Guidance board to get started. If 20
*  seconds pass and the board is still not ready, return a timeout error code.
*  The users usually do not need to use this function, as it is already called 
*  in init_transfer.
*	@return `error code`. Zero if succeed, otherwise e_timout.
*/
SDK_API int wait_for_board_ready();

/**     
*	@fn int set_exposure_param( auto_exp_param param[CAMERA_PAIR_NUM] );
*	@brief Set exposure mode and parameters.
*	@param `param` pointer of exposure parameter struct.
*	@return  `error code`. Non-zero if error occurs.
*/
SDK_API int set_exposure_param( exposure_param *param );

#endif
