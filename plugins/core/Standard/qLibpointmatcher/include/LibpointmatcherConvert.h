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

#ifndef LIBPOINTMATCHER_CONVERT_HEADER
#define LIBPOINTMATCHER_CONVERT_HEADER

#include "pointmatcher/PointMatcher.h"
#include "LibpointmatcherFilter.h"
#include "nabo/nabo.h"
#include "ccGenericPointCloud.h"
#include "CCCoreLib.h"

typedef PointMatcher<float> PM;
typedef PM::DataPoints DP;

class ccGenericPointCloud;


class LibpointmatcherConvert
{
public:
	// Converts a CloudCompare Entity to a Point Matcher Entity
	static DP ccToPointMatcher(ccGenericPointCloud* cloud);
	// Converts a pointMatcher Entity to a Cloudcompare ReferenceCloud 
	static CCCoreLib::ReferenceCloud* pointmatcherToCC(DP* cloud,ccGenericPointCloud* ref);
};
	
#endif
