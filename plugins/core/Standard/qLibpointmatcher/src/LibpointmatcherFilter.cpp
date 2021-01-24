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


#include <LibpointmatcherFilter.h>

DP  LibpointmatcherFilter::filter(DP cloud)
{
	std::shared_ptr<PM::DataPointsFilter> params;
	params= PM::get().DataPointsFilterRegistrar.create(
		"VoxelGridDataPointsFilter",
		{
			{"vSizeX", "0.2"},
			{"vSizeY", "0.2"},
			{"vSizeZ", "0.2"},
			{"useCentroid","1"},
			{"averageExistingDescriptors","0"}
		}
	);
	params->inPlaceFilter(cloud);
	return cloud;
}
CCCoreLib::ReferenceCloud* LibpointmatcherFilter::filterChain(ccGenericPointCloud* cloud) {
	DP convertedCloud = LibpointmatcherConvert::ccToPointMatcher(cloud);
	DP filteredCloud;
	filteredCloud=LibpointmatcherFilter::filter(convertedCloud);
	DP* filteredCloudPtr = &filteredCloud ;
	return LibpointmatcherConvert::pointmatcherToCC(filteredCloudPtr, cloud);
}
;