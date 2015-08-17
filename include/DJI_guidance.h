/** @file  DJI_guidance.h
* 
* @brief Define data struct & data type from DJI_guidance system.
*
*This file define data struct & data type from DJI_guidance system.
*
* @version 1.0.0
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
#define APP_ID_LENGTH        32
#define APP_NAME_LENGTH      45
#define KEY_LENGTH           32

/**
* @enum  e_sdk_err_code
* @brief Define error code of sdk
*/
enum e_sdk_err_code
{
	e_sdk_no_err = 0,
	e_load_libusb_err,
	e_sdk_not_inited,	// SDK software is not ready
	e_guidance_hardware_not_ready,  // must be 3 as Guidance Core will send 3 if the hardware is not ready
	e_disparity_not_allowed,//if work type is standard, disparity is not allowed to be selected
	e_image_frequency_not_allowed,  // image frequency must be one of the enum type e_image_data_frequecy
	e_config_not_ready,		//get config including the work type flag, before you can select data
	e_online_flag_not_ready, // online flag is not ready
	e_max_sdk_err = 100
};

/**
* @enum  e_vbus_index
* @brief Define logical direction of vbus
*/
enum e_vbus_index
{
	e_vbus1 = 1,	/**< logic direction of vbus */
	e_vbus2 = 2,	/**< logic direction of vbus */
	e_vbus3 = 3,	/**< logic direction of vbus */
	e_vbus4 = 4,	/**< logic direction of vbus */
	e_vbus5 = 0	    /**< logic direction of vbus */
};

/**
* @enum  e_image_data_frequecy
* @brief Define frequecy of image data
*/
enum e_image_data_frequecy
{
	e_frequecy_5 =  0,	/**< frequecy of image data */
	e_frequecy_10 = 1,	/**< frequecy of image data */
	e_frequecy_20 = 2	/**< frequecy of image data */
};

/**  
 *     @fn typedef int (*user_call_back)( int event_type, int data_len, char *content );
 *     @brief call back prototypes    
 *     @param event_type use it to identify the data:image,imu,ultrasonic,velocity or obstacle distance
 *     @param data_len length of the input data
 *     @data input data read from GUIDANCE
 *     @return   error code,if error occur,it will be non zero
 */
typedef int (*user_call_back)( int event_type, int data_len, char *data );

/**
* @enum  e_guidance_event
* @brief Define event type of callback
*/
enum e_guidance_event
{
	e_image = 0,	   	      	/**< called back when image comes */
	e_imu,	       	      	    /**< called back when imu comes */
	e_ultrasonic,        	    /**< called back when ultrasonic comes */
	e_velocity,	    	        /**< called back when velocity data comes */
	e_obstacle_distance,	    /**< called back when obstacle data comes */
	e_event_num
};

/**
*@struct  image_data
*@brief Define image data 
*/
typedef struct _image_data
{
	unsigned int     frame_index;	                                  /**< frame index */
	unsigned int     time_stamp;	                                  /**< time stamp of image captured in ms */
	char             *m_greyscale_image_left[CAMERA_PAIR_NUM];	      /**< greyscale image of left camera */
	char             *m_greyscale_image_right[CAMERA_PAIR_NUM];   	  /**< greyscale image of right camera */
	char             *m_depth_image[CAMERA_PAIR_NUM];	              /**< depth image in meters */
}image_data;

/**
*@struct  ultrasonic_data
*@brief Define ultrasonic data 
*/
typedef struct _ultrasonic_data
{
	unsigned int     frame_index;	                        /**< corresponse frame index */
	unsigned int     time_stamp;	                        /**< time stamp of corresponse image captured in ms */
	unsigned short   ultrasonic[CAMERA_PAIR_NUM];	        /**< distance in mm */
	unsigned short   reliability[CAMERA_PAIR_NUM];	        /**< reliability of the distance data */
}ultrasonic_data;

/**
*@struct  velocity
*@brief Define velocity
*/
typedef struct _velocity
{
	unsigned int     frame_index;	          /**< corresponse frame index */
	unsigned int     time_stamp;	          /**< time stamp of corresponse image captured in ms */
	short            vx;	                  /**< velocity of x in mm/s */
	short            vy;	                  /**< velocity of y in mm/s */
	short            vz;	                  /**< velocity of z in mm/s */
}velocity;

