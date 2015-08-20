#include <stdio.h>
#include <string.h>
#include <string>
#include "DJI_guidance.h"
#include "DJI_utility.h"

using namespace std;

#ifdef HAVE_OPENCV
#include "opencv2/video/tracking.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#endif


#define WIDTH 320
#define HEIGHT 240
#define IMAGE_SIZE (HEIGHT * WIDTH)

#define USE_GUIDANCE_ASSISTANT_CONFIG 0 //use GUIDANCE ASSISTANT's configure

#ifdef HAVE_OPENCV
using namespace cv;
Mat     g_greyscale_image_left(HEIGHT, WIDTH, CV_8UC1);
Mat		g_greyscale_image_right(HEIGHT, WIDTH, CV_8UC1);
Mat		g_depth(HEIGHT,WIDTH,CV_16SC1);
#else
char    g_greyscale_image_left[IMAGE_SIZE];
char	g_greyscale_image_right[IMAGE_SIZE];
char    g_depth[IMAGE_SIZE * 2];
#endif
e_vbus_index sensor_id = e_vbus1;

DJI_lock    g_lock;
DJI_event   g_event;
char		key = 0;

int my_callback(int data_type, int data_len, char *content)
{
	g_lock.enter();
	if (e_image == data_type && NULL != content)
	{
		image_data data;
		memcpy( (char*)&data, content, sizeof(data) );
		printf( "frame index:%d,stamp:%d\n", data.frame_index, data.time_stamp );
#if !USE_GUIDANCE_ASSISTANT_CONFIG
#ifdef HAVE_OPENCV
		memcpy( g_greyscale_image_left.data, data.m_greyscale_image_left[sensor_id], IMAGE_SIZE );
		memcpy( g_greyscale_image_right.data, data.m_greyscale_image_right[sensor_id], IMAGE_SIZE );
        memcpy( g_depth.data, data.m_depth_image[sensor_id], IMAGE_SIZE * 2 );
		imshow("left", g_greyscale_image_left);
		imshow("right", g_greyscale_image_right);
#endif
#else
		for ( int d = 0; d < CAMERA_PAIR_NUM; ++d )
		{
			string stmps;
			if ( data.m_greyscale_image_left[d] )
			{
#ifdef HAVE_OPENCV
				memcpy( g_greyscale_image_left.data, data.m_greyscale_image_left[d], IMAGE_SIZE );//maybe select too many image so just overwrite it
				stmps = "left";
				stmps = stmps + (char)('0'+d);
				imshow(stmps.c_str(), g_greyscale_image_left);
#endif
			}
			if ( data.m_greyscale_image_right[d] )
			{
#ifdef HAVE_OPENCV
				memcpy( g_greyscale_image_right.data, data.m_greyscale_image_right[d], IMAGE_SIZE );
				stmps = "right";
				stmps = stmps + (char)('0'+d);
				imshow(stmps, g_greyscale_image_right);
#endif
			}
			if ( data.m_depth_image[d] )
			{
#ifdef HAVE_OPENCV
				Mat depthmap(HEIGHT, WIDTH, CV_16SC1);
				Mat depthmap8(HEIGHT, WIDTH, CV_8UC1);
				memcpy( depthmap.data, data.m_depth_image[d], IMAGE_SIZE * 2 );
				depthmap.convertTo(depthmap8, CV_8UC1);
				stmps = "depthmap";
				stmps = stmps + (char)('0'+d);
				imshow(stmps, depthmap8);
#endif
			}
		}
#endif

#ifdef HAVE_OPENCV
		key = waitKey(1);
#endif

	}

	if ( e_imu == data_type && NULL != content )
	{
		imu *imu_data = (imu*)content;
		printf( "imu:%f %f %f,%f %f %f %f\n", imu_data->acc_x, imu_data->acc_y, imu_data->acc_z, imu_data->q[0], imu_data->q[1], imu_data->q[2], imu_data->q[3] );
		printf( "frame index:%d,stamp:%d\n", imu_data->frame_index, imu_data->time_stamp );
	}

	if ( e_velocity == data_type && NULL != content )
	{
		velocity *vo = (velocity*)content;
		printf( "vx:%f vy:%f vz:%f\n", 0.001f * vo->vx, 0.001f * vo->vy, 0.001f * vo->vz );
		printf( "frame index:%d,stamp:%d\n", vo->frame_index, vo->time_stamp );
	}

	if ( e_obstacle_distance == data_type && NULL != content )
	{
		obstacle_distance *oa = (obstacle_distance*)content;
		printf( "obstacle distance:" );
		for ( int i = 0; i < CAMERA_PAIR_NUM; ++i )
		{
			printf( " %f ", 0.01f * oa->distance[i] );
		}
		printf( "\n" );
		printf( "frame index:%d,stamp:%d\n", oa->frame_index, oa->time_stamp );
	}

	if ( e_ultrasonic == data_type && NULL != content )
	{
		ultrasonic_data *ultrasonic = (ultrasonic_data*)content;
		for ( int d = 0; d < CAMERA_PAIR_NUM; ++d )
		{
			printf( "ultrasonic distance:%f,reliability:%d\n", ultrasonic->ultrasonic[d] * 0.001f, (int)ultrasonic->reliability[d] );
		}
		printf( "frame index:%d,stamp:%d\n", ultrasonic->frame_index, ultrasonic->time_stamp );
	}

	g_lock.leave();
	g_event.set_event();

	return 0;
}

