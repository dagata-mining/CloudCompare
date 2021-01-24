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

#ifndef CC_LIBPOINTMATCHER_FILTER_HEADER
#define CC_LIBPOINTMATCHER_FILTER_HEADER

#include "pointmatcher/PointMatcher.h"
#include "nabo/nabo.h"
#include "CCCoreLib.h"
#include "ccGenericPointCloud.h"
#include "LibpointmatcherConvert.h"

typedef PointMatcher<float> PM;
typedef PM::DataPoints DP;
class ccGenericPointCloud;

class LibpointmatcherFilter 
{	
	//Parameters for filtering 
public:
	static DP filter(DP);
	static CCCoreLib::ReferenceCloud* filterChain(ccGenericPointCloud*);
}

;
#endif