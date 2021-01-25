//##########################################################################
//#                                                                        #
//#                       CLOUDCOMPARE PLUGIN: qM3C2                       #
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

#include "LibpointMatcherDialog.h"

//qCC
#include "ccMainAppInterface.h"

//qCC_db
#include <ccFileUtils.h>
#include <ccPointCloud.h>

//Qt
#include <QMainWindow>
#include <QComboBox>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QstackedWidget>
#include <Qstring>

static bool s_firstTimeInit = true;

/*** HELPERS ***/
static QString GetEntityName(ccHObject* obj)
{
	if (!obj)
	{
		assert(false);
		return QString();
	}

	QString name = obj->getName();
	if (name.isEmpty())
		name = "unnamed";
	name += QString(" [ID %1]").arg(obj->getUniqueID());

	return name;
}



LibpointmatcherDialog::LibpointmatcherDialog(ccMainAppInterface* app)
	: QDialog(app ? app->getMainWindow() : nullptr)
	, Ui::LibpointmatcherDialog()
	, m_app(app)
	, m_corePointsCloud(nullptr)
{
	setupUi(this);


}





void LibpointmatcherDialog::acceptFilterOptions() {
	int indexFilter = Options->currentIndex();
	bool needNormals = false;
	bool useExistingNormals = true;
	

	std::shared_ptr<PM::DataPointsFilter> filterParams;
	
	switch (indexFilter) {
	

	case 0:
	{
		//MaximumDensityFilter
		std::string maxDensityValue = std::to_string(maxDensity->value());
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"MaxDensityDataPointsFilter",
			{
				{"maxDensity", maxDensityValue},
			}
		);
		break;
	}
	case 1:
	{
		//DistanceLimitFilter
		std::string distValue = std::to_string(distanceThreshold->value());

		std::string dimValue;
		if (dimRadial->isChecked()) { dimValue = "-1"; }
		else if (dimX->isChecked()) { dimValue = "0"; }
		else if (dimY->isChecked()) { dimValue = "1"; }
		else { dimValue = "2"; }

		std::string removeInsideValue = "1";
		if (keepDimOuside->isChecked()) { removeInsideValue = "0"; }

		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"DistanceLimitDataPointsFilter",
			{
				{"dim", dimValue},
				{"dist", distValue},
				{"removeInside",removeInsideValue}
			}
		);
		break;
	}
	case 2:
	{	ccLog::Print(QString("Im in"));
		//MaximumPointCountFilter
		std::string seedValue = std::to_string((int)round(srandSeed->value()));
		std::string maxCountValue = std::to_string((int)round(maxPointCount->value()));
		ccLog::Print(QString::fromStdString(maxCountValue));
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"MaxPointCountDataPointsFilter",
			{
				{"seed",seedValue},
				{"maxCount", maxCountValue},
			}
		);
		
		break;
	}
	case 3:
	{
		//MaximumQuantileAxisFilter
		std::string ratioValue = std::to_string(maxPointCount->value());
		std::string dimValue;
		if (dimX_Quantile->isChecked()) { dimValue = "0"; }
		else if (dimY_Quantile->isChecked()) { dimValue = "1"; }
		else { dimValue = "2"; }

		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"MaxQuantileOnAxisDataPointsFilter",
			{
				{"dim", dimValue},
				{"ratio", ratioValue},
			}
		);

		break;
	}
	case 4:
	{
		//RandomSamplingFilter
		std::string probValue = std::to_string(randomRatio->value());
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"RandomSamplingDataPointsFilter",
			{
				{"prob", probValue},

			}
		);
		break;
	}
	case 5:
	{
		//ShadowPointFilter
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"ShadowDataPointsFilter"
		);
		if (!shadowNormals->isChecked()) { useExistingNormals = false; }
		needNormals = true;
		break;
	}
	case 6:
	{
		//VoxelGridFilter
		std::string vSizeXValue = std::to_string(voxelSizeX->value());
		std::string vSizeYValue = std::to_string(voxelSizeY->value());
		std::string vSizeZValue = std::to_string(voxelSizeZ->value());
		std::string useCentroidValue = "1";
		if (voxelCenter->isChecked()) { useCentroidValue = "0"; }
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"VoxelGridDataPointsFilter",
			{
				{"vSizeX", vSizeXValue},
				{"vSizeY", vSizeYValue },
				{"vSizeZ", vSizeZValue},
				{"useCentroid", useCentroidValue},
				{"averageExistingDescriptors","0"}
			}
		);
		break;
	}
	case 7:
	{
		//OctreeGridFilter
		std::string maxPointByNodeValue = std::to_string(stopPointOctree->value());
		std::string maxSizeByNodeValue = std::to_string(octreeSize->value());
		std::string samplingMethodValue;
		if (octreeSampleFirst->isChecked()) { samplingMethodValue = "0"; }
		else if (octreeSampleRandom->isChecked()) { samplingMethodValue = "1"; }
		else if (octreeSampleCentroid->isChecked()) { samplingMethodValue = "2"; }
		else { samplingMethodValue = "3"; }
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"OctreeGridDataPointsFilter",
			{
				{"buildParallel", "1"},
				{"maxPointByNode", maxPointByNodeValue },
				{"maxSizeByNode", maxSizeByNodeValue},
				{"samplingMethod", samplingMethodValue}
			}
		);



		break;
	}
	case 8:
	{
		//NormalSpaceSamplingFilter
		const double halfC = M_PI / 180;
		std::string epsilonValue = std::to_string(epsilonNormal->value());
		std::string seedValue = std::to_string(srandNormal->value());
		std::string nbSampleValue = std::to_string(maxPointCountNormal->value()*halfC);
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"NormalSpaceDataPointsFilter",
			{
				{"nbSample", nbSampleValue},
				{"seed", seedValue},
				{"epsilon", epsilonValue}
			}
		);

		if (!normalNormals_2->isChecked()) { useExistingNormals = false; }
		needNormals = true;
		break;
	}
	case 9:
	{
		//CovarianceSamplingFilter
		std::string torqueNormValue;
		std::string nbSampleValue = std::to_string(maxPointCountNormal->value());
		
		if (torqueNo->isChecked()) { torqueNormValue = "0"; }
		else if (torqueAvg->isChecked()) { torqueNormValue = "1"; }
		else {torqueNormValue = "2";}
		
		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"CovarianceSamplingDataPointsFilter",
			{
				{"nbSample", nbSampleValue},
				{"torqueNorm", torqueNormValue},
			}
		);
		if (!covNormals->isChecked()) { useExistingNormals = false; }
		needNormals = true;
		break;
	}
	case 10:
	{
		//SpectralDecompositionFilter
		std::string kValue = std::to_string(spdfKnn->value());
		std::string sigmaValue = std::to_string(spdfSigma->value());
		std::string radiusValue = std::to_string(spdfRadius->value());
		std::string itMaxValue = std::to_string(spdfIter->value());

		filterParams = PM::get().DataPointsFilterRegistrar.create(
			"SpectralDecompositionDataPointsFilter",
			{
				{"k", kValue},
				{"sigma", sigmaValue},
				{"radius", radiusValue},
				{"itMax", itMaxValue},
				{"keepNormals", "0"},
				{"keepLabels", "0"},
				{"keepLambdas", "0"},
				{"keepTensors", "0"},
			}
		);

		if (!spdfNormals->isChecked()) { useExistingNormals = false; }
		needNormals = true;
		break;
	}
	}
	//Push into the fitler params vector
	m_filters.push_back(filterParams);
}



