//##########################################################################
//#                                                                        #
//#                              CLOUDCOMPARE                              #
//#                                                                        #
//#  This program is free software; you can redistribute it and/or modify  #
//#  it under the terms of the GNU General Public License as published by  #
//#  the Free Software Foundation; version 2 or later of the License.      #
//#                                                                        #
//#  This program is distributed in the hope that it will be useful,       #
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of        #
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          #
//#  GNU General Public License for more details.                          #
//#                                                                        #
//#                    COPYRIGHT: CloudCompare project                     #
//#                                                                        #
//##########################################################################

#ifndef CC_LIBPOINTMATCHER_CONVERT_HEADER
#define CC_LIBPOINTMATCHER_CONVERT_HEADER

#include "pointmatcher/PointMatcher.h"
#include "nabo/nabo.h"
#include "ccGenericPointCloud.h"

typedef PointMatcher<float> PM;
typedef PM::DataPoints DP;

class ccGenericPointCloud;
class CCCoreLib;
class ccPointCloud;

DP ccToPointMatcher(ccGenericPointCloud*);

	

#endif
