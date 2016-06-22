/*
 ============================================================================
 Name        : dji_sdk_node.cpp
 Author      : Mario Chris
 Description : Obstacle Avoidance
 ============================================================================
*/

/* System */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <unistd.h>

/* Onboard */
#include "DJI_Codec.h"
#include "DJI_Link.h"
#include "DJI_App.h"
#include "DJI_API.h"
#include "DJI_Type.h"
#include "DJI_Flight.h"
#include "DJI_HardDriver.h"
#include "DJI_HardDriver_Manifold.h"

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
using namespace DJI;
using namespace DJI::onboardSDK;

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
#include <pthread.h>
#include <thread>

HardDriver_Manifold* m_hd;
CoreAPI *coreAPI;
Flight *flight;

ActivateData user_act_data;
BroadcastData bc_data;

int ret;

/* roslaunch parameters */
std::string serial_name;
unsigned int baud_rate;
int app_id;
int app_api_level;
int app_version;
char app_key[65];
string enc_key;

pthread_t m_recvTid;

/* parameter */
#define WIDTH			320
#define HEIGHT			240
#define IMAGE_SIZE		(HEIGHT * WIDTH)
#define VBUS			e_vbus1
#define RETURN_IF_ERR(err_code) { if( err_code ){ release_transfer(); printf( "error code:%d,%s %d\n", err_code, __FILE__, __LINE__ );}}

/* guidance */
int err_code;
Mat g_greyscale_image_left=Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
Mat g_greyscale_image_right=Mat::zeros(HEIGHT, WIDTH, CV_8UC1);
int iter = 0;
DJI_lock g_lock;
DJI_event g_event;

// cells
#define CELL_ROWS 20
#define CELL_COLS 20
struct cell
{
	double of_x; // optical flow x
	double of_y; // optical flow y
	uint count; // number of keypoints
	bool disc; // discontinuity
};
cell of_cells[CELL_ROWS][CELL_COLS];
#define OF_MARGIN (double) 0.5 // 2 optical flow values considered different if their difference exceeds this margin
#define DISC_MARGIN (double) 0.5 // a cell considered discontinous if the proportion of neighboring cells with different optical flow values exceeds this margin
#define MAX_NEIGH_DIST 1 // max distance allowed between two cells to be considered neighbors
#define MAX_DISC (double) 0.7 // max proportion of discontinuous cells allowed for drone to continue maneuvering

// Shi-Tomasi
#define MIN_CORNERS (uint) 30
#define CORNER_THRESHOLD (uint) 190
#define MAX_CORNERS (uint) 200
#define QUALITY_LEVEL (double) 0.01
#define MIN_DISTANCE (double) 1

// Lucas-Kanade
int frame_num = 0;
vector<Point2f> prevleftpts, prevrightpts, prevlefttracked, prevrighttracked;
Mat prevleftimg, prevrightimg;
bool l_kpt_regen = true;
bool r_kpt_regen = true;

// control
#define CMD_FLAG 0x4A // control horizontal/vertical velocity in body frame and yaw rate in ground frame
#define FWD (double) 0.5 // constant horizontal velocity
#define TURN (double) 10 // constant yaw rate
#define ALT (double) 0.1 // constant vertical velocity
double l_fwd = FWD; // left image forward control
double l_turn = 0; // left image turn control
double l_alt = 0; // left image altitude control
double r_fwd = FWD; // right image forward control
double r_turn = 0; // right image turn control
double r_alt = 0; // right image altitude control