void LibpointmatcherDialog::setCloud1Visibility(bool state)
{
	if (m_cloud1)
	{
		m_cloud1->setVisible(state);
		m_cloud1->prepareDisplayForRefresh();
	}
	if (m_app)
	{
		m_app->refreshAll();
		m_app->updateUI();
	}
}

void LibpointmatcherDialog::setCloud2Visibility(bool state)
{
	if (m_cloud2)
	{
		m_cloud2->setVisible(state);
		m_cloud2->prepareDisplayForRefresh();
	}
	if (m_app)
	{
		m_app->refreshAll();
		m_app->updateUI();
	}
}


LibpointmatcherDialog::ExportOptions LibpointmatcherDialog::getExportOption() const
{
	switch (projDestComboBox->currentIndex())
	{
	case 0:
		return PROJECT_ON_CLOUD1;
	case 1:
		return PROJECT_ON_CLOUD2;
	case 2:
		return PROJECT_ON_CORE_POINTS;
	default:
		assert(false);
		break;
	}

	return PROJECT_ON_CORE_POINTS;
}

void LibpointmatcherDialog::projDestIndexChanged(int index)
{
	useOriginalCloudCheckBox->setEnabled(getExportOption() == PROJECT_ON_CORE_POINTS);
}

bool LibpointmatcherDialog::keepOriginalCloud() const
{
	return useOriginalCloudCheckBox->isEnabled() && useOriginalCloudCheckBox->isChecked();
}

int LibpointmatcherDialog::getMaxThreadCount() const
{
	return maxThreadCountSpinBox->value();
}

unsigned LibpointmatcherDialog::getMinPointsForStats(unsigned defaultValue/*=5*/) const
{
	return useMinPoints4StatCheckBox->isChecked() ? static_cast<unsigned>(std::max(0,minPoints4StatSpinBox->value())) : defaultValue;
}


