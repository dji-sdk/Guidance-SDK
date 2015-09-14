/*************************************
VERSION :	6.0
CREATOR :	ganshuai
DATE	:	2014.11.19
*************************************/
#ifndef __USB_TRANSFER_H__
#define __USB_TRANSFER_H__
#include "DJI_guidance.h"

enum e_master_state
{
	e_master_offline = -1,
	e_master_init = 0,//rbf没加载好，也可以上报状态的，这样就可以区分到底是硬件问题还是rbf没加载好的问题了
	e_fpga_loaded = 1,
	e_master_started = 2,
	e_vo_ok = 3,
	e_oa_ok = 4
};

enum EDATA_TYPE
{
	e_original_left = 0,
	e_original_right,
	e_rect_left,
	e_rect_right,
	e_point_cloud,
	e_disparity1,
	e_disparity2,
	e_reserve2,
	e_reserve3,
	e_reserve4,
	e_reserve5,
	e_image_type_num = e_reserve5,///前半部分表示fpga和arm共享内存里面的图像数据种类
	e_register,
	e_imu_q,
	e_sonar,
	e_vo_output,
	e_bm_output,
	e_branch_info,
	e_real_time_imu,
	e_full_imu, //added by huxiao for testing
	e_uav_pose,
	///如果增加字段，一定要保证这个是最后一个，因为他是用来统计字段的数目的！！！！！！
	e_data_type_num
};

//pc 2 soc
enum EIMAGE_PC2SOC_TRANSFER
{
	e_selected_image = 0,
	e_select_data_type,
	e_cmos_read_address,
	e_cmos_write,
	e_request_version,
	e_stop,
	e_online_flag,
	e_select_imu,
	e_suspend,
	e_start,
	e_set_config,
	e_get_config,
	e_pull_file,
	e_push_file,
	e_select_vo_output,
	e_select_bm_output,
	e_select_branch_info,
	e_select_real_time_imu,
	e_vision_version,
	e_reboot_fastboot,
	e_upgrade_sonar,
	e_set_sonar_serial_code,
	e_can_uart_test,
	e_can_uart_test_result,
	e_set_left_work_time,
	e_upgrade_soc_version,
	e_get_sonar_serial_code,
	e_set_oa_manu_test_flag,
	e_restart_master,
	e_get_sonar_version,
	e_reboot_soc,
	e_save_cali_param,
	e_set_manu_test_flag,//生产测试模式下会关闭上电检测，在机械臂测试环节，把生产测试模式的配置项关闭
	e_request_heartbeat,
	e_set_sdk_image_frequecy,
	e_set_serial_code,
	e_get_serial_code,
	e_enable_adb,
	e_manu_enable_mvo,
	e_set_app_id,
	e_get_app_id,
	e_get_token,
	e_fpga_enc_type,
	e_export_debug_info
};

//soc 2 pc
enum EIMAGE_SOC2PC_TRANSFER
{
	//图像和小数据分开传输，用usb里面不同的通道，从而保证小数据可以实时传输
	e_soc_real_time_data = 0,
	e_soc_image,
	e_soc_imu,
	e_soc_pc_sonar,
	e_soc_register,
	e_soc_reply_version,
	e_soc_stop,
	e_soc_online_flag,
	e_soc_return_config,
	e_soc_suspend_ack,
	e_soc_pull_file_ack,
	e_soc_push_file_ack,
	e_soc_vo_output_ack,
	e_soc_bm_output_ack,
	e_soc_branch_ack,
	e_soc_real_time_imu_ack,
	e_soc_pc_vision_version,
	e_soc_full_imu,
	e_soc_pc_upgrade_sonar_ack,
//	e_soc_pc_online_flag,
	e_soc_pc_work_time_ack,
	e_soc_pc_upgrade_soc_version_ack,
	e_soc_pc_can_uart_test,
	e_soc_pc_can_uart_test_result,
	e_soc_pc_set_sonar_serial_code_ack,
	e_soc_pc_return_sonar_serial_code,
	e_soc_pc_set_oa_manu_test_flag_ack,
	e_soc_pc_restart_master_ack,
	e_soc_pc_sonar_version_ack,
	e_soc_pc_master_started,
	e_soc_pc_save_cali_param_ack,
	e_soc_pc_upgrade_progress,
	e_set_sdk_image_frequecy_ack,
	e_set_serial_code_ack,
	e_get_serial_code_ack,
	e_enable_adb_ack,
	e_manu_enable_mvo_ack,
	e_set_app_id_ack,
	e_get_app_id_ack,
	e_get_token_ack,
	e_soc_pc_fpga_enc_type,
	e_export_debug_info_ack,
	e_send_uav_pose,
	e_soc_pc_data_type_num
};

