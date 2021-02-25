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
//#          COPYRIGHT: EDF R&D / TELECOM ParisTech (ENST-TSI)             #
//#                                                                        #
//##########################################################################

#include "ccImageViewer.h"

//Local
#include "ccPersistentSettings.h"
#include "mainwindow.h"

//qCC_db
#include <ccSingleton.h>

//Qt
#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QKeyEvent>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QThread>
#include <QTime>
#include <QLabel>
#include <Qbitmap>

//system
#include <cassert>
#ifdef QT_DEBUG
#include <iostream>
#endif

/***************
 *** Globals ***
 ***************/
ccImageViewer::ccImageViewer(QLabel* label = nullptr, QScrollArea* scrollArea = nullptr, QWidget* parent = nullptr)
{
	QPixmap pixmap("C:/Users/Asus/Pictures/IMG_0008.PNG");
	label->setPixmap(pixmap);
	label->setMask(pixmap.mask());
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setVisible(true);
	
	

}


