/*
 ============================================================================
 Name        : dji_sdk_node.cpp
 Author      : Mario Chris
 Description : Obstacle Avoidance
 ============================================================================
 */

/* Onboard */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>
#include "DJI_Pro_Codec.h"
#include "DJI_Pro_Hw.h"
#include "DJI_Pro_Link.h"
#include "DJI_Pro_App.h"

/* Guidance */
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
#include "DJI_guidance.h"
#include "DJI_utility.h"
#include "imagetransfer.h"
#include "usb_transfer.h"
using namespace cv;
using namespace std;

/* additional includes */
#include <string.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <ctime>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <thread>

/* enc_key */
char *key;
/* req_id for nav closed by app msg */
req_id_t nav_force_close_req_id = {0};
/* std msg from uav */
sdk_std_msg_t recv_sdk_std_msgs = {0};
/* ros launch param */
std::string	serial_name;
int			baud_rate;
int			app_id;
int			app_api_level;
int			app_version;
std::string	app_bundle_id;
std::string	enc_key;
/* activation */
activation_data_t activation_msg = {14,2,1,""};
bool cmd_complete = false;

/* parameter */
#define TAKEOFF			(uint8_t) 4
#define LAND			(uint8_t) 6
#define WIDTH			320
#define HEIGHT			240
#define IMAGE_SIZE		(HEIGHT * WIDTH)
#define VBUS			e_vbus1
#define RETURN_IF_ERR(err_code) { if( err_code ){ release_transfer(); printf( "error code:%d,%s %d\n", err_code, __FILE__, __LINE__ );}}

/* guidance */
int			err_code;
Mat     	g_greyscale_image_left=Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
Mat 		g_greyscale_image_right=Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
int			iter = 0;
DJI_lock    g_lock;
DJI_event   g_event;

// Shi-Tomasi
#define MIN_CORNERS (uint) 15
#define CORNER_THRESHOLD (uint) 95
#define MAX_CORNERS (uint) 100
#define QUALITY_LEVEL (double) 0.01
#define MIN_DISTANCE (double) 1

// Lucas-Kanade
int frame_num = 0;
vector<Point2f> prevleftpts, prevrightpts;
Mat prevleftimg, prevrightimg;
#define OF_MARGIN (double) 0.5 // 2 optical flow values considered different if their difference exceeds this margin
bool l_kpt_regen = true;
bool r_kpt_regen = true;

// control
#define CMD_FLAG 0x4A // control horizontal/vertical velocity in body frame and yaw rate in ground frame
#define FWD (double) 0.5 // constant horizontal velocity
#define TURN (double) 10 // constant yaw rate
#define ALT (double) 0.01 // constant vertical velocity
double l_fwd = FWD; // left image forward control
double l_turn = 0; // left image turn control
double l_alt = 0; // left image altitude control
double r_fwd = FWD; // right image forward control
double r_turn = 0; // right image turn control
double r_alt = 0; // right image altitude control
double turn_prev = 0; // previous yaw for weighted camera observation

/*************************************/

/*
  * table of sdk req data handler
  */
int16_t sdk_std_msgs_handler(uint8_t cmd_id,uint8_t* pbuf,uint16_t len,req_id_t req_id);
int16_t	nav_force_close_handler(uint8_t cmd_id,uint8_t* pbuf,uint16_t len,req_id_t req_id);
/* cmd id table */
cmd_handler_table_t cmd_handler_tab[] = 
{
	{0x00,sdk_std_msgs_handler				},
	{0x01,nav_force_close_handler			},
	{ERR_INDEX,NULL							}
};
/* cmd set table */
set_handler_table_t set_handler_tab[] =
{
	{0x02,cmd_handler_tab					},
	{ERR_INDEX,NULL							}
};

/*
  * sdk_req_data_callback
  */
