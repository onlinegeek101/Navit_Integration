/*
 * RouteInputAnalyzer.h
 *
 *  Created on: Dec 3, 2011
 *      Author: ubuntu3
 */

#ifndef ROUTEINPUTANALYZER_H_
#define ROUTEINPUTANALYZER_H_

#include "OverlayRendererControl.h"
#include "ImageCreation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


// method for OverlayRendererControl to call to get updated arrow dimensions from mapping and routing data
void processGenericImageData(OverlayRendererControl *control);


#endif /* ROUTEINPUTANALYZER_H_ */
