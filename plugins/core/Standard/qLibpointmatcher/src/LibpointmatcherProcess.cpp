//##########################################################################
//#                                                                        #
//#                       CLOUDCOMPARE PLUGIN: Libpointmatcher             #
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
//#            COPYRIGHT: UNIVERSITE EUROPEENNE DE BRETAGNE                #
//#                                                                        #
//##########################################################################

#include "LibpointmatcherProcess.h"

//local
#include "LibpointmatcherTools.h"
#include "LibpointmatcherDialog.h"

//CCCoreLib
#include <CloudSamplingTools.h>

//qCC_plugins
#include <ccMainAppInterface.h>

//qCC_db
#include <ccGenericPointCloud.h>
#include <ccPointCloud.h>
#include <ccOctree.h>
#include <ccOctreeProxy.h>
#include <ccHObjectCaster.h>
#include <ccProgressDialog.h>
#include <ccNormalVectors.h>
#include <ccScalarField.h>

//Qt
#include <QtGui>
#include <QtCore>
#include <QApplication>
#include <QElapsedTimer>
#include <QtConcurrentMap>
#include <QMessageBox>

//! Default name for M3C2 scalar fields
static const char M3C2_DIST_SF_NAME[]			= "M3C2 distance";
static const char DIST_UNCERTAINTY_SF_NAME[]	= "distance uncertainty";
static const char SIG_CHANGE_SF_NAME[]			= "significant change";
static const char STD_DEV_CLOUD1_SF_NAME[]		= "%1_cloud1";
static const char STD_DEV_CLOUD2_SF_NAME[]		= "%1_cloud2";
static const char DENSITY_CLOUD1_SF_NAME[]		= "Npoints_cloud1";
static const char DENSITY_CLOUD2_SF_NAME[]		= "Npoints_cloud2";
static const char NORMAL_SCALE_SF_NAME[]		= "normal scale";

static void RemoveScalarField(ccPointCloud* cloud, const char sfName[])
{
	int sfIdx = cloud ? cloud->getScalarFieldIndexByName(sfName) : -1;
	if (sfIdx >= 0)
	{
		cloud->deleteScalarField(sfIdx);
	}
}

static ScalarType SCALAR_ZERO = 0;
static ScalarType SCALAR_ONE = 1;

// Precision maps (See "3D uncertainty-based topographic change detection with SfM photogrammetry: precision maps for ground control and directly georeferenced surveys" by James et al.)
struct PrecisionMaps
{
	PrecisionMaps() : sX(nullptr), sY(nullptr), sZ(nullptr), scale(1.0) {}
	bool valid() const { return (sX != nullptr && sY != nullptr && sZ != nullptr); }
	CCCoreLib::ScalarField *sX, *sY, *sZ;
	double scale;
};

// Computes the uncertainty based on 'precision maps' (as scattered scalar fields)
static double ComputePMUncertainty(CCCoreLib::DgmOctree::NeighboursSet& set, const CCVector3& N, const PrecisionMaps& PM)
{
	size_t count = set.size();
	if (count == 0)
	{
		assert(false);
		return 0;
	}
	
	int minIndex = -1;
	if (count == 1)
	{
		minIndex = 0;
	}
	else
	{
		//compute gravity center
		CCVector3d G(0, 0, 0);
		for (size_t i = 0; i < count; ++i)
		{
			G.x += set[i].point->x;
			G.y += set[i].point->y;
			G.z += set[i].point->z;
		}

		G.x /= count;
		G.y /= count;
		G.z /= count;

		//now look for the point that is the closest to the gravity center
		double minSquareDist = -1.0;
		minIndex = -1;
		for (size_t i = 0; i < count; ++i)
		{
			CCVector3d dG(	G.x - set[i].point->x,
							G.y - set[i].point->y,
							G.z - set[i].point->z );
			double squareDist = dG.norm2();
			if (minIndex < 0 || squareDist < minSquareDist)
			{
				minSquareDist = squareDist;
				minIndex = static_cast<int>(i);
			}
		}
	}
	
	assert(minIndex >= 0);
	unsigned pointIndex = set[minIndex].pointIndex;
	CCVector3d sigma(	PM.sX->getValue(pointIndex) * PM.scale,
						PM.sY->getValue(pointIndex) * PM.scale,
						PM.sZ->getValue(pointIndex) * PM.scale);

	CCVector3d NS(	N.x * sigma.x,
					N.y * sigma.y,
					N.z * sigma.z);
	
	return NS.norm();
}

// Structure for parallel call to ComputeM3C2DistForPoint
struct M3C2Params
{
	//input data
	ccPointCloud* outputCloud = nullptr;
	ccPointCloud* corePoints = nullptr;
	NormsIndexesTableType* coreNormals = nullptr;

