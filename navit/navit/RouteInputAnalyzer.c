/*
 * RouteInputAnalyzer.c
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

// method for drawing a generic "one sized fits all" right arrow
static void genericRightTurn(OverlayRendererControl *controller)
{
	int p1x = controller->width / 2;
	int p1y = controller->height / 2 - controller->height / 10;
	int p2x = controller->width - 100;
	int p2y = controller->height / 2;

	// Calculate the pixel locations for the three points of the arrow head
	// x0
	controller->triangle[0][0] = p2x - (p2x-p1x)/3;
	// y0
	controller->triangle[0][1] = p1y;
	// x1
	controller->triangle[1][0] = p2x;
	// y1
	controller->triangle[1][1] = p1y + (p2y-p1y)/2;
	// x2
	controller->triangle[2][0] = p2x - (p2x-p1x)/3;
	// y2
	controller->triangle[2][1] = p2y;

	// Calculate the four pixel locations for the bar of the arrow
	// x0
	controller->square[0][0]  = p1x - 2*(p2y-p1y)/3;
	// y0
	controller->square[0][1]  = p1y + (p2y-p1y)*.33;
	// x1
	controller->square[1][0]  = p2x - (p2x-p1x)/3;
	// y1
	controller->square[1][1]  = p1y + (p2y-p1y)*.33;
	// x2
	controller->square[2][0]  = p2x - (p2x-p1x)/3;
	// y2
	controller->square[2][1]  = p2y - (p2y-p1y)*.33;
	// x3
	controller->square[3][0]  = p1x - 2*(p2y-p1y)/3;
	// y3controller
	controller->square[3][1]  = p2y - (p2y-p1y)*.33;
}

// method for drawing a generic "one sized fits all" left arrow
static void genericLeftTurn(OverlayRendererControl *controller)
{
	int p1x = 100;
	int p1y = controller->height / 2 - controller->height / 10;
	int p2x = controller->width / 2;
	int p2y = controller->height / 2;

	// Calculate the pixel locations for the three points of the arrow head
	// x0
	controller->triangle[0][0] = p1x + (p2x-p1x)/3;
	// y0
	controller->triangle[0][1] = p1y;
	// x1
	controller->triangle[1][0] = p1x;
	// y1
	controller->triangle[1][1] = p1y + (p2y-p1y)/2;
	// x2
	controller->triangle[2][0] = p1x + (p2x-p1x)/3;
	// y2
	controller->triangle[2][1] = p2y;

	// Calculate the four pixel locations for the bar of the arrow
	// x0
	controller->square[0][0]  = p1x + (p2x-p1x)/3;
	// y0
	controller->square[0][1]  = p1y + (p2y-p1y)*.33;
	// x1
	controller->square[1][0]  = p2x;
	// y1
	controller->square[1][1]  = p1x + (p2x-p1x)/3;
	// x2
	controller->square[2][0]  = p2x;
	// y2
	controller->square[2][1]  = p2y - (p2y-p1y)*.33;
	// x3
	controller->square[3][0]  = p1x + (p2x-p1x)/3;
	// y3
	controller->square[3][1]  = p2y - (p2y-p1y)*.33;
}

// method for drawing a generic "one sized fits all" straight arrow
static void genericStraightRoad(OverlayRendererControl *controller)
{
	int p1x = controller->width / 2 - controller->width / 20;
	int p1y = 100;
	int p2x = controller->width / 2 + controller->width / 20;
	int p2y = controller->height - 50;

	// Calculate the pixel locations for the three points of the arrow head
	// x0
	controller->triangle[0][0] = p1x;
	// y0
	controller->triangle[0][1] = p1y+(p2y-p1y)/3;
	// x1
	controller->triangle[1][0] = (p1x+p2x)/2;
	// y1
	controller->triangle[1][1] = p1y;
	// x2
	controller->triangle[2][0] = p2x;
	// y2
	controller->triangle[2][1] = p1y+(p2y-p1y)/3;

	// Calculate the four pixel locations for the bar of the arrow
	// x0
	controller->square[0][0]  = p1x + (p2x-p1x)*.33;
	// y0
	controller->square[0][1]  = p1y+(p2y-p1y)/3;
	// x1
	controller->square[1][0]  = p2x-(p2x-p1x)*.33;
	// y1
	controller->square[1][1]  = p1y+(p2y-p1y)/3;
	// x2
	controller->square[2][0]  = p2x-(p2x-p1x)*.33;
	// y2
	controller->square[2][1]  = p2y;
	// x3
	controller->square[3][0]  = p1x + (p2x-p1x)*.33;
	// y3
	controller->square[3][1]  = p2y;
}

// method for OverlayRendererControl to call to get updated arrow dimensions from mapping and routing data
void processGenericImageData(OverlayRendererControl *control)
{
	// pick which arrow to draw based on turn angle and distance to next turn
	if(control->turn_angle < 0 && control->distance < 150)
		genericLeftTurn(control);
	else if(control->turn_angle > 0 && control->distance < 150)
		genericRightTurn(control);
	else
		genericStraightRoad(control);
}


