//##########################################################################
//#                                                                        #
//#                      CLOUDCOMPARE PLUGIN: qPclIO                       #
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

#include <LibpointmatcherConvert.h>

//qCC_db
#include <ccHObject.h>

DP ccToPointMatcher(ccGenericPointCloud* cloud)
{
	typedef DP::Label Label;
	typedef DP::Labels Labels;
	typedef DP::View View;

	if (cloud-> size() <=0 )
		return DP();
	Labels featLabels;
	Labels descLabels;
	std::vector<bool> isFeature;
	featLabels.push_back(Label("x", 1));
	isFeature.push_back(true);
	featLabels.push_back(Label("y", 1));
	isFeature.push_back(true);
	featLabels.push_back(Label("z", 1));
	isFeature.push_back(true);
	featLabels.push_back(Label("i", 1));
	isFeature.push_back(true);
	featLabels.push_back(Label("pad", 1));

	DP cloudDP(featLabels, descLabels, cloud->size());
	cloudDP.getFeatureViewByName("pad").setConstant(1);
	// fill cloud
	View view(cloudDP.getFeatureViewByName("x"));
	const CCVector3* P3D;
#if defined(_OPENMP)
#pragma omp parallel for
#endif
	for (int i = 0; i < cloud->size(); ++i)
	{
		P3D = cloud->getPoint(i);
		view(0, i) = P3D-> x ;
		view(1, i) = P3D-> y;
		view(2, i) = P3D-> z;
		view(3, i) = i;

	}

	return cloudDP;
}
;
