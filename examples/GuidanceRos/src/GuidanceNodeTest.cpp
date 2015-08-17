/*
 * main_sdk0428.cpp
 *
 *  Created on: Apr 29, 2015
 *      Author: craig
 */

#include <stdio.h>
#include <string.h>
#include <ros/ros.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/image_encodings.h>

#include <opencv/cv.h>
#include <opencv/highgui.h>

#include <geometry_msgs/TransformStamped.h> //IMU
#include <geometry_msgs/Vector3Stamped.h> //velocity
#include <sensor_msgs/LaserScan.h> //obstacle distance && ultrasonic

ros::Subscriber left_image_sub;
ros::Subscriber right_image_sub;
ros::Subscriber depth_image_sub;
ros::Subscriber imu_sub;
ros::Subscriber velocity_sub;
ros::Subscriber obstacle_distance_sub;
ros::Subscriber ultrasonic_sub;

using namespace cv;
#define WIDTH 320
#define HEIGHT 240

/* left greyscale image */
void left_image_callback(const sensor_msgs::ImageConstPtr& left_img)
{
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(left_img, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::imshow("left_image", cv_ptr->image);
    cv::waitKey(1);
}

/* right greyscale image */
void right_image_callback(const sensor_msgs::ImageConstPtr& right_img)
{
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(right_img, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::imshow("right_image", cv_ptr->image);
    cv::waitKey(1);
}

/* depth greyscale image */
void depth_image_callback(const sensor_msgs::ImageConstPtr& depth_img)
{
    cv_bridge::CvImagePtr cv_ptr;
    try {
        cv_ptr = cv_bridge::toCvCopy(depth_img, sensor_msgs::image_encodings::MONO16);
    }
    catch (cv_bridge::Exception& e) {
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::Mat depth8(HEIGHT, WIDTH, CV_8UC1);
    cv_ptr->image.convertTo(depth8, CV_8UC1);
    cv::imshow("depth_image", depth8);
    cv::waitKey(1);
}

/* imu */
void imu_callback(const geometry_msgs::TransformStamped& g_imu)
{ 
    printf( "frame_id: %s stamp: %d\n", g_imu.header.frame_id.c_str(), g_imu.header.stamp.sec );
    printf( "imu: [%f %f %f %f %f %f %f]\n", g_imu.transform.translation.x, g_imu.transform.translation.y, g_imu.transform.translation.z, 
						g_imu.transform.rotation.x, g_imu.transform.rotation.y, g_imu.transform.rotation.z, g_imu.transform.rotation.w );
}

/* velocity */
void velocity_callback(const geometry_msgs::Vector3Stamped& g_vo)
{ 
    printf( "frame_id: %s stamp: %d\n", g_vo.header.frame_id.c_str(), g_vo.header.stamp.sec );
    printf( "velocity: [%f %f %f]\n", g_vo.vector.x, g_vo.vector.y, g_vo.vector.z );
}

/* obstacle distance */
void obstacle_distance_callback(const sensor_msgs::LaserScan& g_oa)
{ 
    printf( "frame_id: %s stamp: %d\n", g_oa.header.frame_id.c_str(), g_oa.header.stamp.sec );
    printf( "obstacle distance: [%f %f %f %f %f]\n", g_oa.ranges[0], g_oa.ranges[1], g_oa.ranges[2], g_oa.ranges[3], g_oa.ranges[4] );
}

/* ultrasonic */
void ultrasonic_callback(const sensor_msgs::LaserScan& g_ul)
{ 
    printf( "frame_id: %s stamp: %d\n", g_ul.header.frame_id.c_str(), g_ul.header.stamp.sec );
    for (int i = 0; i < 5; i++)
        printf( "ultrasonic distance: [%f]  reliability: [%d]\n", g_ul.ranges[i], (int)g_ul.intensities[i] );
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "GuidanceNodeTest");
    ros::NodeHandle my_node;

    left_image_sub        = my_node.subscribe("/guidance/left_image",  10, left_image_callback);
    right_image_sub       = my_node.subscribe("/guidance/right_image", 10, right_image_callback);
    depth_image_sub       = my_node.subscribe("/guidance/depth_image", 10, depth_image_callback);
    imu_sub               = my_node.subscribe("/guidance/imu", 1, imu_callback);
    velocity_sub          = my_node.subscribe("/guidance/velocity", 1, velocity_callback);
    obstacle_distance_sub = my_node.subscribe("/guidance/obstacle_distance", 1, obstacle_distance_callback);
    ultrasonic_sub        = my_node.subscribe("/guidance/ultrasonic", 1, ultrasonic_callback);

    while (ros::ok())
        ros::spinOnce();

    return 0;
}
