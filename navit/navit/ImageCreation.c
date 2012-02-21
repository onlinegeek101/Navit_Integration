/*
 * ImageCreation.c
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

#include <wand/magick_wand.h>
#include <MagickWand.h>


void drawImage(OverlayRendererControl *control)
{
	MagickWand *m_wand = NULL;
	DrawingWand *d_wand = NULL;
	PixelWand *c_wand = NULL;

	MagickWandGenesis();

	m_wand = NewMagickWand();
	d_wand = NewDrawingWand();
	c_wand = NewPixelWand();

	PixelSetColor(c_wand, "black"); // set color #include <time.h>of pixel colors to be drawn for background

	MagickNewImage(m_wand,control->width,control->height,c_wand); // create the image

	DrawSetStrokeOpacity(d_wand,1); // set opacity of the drawing
	PixelSetColor(c_wand,control->color); // set color of pixel colors to be drawn for edge of shape


	/* Start Draw Bar of Arrow */
	PushDrawingWand(d_wand);

	DrawSetStrokeColor(d_wand,c_wand); // set stroke color
	DrawSetStrokeWidth(d_wand,1); // set the width of the stroke
	DrawSetStrokeAntialias(d_wand,1); // no clue what this does =(
	DrawSetFillColor(d_wand,c_wand); // set the shape fill color
	DrawRectangle(d_wand,control->square[0][0], control->square[0][1], control->square[2][0], control->square[2][1]); // Draw out the Rectangle using points 0 and 2

	PopDrawingWand(d_wand);

	/* End Draw Bar of Arrow */


	/* Start Draw Arrow Head */
	PushDrawingWand(d_wand);
	{
		const PointInfo points[3] =
	    {
	    		{ control->triangle[0][0], control->triangle[0][1] },
	    		{ control->triangle[1][0], control->triangle[1][1] },
	    		{ control->triangle[2][0], control->triangle[2][1] }
	    };

		DrawSetStrokeAntialias(d_wand, MagickTrue);
		DrawSetStrokeWidth(d_wand, 1);
		DrawSetStrokeLineCap(d_wand, RoundCap);
		DrawSetStrokeLineJoin(d_wand, RoundJoin);
		(void) DrawSetStrokeDashArray(d_wand, 0, (const double *)NULL);
		(void) PixelSetColor(c_wand, control->color);
		DrawSetStrokeColor(d_wand, c_wand);
		//DrawSetFillRule(d_wand, EvenOddRule);
		//(void) PixelSetColor(c_wand,control->color);
		DrawSetFillColor(d_wand,c_wand);
		DrawPolygon(d_wand,3,points);
	}

	PopDrawingWand(d_wand);
	/* End Draw Arrow Head */

	MagickDrawImage(m_wand,d_wand);

	DrawSetStrokeOpacity(d_wand,1); // set opacity of the drawing
	PixelSetColor(c_wand,control->color); // set color of pixel colors to be drawn for edge of shape
	DrawSetStrokeColor(d_wand,c_wand); // set stroke color
	DrawSetStrokeWidth(d_wand,1); // set the width of the stroke
	DrawSetStrokeAntialias(d_wand,1); // no clue what this does =(
	DrawSetFillColor(d_wand,c_wand); // set the shape fill color
	MagickAnnotateImage(m_wand, d_wand, control->textPosition[0][0], control->textPosition[0][1], 0, control->speed_text);
	MagickAnnotateImage(m_wand, d_wand, control->textPosition[1][0], control->textPosition[1][1], 0, control->distance_to_next_turn_text);


	MagickWriteImage(m_wand,"arrow_test.jpg");

	c_wand = DestroyPixelWand(c_wand);
	m_wand = DestroyMagickWand(m_wand);
	d_wand = DestroyDrawingWand(d_wand);

	MagickWandTerminus();
}




