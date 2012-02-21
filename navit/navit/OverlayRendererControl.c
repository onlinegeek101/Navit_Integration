/*
 * OverlayRendererControl.c
 *
 *  Created on: Dec 3, 2011
 *      Author: ubuntu3
 */

#include "RouteInputAnalyzer.h"
#include "OverlayRendererControl.h"
#include "ImageCreation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

typedef char * string;

static OverlayRendererControl controller;
static int initialized = 0;

// static method to set pixel position of text in projection
static void setInfoTextPosition()
{
	int a = controller.width;
	int b = controller.height;
	// set position of speed -> [0][] text [0][x] and [0][y]
	controller.textPosition[0][0] = a/2 - a*.2;
	controller.textPosition[0][1] = b - b*.1;

	// set position of distance to next turn -> [1][] text [1][x] and [1][y]
	controller.textPosition[1][0] = a/2 - a*.2;
	controller.textPosition[1][1] = b - b*.08;
}

// static method to set the speed and distance text to be placed on the windshield
static void setTextToDisplay()
{
	// set text of speed -> [0]
	sprintf(controller.speed_text, "%d", controller.velocity); // convert speed to string and place in array
	strcat(controller.speed_text, " km/h"); // Concatenate text onto it

	// set text of distance to next turn -> [1]
	sprintf(controller.distance_to_next_turn_text, "%d", controller.distance); // convert distance to string and place in array
	strcat(controller.distance_to_next_turn_text, " meters before next turn"); // Concatenate text onto it
}

// method to initialize  the overlay rendering system - projector size and color
void initializeSystem(int xdimension, int ydimension, string projection_color)
{
	controller.width = xdimension;
	controller.height = ydimension;
	strcpy(controller.color, projection_color);

	controller.data_currently_in_use = 0;
	controller.velocity = 0;
	controller.road_angle = 0;
	controller.distance = 0;
	controller.turn_angle = 0;
	controller.x1 = 0;
	controller.y1 = 0;
	controller.x2 = 0;
	controller.y2 = 0;
	controller.time_since_mapping_update = 0;

	// set text and position of speed/distance
	setInfoTextPosition();

	initialized = 1;
}

// method to change the color of the projection
void setProjectionColor(char projection_color[])
{
	strcpy(controller.color, projection_color);
}

// method to update the current route information returns 1 for successful, returns 0 if failed
int updateRouteData(int velocity, int road_angle, int distance, int turn_angle, int x1, int y1, int x2, int y2)
{
	if(controller.data_currently_in_use == 0)
	{
		controller.data_currently_in_use = 1;
		controller.velocity = velocity;
		controller.road_angle = road_angle;
		controller.distance = distance;
		controller.turn_angle = turn_angle;
		controller.x1 = x1;
		controller.y1 = y1;
		controller.x2 = x2;
		controller.y2 = y2;
		controller.time_since_mapping_update = time(NULL);
		setTextToDisplay();

		return 1;
	}
	else
		return 0;
}

// method responsible for controlling the RouteInputAnalyzer, ChoicePath, and ImageCreation
void processImageData()
{
	if(initialized == 0)
	{
		char projection_color[] = "red";
		initializeSystem(800, 600,projection_color);
	}

	// calculated pixels for arrow placement
	processGenericImageData(&controller);
	drawImage(&controller);
	controller.data_currently_in_use = 0;

	// Display Image
	IplImage* img;
	cvNamedWindow( "Projection", CV_WINDOW_NORMAL );
	cvSetWindowProperty("Projection", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	img = cvLoadImage( "arrow_test.jpg", CV_LOAD_IMAGE_UNCHANGED );
	cvShowImage("Projection", img);
	cvWaitKey(1);

}