int16_t nav_force_close_handler(uint8_t cmd_id,uint8_t* pbuf,uint16_t len,req_id_t req_id)
{
	if(len != sizeof(uint8_t))
		return -1;
	uint8_t msg;
	memcpy(&msg, pbuf, sizeof(msg));
	/* test session ack */
	nav_force_close_req_id.sequence_number = req_id.sequence_number;
	nav_force_close_req_id.session_id      = req_id.session_id;
	nav_force_close_req_id.reserve	       = 1;

	printf("WARNING nav close by app %d !!!!!!!!!!!!!! \n", msg);
	return 0;

}

#define _recv_std_msgs(_flag, _enable, _data, _buf, _datalen) \
	if( (_flag & _enable))\
	{\
		memcpy((uint8_t *)&(_data),(uint8_t *)(_buf)+(_datalen), sizeof(_data));\
		_datalen += sizeof(_data);\
	}

int16_t sdk_std_msgs_handler(uint8_t cmd_id,uint8_t* pbuf,uint16_t len,req_id_t req_id)
{
	uint16_t *msg_enable_flag = (uint16_t *)pbuf;
	uint16_t data_len = MSG_ENABLE_FLAG_LEN;
	
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_TIME	, recv_sdk_std_msgs.time_stamp			, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_Q		, recv_sdk_std_msgs.q				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_A		, recv_sdk_std_msgs.a				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_V		, recv_sdk_std_msgs.v				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_W		, recv_sdk_std_msgs.w				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_POS	, recv_sdk_std_msgs.pos				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_MAG	, recv_sdk_std_msgs.mag				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_RC		, recv_sdk_std_msgs.rc				, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_GIMBAL	, recv_sdk_std_msgs.gimbal			, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_STATUS	, recv_sdk_std_msgs.status			, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_BATTERY	, recv_sdk_std_msgs.battery_remaining_capacity	, pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_DEVICE	, recv_sdk_std_msgs.ctrl_device			, pbuf, data_len);

	return 0;
}

/* test cmd agency */
uint8_t test_cmd_send_flag = 1;
uint8_t test_cmd_is_resend = 0;
void cmd_callback_test_fun(uint16_t *ack)
{
	char result[6][50]={{"REQ_TIME_OUT"},{"REQ_REFUSE"},{"CMD_RECIEVE"},{"STATUS_CMD_EXECUTING"},{"STATUS_CMD_EXE_FAIL"},{"STATUS_CMD_EXE_SUCCESS"}};
	uint16_t recv_ack = *ack;
	printf("[DEBUG] recv_ack %#x \n", recv_ack);
	printf("[TEST_CMD] Cmd result: %s \n", *(result+recv_ack));
	test_cmd_send_flag = 1;
	if(recv_ack != STATUS_CMD_EXE_SUCCESS)
	{
		test_cmd_is_resend = 1;
	}

	/* for debug */
	if(recv_ack != STATUS_CMD_EXE_SUCCESS)
	{
		test_cmd_send_flag  = 0;
		printf("[ERROR] APP LAYER NOT STATUS_CMD_EXE_SUCCESS !!!!!!!!!!!!!!!!!!\n");
	}	
	cmd_complete = true;
	printf("Completed Maneuver...\n");
} 

/* test activation */
void test_activation_ack_cmd_callback(ProHeader *header)
{
	uint16_t ack_data;
	printf("Sdk_ack_cmd0_callback,sequence_number=%d,session_id=%d,data_len=%d\n", header->sequence_number, header->session_id, header->length - EXC_DATA_SIZE);
	memcpy((uint8_t *)&ack_data,(uint8_t *)&header->magic, (header->length - EXC_DATA_SIZE));

	if( is_sys_error(ack_data))
	{
		printf("[DEBUG] SDK_SYS_ERROR!!! \n");
	}
	else
	{
		char result[][50]={{"ACTIVATION_SUCCESS"},{"PARAM_ERROR"},{"DATA_ENC_ERROR"},{"NEW_DEVICE_TRY_AGAIN"},{"DJI_APP_TIMEOUT"},{" DJI_APP_NO_INTERNET"},{"SERVER_REFUSED"},{"LEVEL_ERROR"}};
		printf("[ACTIVATION] Activation result: %s \n", *(result+ack_data));
		if(ack_data == 0)
		{
			Pro_Config_Comm_Encrypt_Key(key);
			printf("[ACTIVATION] set key %s\n",key);
		}
	}
	cmd_complete = true;
	printf("Completed Activation...\n");
}