#define RETURN_IF_ERR(err_code) { if( err_code ){ release_transfer(); printf( "error code:%d,%s %d\n", err_code, __FILE__, __LINE__ ); return -1;}}

int main(int argc, const char** argv)
{
	if(argc>1){
		printf("This is demo program showing data from Guidance.\n\t" 
			"Type 'a','d','w','s','x' to select sensor direction.\n\t"
			"Type 'q' to quit.");
		return 0;
	}

	reset_config();
	int err_code = init_transfer();
	RETURN_IF_ERR( err_code );

#if !USE_GUIDANCE_ASSISTANT_CONFIG
	err_code = select_greyscale_image( sensor_id, true );
	RETURN_IF_ERR( err_code );
	err_code = select_greyscale_image( sensor_id, false );
	RETURN_IF_ERR( err_code );
	err_code = select_depth_image( sensor_id );
	RETURN_IF_ERR( err_code );
	select_imu();
	select_ultrasonic();
	select_obstacle_distance();
	select_velocity();
#endif

	err_code = set_sdk_event_handler( my_callback );
	RETURN_IF_ERR( err_code );
	err_code = start_transfer();
	RETURN_IF_ERR( err_code );

	// loop for 1000 iterations, i.e. 50 seconds when frequency is 20Hz
	for ( int j = 0; j < 1000; ++j )
	{
		g_event.wait_event();
		if (key != 0)
		{
			err_code = stop_transfer();
			RETURN_IF_ERR(err_code);
			reset_config();

			if (key == 'q') break;
			if (key == 'w') sensor_id = e_vbus1;
			if (key == 'd') sensor_id = e_vbus2;
			if (key == 'x') sensor_id = e_vbus3;
			if (key == 'a') sensor_id = e_vbus4;	   
			if (key == 's') sensor_id = e_vbus5;

			select_greyscale_image(sensor_id, true);
			select_greyscale_image(sensor_id, false);
			select_depth_image(sensor_id);

			err_code = start_transfer();
			RETURN_IF_ERR(err_code);
		}
	}

	err_code = stop_transfer();
	RETURN_IF_ERR( err_code );
	//make sure the ack packet from GUIDANCE is received
	sleep( 1000000 );
	err_code = release_transfer();
	RETURN_IF_ERR( err_code );

	return 0;
}
