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

#ifndef LIBPOINTMATCHER_OUTLIER_DIALOG_HEADER
#define LIBPOINTMATCHER__OUTLIER_DIALOG_HEADER

#include <ui_LibpointmatcherOutlierDialog.h>

//Local
#include <LibpointmatcherTools.h>

//Qt
#include <QSettings>

class ccMainAppInterface;
class ccPointCloud;
class ccHObject;

//! Libpointmatcher plugin's main dialog
class LibpointmatcherOutlierDialog : public QDialog, public Ui::LibpointmatcherOutlierDialog
{
	Q_OBJECT

public:

	//! Default constructor
	LibpointmatcherOutlierDialog(ccMainAppInterface* app);

	//! Returns cloud #1
	ccPointCloud* getCloud1() const { return m_cloud1; }
	//! Returns cloud #2
	ccPointCloud* getCloud2() const { return m_cloud2; }

	//! Do we use existing normals will tell to convert to DP with normals descriptors 
	std::vector<bool> useExistingNormalsRef() const { return m_useExistingNormalsRef; }

	//! Do we need normals to be calculated
	std::vector<bool> needNormalsRef() const { return m_needNormalsRef; }

	//! Do we need normals to be calculated at index
	bool getNeedNormalsRef(int i) const { return m_needNormalsRef[i]; }


	//! Verify is we need at least normals one time
	bool needAtLeastOneNormalRef() const { return std::all_of(m_needNormalsRef.begin(), m_needNormalsRef.end(), [](bool v) { return v; }); };

	//! Verify is we import normals one time
	bool useAtLeastOneNormal() const { return std::all_of(m_useExistingNormalsRef.begin(), m_useExistingNormalsRef.end(), [](bool v) { return v; }); };
	
	//! Returns vector of parameters
	std::vector< std::shared_ptr<PM::DataPointsFilter>> getFiltersRef() const { return m_filtersRef; }

	std::shared_ptr<PM::DataPointsFilter> getNormalParams() const {return m_normalParams;}
	
	//! change the filter options
	void acceptFilterOptions(bool);
	//! changing the selected item on the filters list disabling and enabling position change and deleting filter
	void selectingFilterItemRef();
	//! accept Normals Options
	void acceptNormalOptionsRef();
	//! add to filter list
	void addToFilterListRef();
	//! changeFilterPositionUp
	void changeFilterPositionUpRef();
	//! changeFilterPositionDown
	void changeFilterPositionDownRef();
	//! remove a Filter to the filter List
	void removeFromFilterListRef();
	//! disable list filter buttons;
	void disableFilterListButtonsRef();
	//! return current filter tab widget
	int getCurrentFilterTabWidget();

	//!Outlier filters
	void acceptOutlierOption();

protected:

	void setCloud1Visibility(bool);
	void setCloud2Visibility(bool);


	


protected: //members

	ccMainAppInterface* m_app;


	ccPointCloud* m_cloud1;
	ccPointCloud* m_cloud2;
	ccPointCloud* m_corePointsCloud;
	std::vector< std::shared_ptr<PM::DataPointsFilter>> m_filtersRef;
	std::vector<bool> m_needNormalsRef;
	std::vector<bool> m_useExistingNormalsRef;
	std::shared_ptr<PM::DataPointsFilter> m_normalParams;
	QString m_currentFilterName;
	int m_filterItemRef;

};

#endif 