void test_activation(void)
{
	App_Send_Data( 2, 0, MY_ACTIVATION_SET, API_USER_ACTIVATION,(uint8_t*)&activation_msg,sizeof(activation_msg), test_activation_ack_cmd_callback, 1000, 1);
	printf("[ACTIVATION] send acticition msg: %d %d %d %d \n", activation_msg.app_id, activation_msg.app_api_level, activation_msg.app_ver ,activation_msg.app_bundle_id[0]);
}

void sdk_ack_nav_open_close_callback(ProHeader *header)
{
	uint16_t ack_data;
	printf("call %s\n",__func__);
	printf("Recv ACK,sequence_number=%d,session_id=%d,data_len=%d\n", header->sequence_number, header->session_id, header->length - EXC_DATA_SIZE);
	memcpy((uint8_t *)&ack_data,(uint8_t *)&header->magic, (header->length - EXC_DATA_SIZE));

	if( is_sys_error(ack_data))
	{
		printf("[DEBUG] SDK_SYS_ERROR!!! \n");
	}
	cmd_complete = true;
	printf("Completed API Control...\n");
}

// onboard monitor command set
void monitor()
{
	uint8_t pbuf;
	uint16_t *msg_enable_flag = (uint16_t *)(&pbuf);
	uint16_t data_len = MSG_ENABLE_FLAG_LEN;
	
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_TIME	, recv_sdk_std_msgs.time_stamp			, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_Q		, recv_sdk_std_msgs.q				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_A		, recv_sdk_std_msgs.a				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_V		, recv_sdk_std_msgs.v				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_W		, recv_sdk_std_msgs.w				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_POS	, recv_sdk_std_msgs.pos				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_MAG	, recv_sdk_std_msgs.mag				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_RC		, recv_sdk_std_msgs.rc				, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_GIMBAL	, recv_sdk_std_msgs.gimbal			, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_STATUS	, recv_sdk_std_msgs.status			, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_BATTERY	, recv_sdk_std_msgs.battery_remaining_capacity	, &pbuf, data_len);
	_recv_std_msgs( *msg_enable_flag, ENABLE_MSG_DEVICE	, recv_sdk_std_msgs.ctrl_device			, &pbuf, data_len);
}

// maneuvering
void maneuver(int ctrl_flag, double roll_or_x, double pitch_or_y, double thr_z, double yaw)
{
	api_ctrl_without_sensor_data_t motion = {0}; // initialize motion commands
	motion.ctrl_flag = ctrl_flag;
	motion.roll_or_x = roll_or_x;
	motion.pitch_or_y = pitch_or_y;
	motion.thr_z = thr_z;
	motion.yaw = yaw;
	while(true)
	{
		if(cmd_complete)
		{
			cmd_complete = false;
			App_Send_Data(0, 0, MY_CTRL_CMD_SET, API_CTRL_REQUEST, (uint8_t*)&motion, sizeof(motion), NULL, 0, 0); // send command
			cmd_complete = true;
			break;
		}
	}
}

/*
 * guidance callback function
 */

