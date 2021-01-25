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

#ifndef LIBPOINTMATCHER_DIALOG_HEADER
#define LIBPOINTMATCHER_DIALOG_HEADER

#include <ui_LibpointmatcherDialog.h>

//Local
#include <LibpointmatcherTools.h>

//Qt
#include <QSettings>

class ccMainAppInterface;
class ccPointCloud;
class ccHObject;

//! M3C2 plugin's main dialog
class LibpointmatcherDialog : public QDialog, public Ui::LibpointmatcherDialog
{
	Q_OBJECT

public:

	//! Default constructor
	LibpointmatcherDialog(ccMainAppInterface* app);

	//! Returns cloud #1
	ccPointCloud* getCloud1() const { return m_cloud1; }
	//! Returns cloud #2
	ccPointCloud* getCloud2() const { return m_cloud2; }



	//! Returns vector of parameters
	std::vector< std::shared_ptr<PM::DataPointsFilter>> getFilters() const { return m_filters; }

	//! Returns the minimum number of points to compute stats (confidence mainly)
	unsigned getMinPointsForStats(unsigned defaultValue = 5) const;

	//! Exportation options
	enum ExportOptions {	PROJECT_ON_CLOUD1,
							PROJECT_ON_CLOUD2,
							PROJECT_ON_CORE_POINTS,
	};

	//! Returns selected export option
	ExportOptions getExportOption() const;

	//! Returns whether the original cloud should be kept instead of creating a new output one
	/** Only valid if the export option is PROJECT_ON_CORE_POINTS.
	**/
	bool keepOriginalCloud() const;

	//! Returns the max number of threads to use
	int getMaxThreadCount() const;
	// change the filter options
	void acceptFilterOptions();


protected:

	void setCloud1Visibility(bool);
	void setCloud2Visibility(bool);
	void projDestIndexChanged(int);

	


protected: //members

	ccMainAppInterface* m_app;


	ccPointCloud* m_cloud1;
	ccPointCloud* m_cloud2;
	ccPointCloud* m_corePointsCloud;
	std::vector< std::shared_ptr<PM::DataPointsFilter>> m_filters;
};

#endif 