/**
*@struct  obstacle_distance
*@brief Define obstacle distance in cm
*/
typedef struct _obstacle_distance
{
	unsigned int     frame_index;	                /**< corresponse frame index */
	unsigned int     time_stamp;	                /**< time stamp of corresponse image captured in ms */
	unsigned short   distance[CAMERA_PAIR_NUM];     /**< distance of obstacle in cm */
}obstacle_distance;

/**
*@struct  imu
*@brief Define imu 
*/
typedef struct _imu
{
	unsigned int     frame_index;	          /**< corresponse frame index */
	unsigned int     time_stamp;	          /**< time stamp of corresponse image captured in ms */
	float            acc_x;	                  /**< acceleration of x m/s^2 */
	float            acc_y;	                  /**< acceleration of y m/s^2 */
	float            acc_z;	                  /**< acceleration of z m/s^2 */

	float            q[4];	                  /**< attitude data */
}imu;



/**  
 *     @fn int init_transfer( void );
 *     @brief init SDK 
 *     @return   error code,if error occur,it will be non zero
 */
SDK_API int init_transfer( void );

/**  
 *     @fn int start_transfer();
 *     @brief send message to GUIDANCE to start data transfer.  
 *     @return   error code,if error occur,it will be non zero
 */
SDK_API int start_transfer( void );

/**  
*     @fn int stop_transfer();
*     @brief send message to GUIDANCE to stop data transfer.    
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int stop_transfer( void );

/**  
*     @fn int release_transfer();
*     @brief release SDK.    
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int release_transfer( void );

/**  
*     @fn int set_sdk_event_handler( user_call_back handler );
*     @brief  set callback,when data from GUIDANCE comes,it will be called by transfer thread.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int set_sdk_event_handler( user_call_back handler );

/**  
*     @fn int reset_config();
*     @brief  reset subscribe configure,if you want to subscribe data different from last time.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int reset_config( void );

/**  
*     @fn int select_imu();
*     @brief  subscribe to imu.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API void select_imu( void );

/**  
*     @fn int select_ultrasonic();
*     @brief  subscribe to ultrasonic.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API void select_ultrasonic( void );

/**  
*     @fn int select_velocity();
*     @brief  subscribe to velocity data, i.e. velocity of GUIDANCE in body coordinate system.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API void select_velocity( void );

/**  
*     @fn int select_obstacle_distance();
*     @brief  subscribe to obstacle distance, i.e. distance from obstacle.
*     @return   error code,if error occur,it will be non zero
*/
SDK_API void select_obstacle_distance( void );

/**  
*     @fn int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left );
*     @brief  subscribe to rectified image.
*     @param camera_pair_index index of camera pair selected
*     @param is_left whether the image data selected is left
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int select_greyscale_image( e_vbus_index camera_pair_index, bool is_left );

/**  
*     @fn int select_depth_image( e_vbus_index camera_pair_index );
*     @brief  subscribe depth data.
*     @param camera_pair_index index of camera pair selected
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int select_depth_image( e_vbus_index camera_pair_index );

/**  
*     @fn int set_image_frequecy( e_image_data_frequecy frequecy );
*     @brief set frequecy of image transfer
*     Set the frequecy of image transfer. As the bandwidth of USB is limited,
*     if you subscribe too much images(greyscale image or depth image),the frequecy
*     should be relatively small,otherwise the SDK cannot guarantee the continuity of image transfer.
*     @param frequecy frequecy of image transfer
*     @return   error code,if error occur,it will be non zero
*/
SDK_API int set_image_frequecy( e_image_data_frequecy frequecy );

/**     
*	@fn int get_online_status(int online_status[CAMERA_PAIR_NUM]);
*	@brief Get the online status of Guidance sensors.
*	@return  error code. Zero if succeed, otherwise check the error code for failure reason.
*/
SDK_API int get_online_status(int online_status[CAMERA_PAIR_NUM]);

#endif
