//##########################################################################
//#                                                                        #
//#                      CLOUDCOMPARE PLUGIN: qM3C2                        #
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
//#                         COPYRIGHT: CNRS / OSUR                         #
//#                                                                        #
//##########################################################################

#ifndef LIBPOINTMATCHER_PLUGIN_COMMANDS_HEADER
#define LIBPOINTMATCHER_PLUGIN_COMMANDS_HEADER

//CloudCompare
#include "ccCommandLineInterface.h"

//Local
#include "LibpointmatcherProcess.h"

static const char COMMAND_LIBPOINTMATCHER[] = "LIBPOINTMATCHER";

struct CommandLibpointmatcher : public ccCommandLineInterface::Command
{
	CommandLibpointmatcher() : ccCommandLineInterface::Command("LIBPOINTMATCHER", COMMAND_LIBPOINTMATCHER) {}

	virtual bool process(ccCommandLineInterface& cmd) override
	{
		cmd.print("[LIBPOINTMATCHER]");
		if (cmd.arguments().empty())
		{
			return cmd.error(QString("Missing parameter: parameters filename after \"-%1\"").arg(COMMAND_LIBPOINTMATCHER));
		}

		//open specified file
		QString paramFilename(cmd.arguments().takeFirst());
		cmd.print(QString("Parameters file: '%1'").arg(paramFilename));

		if (cmd.clouds().size() < 2)
		{
			cmd.error("Not enough clouds loaded (2 or 3 are expected: cloud 1, cloud 2 and optionally some core points)");
			return false;
		}

		ccPointCloud* cloud1 = ccHObjectCaster::ToPointCloud(cmd.clouds()[0].pc);
		ccPointCloud* cloud2 = ccHObjectCaster::ToPointCloud(cmd.clouds()[1].pc);
		ccPointCloud* corePointsCloud = (cmd.clouds().size() > 2 ? cmd.clouds()[2].pc : nullptr);

		//display dialog
		LibpointmatcherDialog dlg(cloud1, cloud2, nullptr);
		if (!dlg.loadParamsFromFile(paramFilename))
		{
			return false;
		}
		dlg.setCorePointsCloud(corePointsCloud);

		QString errorMessage;
		ccPointCloud* outputCloud = nullptr; //only necessary for the command line version in fact
		if (!LibpointmatcherProcess::Compute(dlg, errorMessage, outputCloud, !cmd.silentMode(), cmd.widgetParent()))
		{
			return cmd.error(errorMessage);
		}

		if (outputCloud)
		{
			CLCloudDesc cloudDesc(outputCloud, cmd.clouds()[0].basename + QObject::tr("_LIBPOINTMATCHER"), cmd.clouds()[0].path);
			if (cmd.autoSaveMode())
			{
				QString errorStr = cmd.exportEntity(cloudDesc, QString(), 0, ccCommandLineInterface::ExportOption::ForceNoTimestamp);
				if (!errorStr.isEmpty())
				{
					cmd.error(errorStr);
				}
			}
			//add cloud to the current pool
			cmd.clouds().push_back(cloudDesc);
		}

		return true;
	}
};

#endif //LIBPOINTMATCHER_PLUGIN_COMMANDS_HEADER