	//main options
	PointCoordinateType projectionRadius = 0;
	PointCoordinateType projectionDepth = 0;
	bool updateNormal = false;
	bool exportNormal = false;
	bool useMedian = false;
	bool computeConfidence = false;
	bool progressiveSearch = false;
	bool onlyPositiveSearch = false;
	unsigned minPoints4Stats = 3;
	double registrationRms = 0;

	//export
	LibpointmatcherDialog::ExportOptions exportOption;
	bool keepOriginalCloud = false;

	//octrees
	ccOctree::Shared cloud1Octree;
	unsigned char level1 = 0;
	ccOctree::Shared cloud2Octree;
	unsigned char level2 = 0;

	//scalar fields
	ccScalarField* m3c2DistSF = nullptr;		//M3C2 distance
	ccScalarField* distUncertaintySF = nullptr;	//distance uncertainty
	ccScalarField* sigChangeSF = nullptr;		//significant change
	ccScalarField* stdDevCloud1SF = nullptr;	//standard deviation information for cloud #1
	ccScalarField* stdDevCloud2SF = nullptr;	//standard deviation information for cloud #2
	ccScalarField* densityCloud1SF = nullptr;	//export point density at projection scale for cloud #1
	ccScalarField* densityCloud2SF = nullptr;	//export point density at projection scale for cloud #2

	//precision maps
	PrecisionMaps cloud1PM, cloud2PM;
	bool usePrecisionMaps = false;

	//progress notification
	CCCoreLib::NormalizedProgress* nProgress = nullptr;
	bool processCanceled = false;
};
static M3C2Params s_M3C2Params;