enum ESOCKET_STATUS
{
	e_init_socket = 0,
	e_recv_sync = 1,
	e_ack_sync = 3,
	//todoganshuai 可能并不需要严格的三次握手，只要发起握手的一方收到了ack，并且他在发起sync请求的时候，已经有接收线程正确地在等待ack了，则一旦收到ack，收发双方都可以认为已经正确握手上了
	e_ack_ack = 3,
	e_connected = 3,//e_ack_sync,e_ack_ack合并为e_connected
	e_recv_close = 4,
	e_ack_close = 5,
	e_ack_close_ack = 6
};

enum e_libusb_errcode
{
	e_libusb_ok = 0,
	e_version_err,
	e_init_usb_err,
	e_init_not_connected,
	e_param_err,
	e_data_type_err,
	e_wrong_length,
	e_pull_file_write_file_err,
	e_online_flag_not_ready
};

class camera_config
{
public:
	int                 m_sensor_active_flag;
	float               m_camera_to_body_translation_x;
	float               m_camera_to_body_translation_y;
	float               m_camera_to_body_translation_z;
	float               m_camera_to_body_rotation_roll;
	float               m_camera_to_body_rotation_pitch;
	float               m_camera_to_body_rotation_yaw;
	float               m_camera_exposure_time;
	float               m_max_camera_exposure_time;
	int                 m_camera_exposure_type;
	int                 m_camera_sensitivity;
	int                 m_expected_light_intensity;//期望亮度
	int                 m_image_selected_flag[e_image_type_num];
};

class config
{
public:
	int                 m_visual_sensor_work_type;//diy?
	int                 m_uav_body_type;
	int                 m_fly_controler_type;
	int                 m_is_obstacle_avoidiance_on;
	float               m_imu_to_body_translation_x;
	float               m_imu_to_body_translation_y;
	float               m_imu_to_body_translation_z;
	float               m_imu_to_body_rotation_roll;
	float               m_imu_to_body_rotation_pitch;
	float               m_imu_to_body_rotation_yaw;
	float               m_uav_body_size_x;
	float               m_uav_body_size_y;
	float               m_uav_body_size_z;
	float				m_safe_distance;
	float				m_max_speed_limit;
	float				m_speed_ratio;
	float				m_oa_attitude;
	int                 m_frame_rate;//image sensor frequecy
	int                 m_interface_enabled[2];//uart/usb
	int                 m_imu_frequecy[2];
	int                 m_sonar_frequecy[2];
	int                 m_vo_output_frequecy[2];
	int                 m_oa_output_frequecy[2];
	int                 m_image_frequecy;//output image frequecy
	int                 m_image_width;
	int                 m_image_height;
	camera_config       m_camera_config[CAMERA_PAIR_NUM];
	float               m_velo_level;
	int					m_cali_state[5];
	int                 m_reserved[194];
};

class soc2pc_vo_can_output
{
public:
	short cnt;

	short vx;
	short vy;
	short vz;

	float x;
	float y;
	float z;

	float uncertainty_location_1;
	float uncertainty_location_2;
	float uncertainty_location_3;
	float uncertainty_location_4;
	float uncertainty_location_5;
	float uncertainty_location_6;

	float uncertainty_velocity_1;
	float uncertainty_velocity_2;
	float uncertainty_velocity_3;
	float uncertainty_velocity_4;
	float uncertainty_velocity_5;
	float uncertainty_velocity_6;

