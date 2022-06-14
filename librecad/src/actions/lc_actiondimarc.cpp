/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2021 Melwyn Francis Carlo <carlo.melwyn@outlook.com>
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file gpl-2.0.txt included in the
** packaging of this file.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
**
** This copyright notice MUST APPEAR in all copies of the script!
**
**********************************************************************/


#include <iostream>

#include <QAction>
#include <QMouseEvent>

#include "rs_arc.h"
#include "rs_debug.h"
#include "rs_preview.h"
#include "rs_graphicview.h"
#include "rs_commandevent.h"
#include "rs_dialogfactory.h"
#include "rs_coordinateevent.h"

#include "lc_actiondimarc.h"


LC_ActionDimArc::LC_ActionDimArc(RS_EntityContainer& container, RS_GraphicView& graphicView) 
                                 :
                                 RS_ActionDimension("Draw Arc Dimensions", container, graphicView)
{
    reset();
}


LC_ActionDimArc::~LC_ActionDimArc() = default;


void LC_ActionDimArc::reset()
{
    RS_DEBUG->print("LC_ActionDimArc::reset - enter\n");

    RS_ActionDimension::reset();

    actionType = RS2::ActionDimArc;

    dimArcData = LC_DimArcData( 0.0, RS_Vector(false), 0.0, 0.0 );

    selectedArcEntity = nullptr;

    RS_DIALOGFACTORY->requestOptions (this, true, true);

    RS_DEBUG->print("LC_ActionDimArc::reset - exit\n");
}


void LC_ActionDimArc::trigger()
{
    RS_DEBUG->print("LC_ActionDimArc::trigger - enter\n");

    RS_PreviewActionInterface::trigger();

    if (selectedArcEntity == nullptr)
    {
        RS_DEBUG->print(RS_Debug::D_ERROR, "LC_ActionDimArc::trigger: selectedArcEntity is nullptr.\n");
        return;
    }

    if ( ! dimArcData.centre.valid)
    {
        RS_DEBUG->print(RS_Debug::D_ERROR, "LC_ActionDimArc::trigger: dimArcData.centre is not valid.\n");
        return;
    }

    LC_DimArc* new_dimArc_entity { new LC_DimArc (container, *data, dimArcData) };

    new_dimArc_entity->setLayerToActive();
    new_dimArc_entity->setPenToActive();
    new_dimArc_entity->update();
    container->addEntity(new_dimArc_entity);

    if (document)
    {
        document->startUndoCycle();
        document->addUndoable(new_dimArc_entity);
        document->endUndoCycle();
    }

    RS_Vector relativeZeroPos { graphicView->getRelativeZero() };

    setStatus (SetEntity);

    graphicView->redraw (RS2::RedrawDrawing);
    graphicView->moveRelativeZero (relativeZeroPos);

    RS_Snapper::finish();

    RS_DEBUG->print("LC_ActionDimArc::trigger - exit\n");
}


void LC_ActionDimArc::mouseMoveEvent(QMouseEvent* e)
{
    RS_DEBUG->print( "LC_ActionDimArc::mouseMoveEvent begin");

    switch (getStatus())
    {
        case SetPos:
        {
            setDimLine (snapPoint(e));

            LC_DimArc *temp_dimArc_entity { new LC_DimArc (preview.get(), *data, dimArcData) };

            std::cout << *data;
            std::cout << *temp_dimArc_entity;

            deletePreview();
            preview->addEntity(temp_dimArc_entity);

            drawPreview();
        }
        break;

        default:
            break;
    }

    RS_DEBUG->print("LC_ActionDimArc::mouseMoveEvent end");
}