/*************************************/



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
					// populating cell data
					for(int row = 0; row < CELL_ROWS; row++)
					{
						for(int col = 0; col < CELL_COLS; col++)
						{
							of_cells[row][col].of_x = 0;
							of_cells[row][col].of_y = 0;
							of_cells[row][col].count = 0;
							of_cells[row][col].disc = false;
						}
					}
					for(size_t i = 0; i < LKleftpts.size(); i++)
					{
						if(LKleftpts[i].x >= 0 && LKleftpts[i].y >= 0 && LKleftpts[i].x <= WIDTH && LKleftpts[i].y <= HEIGHT)
						{
							// calculating corresponding cell and optical flow
							int row = LKleftpts[i].y*CELL_ROWS/HEIGHT;
							int col = LKleftpts[i].x*CELL_COLS/WIDTH;
							double of_x = LKleftpts[i].x - prevleftpts[i].x;
							double of_y = LKleftpts[i].y - prevleftpts[i].y;
							// updating cell
							of_cells[row][col].of_x += of_x;
							of_cells[row][col].of_y += of_y;
							of_cells[row][col].count++;
							// saving optical flow values for control feedback
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
					// finding discontinuous cells
					int left_disc = 0, right_disc = 0, up_disc = 0, down_disc = 0;
					for(int row = 0; row < CELL_ROWS; row++)
					{
						for(int col = 0; col < CELL_COLS; col++)
						{
							int neighbor_count = 0;
							int neighbor_diff = 0;
							// iterating over neighbor cells
							for(int x_dist = -1*MAX_NEIGH_DIST; x_dist <= MAX_NEIGH_DIST; x_dist++)
							{
								for(int y_dist = -1*MAX_NEIGH_DIST; y_dist <= MAX_NEIGH_DIST; y_dist++)
								{
									if(x_dist == 0 && y_dist == 0) continue; // do not count self as neighbor
									if(row - y_dist < 0 || row + y_dist > CELL_ROWS - 1 || col - x_dist < 0 || col + x_dist > CELL_COLS - 1) continue; // do not exceed boundaries
									// assign shorter vector as a and longer vector as b
									double a_x;
									double a_y;
									double b_x;
									double b_y;
									if(pow((pow(of_cells[row][col].of_x, 2) + pow(of_cells[row][col].of_y, 2))/pow((double)of_cells[row][col].count, 2), 0.5) < pow((pow(of_cells[row + y_dist][col + x_dist].of_x, 2) + pow(of_cells[row + y_dist][col + x_dist].of_y, 2))/pow((double)of_cells[row + y_dist][col + x_dist].count, 2), 0.5))
									{
										a_x = of_cells[row][col].of_x/(double)of_cells[row][col].count;
										a_y = of_cells[row][col].of_y/(double)of_cells[row][col].count;
										b_x = of_cells[row + y_dist][col + x_dist].of_x/(double)of_cells[row + y_dist][col + x_dist].count;
										b_y = of_cells[row + y_dist][col + x_dist].of_y/(double)of_cells[row + y_dist][col + x_dist].count;
									}
									else
									{
										b_x = of_cells[row][col].of_x/(double)of_cells[row][col].count;
										b_y = of_cells[row][col].of_y/(double)of_cells[row][col].count;
										a_x = of_cells[row + y_dist][col + x_dist].of_x/(double)of_cells[row + y_dist][col + x_dist].count;
										a_y = of_cells[row + y_dist][col + x_dist].of_y/(double)of_cells[row + y_dist][col + x_dist].count;
									}
									if(1.0 - (a_x*b_x + a_y*b_y)/(pow(b_x, 2) + pow(b_y, 2)) > OF_MARGIN) neighbor_diff++;
								}
							}
							if((double)neighbor_diff/(double)neighbor_count > DISC_MARGIN)
							{
								if(CELL_COLS/(col + 1) >= 2)
								{
									left_disc++;
								}
								else
								{
									right_disc++;
								}
								if(CELL_ROWS/(row + 1) >= 2)
								{
									up_disc++;
								}
								else
								{
									down_disc++;
								}
								of_cells[row][col].disc = true;
							}
						}
					}
					// move away from high image section discontinuity
					l_fwd = FWD;
					double turn_prev = l_turn;
					double alt_prev = l_alt;
					l_turn = 0;
					l_alt = 0;
					if(left_disc > right_disc)
					{
						ctrl_text += " RIGHT";
						if(turn_prev > 0) l_turn = (1 + (left_of - right_of)/(left_of + right_of))*turn_prev;
						else l_turn = TURN;
					}
					else if(right_disc > left_disc)
					{
						ctrl_text += " LEFT";
						if(turn_prev < 0) l_turn = (1 + (right_of - left_of)/(right_of + left_of))*turn_prev;
						else l_turn = -1.0*TURN;
					}
					if(up_disc > down_disc)
					{
						ctrl_text += " DOWN";
						if(alt_prev < 0) l_alt = (1 + (up_of - down_of)/(up_of + down_of))*alt_prev;
						else l_alt = -1.0*ALT;
					}
					else if(down_disc > up_disc)
					{
						ctrl_text += " UP";
						if(alt_prev > 0) l_alt = (1 + (down_of - up_of)/(down_of + up_of))*alt_prev;
						else l_alt = ALT;
					}
					if((double)(left_disc + right_disc + up_disc + down_disc)/(double)(CELL_ROWS*CELL_COLS) > MAX_DISC)
					{
						ctrl_text = "STOP MOVING";
						l_fwd = 0;
						l_turn = 0;
						l_alt = 0;
					}
					if(ctrl_text.compare("GO") == 0) ctrl_text += " STRAIGHT";

					prevlefttracked.assign(LKleftpts.begin(), LKleftpts.end());
				}

				/* Shi-Tomasi */
				
				vector<Point2f> left_corners;
				if(frame_num > 0)
				{
					vector<Point2f> tracked_left_corners;
					for(size_t i = 0; i < prevlefttracked.size(); i++)
					{
						if(prevlefttracked[i].x >= 0 && prevlefttracked[i].x <= WIDTH && prevlefttracked[i].y >= 0 && prevlefttracked[i].y <= HEIGHT) tracked_left_corners.push_back(prevlefttracked[i]);
					}
					if(tracked_left_corners.size() >= CORNER_THRESHOLD)
					{
						left_corners.assign(tracked_left_corners.begin(), tracked_left_corners.end()); // retain previously tracked keypoints
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
				// cells display
				for(int row = 1; row < CELL_ROWS; row++)
				{
					line(left_rgb, Point2f(0, row*HEIGHT/CELL_ROWS), Point2f(WIDTH, row*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
				}
				for(int col = 1; col < CELL_COLS; col++)
				{
					line(left_rgb, Point2f(col*WIDTH/CELL_COLS, 0), Point2f(col*WIDTH/CELL_COLS, HEIGHT), Scalar(0, 0, 255));
				}
				for(int row = 0; row < CELL_ROWS; row++)
				{
					for(int col = 0; col < CELL_COLS; col++)
					{
						if(of_cells[row][col].disc)
						{
							line(left_rgb, Point2f(col*WIDTH/CELL_COLS, row*HEIGHT/CELL_ROWS), Point2f((col + 1)*WIDTH/CELL_COLS, (row + 1)*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
							line(left_rgb, Point2f((col + 1)*WIDTH/CELL_COLS, row*HEIGHT/CELL_ROWS), Point2f(col*WIDTH/CELL_COLS, (row + 1)*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
						}
					}
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
					// populating cell data
					for(int row = 0; row < CELL_ROWS; row++)
					{
						for(int col = 0; col < CELL_COLS; col++)
						{
							of_cells[row][col].of_x = 0;
							of_cells[row][col].of_y = 0;
							of_cells[row][col].count = 0;
							of_cells[row][col].disc = false;
						}
					}
					for(size_t i = 0; i < LKrightpts.size(); i++)
					{
						if(LKrightpts[i].x >= 0 && LKrightpts[i].y >= 0 && LKrightpts[i].x <= WIDTH && LKrightpts[i].y <= HEIGHT)
						{
							// calculating corresponding cell and optical flow
							int row = LKrightpts[i].y*CELL_ROWS/HEIGHT;
							int col = LKrightpts[i].x*CELL_COLS/WIDTH;
							double of_x = LKrightpts[i].x - prevrightpts[i].x;
							double of_y = LKrightpts[i].y - prevrightpts[i].y;
							// updating cell
							of_cells[row][col].of_x += of_x;
							of_cells[row][col].of_y += of_y;
							of_cells[row][col].count++;
							// saving optical flow values for control feedback
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
					// finding discontinuous cells
					int left_disc = 0, right_disc = 0, up_disc = 0, down_disc = 0;
					for(int row = 0; row < CELL_ROWS; row++)
					{
						for(int col = 0; col < CELL_COLS; col++)
						{
							int neighbor_count = 0;
							int neighbor_diff = 0;
							// iterating over neighbor cells
							for(int x_dist = -1*MAX_NEIGH_DIST; x_dist <= MAX_NEIGH_DIST; x_dist++)
							{
								for(int y_dist = -1*MAX_NEIGH_DIST; y_dist <= MAX_NEIGH_DIST; y_dist++)
								{
									if(x_dist == 0 && y_dist == 0) continue; // do not count self as neighbor
									if(row - y_dist < 0 || row + y_dist > CELL_ROWS - 1 || col - x_dist < 0 || col + x_dist > CELL_COLS - 1) continue; // do not exceed boundaries
									// assign shorter vector as a and longer vector as b
									double a_x;
									double a_y;
									double b_x;
									double b_y;
									if(pow((pow(of_cells[row][col].of_x, 2) + pow(of_cells[row][col].of_y, 2))/pow((double)of_cells[row][col].count, 2), 0.5) < pow((pow(of_cells[row + y_dist][col + x_dist].of_x, 2) + pow(of_cells[row + y_dist][col + x_dist].of_y, 2))/pow((double)of_cells[row + y_dist][col + x_dist].count, 2), 0.5))
									{
										a_x = of_cells[row][col].of_x/(double)of_cells[row][col].count;
										a_y = of_cells[row][col].of_y/(double)of_cells[row][col].count;
										b_x = of_cells[row + y_dist][col + x_dist].of_x/(double)of_cells[row + y_dist][col + x_dist].count;
										b_y = of_cells[row + y_dist][col + x_dist].of_y/(double)of_cells[row + y_dist][col + x_dist].count;
									}
									else
									{
										b_x = of_cells[row][col].of_x/(double)of_cells[row][col].count;
										b_y = of_cells[row][col].of_y/(double)of_cells[row][col].count;
										a_x = of_cells[row + y_dist][col + x_dist].of_x/(double)of_cells[row + y_dist][col + x_dist].count;
										a_y = of_cells[row + y_dist][col + x_dist].of_y/(double)of_cells[row + y_dist][col + x_dist].count;
									}
									if(1.0 - (a_x*b_x + a_y*b_y)/(pow(b_x, 2) + pow(b_y, 2)) > OF_MARGIN) neighbor_diff++;
								}
							}
							if((double)neighbor_diff/(double)neighbor_count > DISC_MARGIN)
							{
								if(CELL_COLS/(col + 1) >= 2)
								{
									left_disc++;
								}
								else
								{
									right_disc++;
								}
								if(CELL_ROWS/(row + 1) >= 2)
								{
									up_disc++;
								}
								else
								{
									down_disc++;
								}
								of_cells[row][col].disc = true;
							}
						}
					}
					// move away from high image section discontinuity
					r_fwd = FWD;
					double turn_prev = r_turn;
					double alt_prev = r_alt;
					r_turn = 0;
					r_alt = 0;
					if(left_disc > right_disc)
					{
						ctrl_text += " RIGHT";
						if(turn_prev > 0) r_turn = (1 + (left_of - right_of)/(left_of + right_of))*turn_prev;
						else r_turn = TURN;
					}
					else if(right_disc > left_disc)
					{
						ctrl_text += " LEFT";
						if(turn_prev < 0) r_turn = (1 + (right_of - left_of)/(right_of + left_of))*turn_prev;
						else r_turn = -1.0*TURN;
					}
					if(up_disc > down_disc)
					{
						ctrl_text += " DOWN";
						if(alt_prev < 0) r_alt = (1 + (up_of - down_of)/(up_of + down_of))*alt_prev;
						else r_alt = -1.0*ALT;
					}
					else if(down_disc > up_disc)
					{
						ctrl_text += " UP";
						if(alt_prev > 0) r_alt = (1 + (down_of - up_of)/(down_of + up_of))*alt_prev;
						else r_alt = ALT;
					}
					if((double)(left_disc + right_disc + up_disc + down_disc)/(double)(CELL_ROWS*CELL_COLS) > MAX_DISC)
					{
						ctrl_text = "STOP MOVING";
						r_fwd = 0;
						r_turn = 0;
						r_alt = 0;
					}
					if(ctrl_text.compare("GO") == 0) ctrl_text += " STRAIGHT";

					prevrighttracked.assign(LKrightpts.begin(), LKrightpts.end());
				}

				/* Shi-Tomasi */
				
				vector<Point2f> right_corners;
				if(frame_num > 0)
				{
					vector<Point2f> tracked_right_corners;
					for(size_t i = 0; i < prevrighttracked.size(); i++)
					{
						if(prevrighttracked[i].x >= 0 && prevrighttracked[i].x <= WIDTH && prevrighttracked[i].y >= 0 && prevrighttracked[i].y <= HEIGHT) tracked_right_corners.push_back(prevrighttracked[i]);
					}
					if(tracked_right_corners.size() >= CORNER_THRESHOLD)
					{
						right_corners.assign(tracked_right_corners.begin(), tracked_right_corners.end()); // retain previously tracked keypoints
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
				// cells display
				for(int row = 1; row < CELL_ROWS; row++)
				{
					line(right_rgb, Point2f(0, row*HEIGHT/CELL_ROWS), Point2f(WIDTH, row*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
				}
				for(int col = 1; col < CELL_COLS; col++)
				{
					line(right_rgb, Point2f(col*WIDTH/CELL_COLS, 0), Point2f(col*WIDTH/CELL_COLS, HEIGHT), Scalar(0, 0, 255));
				}
				for(int row = 0; row < CELL_ROWS; row++)
				{
					for(int col = 0; col < CELL_COLS; col++)
					{
						if(of_cells[row][col].disc)
						{
							line(right_rgb, Point2f(col*WIDTH/CELL_COLS, row*HEIGHT/CELL_ROWS), Point2f((col + 1)*WIDTH/CELL_COLS, (row + 1)*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
							line(right_rgb, Point2f((col + 1)*WIDTH/CELL_COLS, row*HEIGHT/CELL_ROWS), Point2f(col*WIDTH/CELL_COLS, (row + 1)*HEIGHT/CELL_ROWS), Scalar(0, 0, 255));
						}
					}
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

			double x = (l_fwd + r_fwd)/2.0;
			double y = 0;
			double z = (l_alt + r_alt)/2.0;
			#ifdef NO_ALT_CTRL
				z = 0;
			#endif
			double yaw = (l_turn + r_turn)/2.0;
			flight->control(CMD_FLAG, x, y, z, yaw);
		
	}

	g_lock.leave();
	g_event.set_DJI_event();

	return 0;
}



// run mission
void run()
{
	/* Request Control */
	coreAPI->setControl(1);
	/* Takeoff */
	usleep(2000000);// wait for 2 seconds
	flight->task(Flight::TASK_TAKEOFF);
	printf("\nTaking off...\n");

	/* Maneuvering and avoiding obstacles */
	usleep(1000000); // wait for 1 second
	printf("\nBeginning Obstacle Avoidance...\n");
	err_code = start_transfer(); // start guidance data collection
	RETURN_IF_ERR( err_code );

	/* Landing */
	getchar(); // press key to begin autonomous mode
	printf("\nEnding Obstacle Avoidance...\n");
	err_code = stop_transfer(); // stop guidance
	RETURN_IF_ERR( err_code );
	
	bc_data = coreAPI->getBroadcastData();
	
	printf("\nLanding...\n");


	flight->task(Flight::TASK_LANDING); // land
	usleep(1000000);
	
	/* Release Control */
	coreAPI->setControl(0);	
}


void* APIRecvThread(void* param) {

            CoreAPI* p_coreAPI = (CoreAPI*)param;
            while(true) {
                p_coreAPI->readPoll();
                p_coreAPI->sendPoll();
                usleep(1000);
            }
	return nullptr;
        }


int main (int argc, char** argv)
{
	
	/* Onboard */
	printf("Test SDK Protocol demo\n");

	serial_name = std::string("/dev/ttyTHS1");
	baud_rate = 230400;
	app_id = 1027749;
	app_api_level = 2;
	app_version = 1;
	enc_key = std::string("5de4dfff4bea190522bd6d9bb7da434a213c5e30611a8d5f01c8ed1146e1712a");
 
	user_act_data.ID		= app_id;
	user_act_data.version = (uint32_t)0x03010a00;// M100
	user_act_data.encKey = app_key;
    	strcpy(user_act_data.encKey, enc_key.c_str());

    	printf("=================================================\n");
    	printf("app id: %d\n", user_act_data.ID);
    	printf("app version: 0x0%X\n", user_act_data.version);
    	printf("app key: %s\n", user_act_data.encKey);
    	printf("=================================================\n");
	
	m_hd = new HardDriver_Manifold(serial_name, baud_rate);
	m_hd->init();
	coreAPI = new CoreAPI( (HardDriver*)m_hd );
	coreAPI->setVersion(user_act_data.version);
	
        ret = pthread_create(&m_recvTid, 0, APIRecvThread, (void *)coreAPI);

	coreAPI->activate(&user_act_data,NULL);

	flight = new Flight(coreAPI);
	
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
