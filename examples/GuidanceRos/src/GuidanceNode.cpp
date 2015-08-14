/*
 * GuidanceNode.cpp
 *
 *  Created on: Apr 29, 2015
 */

#include <stdio.h>
#include <string.h>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>


#include <opencv/cv.h>
#include <opencv/highgui.h>

#include "DJI_guidance.h"
#include "DJI_utility.h"

e_vbus_index CAMERA_ID = e_vbus2;
#define WIDTH 320
#define HEIGHT 240
#define IMAGE_SIZE (HEIGHT * WIDTH)

using namespace cv;

ros::Publisher depth_image_pub;
ros::Publisher left_image_pub;
ros::Publisher right_image_pub;

Mat     g_greyscale_image_left(HEIGHT, WIDTH, CV_8UC1);
Mat		g_greyscale_image_right(HEIGHT, WIDTH, CV_8UC1);
Mat		g_depth(HEIGHT,WIDTH,CV_16SC1);
DJI_lock    g_lock;
DJI_event   g_event;
char    key = 0;

int my_callback(int data_type, int data_len, char *content)
{
    g_lock.enter();

    if (e_image == data_type && NULL != content)
    {        
        image_data data;
        memcpy( (char*)&data, content, sizeof(data) );

        memcpy( g_greyscale_image_left.data, data.m_greyscale_image_left[CAMERA_ID], IMAGE_SIZE );
        memcpy( g_greyscale_image_right.data, data.m_greyscale_image_right[CAMERA_ID], IMAGE_SIZE );
        memcpy( g_depth.data, data.m_depth_image[CAMERA_ID], IMAGE_SIZE * 2 );

        Mat depth8(HEIGHT, WIDTH, CV_8UC1);
        g_depth.convertTo(depth8, CV_8UC1);
//        imshow("left", g_greyscale_image_left);
        imshow("depth", depth8);
        key = waitKey(1);

        //publish depth image
        cv_bridge::CvImage mat_16;
        g_depth.copyTo(mat_16.image);
        mat_16.header.frame_id  = "guidance";
        mat_16.header.stamp		= ros::Time::now();
        mat_16.encoding		 	= sensor_msgs::image_encodings::MONO16;
        depth_image_pub.publish(mat_16.toImageMsg());

        // publish left grayscale image
        cv_bridge::CvImage mat_8;
        g_greyscale_image_left.copyTo(mat_8.image);
        mat_8.header.frame_id  = "guidance";
        mat_8.header.stamp		= ros::Time::now();
        mat_8.encoding		 	= sensor_msgs::image_encodings::MONO8;
        left_image_pub.publish(mat_8.toImageMsg());
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

int main(int argc, char** argv)
{
    ros::init(argc, argv, "GuidanceNode");

    ros::NodeHandle my_node;
    depth_image_pub = my_node.advertise<sensor_msgs::Image>("/depth_image",1);
    left_image_pub  = my_node.advertise<sensor_msgs::Image>("/left_image",1);
    right_image_pub  = my_node.advertise<sensor_msgs::Image>("/right_image",1);

    reset_config();
    init_transfer();
    select_greyscale_image(CAMERA_ID, true);
    select_greyscale_image(CAMERA_ID, false);
    select_depth_image(CAMERA_ID);

    select_imu();
    select_ultrasonic();
    select_obstacle_distance();
    select_velocity();

    set_sdk_event_handler(my_callback);
    start_transfer();
    std::cout<<"start_transfer"<<std::endl;

    ros::spin();

    stop_transfer();
    //make sure the ack packet from GUIDANCE is received
    sleep(1);
    std::cout<<"release_transfer"<<std::endl;
    release_transfer();

    return 0;
}