void LC_ActionDimArc::mouseReleaseEvent(QMouseEvent* e)
{
    RS_DEBUG->print("LC_ActionDimArc::mouseReleaseEvent - enter\n");

    if (Qt::LeftButton == e->button())
    {
        switch (getStatus())
        {
            case SetEntity:
            {
                selectedArcEntity = catchEntity (e, RS2::ResolveAll);

                if (selectedArcEntity != nullptr)
                {
                    if (selectedArcEntity->rtti() == RS2::EntityArc)
                    {
                        dimArcData.radius     = selectedArcEntity->getRadius();
                        dimArcData.centre     = selectedArcEntity->getCenter();

                        if (((RS_Arc *) selectedArcEntity)->isReversed())
                        {
                            dimArcData.startAngle = ((RS_Arc *) selectedArcEntity)->getAngle2();
                            dimArcData.endAngle   = ((RS_Arc *) selectedArcEntity)->getAngle1();
                        } else {
                            dimArcData.startAngle = ((RS_Arc *) selectedArcEntity)->getAngle1();
                            dimArcData.endAngle   = ((RS_Arc *) selectedArcEntity)->getAngle2();
                        }

                        data->definitionPoint.setPolar(dimArcData.radius, dimArcData.startAngle);
                        setStatus (SetPos);
                    }
                    else
                    {
                        RS_DEBUG->print( RS_Debug::D_ERROR, 
                                         "LC_ActionDimArc::mouseReleaseEvent: selectedArcEntity is not an arc.");

                        selectedArcEntity = nullptr;
                    }
                }
            }
            break;

            case SetPos:
            {
                RS_CoordinateEvent ce (snapPoint (e));
                coordinateEvent (&ce);
            }
            break;
        }
    }
    else if (e->button() == Qt::RightButton)
    {
        deletePreview();
        init (getStatus() - 1);
    }
    RS_DEBUG->print("LC_ActionDimArc::mouseReleaseEvent - exit\n");
}


void LC_ActionDimArc::showOptions()
{
    RS_DEBUG->print("LC_ActionDimArc::showOptions - enter\n");

    RS_ActionInterface::showOptions();

    RS_DIALOGFACTORY->requestOptions (this, true);

    RS_DEBUG->print("LC_ActionDimArc::showOptions - exit\n");
}


void LC_ActionDimArc::hideOptions()
{
    RS_DEBUG->print("LC_ActionDimArc::hideOptions - enter\n");

    RS_ActionInterface::hideOptions();

    RS_DIALOGFACTORY->requestOptions (this, false);

    RS_DEBUG->print("LC_ActionDimArc::hideOptions - exit\n");
}


void LC_ActionDimArc::coordinateEvent(RS_CoordinateEvent* e)
{
    RS_DEBUG->print("LC_ActionDimArc::coordinateEvent - enter\n");

    if (e == nullptr) return;

    switch (getStatus())
    {
        case SetPos:
            setDimLine (e->getCoordinate());
            trigger();
            reset();
            setStatus (SetEntity);
            break;

        default:
            break;
    }

    RS_DEBUG->print("LC_ActionDimArc::coordinateEvent - exit\n");
}


void LC_ActionDimArc::commandEvent(RS_CommandEvent* e)
{
    RS_DEBUG->print("LC_ActionDimArc::commandEvent - enter\n");

    QString inputCommand (e->getCommand().toLower());

    if (checkCommand (QStringLiteral ("help"), inputCommand))
    {
        RS_DIALOGFACTORY->commandMessage(getAvailableCommands().join(", "));
        return;
    }

    if (checkCommand (QStringLiteral ("exit"), inputCommand))
    {
        init (-1);
        return;
    }

    RS_DEBUG->print("LC_ActionDimArc::commandEvent - exit\n");
}


QStringList LC_ActionDimArc::getAvailableCommands()
{
    RS_DEBUG->print("LC_ActionDimArc::getAvailableCommands - enter\n");

    QStringList availableCommandsList { "help", "exit" };

    RS_DEBUG->print("LC_ActionDimArc::getAvailableCommands - exit\n");

    return availableCommandsList;
}


void LC_ActionDimArc::updateMouseButtonHints()
{
    RS_DEBUG->print("LC_ActionDimArc::updateMouseButtonHints - enter\n");

    switch (getStatus())
    {
        case SetEntity:
            RS_DIALOGFACTORY->updateMouseWidget( tr("Select arc entity"),
                                                 tr("Cancel"));
            break;

        case SetPos:
            RS_DIALOGFACTORY->updateMouseWidget( tr("Specify dimension arc location"),
                                                 tr("Cancel"));
            break;

        default:
            RS_DIALOGFACTORY->updateMouseWidget();
            break;
    }

    RS_DEBUG->print("LC_ActionDimArc::updateMouseButtonHints - exit\n");
}


void LC_ActionDimArc::setDimLine(const RS_Vector& selectedPosition)
{
    RS_DEBUG->print("LC_ActionDimArc::setDimLine - enter\n");

    double dimLineRadius = selectedPosition.distanceTo (dimArcData.centre);

    data->definitionPoint.setPolar(dimLineRadius, dimArcData.startAngle);

    RS_DEBUG->print("LC_ActionDimArc::setDimLine - exit\n");
}