void ComputeM3C2DistForPoint(unsigned index)
{
	if (s_M3C2Params.processCanceled)
		return;

	ScalarType dist = CCCoreLib::NAN_VALUE;

	//get core point #i
	CCVector3 P;
	s_M3C2Params.corePoints->getPoint(index, P);

	//get core point's normal #i
	CCVector3 N(0, 0, 1);
	if (s_M3C2Params.updateNormal) //i.e. all cases but the VERTICAL mode
	{
		N = ccNormalVectors::GetNormal(s_M3C2Params.coreNormals->getValue(index));
	}

	//output point
	CCVector3 outputP = P;

	//compute M3C2 distance
	{
		double mean1 = 0;
		double stdDev1 = 0;
		bool validStats1 = false;

		//extract cloud #1's neighbourhood
		CCCoreLib::DgmOctree::ProgressiveCylindricalNeighbourhood cn1;
		cn1.center = P;
		cn1.dir = N;
		cn1.level = s_M3C2Params.level1;
		cn1.maxHalfLength = s_M3C2Params.projectionDepth;
		cn1.radius = s_M3C2Params.projectionRadius;
		cn1.onlyPositiveDir = s_M3C2Params.onlyPositiveSearch;

		if (s_M3C2Params.progressiveSearch)
		{
			//progressive search
			size_t previousNeighbourCount = 0;
			while (cn1.currentHalfLength < cn1.maxHalfLength)
			{
				size_t neighbourCount = s_M3C2Params.cloud1Octree->getPointsInCylindricalNeighbourhoodProgressive(cn1);
				if (neighbourCount != previousNeighbourCount)
				{
					//do we have enough points for computing stats?
					if (neighbourCount >= s_M3C2Params.minPoints4Stats)
					{
						LibpointmatcherTools::ComputeStatistics(cn1.neighbours, s_M3C2Params.useMedian, mean1, stdDev1);
						validStats1 = true;
						//do we have a sharp enough 'mean' to stop?
						if (fabs(mean1) + 2 * stdDev1 < static_cast<double>(cn1.currentHalfLength))
							break;
					}
					previousNeighbourCount = neighbourCount;
				}
			}
		}
		else
		{
			s_M3C2Params.cloud1Octree->getPointsInCylindricalNeighbourhood(cn1);
		}
		
		size_t n1 = cn1.neighbours.size();
		if (n1 != 0)
		{
			//compute stat. dispersion on cloud #1 neighbours (if necessary)
			if (!validStats1)
			{
				LibpointmatcherTools::ComputeStatistics(cn1.neighbours, s_M3C2Params.useMedian, mean1, stdDev1);
			}

			if (s_M3C2Params.usePrecisionMaps && (s_M3C2Params.computeConfidence || s_M3C2Params.stdDevCloud1SF))
			{
				//compute the Precision Maps derived sigma
				stdDev1 = ComputePMUncertainty(cn1.neighbours, N, s_M3C2Params.cloud1PM);
			}

			if (s_M3C2Params.exportOption == LibpointmatcherDialog::PROJECT_ON_CLOUD1)
			{
				//shift output point on the 1st cloud
				outputP += static_cast<PointCoordinateType>(mean1) * N;
			}

			//save cloud #1's std. dev.
			if (s_M3C2Params.stdDevCloud1SF)
			{
				ScalarType val = static_cast<ScalarType>(stdDev1);
				s_M3C2Params.stdDevCloud1SF->setValue(index, val);
			}
		}

		//save cloud #1's density
		if (s_M3C2Params.densityCloud1SF)
		{
			ScalarType val = static_cast<ScalarType>(n1);
			s_M3C2Params.densityCloud1SF->setValue(index, val);
		}

		//now we can process cloud #2
		if (	n1 != 0
			||	s_M3C2Params.exportOption == LibpointmatcherDialog::PROJECT_ON_CLOUD2
			||	s_M3C2Params.stdDevCloud2SF
			||	s_M3C2Params.densityCloud2SF
			)
		{
			double mean2 = 0;
			double stdDev2 = 0;
			bool validStats2 = false;
			
			//extract cloud #2's neighbourhood
			CCCoreLib::DgmOctree::ProgressiveCylindricalNeighbourhood cn2;
			cn2.center = P;
			cn2.dir = N;
			cn2.level = s_M3C2Params.level2;
			cn2.maxHalfLength = s_M3C2Params.projectionDepth;
			cn2.radius = s_M3C2Params.projectionRadius;
			cn2.onlyPositiveDir = s_M3C2Params.onlyPositiveSearch;

			if (s_M3C2Params.progressiveSearch)
			{
				//progressive search
				size_t previousNeighbourCount = 0;
				while (cn2.currentHalfLength < cn2.maxHalfLength)
				{
					size_t neighbourCount = s_M3C2Params.cloud2Octree->getPointsInCylindricalNeighbourhoodProgressive(cn2);
					if (neighbourCount != previousNeighbourCount)
					{
						//do we have enough points for computing stats?
						if (neighbourCount >= s_M3C2Params.minPoints4Stats)
						{
							LibpointmatcherTools::ComputeStatistics(cn2.neighbours, s_M3C2Params.useMedian, mean2, stdDev2);
							validStats2 = true;
							//do we have a sharp enough 'mean' to stop?
							if (fabs(mean2) + 2 * stdDev2 < static_cast<double>(cn2.currentHalfLength))
								break;
						}
						previousNeighbourCount = neighbourCount;
					}
				}
			}
			else
			{
				s_M3C2Params.cloud2Octree->getPointsInCylindricalNeighbourhood(cn2);
			}

			size_t n2 = cn2.neighbours.size();
			if (n2 != 0)
			{
				//compute stat. dispersion on cloud #2 neighbours (if necessary)
				if (!validStats2)
				{
					LibpointmatcherTools::ComputeStatistics(cn2.neighbours, s_M3C2Params.useMedian, mean2, stdDev2);
				}
				assert(stdDev2 != stdDev2 || stdDev2 >= 0); //first inequality fails if stdDev2 is NaN ;)

				if (s_M3C2Params.exportOption == LibpointmatcherDialog::PROJECT_ON_CLOUD2)
				{
					//shift output point on the 2nd cloud
					outputP += static_cast<PointCoordinateType>(mean2) * N;
				}

				if (s_M3C2Params.usePrecisionMaps && (s_M3C2Params.computeConfidence || s_M3C2Params.stdDevCloud2SF))
				{
					//compute the Precision Maps derived sigma
					stdDev2 = ComputePMUncertainty(cn2.neighbours, N, s_M3C2Params.cloud2PM);
				}

				if (n1 != 0)
				{
					//m3c2 dist = distance between i1 and i2 (i.e. either the mean or the median of both neighborhoods)
					dist = static_cast<ScalarType>(mean2 - mean1);
					s_M3C2Params.m3c2DistSF->setValue(index, dist);

					//confidence interval
					if (s_M3C2Params.computeConfidence)
					{
						ScalarType LODStdDev = CCCoreLib::NAN_VALUE;
						if (s_M3C2Params.usePrecisionMaps)
						{
							LODStdDev = stdDev1*stdDev1 + stdDev2*stdDev2; //equation (2) in M3C2-PM article
						}
						//standard M3C2 algortihm: have we enough points for computing the confidence interval?
						else if (n1 >= s_M3C2Params.minPoints4Stats && n2 >= s_M3C2Params.minPoints4Stats)
						{
							LODStdDev = (stdDev1*stdDev1) / n1 + (stdDev2*stdDev2) / n2;
						}

						if (!std::isnan(LODStdDev))
						{
							//distance uncertainty (see eq. (1) in M3C2 article)
							ScalarType LOD = static_cast<ScalarType>(1.96 * (sqrt(LODStdDev) + s_M3C2Params.registrationRms));

							if (s_M3C2Params.distUncertaintySF)
							{
								s_M3C2Params.distUncertaintySF->setValue(index, LOD);
							}

							if (s_M3C2Params.sigChangeSF)
							{
								bool significant = (dist < -LOD || dist > LOD);
								if (significant)
								{
									s_M3C2Params.sigChangeSF->setValue(index, SCALAR_ONE); //already equal to SCALAR_ZERO otherwise
								}
							}
						}
						//else //DGM: scalar fields have already been initialized with the right 'default' values
						//{
						//	if (distUncertaintySF)
						//		distUncertaintySF->setValue(index, CCCoreLib::NAN_VALUE);
						//	if (sigChangeSF)
						//		sigChangeSF->setValue(index, SCALAR_ZERO);
						//}
					}
				}

				//save cloud #2's std. dev.
				if (s_M3C2Params.stdDevCloud2SF)
				{
					ScalarType val = static_cast<ScalarType>(stdDev2);
					s_M3C2Params.stdDevCloud2SF->setValue(index, val);
				}
			}

			//save cloud #2's density
			if (s_M3C2Params.densityCloud2SF)
			{
				ScalarType val = static_cast<ScalarType>(n2);
				s_M3C2Params.densityCloud2SF->setValue(index, val);
			}
		}
	}

	//output point
	if (s_M3C2Params.outputCloud != s_M3C2Params.corePoints)
	{
		*const_cast<CCVector3*>(s_M3C2Params.outputCloud->getPoint(index)) = outputP;
	}
	if (s_M3C2Params.exportNormal)
	{
		s_M3C2Params.outputCloud->setPointNormal(index, N);
	}

	//progress notification
	if (s_M3C2Params.nProgress && !s_M3C2Params.nProgress->oneStep())
	{
		s_M3C2Params.processCanceled = true;
	}
}
bool LibpointmatcherProcess::Subsample(const LibpointmatcherDialog& dlg, ccHObject* entity, QString& errorMessage, QWidget* parentWidget/*=nullptr*/, ccMainAppInterface* app/*=nullptr*/)
{
	errorMessage.clear();
	
	//get the clouds in the right order


	if (!entity->isA(CC_TYPES::POINT_CLOUD))
	{
		assert(false);
		return false;
	}
	ccPointCloud* cloud1 = ccHObjectCaster::ToPointCloud(entity);


	//start the job
	bool error = false;

	//start Subsampling 
	bool hasNormalDescriptors;
	//Converting to Pointmatcher format
	//verify the type of transform depending if it has normal or not
	DP convertedCloud;
	if (cloud1->hasNormals() && dlg.needAtLeastOneNormal() && dlg.useAtLeastOneNormal())
	{
		//Has Normals and will be added
		convertedCloud = LibpointmatcherTools::ccNormalsToPointMatcher(cloud1);
		hasNormalDescriptors = true;
	}
	else
	{
		convertedCloud = LibpointmatcherTools::ccToPointMatcher(cloud1);
		hasNormalDescriptors = false;
		
	}
	// Iterate through the different filters
	
	
	// Filtering with DP format
	DP filteredCloud;
	filteredCloud = LibpointmatcherTools::filter(convertedCloud, dlg.getFilters(), dlg.getNormalParams(),dlg.needNormals(), hasNormalDescriptors);
	if (!filteredCloud.getNbPoints()>0)
	{
		errorMessage = "Failed to compute!";
		return true;
	}
	DP* filteredCloudPtr = &filteredCloud;
	//Transforming the Pointmatcher subsampled to a ref cloud
	CCCoreLib::ReferenceCloud* subsampled = LibpointmatcherTools::pointmatcherToCC(filteredCloudPtr, cloud1);

	//Applying the ref cloud to a new generated point cloud
	ccPointCloud* cloud1subsampled = static_cast<ccPointCloud*>(cloud1)->partialClone(subsampled);
	//don't need those references anymore
	delete subsampled;
	subsampled = nullptr;
	if (cloud1subsampled->size()>0) 
	{
		cloud1subsampled->setName(QString("%1.subsample").arg(cloud1->getName()));
		cloud1subsampled->setVisible(true);
		cloud1subsampled->setDisplay(cloud1->getDisplay());
		if (app)
		{
			app->dispToConsole(QString("[LIBPOINTMATCHER] Sub-sampled cloud has been saved ('%1')").arg(cloud1subsampled->getName()), ccMainAppInterface::STD_CONSOLE_MESSAGE);
			app->addToDB(cloud1subsampled);
		}
	
	}
	else
	{
		errorMessage = "Failed to compute!";
		error = true;
	}
	
	return !error;
}