int guidance_callback(int data_type, int data_len, char *content)
{
	g_lock.enter();
	if (e_image == data_type && NULL != content)
	{
		printf("image data...................\n");
		image_data data;
		memcpy( (char*)&data, content, sizeof(data) );
		printf( "frame index:%d,stamp:%d\n", data.frame_index, data.time_stamp );
		for ( int d = 0; d < CAMERA_PAIR_NUM; ++d )
		{
			if ( data.m_greyscale_image_left[d] )
			{
				memcpy( g_greyscale_image_left.data, data.m_greyscale_image_left[d], IMAGE_SIZE );//maybe select too many image so just overwrite it

				// control text
				string ctrl_text = "";

				// potential obstacle display
				vector<Point2f> kpts;
				vector<double> kpt_of;
				double left_avg, right_avg, up_avg, down_avg;

				/* Lucas-Kanade */

				if(prevleftpts.size() < MIN_CORNERS)
				{
					// control decision
					ctrl_text = "STOP MOVING";
					l_fwd = 0;
					l_turn = 0;
					l_alt = 0;
				}
				else if(frame_num > 0)
				{
					vector<Point2f> LKleftpts;
					vector<uchar> status;
					vector<float> err;
					vector<Mat> prevleftpyr;
					buildOpticalFlowPyramid(prevleftimg, prevleftpyr, Size(21, 21), 3);
					vector<Mat> currleftpyr;
					buildOpticalFlowPyramid(g_greyscale_image_left, currleftpyr, Size(21, 21), 3);
					calcOpticalFlowPyrLK(prevleftpyr, currleftpyr, prevleftpts, LKleftpts, status, err, Size(21, 21), 3, TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), OPTFLOW_LK_GET_MIN_EIGENVALS);
				
					// control decision
					ctrl_text = "GO";
					double left_of_x = 0, left_of_y = 0, right_of_x = 0, right_of_y = 0, up_of_x = 0, up_of_y = 0, down_of_x = 0, down_of_y = 0;
					int left_count = 0, right_count = 0, up_count = 0, down_count = 0;
					for(size_t i = 0; i < LKleftpts.size(); i++)
					{
						if(LKleftpts[i].x >= 0 && LKleftpts[i].x <= WIDTH && LKleftpts[i].y >= 0 && LKleftpts[i].y <= HEIGHT)
						{
							if(LKleftpts[i].x < (double)WIDTH/2.0)
							{
								left_of_x += LKleftpts[i].x - prevleftpts[i].x;
								left_of_y += LKleftpts[i].y - prevleftpts[i].y;
								left_count++;
							}
							else
							{
								right_of_x += LKleftpts[i].x - prevleftpts[i].x;
								right_of_y += LKleftpts[i].y - prevleftpts[i].y;
								right_count++;
							}
							if(LKleftpts[i].y < (double)HEIGHT/2.0)
							{
								up_of_x += LKleftpts[i].x - prevleftpts[i].x;
								up_of_y += LKleftpts[i].y - prevleftpts[i].y;
								up_count++;
							}
							else
							{
								down_of_x += LKleftpts[i].x - prevleftpts[i].x;
								down_of_y += LKleftpts[i].y - prevleftpts[i].y;
								down_count++;
							}

							kpts.push_back(LKleftpts[i]);
							kpt_of.push_back(pow(pow(LKleftpts[i].x - prevleftpts[i].x, 2) + pow(LKleftpts[i].y - prevleftpts[i].y, 2), 0.5));
						}
					}
					left_of_x /= (double)left_count;
					left_of_y /= (double)left_count;
					double left_of = pow(pow(left_of_x, 2) + pow(left_of_y, 2), 0.5);
					right_of_x /= (double)right_count;
					right_of_y /= (double)right_count;
					double right_of = pow(pow(right_of_x, 2) + pow(right_of_y, 2), 0.5);
					up_of_x /= (double)up_count;
					up_of_y /= (double)up_count;
					double up_of = pow(pow(up_of_x, 2) + pow(up_of_y, 2), 0.5);
					down_of_x /= (double)down_count;
					down_of_y /= (double)down_count;
					double down_of = pow(pow(down_of_x, 2) + pow(down_of_y, 2), 0.5);
					// move away from high image section optical flow
					l_fwd = FWD;
					double turn_prev = l_turn;
					double alt_prev = l_alt;
					l_turn = 0;
					l_alt = 0;
					if(1.0 - right_of/left_of > OF_MARGIN)
					{
						ctrl_text += " RIGHT";
						if(turn_prev > 0) l_turn = (1 + (left_of - right_of)/(left_of + right_of))*turn_prev;
						else l_turn = TURN;
					}
					else if(1.0 - left_of/right_of > OF_MARGIN)
					{
						ctrl_text += " LEFT";
						if(turn_prev < 0) l_turn = (1 + (right_of - left_of)/(right_of + left_of))*turn_prev;
						else l_turn = -1.0*TURN;
					}
					if(1.0 - down_of/up_of > OF_MARGIN)
					{
						ctrl_text += " DOWN";
						if(alt_prev < 0) l_alt = (1 + (up_of - down_of)/(up_of + down_of))*alt_prev;
						else l_alt = -1.0*ALT;
					}
					else if(1.0 - up_of/down_of > OF_MARGIN)
					{
						ctrl_text += " UP";
						if(alt_prev > 0) l_alt = (1 + (down_of - up_of)/(down_of + up_of))*alt_prev;
						else l_alt = ALT;
					}
					if(ctrl_text.compare("GO") == 0) ctrl_text += " STRAIGHT";

					left_avg = left_of;
					right_avg = right_of;
					up_avg = up_of;
					down_avg = down_of;
				}

				/* Shi-Tomasi */

				vector<Point2f> left_corners;
				if(frame_num > 0)
				{
					if(kpts.size() >= CORNER_THRESHOLD)
					{
						left_corners.assign(kpts.begin(), kpts.end()); // retain previously tracked keypoints
						l_kpt_regen = false;
					}
					else
					{
						goodFeaturesToTrack(g_greyscale_image_left, left_corners, MAX_CORNERS, QUALITY_LEVEL, MIN_DISTANCE); // generate new keypoints
						l_kpt_regen = true;
					}
				}

				// allow for color overlaid on greyscale image
				Mat left_rgb(g_greyscale_image_left.size(), CV_8UC3);
				cvtColor(g_greyscale_image_left, left_rgb, CV_GRAY2RGB);
				// keypoints display
				for(size_t i = 0; i < left_corners.size(); i++)
				{
					circle(left_rgb, left_corners[i], 4, Scalar(255, 0, 0), -1);
				}
				g_greyscale_image_left.copyTo(prevleftimg);
				prevleftpts.assign(left_corners.begin(), left_corners.end());
				// potential obstacle display
				for(size_t i = 0; i < kpts.size(); i++)
				{
					bool draw_obstacle = false;
					if(kpts[i].x < (double)WIDTH/2.0)
					{
						if(1.0 - right_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					else
					{
						if(1.0 - left_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					if(kpts[i].y < (double)HEIGHT/2.0)
					{
						if(1.0 - down_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					else
					{
						if(1.0 - up_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					if(draw_obstacle) rectangle(left_rgb, Point2f(kpts[i].x - 5, kpts[i].y - 5), Point2f(kpts[i].x + 5, kpts[i].y + 5), Scalar(0, 0, 255), -1);
				}
				// control display
				putText(left_rgb, ctrl_text, Point2f(10, HEIGHT - 10), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 3, 3);
				
				imshow("left", left_rgb);
				moveWindow("left", 0, 0);
			}
			if ( data.m_greyscale_image_right[d] )
			{
				memcpy( g_greyscale_image_right.data, data.m_greyscale_image_right[d], IMAGE_SIZE );

				// control text
				string ctrl_text = "";

				// potential obstacle display
				vector<Point2f> kpts;
				vector<double> kpt_of;
				double left_avg, right_avg, up_avg, down_avg;

				/* Lucas-Kanade */
				
				if(prevrightpts.size() < MIN_CORNERS)
				{
					// control decision
					ctrl_text = "STOP MOVING";
					r_fwd = 0;
					r_turn = 0;
					r_alt = 0;
				}
				else if(frame_num > 0)
				{
					vector<Point2f> LKrightpts;
					vector<uchar> status;
					vector<float> err;
					vector<Mat> prevrightpyr;
					buildOpticalFlowPyramid(prevrightimg, prevrightpyr, Size(21, 21), 3);
					vector<Mat> currrightpyr;
					buildOpticalFlowPyramid(g_greyscale_image_right, currrightpyr, Size(21, 21), 3);
					calcOpticalFlowPyrLK(prevrightpyr, currrightpyr, prevrightpts, LKrightpts, status, err, Size(21, 21), 3, TermCriteria(TermCriteria::COUNT+TermCriteria::EPS, 30, 0.01), OPTFLOW_LK_GET_MIN_EIGENVALS);
				
					// control decision
					ctrl_text = "GO";
					double left_of_x = 0, left_of_y = 0, right_of_x = 0, right_of_y = 0, up_of_x = 0, up_of_y = 0, down_of_x = 0, down_of_y = 0;
					int left_count = 0, right_count = 0, up_count = 0, down_count = 0;
					for(size_t i = 0; i < LKrightpts.size(); i++)
					{
						if(LKrightpts[i].x >= 0 && LKrightpts[i].x <= WIDTH && LKrightpts[i].y >= 0 && LKrightpts[i].y <= HEIGHT)
						{
							if(LKrightpts[i].x < (double)WIDTH/2.0)
							{
								left_of_x += LKrightpts[i].x - prevrightpts[i].x;
								left_of_y += LKrightpts[i].y - prevrightpts[i].y;
								left_count++;
							}
							else
							{
								right_of_x += LKrightpts[i].x - prevrightpts[i].x;
								right_of_y += LKrightpts[i].y - prevrightpts[i].y;
								right_count++;
							}
							if(LKrightpts[i].y < (double)HEIGHT/2.0)
							{
								up_of_x += LKrightpts[i].x - prevrightpts[i].x;
								up_of_y += LKrightpts[i].y - prevrightpts[i].y;
								up_count++;
							}
							else
							{
								down_of_x += LKrightpts[i].x - prevrightpts[i].x;
								down_of_y += LKrightpts[i].y - prevrightpts[i].y;
								down_count++;
							}

							kpts.push_back(LKrightpts[i]);
							kpt_of.push_back(pow(pow(LKrightpts[i].x - prevrightpts[i].x, 2) + pow(LKrightpts[i].y - prevrightpts[i].y, 2), 0.5));
						}
					}
					left_of_x /= (double)left_count;
					left_of_y /= (double)left_count;
					double left_of = pow(pow(left_of_x, 2) + pow(left_of_y, 2), 0.5);
					right_of_x /= (double)right_count;
					right_of_y /= (double)right_count;
					double right_of = pow(pow(right_of_x, 2) + pow(right_of_y, 2), 0.5);
					up_of_x /= (double)up_count;
					up_of_y /= (double)up_count;
					double up_of = pow(pow(up_of_x, 2) + pow(up_of_y, 2), 0.5);
					down_of_x /= (double)down_count;
					down_of_y /= (double)down_count;
					double down_of = pow(pow(down_of_x, 2) + pow(down_of_y, 2), 0.5);
					// move away from high image section optical flow
					r_fwd = FWD;
					double turn_prev = r_turn;
					double alt_prev = r_alt;
					r_turn = 0;
					r_alt = 0;
					if(1.0 - right_of/left_of > OF_MARGIN)
					{
						ctrl_text += " RIGHT";
						if(turn_prev > 0) r_turn = (1 + (left_of - right_of)/(left_of + right_of))*turn_prev;
						else r_turn = TURN;
					}
					else if(1.0 - left_of/right_of > OF_MARGIN)
					{
						ctrl_text += " LEFT";
						if(turn_prev < 0) r_turn = (1 + (right_of - left_of)/(right_of + left_of))*turn_prev;
						else r_turn = -1.0*TURN;
					}
					if(1.0 - down_of/up_of > OF_MARGIN)
					{
						ctrl_text += " DOWN";
						if(alt_prev < 0) r_alt = (1 + (up_of - down_of)/(up_of + down_of))*alt_prev;
						else r_alt = -1.0*ALT;
					}
					else if(1.0 - up_of/down_of > OF_MARGIN)
					{
						ctrl_text += " UP";
						if(alt_prev > 0) r_alt = (1 + (down_of - up_of)/(down_of + up_of))*alt_prev;
						else r_alt = ALT;
					}
					if(ctrl_text.compare("GO") == 0) ctrl_text += " STRAIGHT";

					left_avg = left_of;
					right_avg = right_of;
					up_avg = up_of;
					down_avg = down_of;
				}

				/* Shi-Tomasi */
				
				vector<Point2f> right_corners;
				if(frame_num > 0)
				{
					if(kpts.size() >= CORNER_THRESHOLD)
					{
						right_corners.assign(kpts.begin(), kpts.end()); // retain previously tracked keypoints
						r_kpt_regen = false;
					}
					else
					{
						goodFeaturesToTrack(g_greyscale_image_right, right_corners, MAX_CORNERS, QUALITY_LEVEL, MIN_DISTANCE); // generate new keypoints
						r_kpt_regen = true;
					}
				}

				// allow for color overlaid on greyscale image
				Mat right_rgb(g_greyscale_image_right.size(), CV_8UC3);
				cvtColor(g_greyscale_image_right, right_rgb, CV_GRAY2RGB);
				// keypoints display
				for(size_t i = 0; i < right_corners.size(); i++)
				{
					circle(right_rgb, right_corners[i], 4, Scalar(255, 0, 0), -1);
				}
				g_greyscale_image_right.copyTo(prevrightimg);
				prevrightpts.assign(right_corners.begin(), right_corners.end());
				// potential obstacle display
				for(size_t i = 0; i < kpts.size(); i++)
				{
					bool draw_obstacle = false;
					if(kpts[i].x < (double)WIDTH/2.0)
					{
						if(1.0 - right_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					else
					{
						if(1.0 - left_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					if(kpts[i].y < (double)HEIGHT/2.0)
					{
						if(1.0 - down_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					else
					{
						if(1.0 - up_avg/kpt_of[i] > OF_MARGIN) draw_obstacle = true;
					}
					if(draw_obstacle) rectangle(right_rgb, Point2f(kpts[i].x - 5, kpts[i].y - 5), Point2f(kpts[i].x + 5, kpts[i].y + 5), Scalar(0, 0, 255), -1);
				}
				// control display
				putText(right_rgb, ctrl_text, Point2f(10, HEIGHT - 10), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 255, 0), 3, 3);

				imshow("right", right_rgb);
				moveWindow("right", 500, 0);

				frame_num++;
			}
		}

		waitKey(1);

		// maneuver
		if(ctrl_strat != NONE)
		{
			double x = (l_fwd + r_fwd)/2.0;
			double y = 0;
			double z = (l_alt + r_alt)/2.0;
			#ifdef NO_ALT_CTRL
				z = 0;
			#endif
			double yaw = (l_turn + r_turn)/2.0;
			maneuver(CMD_FLAG, x, y, z, yaw);
		}
	}

	g_lock.leave();
	g_event.set_DJI_event();

	return 0;
}

// opening and closing api
void nav_open_close(uint8_t open_close, char *task)
{
	uint8_t send_data = open_close;
	while(true)
	{
		if(cmd_complete)
		{
			printf("\n%s\n", task);
			cmd_complete = false;
			App_Send_Data(1, 1, MY_CTRL_CMD_SET, API_OPEN_SERIAL, (uint8_t *)&send_data, sizeof(send_data), sdk_ack_nav_open_close_callback,  1000, 0); // send command
			break;
		}
	}
}

// takeoff and landing
void take_off_land(uint8_t send_data, char *task)
{
	while(true)
	{
		if(cmd_complete)
		{
			printf("\n%s\n", task);
			cmd_complete = false; // reset cmd_complete
			App_Complex_Send_Cmd(send_data, cmd_callback_test_fun); // send command
			break;
		}
	}
}

// run mission
void run()
{
	/* Takeoff */
	test_activation(); // activate
	nav_open_close(1, (char *)"Opening API Control..."); // open api
	take_off_land(TAKEOFF, (char *)"Taking off..."); // take off

	/* Maneuvering and avoiding obstacles */
	usleep(15000000); // pause 15 seconds
	printf("\nBeginning Obstacle Avoidance...\n");
	err_code = start_transfer(); // start guidance data collection
	RETURN_IF_ERR( err_code );

	/* Landing */
	getchar(); // press key to begin autonomous mode
	printf("\nEnding Obstacle Avoidance...\n");
	err_code = stop_transfer(); // stop guidance
	RETURN_IF_ERR( err_code );
	monitor();
	while(recv_sdk_std_msgs.pos.height > 0) // check if drone reached ground level yet
	{
		take_off_land(LAND, (char *)"Landing..."); // land
		monitor();
	}
	nav_open_close(0, (char *)"Closing API Control..."); // close api
}

/*
  * main_function
  */
int main (int argc, char** argv)
{
	/* Onboard */

	printf("Test SDK Protocol demo\n");

	serial_name = std::string("/dev/ttyUSB0");
	baud_rate = 230400;
	app_id = 1010572;
	app_api_level = 2;
	app_version = 1;
	app_bundle_id = std::string("12345678901234567890123456789012");
	enc_key = std::string("ca5aed46d675076dd100ec73a8d3b8d3dbeea66392c77af62ac65cf9b5be8520");

	activation_msg.app_id 		= (uint32_t)app_id;
	activation_msg.app_api_level 	= (uint32_t)app_api_level;
	activation_msg.app_ver		= (uint32_t)app_version;
	memcpy(activation_msg.app_bundle_id, app_bundle_id.c_str(), 32);
	
	key = (char*)enc_key.c_str();
	
	printf("[INIT] SET serial_port	: %s \n", serial_name.c_str());
	printf("[INIT] SET baud_rate	: %d \n", baud_rate);
	printf("[INIT] ACTIVATION INFO	: \n");
	printf("[INIT] 	  app_id     	  %d \n", activation_msg.app_id);
	printf("[INIT]    app_api_level	  %d \n", activation_msg.app_api_level);
	printf("[INIT]    app_version     %d \n", activation_msg.app_ver);
	printf("[INIT]    app_bundle_id	  %s \n", activation_msg.app_bundle_id);
	printf("[INIT]    enc_key	  %s \n", key);

	/* open serial port */
	Pro_Hw_Setup((char *)serial_name.c_str(),baud_rate);
	Pro_Link_Setup();
	App_Recv_Set_Hook(App_Recv_Req_Data);
	App_Set_Table(set_handler_tab, cmd_handler_tab);

	CmdStartThread();

	Pro_Config_Comm_Encrypt_Key(key);

	/* Guidance */

	reset_config();
	err_code = init_transfer();
	RETURN_IF_ERR( err_code );

	err_code = select_greyscale_image( VBUS, true );
	RETURN_IF_ERR( err_code );
	err_code = select_greyscale_image( VBUS, false );
	RETURN_IF_ERR( err_code );

	err_code = set_sdk_event_handler( guidance_callback ); // set guidance callback
	RETURN_IF_ERR( err_code );
	
	/* Mission */
	
	printf("\nRunning Mission...\n");
	run();

	//make sure the ack packet from GUIDANCE is received
	sleep( 1000000 );
	err_code = release_transfer();
	RETURN_IF_ERR( err_code );

	return 0;
}