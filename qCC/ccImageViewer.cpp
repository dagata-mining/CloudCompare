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
#include <QWheelEvent>

//system
#include <cassert>
#ifdef QT_DEBUG
#include <iostream>
#endif

/***************
 *** Globals ***
 ***************/
ccImageViewer::ccImageViewer(QWidget* parent = nullptr)
	:QGraphicsView(parent)
{
	//connect(parent->imageViewer, &QAction::wheelEvent, this, &ccImageViewer::wheelEvent);
	
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setDragMode(QGraphicsView::ScrollHandDrag);
	
	QGraphicsPixmapItem *pixmapItem = new QGraphicsPixmapItem(QPixmap("C:/Users/Asus/Pictures/IMG_0007.PNG"));
	pixmapItem->setTransformationMode(Qt::SmoothTransformation);
	QGraphicsScene *scene = new QGraphicsScene();
	scene->addItem(pixmapItem);
	setScene(scene);
	
}

void ccImageViewer::wheelEvent(QWheelEvent *event)
{
	if (event->delta() > 0)
		scale(1.15, 1.15);
	else
		scale(0.9, 0.9);
}

void ccImageViewer::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Left)
		rotate(-90);
	else if (event->key() == Qt::Key_Right)
		rotate(90);
}