	float height;
	float uncertainty_height;

	unsigned char reserve[4];
	unsigned int  m_frame_index;
	unsigned int  m_time_stamp;
	unsigned int  m_reserved[9];
};

class soc2pc_imu
{
public:
	double 	longti;
	double 	lati;
	float	alti;

	float acc_x;
	float acc_y;
	float acc_z;

	float gyro_x;	//wx
	float gyro_y;
	float gyro_z;

	float	press;	//0
	float   sonar;

	float	q0;
	float	q1;
	float	q2;
	float 	q3;

	float	ag_x;	//ax
	float	ag_y;
	float	ag_z;

	float	vg_x; //0
	float	vg_y;
	float	vg_z;

	float	gb_x; //0
	float	gb_y;
	float	gb_z;

	short	mx;	//0
	short	my;
	short	mz;
	short	temp[3];

	unsigned short sensor_overflow;
	unsigned short filter_status;
	unsigned short gps_svn;
	unsigned short cnt;
	unsigned int   m_frame_index;//与input里面的frameindex一样
	unsigned int   m_time_stamp;
	unsigned int   m_reserved[9];
};

class soc2pc_sonar_data
{
public:
	unsigned short m_sonar;
	unsigned short m_obstacle_reliability;
	unsigned short m_potential_obstacle_dis;//潜在障碍物距离
	unsigned short m_potential_obstacle_reliability;
	unsigned char  m_strongest_energy;
	unsigned char  m_second_strong_energy;
	unsigned char  m_nearest_energy;
	unsigned char  m_second_near_energy;
	unsigned char  m_blind_energy;
	unsigned char  m_flag;
	unsigned char  m_reserved;
	unsigned char  m_cnt;
	unsigned int   m_frame_index;
	unsigned int   m_time_stamp;
};

struct match_out1
{
	//vector3f	point3d;				//3D Information, 3 x 1 vector
	int		lu, lv;					//current frame left image (lu, lv)
	int		ru, rv;					//current frame right image (ru, rv)
	int		nu, nv;					//next frame image (u, v)
	float   x;
	float   y;
	float   z;
	//int16 		d;
};

class soc2pc_branch_info
{
public:
	short n_left_kpoint;
	short n_right_kpoint;
	short n_stereo_match;
	short n_frame_match;
	short n_inlier_match;
	short n_svn;

	int   flag;

	/* velocity(NEG) */
	float tgx;
	float tgy;
	float tgz;

	/* position(NEG) */
	float pgx;
	float pgy;
	float pgz;

	float disparity;

	float frev[2];
	match_out1 *stereo_match_point;
	int *frame_match_link;
	int *inlier_match_link;
	float cali_para[4];
	bool caled;
	unsigned int   m_frame_index;
	unsigned int   m_time_stamp;
};

#pragma pack(1)
/*class soc2pc_oa_avoid_output
{
public:
	int stop;
	int speed_limit;
	float object_distance;
	int reserved[4];
};*/

class soc2pc_oa_avoid_output
{
public:
	int reserved[4];
	short object_distance;
	bool stop;
	unsigned char speed_limit;
	bool fail_OBA;
	unsigned int   m_frame_index;
	unsigned int   m_time_stamp;
};
#pragma pack()

typedef struct _account_item
{
public:
	char        m_app_id[APP_ID_LENGTH+1];
	char        m_app_name[APP_NAME_LENGTH+1];
	char        m_key[KEY_LENGTH+1];
	int         m_activated;
}account_item;

typedef struct _uav_pose
{
	int cnt;

	float q0;
	float q1;
	float q2;
	float q3;
	int	  attitude_ok;

	float pg_x;
	float pg_y;
	float pg_z;
	int	  position_ok;

	float vg_x;
	float vg_y;
	float vg_z;
	int	  veclocity_ok;

	float	reserve_float[4];
	int		reserve_int[2];

	unsigned int   m_frame_index;
	unsigned int   m_time_stamp;
} uav_pose;

#endif
