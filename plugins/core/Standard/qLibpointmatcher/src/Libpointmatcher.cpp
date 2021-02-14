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

#include "Libpointmatcher.h"

//Qt
#include <QMainWindow>
#include <QProgressDialog>

//local
#include "LibpointmatcherTools.h"
#include "LibpointmatcherDialog.h"
#include "LibpointmatcherOutlierDialog.h"
#include "LibpointmatcherDisclaimerDialog.h"
#include "LibpointmatcherProcess.h"

//qCC_db
#include <ccPointCloud.h>


Libpointmatcher::Libpointmatcher(QObject* parent)
	: QObject(parent)
	, ccStdPluginInterface( ":/CC/plugin/qLibpointmatcher/info.json" )

	, m_actionFilter(nullptr)
	, m_actionICP(nullptr)
	,m_actionConvergence(nullptr)
{
}

void Libpointmatcher::onNewSelection(const ccHObject::Container& selectedEntities)
{
	if (m_actionFilter)
	{
		m_actionFilter->setEnabled(selectedEntities.size() >= 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
	}
	if (m_actionICP)
	{
		m_actionICP->setEnabled(selectedEntities.size() == 2 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD) && selectedEntities[1]->isA(CC_TYPES::POINT_CLOUD));
	}
	if (m_actionConvergence)
	{
		m_actionConvergence->setEnabled(selectedEntities.size() >= 1 && selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD));
	}

	m_selectedEntities = selectedEntities;
}

QList<QAction *> Libpointmatcher::getActions()
{
	if (!m_actionFilter)
	{
		m_actionFilter = new QAction("Subsample",this);
		m_actionFilter->setToolTip("Subsample with Libpointmatcher chains");
		m_actionFilter->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qLibpointmatcher/images/filterIcon.png")));
		connect(m_actionFilter, &QAction::triggered, this, &Libpointmatcher::doActionFilter);
	}

	if (!m_actionICP)
	{
		m_actionICP = new QAction("ICP", this);
		m_actionICP->setToolTip("ICP with Libpointmatcher chains");
		m_actionICP->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qLibpointmatcher/images/ICPIcon.png")));
		connect(m_actionICP, &QAction::triggered, this, &Libpointmatcher::doActionICP);
	}
	if (!m_actionConvergence)
	{
		m_actionConvergence = new QAction("Convergence", this);
		m_actionConvergence->setToolTip("Convergence with Libpointmatcher/M3C2 chains");
		m_actionConvergence->setIcon(QIcon(QString::fromUtf8(":/CC/plugin/qLibpointmatcher/images/convergenceIcon.png")));
		connect(m_actionConvergence, &QAction::triggered, this, &Libpointmatcher::doActionFilter);
	}

	return QList<QAction *>{ m_actionFilter,
							m_actionICP,
							m_actionConvergence};
}

void Libpointmatcher::doActionFilter()
{
	//disclaimer accepted?
	if (!DisclaimerDialog::show(m_app))
		return;

	//m_app should have already been initialized by CC when plugin is loaded!
	assert(m_app);
	if (!m_app)
		return;

	if (m_selectedEntities.size() < 1 || !m_selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole("Select a pointcloud", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return;
	}

	//display dialog
	LibpointmatcherDialog dlg(m_app);
	if (!dlg.exec())
	{
		//process cancelled by the user
		return;
	}
	// verify on which widget you are
	if (dlg.getCurrentFilterTabWidget() == 0 && dlg.getFilters().size()==0)
	{
		
		dlg.acceptNormalOptions();
		dlg.acceptFilterOptions();
		
	}
	if (dlg.getCurrentFilterTabWidget() == 1 && dlg.getFilters().size() == 0)
	{
		return;
	}
	if (dlg.getCurrentFilterTabWidget() == 2 && dlg.getFilters().size() == 0)
	{
		return;
	}

	QString errorMessage;
	ccPointCloud* outputCloud = nullptr; //only necessary for the command line version in fact
	QProgressDialog pDlg("Please wait...", "Cancel", 0, 0);
	pDlg.setWindowTitle("Libpointmatcher");
	pDlg.show();
	for (int i = 0; i < m_selectedEntities.size(); i++) {
		if (!pDlg.wasCanceled()) {
			if (!LibpointmatcherProcess::Subsample(dlg, m_selectedEntities[i], errorMessage, m_app->getMainWindow(), m_app))
			{
				m_app->dispToConsole(errorMessage, ccMainAppInterface::ERR_CONSOLE_MESSAGE);
			}
		}
		else 
		{
			break;
		}
	}
	if (m_app)
	{
		m_app->refreshAll();
		pDlg.reset();
	}

}

void Libpointmatcher::doActionICP()
{
	//disclaimer accepted?
	if (!DisclaimerDialog::show(m_app))
		return;

	//m_app should have already been initialized by CC when plugin is loaded!
	assert(m_app);
	if (!m_app)
		return;

	if (m_selectedEntities.size() != 2 || !m_selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD) || !m_selectedEntities[1]->isA(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole("Select two pointclouds", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return;
	}

	//display dialog
	ccPointCloud* cloud1 = ccHObjectCaster::ToPointCloud(m_selectedEntities[0]);
	ccPointCloud* cloud2 = ccHObjectCaster::ToPointCloud(m_selectedEntities[1]);
	LibpointmatcherOutlierDialog dlgICP(cloud1,cloud2,m_app);
	if (!dlgICP.exec())
	{
		//process cancelled by the user
		return;
	}
	// verify on which widget you are


	dlgICP.acceptNormalOptions();


	QString errorMessage;
	ccPointCloud* outputCloud = nullptr; //only necessary for the command line version in fact
	QProgressDialog pDlg("Please wait...", "Cancel", 0, 0);
	pDlg.setWindowTitle("Libpointmatcher");
	pDlg.show();
	for (int i = 0; i < m_selectedEntities.size(); i++) {
		if (!pDlg.wasCanceled()) {
			if (!LibpointmatcherProcess::ICP(dlgICP, m_selectedEntities[i], errorMessage, m_app->getMainWindow(), m_app))
			{
				m_app->dispToConsole(errorMessage, ccMainAppInterface::ERR_CONSOLE_MESSAGE);
			}
		}
		else
		{
			break;
		}
	}
	if (m_app)
	{
		m_app->refreshAll();
		pDlg.reset();
	}

}
;

