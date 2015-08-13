#include <stdio.h>
#include <string.h>

#include "DJI_guidance.h"
#include "DJI_utility.h"

using namespace std;
#define WIDTH 320
#define HEIGHT 240
#define IMAGE_SIZE (HEIGHT * WIDTH)

lock    g_lock;
event   g_event;
FILE*	f_imleft;
FILE*	f_imright;
FILE*	f_depth;
FILE*	f_imu;
FILE*	f_vo;
unsigned int hasData = 1;
e_vbus_index selected_vbus = e_vbus1;

int my_callback(int data_type, int data_len, char *content)
{
	g_lock.enter();
	if (e_image == data_type && NULL != content)
	{
		image_data data;
		memcpy( (char*)&data, content, sizeof(data) );

		unsigned int posInd = 0;
		fseek(f_imleft, IMAGE_SIZE, SEEK_SET);
		fread(&posInd, sizeof(posInd), 1, f_imleft);
		if(posInd==0){
			rewind(f_imleft);
			fwrite(data.m_greyscale_image_left[selected_vbus], 1, IMAGE_SIZE, f_imleft);
			fwrite(&hasData, sizeof(hasData), 1, f_imleft);		
		}

		fseek(f_imright, IMAGE_SIZE, SEEK_SET);
		fread(&posInd, sizeof(posInd), 1, f_imright);
		if(posInd==0){
			rewind(f_imright);
			fwrite(data.m_greyscale_image_right[selected_vbus], 1, IMAGE_SIZE, f_imright);
			fwrite(&hasData, sizeof(hasData), 1, f_imright);		
		}

		fseek(f_depth, IMAGE_SIZE*2, SEEK_SET);
		fread(&posInd, sizeof(posInd), 1, f_depth);
		if(posInd==0){
			rewind(f_depth);
			fwrite(data.m_depth_image[selected_vbus], 1, IMAGE_SIZE*2, f_depth);
			fwrite(&hasData, sizeof(hasData), 1, f_depth);	
		}
	}

	if ( e_imu == data_type && NULL != content )
	{
		imu *imu_data = (imu*)content;
		printf( "imu:%f %f %f,%f %f %f %f\n", imu_data->acc_x, imu_data->acc_y, imu_data->acc_z, imu_data->q[0], imu_data->q[1], imu_data->q[2], imu_data->q[3] );
		rewind(f_imu);
		fwrite(&hasData, sizeof(hasData), 1, f_imu);		
		fwrite(&imu_data->acc_x, sizeof(float), 3, f_imu);
		fwrite(imu_data->q, sizeof(float), 4, f_imu);
	}

	if ( e_velocity == data_type && NULL != content )
	{
		velocity *vo = (velocity*)content;
		printf( "vx:%f vy:%f vz:%f\n", 0.001f * vo->vx, 0.001f * vo->vy, 0.001f * vo->vz );
		rewind(f_vo);
		fwrite(&hasData, sizeof(hasData), 1, f_vo);		
		fwrite(&vo->vx, sizeof(short), 3, f_vo);
	}

	if ( e_obstacle_distance == data_type && NULL != content )
	{
		obstacle_distance *oa = (obstacle_distance*)content;
		printf( "obstacle distance:" );
		for ( int i = 0; i < 4; ++i )
		{
			printf( " %f ", 0.01f * oa->distance[i] );
		}
		printf( "\n" );
		printf( "frame index:%d,stamp:%d,%s %d\n", oa->frame_index, oa->time_stamp, __FILE__, __LINE__ );
	}

	if ( e_ultrasonic == data_type && NULL != content )
	{
		ultrasonic_data *ultrasonic = (ultrasonic_data*)content;
		printf( "ultrasonic distance,\treliability:\n");
		for ( int d = 0; d < CAMERA_PAIR_NUM; ++d )
		{
			printf( "%f,\t%d\n", ultrasonic->ultrasonic[d] * 0.001f, (int)ultrasonic->reliability[d] );
		}
		printf( "frame index:%d,stamp:%d,%s %d\n", ultrasonic->frame_index, ultrasonic->time_stamp, __FILE__, __LINE__ );
	}

	g_lock.leave();
	g_event.set_event();

	return 0;
}

#define RETURN_IF_ERR(err_code) { if( err_code ){ release_transfer(); printf( "error code:%d,%s %d\n", err_code, __FILE__, __LINE__ );}}

int main(int argc, const char** argv)
{
	// create file for sharing memory with Matlab
	f_imleft = fopen("imleft.dat","wb+");
	if(!f_imleft)
		printf("File imleft.dat cannot be created!\n");
	if(!(f_imright = fopen("imright.dat","wb+")))
		printf("File imright.dat cannot be created!\n");
	if(!(f_depth = fopen("imdepth.dat","wb+")))
		printf("File imdepth.dat cannot be created!\n");

	if(!(f_imu = fopen("imu.dat","wb+")))
		printf("File imu.dat cannot be created!\n");
	if(!(f_vo = fopen("vo.dat","wb+")))
		printf("File vo.dat cannot be created!\n");

	reset_config();
	int err_code = init_transfer();
	RETURN_IF_ERR( err_code );
	err_code = select_greyscale_image( selected_vbus, true );
	RETURN_IF_ERR( err_code );
	err_code = select_greyscale_image( selected_vbus, false );
	RETURN_IF_ERR( err_code );
	err_code = select_depth_image( selected_vbus );
	RETURN_IF_ERR( err_code );
	select_imu();
	select_ultrasonic();
	select_obstacle_distance();
	select_velocity();
	err_code = set_sdk_event_handler( my_callback );
	RETURN_IF_ERR( err_code );
	err_code = start_transfer();
	RETURN_IF_ERR( err_code );

	for ( int j = 0; j < 3000; ++j )
	{
		g_event.wait_event();
		printf( "wait event %d\n", j );
	}

	err_code = stop_transfer();
	RETURN_IF_ERR( err_code );
	//make sure the ack packet from GUIDANCE is received
	sleep( 1000000 );
	err_code = release_transfer();
	RETURN_IF_ERR( err_code );

	fclose(f_imleft);
	fclose(f_imright);
	fclose(f_depth);
	fclose(f_vo);
	fclose(f_imu);
	return 0;
}
