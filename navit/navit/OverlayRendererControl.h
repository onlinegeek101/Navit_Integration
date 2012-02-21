/*
 * OverlayRendererControl.h
 *
 *  Created on: Dec 3, 2011
 *      Author: ubuntu3
 */

#ifndef OVERLAYRENDERERCONTROL_H_
#define OVERLAYRENDERERCONTROL_H_

#include "RouteInputAnalyzer.h"
#include "ImageCreation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


typedef struct
{
	// projector setup
	int width; // width of projection
	int height; // height of projection
	char color[40]; // color of projection
	int textPosition[2][2]; // (x,y) position of speed and turn distance text

	// mapping data set by mapping and routing
	int velocity; // velocity in meters/sseconds
	int road_angle; // road angle in +/- 90 degrees with respect to vehicle
	int distance; // distance before next turn is made in meters
	int turn_angle; // angle of next turn in +/- 180 degrees with respect to road currently on
	int x1; // pixel box range for guess of road location
	int y1;
	int x2;
	int y2;

	// mapping data set by OverlayControl
	time_t time_since_mapping_update;
	int data_currently_in_use; // 1 for yes, 0 for no
	char speed_text[40]; // text of speed to be displayed on window
	char distance_to_next_turn_text[40]; // text of distance to next turn to be displayed on window

	// calculated pixels for arrow placement
	int triangle[3][2];
	int square[4][2];

} OverlayRendererControl;


// method to initialize  the overlay rendering system - projector size and color
void initializeSystem(int xdimension, int ydimension, char projection_color[]);

// mrolethod to change the color of the projection
void setProjectionColor(char projection_color[]);

// method to update the current route information
int updateRouteData(int velocity, int road_angle, int distance, int turn_angle, int x1, int y1, int x2, int y2);

// method responsible for controlling the RouteInputAnalyzer, ChoicePath, and ImageCreation
void processImageData();


#endif /* OVERLAYRENDERERCONTROL_H_ */
