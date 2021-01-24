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

//local
#include "LibpointmatcherTools.h"
#include "LibpointmatcherDialog.h"
#include "LibpointmatcherDisclaimerDialog.h"
#include "LibpointmatcherCommands.h"
#include "LibpointmatcherProcess.h"

//qCC_db
#include <ccPointCloud.h>


Libpointmatcher::Libpointmatcher(QObject* parent)
	: QObject(parent)
	, ccStdPluginInterface( ":/CC/plugin/qLibpointmatcher/info.json" )
	, m_action(nullptr)
{
}

void Libpointmatcher::onNewSelection(const ccHObject::Container& selectedEntities)
{
	if (m_action)
	{
		m_action->setEnabled(	selectedEntities.size() == 2
							&&	selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD)
							&&	selectedEntities[1]->isA(CC_TYPES::POINT_CLOUD) );
	}

	m_selectedEntities = selectedEntities;
}

QList<QAction *> Libpointmatcher::getActions()
{
	if (!m_action)
	{
		m_action = new QAction(getName(),this);
		m_action->setToolTip(getDescription());
		m_action->setIcon(getIcon());
		connect(m_action, &QAction::triggered, this, &Libpointmatcher::doAction);
	}

	return QList<QAction *>{ m_action };
}

void Libpointmatcher::doAction()
{
	//disclaimer accepted?
	if (!DisclaimerDialog::show(m_app))
		return;

	//m_app should have already been initialized by CC when plugin is loaded!
	assert(m_app);
	if (!m_app)
		return;

	if (m_selectedEntities.size() != 2
		|| !m_selectedEntities[0]->isA(CC_TYPES::POINT_CLOUD)
		|| !m_selectedEntities[1]->isA(CC_TYPES::POINT_CLOUD))
	{
		m_app->dispToConsole("Select two point clouds!", ccMainAppInterface::ERR_CONSOLE_MESSAGE);
		return;
	}

	ccPointCloud* cloud1 = ccHObjectCaster::ToPointCloud(m_selectedEntities[0]);
	ccPointCloud* cloud2 = ccHObjectCaster::ToPointCloud(m_selectedEntities[1]);

	//display dialog
	LibpointmatcherDialog dlg(cloud1, cloud2, m_app);
	if (!dlg.exec())
	{
		//process cancelled by the user
		return;
	}

	QString errorMessage;
	ccPointCloud* outputCloud = nullptr; //only necessary for the command line version in fact
	if (!LibpointmatcherProcess::Subsample(dlg, errorMessage, m_app->getMainWindow(), m_app))
	{
		m_app->dispToConsole(errorMessage, ccMainAppInterface::ERR_CONSOLE_MESSAGE);
	}

	//'Compute' may change some parameters of the dialog
	dlg.saveParamsToPersistentSettings();
}

void Libpointmatcher::registerCommands(ccCommandLineInterface* cmd)
{
	if (!cmd)
	{
		assert(false);
		return;
	}
	cmd->registerCommand(ccCommandLineInterface::Command::Shared(new CommandLibpointmatcher));
}
