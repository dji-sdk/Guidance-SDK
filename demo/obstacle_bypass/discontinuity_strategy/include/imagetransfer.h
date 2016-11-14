#ifndef __IMAGE_TRANSFER_H__
#define __IMAGE_TRANSFER_H__
#include "usb_transfer.h"
#include "DJI_guidance.h"

#define  LOWEST_VERSION 2
#define  PC_VERSION 2

typedef void (*update_dialog_handler)( char **camera_pair_data[], char **other_data );

SDK_API void set_can_protocal( int nProtocal );

SDK_API void set_save_file_flag( bool bSaveFlag );

SDK_API int get_online_flag( int *online_flag );

SDK_API void set_update_dialog_handler( update_dialog_handler handler );

SDK_API int write_cmos( unsigned int *p_address_value, unsigned int un_address_num );

SDK_API int send_data( unsigned int id, char *data, unsigned int length );

SDK_API int get_data( unsigned int id, char *data, unsigned int max_length );

//DLL_API int get_config( char *data, unsigned int max_length );

SDK_API int push_file( const char *local_file_name, const char *remote_file_path );

SDK_API int push_file( const char *content, unsigned int file_len, const char *remote_file_full_name );

SDK_API int pull_file( const char *remote_file_name, const char *local_file_path );

SDK_API void set_register_param( unsigned int *register_addr, unsigned int *p_register_, unsigned int un_register_num );

SDK_API int set_selected_datetype( unsigned int *p_datatype_index, unsigned int un_datatype_num );

SDK_API int select_image( unsigned int camera_pair_index, unsigned int data_type );

SDK_API int set_cmos_read_address( unsigned int *p_address, unsigned int un_address_num );

SDK_API int suspend_image_transfer();

SDK_API int start_image_transfer();

SDK_API int init_image_transfer();

SDK_API int release_image_transfer();

SDK_API bool is_libusb_connected();

#ifdef WIN32
SDK_API int open_console();
#endif

SDK_API int get_soc_version();

SDK_API int get_vision_version( int &vision_version );

SDK_API int upgrade_sonar( unsigned int direction );

SDK_API int reboot_fastboot();

SDK_API int start_can_uart_test();

SDK_API int get_can_uart_test_result();

SDK_API int set_left_work_time( int left_work_time );

SDK_API int upgrade_soc_version();

SDK_API int set_sonar_serial_code( unsigned int logic_direction, const unsigned char *serial_code );

SDK_API int get_sonar_serial_code( unsigned int logic_direction, unsigned char *serial_code );

SDK_API int set_oa_manu_test_flag();

SDK_API int restart_master();

SDK_API int reboot_soc();

SDK_API int get_sonar_version( unsigned int logic_direction, char *hardware_version, int &loader_version, int &app_version );

SDK_API int set_event_handler( user_call_back handler );

SDK_API int save_cali_param( unsigned int logic_direction, char *cali_data, unsigned int data_len );

SDK_API int send_heartbeat_request();

SDK_API int set_manu_test_flag( int flag );

SDK_API void set_sdk_protocal( int protocal_version );

SDK_API int set_serial_code( const char *product_id, const char *code );

SDK_API int get_serial_code( char *code );

SDK_API int enable_adb( bool enable_flag );

SDK_API int enable_mvo( bool enable_flag );

SDK_API int get_token();

SDK_API int get_app_id();

#endif
