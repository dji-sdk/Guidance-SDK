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

#include "DJI_guidance.h"

#define WIDTH 320
#define HEIGHT 240
#define IMAGE_SIZE (HEIGHT * WIDTH)

using namespace cv;

ros::Subscriber left_image_sub;

Mat     g_greyscale_image_left(HEIGHT, WIDTH, CV_8UC1);

void my_callback(const sensor_msgs::ImageConstPtr& img)
{
    cv_bridge::CvImagePtr cv_ptr;
    try{
        cv_ptr = cv_bridge::toCvCopy(img, sensor_msgs::image_encodings::MONO8);
    }
    catch (cv_bridge::Exception& e){
        ROS_ERROR("cv_bridge exception: %s", e.what());
        return;
    }

    cv::imshow("image", cv_ptr->image);
    cv::waitKey(1);
}

int main(int argc, char** argv)
{
    ros::init(argc, argv, "GuidanceNodeTest");

    ros::NodeHandle my_node;
    left_image_sub  = my_node.subscribe("/left_image",10,my_callback);


    ros::spin();


    return 0;
}
