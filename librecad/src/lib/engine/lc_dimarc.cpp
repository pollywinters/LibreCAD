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


#include <cmath>
#include <iostream>

#include "rs_arc.h"
#include "rs_line.h"
#include "rs_debug.h"
#include "rs_mtext.h"
#include "rs_solid.h"
#include "rs_graphic.h"
#include "rs_math.h"

#include "lc_dimarc.h"


LC_DimArcData::LC_DimArcData()
               :
               radius     (0.0), 
               arcLength  (0.0), 
               centre     (false), 
               startAngle (0.0),
               endAngle   (0.0),
               partial    (false),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}


LC_DimArcData::LC_DimArcData(const LC_DimArcData &input_dimArcData)
               :
               radius     (input_dimArcData.radius), 
               arcLength  (input_dimArcData.arcLength), 
               centre     (input_dimArcData.centre), 
               startAngle (input_dimArcData.startAngle),
               endAngle   (input_dimArcData.endAngle), 
               partial    (input_dimArcData.partial),
               leader     (input_dimArcData.leader),
               leaderStart(input_dimArcData.leaderStart),
               leaderEnd  (input_dimArcData.leaderEnd)
{
}


LC_DimArcData::LC_DimArcData( const double& input_radius, 
                              const double& input_arcLength, 
                              const RS_Vector& input_centre, 
                              const double& input_startAngle, 
                              const double& input_endAngle)
               :
               radius     (input_radius), 
               arcLength  (input_arcLength), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (false),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}

LC_DimArcData::LC_DimArcData( const double& input_radius, 
                              const double& input_arcLength, 
                              const RS_Vector& input_centre, 
                              const double& input_startAngle,
                              const double& input_endAngle,
                              const bool& input_partial)
               :
               radius     (input_radius), 
               arcLength  (input_arcLength), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (input_partial),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}

LC_DimArcData::LC_DimArcData( const double& input_radius, 
                              const double& input_arcLength, 
                              const RS_Vector& input_centre, 
                              const double& input_startAngle,
                              const double& input_endAngle,
                              const bool& input_partial,
                              const bool& input_leader,
                              const RS_Vector& input_leaderStart,
                              const RS_Vector& input_leaderEnd)
               :
               radius     (input_radius), 
               arcLength  (input_arcLength), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (input_partial),
               leader     (input_leader),
               leaderStart(input_leaderStart),
               leaderEnd  (input_leaderEnd)
{
}


LC_DimArc::LC_DimArc( RS_EntityContainer* parent, 
                      const RS_DimensionData& input_commonDimData, 
                      const LC_DimArcData& input_dimArcData)
                      :
                      RS_Dimension (parent, input_commonDimData), 
                      dimArcData (input_dimArcData), 
                      dimStartPoint (false), 
                      dimEndPoint (false), 
                      extLine1 (nullptr), 
                      extLine2 (nullptr), 
                      dimArc1 (nullptr), 
                      dimArc2 (nullptr)
{
    update();
}


RS_Entity* LC_DimArc::clone() const
{
//    RS_DEBUG->print("LC_DimArc::clone - enter\n");

    LC_DimArc *cloned_dimArc_entity { new LC_DimArc(*this) };

    cloned_dimArc_entity->setOwner(isOwner());
    cloned_dimArc_entity->initId();
    cloned_dimArc_entity->detach();
    cloned_dimArc_entity->update();

//    RS_DEBUG->print("LC_DimArc::clone - exit\n");

    return cloned_dimArc_entity;
}


QString LC_DimArc::getMeasuredLabel()
{
//    RS_DEBUG->print("LC_DimArc::getMeasuredLabel - enter\n");

    RS_Graphic* currentGraphic = getGraphic();

    QString measuredLabel;

    if (currentGraphic)
    {
        const int dimlunit { getGraphicVariableInt (QStringLiteral("$DIMLUNIT"), 2) };
        const int dimdec   { getGraphicVariableInt (QStringLiteral("$DIMDEC"),   4) };
        const int dimzin   { getGraphicVariableInt (QStringLiteral("$DIMZIN"),   1) };

        RS2::LinearFormat format = currentGraphic->getLinearFormat(dimlunit);

        measuredLabel = RS_Units::formatLinear(dimArcData.arcLength, RS2::None, format, dimdec);

        if (format == RS2::Decimal) measuredLabel = stripZerosLinear(measuredLabel, dimzin);

        if ((format == RS2::Decimal) || (format == RS2::ArchitecturalMetric))
        {
            if (getGraphicVariableInt("$DIMDSEP", 0) == 44) measuredLabel.replace(QChar('.'), QChar(','));
        }
    }
    else
    {
        measuredLabel = QString("%1").arg(dimArcData.arcLength);
    }

//    RS_DEBUG->print("LC_DimArc::getMeasuredLabel - exit\n");

    return measuredLabel;
}


void LC_DimArc::arrow( const RS_Vector& point, 
                       const double angle, 
                       const double direction, 
                       const RS_Pen& pen)
{
//    RS_DEBUG->print("LC_DimArc::arrow - enter\n");

    if ((getTickSize() * getGeneralScale()) < 0.01)
    {
        double endAngle { 0.0 };

        if (dimArcData.radius > RS_TOLERANCE_ANGLE) endAngle = getArrowSize() / dimArcData.radius;

        const RS_Vector arrowEnd = RS_Vector::polar(dimArcData.radius, angle + std::copysign(endAngle, direction)) 
                                 + dimArcData.centre;

        const double arrowAngle { arrowEnd.angleTo(point) };

        RS_SolidData dummyVar;

        RS_Solid* arrow = new RS_Solid(this, dummyVar);
        arrow->shapeArrow(point, arrowAngle, getArrowSize());
        arrow->setPen( pen);
        arrow->setLayer(nullptr);
        addEntity(arrow);
    }
    else
    {
        const double deg45 = M_PI_2 / 2.0;

        const double midAngle = (dimArcData.startAngle + dimArcData.endAngle) / 2.0;

        const RS_Vector tickVector = RS_Vector::polar(getTickSize() * getGeneralScale(), midAngle - deg45);

        RS_Line* tick = new RS_Line(this, point - tickVector, point + tickVector);
        tick->setPen(pen);
        tick->setLayer(nullptr);
        addEntity(tick);
    }

//    RS_DEBUG->print("LC_DimArc::arrow - exit\n");
}


void LC_DimArc::updateDim(bool autoText /* = false */)
{
//    RS_DEBUG->print("LC_DimArc::updateDim - enter\n");

    Q_UNUSED (autoText)

    clear();

    if (isUndone()) {
//        RS_DEBUG->print("LC_DimArc::updateDim - exit, isUndone\n");
        return;
    }

    if ( ! dimArcData.centre.valid) {
//        RS_DEBUG->print("LC_DimArc::updateDim - exit, ! dimArcData.centre.valid\n");
        return;
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 1\n");

    calcDimension();

//    RS_DEBUG->print("LC_DimArc::updateDim - 2\n");

    RS_Pen pen (getExtensionLineColor(), getExtensionLineWidth(), RS2::LineByBlock);

    pen.setWidth (getDimensionLineWidth());
    pen.setColor (getDimensionLineColor());

    extLine1->setPen (pen);
    extLine2->setPen (pen);

    extLine1->setLayer (nullptr);
    extLine2->setLayer (nullptr);

//    RS_DEBUG->print("LC_DimArc::updateDim - 3\n");

    addEntity (extLine1);
    addEntity (extLine2);

//    RS_DEBUG->print("LC_DimArc::updateDim - 4\n");

    RS_Arc* refArc
    {
        new RS_Arc( this, 
                    RS_ArcData( dimArcData.centre, 
                                dimArcData.radius, 
                                dimArcData.startAngle, 
                                dimArcData.endAngle, 
                                false) 
                  ) 
    };

//    RS_DEBUG->print("LC_DimArc::updateDim - 5\n");

    arrow (arrowStartPoint, dimArcData.startAngle, +1.0, pen);
    arrow (arrowEndPoint,   dimArcData.endAngle,   -1.0, pen);

    double textAngle  { 0.0 };

    RS_Vector textPos { refArc->getMiddlePoint() };

    const double textAngle_preliminary { std::trunc((textPos.angleTo(dimArcData.centre) - M_PI) * 1.0E+10) * 1.0E-10 };

//    RS_DEBUG->print("LC_DimArc::updateDim - 6\n");

    if ( ! this->getInsideHorizontalText())
    {
        RS_Vector textPosOffset;

        const double deg360 { M_PI * 2.0 };

        const double degTolerance { 1.0E-3 };

        /* With regards to Quadrants #1 and #2 */
        if (((textAngle_preliminary >= -degTolerance) && (textAngle_preliminary <= (M_PI + degTolerance))) 
        ||  ((textAngle_preliminary <= -(M_PI - degTolerance)) && (textAngle_preliminary >= -(deg360 + degTolerance))))
        {
            textPosOffset.setPolar (getDimensionLineGap(), textAngle_preliminary);
            textAngle = textAngle_preliminary + M_PI + M_PI_2;
        }
        /* With regards to Quadrants #3 and #4 */
        else
        {
            textPosOffset.setPolar (getDimensionLineGap(), textAngle_preliminary + M_PI);
            textAngle = textAngle_preliminary + M_PI_2;
        }
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 7\n");

    QString dimLabel { getLabel() };

//    RS_DEBUG->print("LC_DimArc::updateDim - 8\n");

    bool ok;

    const double dummyVar = dimLabel.toDouble(&ok);

    if (dummyVar) { /* This is a dummy code, to suppress the unused variable compiler warning. */ }

    if (ok) dimLabel.prepend("∩ ");

//    RS_DEBUG->print("LC_DimArc::updateDim - 9\n");

    RS_MTextData textData
    {
        RS_MTextData( textPos, 
                      getTextHeight(), 
                      30.0, 
                      RS_MTextData::VABottom, 
                      RS_MTextData::HACenter, 
                      RS_MTextData::LeftToRight, 
                      RS_MTextData::Exact, 
                      1.0, 
                      dimLabel, 
                      QString("unicode"), 
                      textAngle) 
    };

//    RS_DEBUG->print("LC_DimArc::updateDim - 10\n");

    RS_MText* text { new RS_MText (this, textData) };

    text->setPen (RS_Pen (getTextColor(), RS2::WidthByBlock, RS2::SolidLine));
    text->setLayer (nullptr);
    addEntity (text);

//    RS_DEBUG->print("LC_DimArc::updateDim - 11\n");

    double halfWidth_plusGap  = (text->getUsedTextWidth() / 2.0) + getDimensionLineGap();
    double halfHeight_plusGap = (getTextHeight()          / 2.0) + getDimensionLineGap();

    text->move(-RS_Vector::polar(getTextHeight() / 2.0, textAngle + M_PI_2));

//    RS_DEBUG->print("LC_DimArc::updateDim - 12\n");

    /* Text rectangle's corners : top left, top right, bottom right, bottom left. */
    RS_Vector textRectCorners [4] = 
    {
        RS_Vector(false), 
        RS_Vector(textPos + RS_Vector(+halfWidth_plusGap, +halfHeight_plusGap)), 
        RS_Vector(false), 
        RS_Vector(textPos + RS_Vector(-halfWidth_plusGap, -halfHeight_plusGap))
    };

    RS_Vector cornerTopRight   { textRectCorners [1] };
    RS_Vector cornerBottomLeft { textRectCorners [3] };

    textRectCorners [0] = RS_Vector(cornerBottomLeft.x, cornerTopRight.y);
    textRectCorners [2] = RS_Vector(cornerTopRight.x,   cornerBottomLeft.y);

//    RS_DEBUG->print("LC_DimArc::updateDim - 13\n");

    for (int i = 0; i < 4; i++)
    {
        textRectCorners[i].rotate(textPos, text->getAngle());
        textRectCorners[i].x = std::trunc(textRectCorners[i].x * 1.0E+4) * 1.0E-4;
        textRectCorners[i].y = std::trunc(textRectCorners[i].y * 1.0E+4) * 1.0E-4;
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 14\n");

    if (RS_DEBUG->getLevel() == RS_Debug::D_INFORMATIONAL)
    {
        std::cout << std::endl 
                  << " LC_DimArc::updateDim: Text position / angle : " << textPos << " / " << text->getAngle() 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::updateDim: Reference arc middle point : " << refArc->getMiddlePoint() 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::updateDim: DimArc-1 start point : " << dimArc1->getStartpoint() 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::updateDim: DimArc-2 start point : " << dimArc2->getStartpoint() 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::updateDim: Text rectangle corners : " << textRectCorners [0] << ", " 
                                                                        << textRectCorners [1] << ", " 
                                                                        << textRectCorners [2] << ", " 
                                                                        << textRectCorners [3] 
                  << std::endl;
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 15\n");

    const double cornerLeftX
    {
        std::min(std::min(std::min(textRectCorners[0].x, textRectCorners[1].x), textRectCorners[2].x), textRectCorners[3].x)
    };

    const double cornerRightX
    {
        std::max(std::max(std::max(textRectCorners[0].x, textRectCorners[1].x), textRectCorners[2].x), textRectCorners[3].x)
    };

    const double cornerBottomY
    {
        std::min(std::min(std::min(textRectCorners[0].y, textRectCorners[1].y), textRectCorners[2].y), textRectCorners[3].y)
    };

    const double cornerTopY
    {
        std::max(std::max(std::max(textRectCorners[0].y, textRectCorners[1].y), textRectCorners[2].y), textRectCorners[3].y)
    };

    const double deltaOffset { 1.0E-2 };

//    RS_DEBUG->print("LC_DimArc::updateDim - 16\n");

    while (((dimArc1->getEndpoint().x < cornerLeftX)   || (dimArc1->getEndpoint().x > cornerRightX) 
    ||      (dimArc1->getEndpoint().y < cornerBottomY) || (dimArc1->getEndpoint().y > cornerTopY))

    &&     (dimArc1->getAngle2() < RS_MAXDOUBLE) 
    &&     (dimArc1->getAngle2() > RS_MINDOUBLE))
    {
        dimArc1->setAngle2(dimArc1->getAngle2() + deltaOffset);
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 17\n");

    while (((dimArc2->getStartpoint().x < cornerLeftX)   || (dimArc2->getStartpoint().x > cornerRightX) 
    ||      (dimArc2->getStartpoint().y < cornerBottomY) || (dimArc2->getStartpoint().y > cornerTopY)) 

    &&     (dimArc2->getAngle1() < RS_MAXDOUBLE) 
    &&     (dimArc2->getAngle1() > RS_MINDOUBLE))
    {
        dimArc2->setAngle1(dimArc2->getAngle1() - deltaOffset);
    }

//    RS_DEBUG->print("LC_DimArc::updateDim - 18\n");

    dimArc1->setPen (pen);
    dimArc2->setPen (pen);

    dimArc1->setLayer (nullptr);
    dimArc2->setLayer (nullptr);

//    RS_DEBUG->print("LC_DimArc::updateDim - 19\n");

    addEntity (dimArc1);
    addEntity (dimArc2);

    calculateBorders();

//    RS_DEBUG->print("LC_DimArc::updateDim - exit\n");
}


void LC_DimArc::update()
{
//    RS_DEBUG->print("LC_DimArc::update - enter\n");

    updateDim();
    RS_Dimension::update();

//    RS_DEBUG->print("LC_DimArc::update - exit\n");
}


void LC_DimArc::move(const RS_Vector& offset)
{
//    RS_DEBUG->print("LC_DimArc::move - enter\n");

    RS_Dimension::move (offset);

    dimArcData.centre.move (offset);
    dimArcData.leaderEnd.move (offset);
    dimArcData.leaderStart.move (offset);

    update();

//    RS_DEBUG->print("LC_DimArc::move - exit\n");
}


void LC_DimArc::rotate(const RS_Vector& center, const double& angle)
{
//    RS_DEBUG->print("LC_DimArc::rotate C,A - enter\n");

    RS_Vector angleVector(angle);

    RS_Dimension::rotate (center, angleVector);
    dimArcData.centre.rotate (center, angleVector);
    dimArcData.leaderEnd.rotate (center, angleVector);
    dimArcData.leaderStart.rotate (center, angleVector);
    dimArcData.startAngle = RS_Math::correctAngle(dimArcData.startAngle + angle);
    dimArcData.endAngle = RS_Math::correctAngle(dimArcData.endAngle + angle);

    update();

//    RS_DEBUG->print("LC_DimArc::rotate C,A - exit\n");
}


void LC_DimArc::rotate(const RS_Vector& center, const RS_Vector& angleVector)
{
//    RS_DEBUG->print("LC_DimArc::rotate C,Avec - enter\n");

    double angle = angleVector.angle();

    RS_Dimension::rotate (center, angleVector);
    dimArcData.centre.rotate (center, angleVector);
    dimArcData.leaderEnd.rotate (center, angleVector);
    dimArcData.leaderStart.rotate (center, angleVector);
    dimArcData.startAngle = RS_Math::correctAngle(dimArcData.startAngle + angle);
    dimArcData.endAngle = RS_Math::correctAngle(dimArcData.endAngle + angle);

    update();

//    RS_DEBUG->print("LC_DimArc::rotate C,Avec - exit\n");
}


void LC_DimArc::scale(const RS_Vector& center, const RS_Vector& factor)
{
//    RS_DEBUG->print("LC_DimArc::scale - enter\n");

    const double adjustedFactor = factor.x < factor.y 
                                ? factor.x 
                                : factor.y;

    const RS_Vector adjustedFactorVector(adjustedFactor, adjustedFactor);

    RS_Dimension::scale (center, adjustedFactorVector);

    dimArcData.centre.scale (center, adjustedFactorVector);
    dimArcData.leaderEnd.scale (center, adjustedFactorVector);
    dimArcData.leaderStart.scale (center, adjustedFactorVector);

    dimArcData.radius *= adjustedFactor;

    update();

//    RS_DEBUG->print("LC_DimArc::scale - exit\n");
}


void LC_DimArc::mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2)
{
//    RS_DEBUG->print("LC_DimArc::mirror - enter\n");

    const RS_Vector previousDefinitionPoint = data.definitionPoint;

    RS_Dimension::mirror (axisPoint1, axisPoint2);

    dimArcData.centre.mirror (axisPoint1, axisPoint2);
    dimArcData.leaderEnd.mirror (axisPoint1, axisPoint2);
    dimArcData.leaderStart.mirror (axisPoint1, axisPoint2);

    double twiceMirrorAngle = axisPoint1.angleTo(axisPoint2) * 2.0;

    dimArcData.startAngle = RS_Math::correctAngle(twiceMirrorAngle - dimArcData.startAngle);
    dimArcData.endAngle = RS_Math::correctAngle(twiceMirrorAngle - dimArcData.endAngle);

    update();

//    RS_DEBUG->print("LC_DimArc::mirror - exit\n");
}


RS_Vector LC_DimArc::truncateVector(const RS_Vector input_vector)
{
    return RS_Vector( std::trunc(input_vector.x * 1.0E+10) * 1.0E-10, 
                      std::trunc(input_vector.y * 1.0E+10) * 1.0E-10, 
                      0.0);
}


void LC_DimArc::calcDimension()
{
//    RS_DEBUG->print("LC_DimArc::calcDimension - enter\n");

    dimArc1 = new RS_Arc (this, RS_ArcData(dimArcData.centre, dimArcData.radius, dimArcData.startAngle, dimArcData.startAngle, false));
    dimArc2 = new RS_Arc (this, RS_ArcData(dimArcData.centre, dimArcData.radius, dimArcData.endAngle,   dimArcData.endAngle, false));

    RS_Vector entityStartPoint = truncateVector(data.definitionPoint);

    const double entityRadius  = dimArcData.centre.distanceTo(entityStartPoint);

    RS_Vector startAngleVector = RS_Vector(dimArcData.startAngle);
    RS_Vector endAngleVector   = RS_Vector(dimArcData.endAngle);

    RS_Vector entityEndPoint   = truncateVector(dimArcData.centre + endAngleVector.scale(entityRadius));

    dimStartPoint = dimArcData.centre + startAngleVector.scale(dimArcData.radius);
    dimEndPoint   = dimArcData.centre + endAngleVector.scale(dimArcData.radius);

    arrowStartPoint = dimStartPoint;
    arrowEndPoint   = dimEndPoint;

    entityStartPoint += RS_Vector::polar (getExtensionLineOffset(),    entityStartPoint.angleTo(dimStartPoint));
    entityEndPoint   += RS_Vector::polar (getExtensionLineOffset(),    entityEndPoint.angleTo(dimEndPoint));
    dimStartPoint    += RS_Vector::polar (getExtensionLineExtension(), entityStartPoint.angleTo(dimStartPoint));
    dimEndPoint      += RS_Vector::polar (getExtensionLineExtension(), entityEndPoint.angleTo(dimEndPoint));

    extLine1 = new RS_Line (this, entityStartPoint, dimStartPoint);
    extLine2 = new RS_Line (this, entityEndPoint,   dimEndPoint);

    /* RS_DEBUG->setLevel(RS_Debug::D_INFORMATIONAL); */

    RS_DEBUG->print( RS_Debug::D_INFORMATIONAL, 
                     "\n LC_DimArc::calcDimension: Start / end angles : %lf / %lf\n", 
                     dimArcData.startAngle, dimArcData.endAngle);

    RS_DEBUG->print( RS_Debug::D_INFORMATIONAL, 
                     "\n LC_DimArc::calcDimension: Dimension / entity radii : %lf / %lf\n", 
                     dimArcData.radius, entityRadius);

    if (RS_DEBUG->getLevel() == RS_Debug::D_INFORMATIONAL)
    {
        std::cout << std::endl 
                  << " LC_DimArc::calcDimension: Start Points : " << entityStartPoint << " to " << dimStartPoint 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::calcDimension: End Points : " << entityEndPoint << " to " << dimEndPoint 
                  << std::endl;
    }

//    RS_DEBUG->print("LC_DimArc::calcDimension - exit\n");
}


std::ostream& operator << (std::ostream& os, const LC_DimArc& input_dimArc)
{
    os << " DimArc Information : \n" 
       << input_dimArc.getData() << std::endl << std::endl;

    return os;
}


std::ostream& operator << (std::ostream& os, const LC_DimArcData& input_dimArcData)
{
    os << " {\n\tCentre      : " << input_dimArcData.centre 
       <<   "\n\tRadius      : " << input_dimArcData.radius 
       <<   "\n\tStart Angle : " << input_dimArcData.startAngle 
       <<   "\n\tEnd   Angle : " << input_dimArcData.endAngle 
       <<   "\n\tPartial     : " << input_dimArcData.partial
       <<   "\n\tLeader      : " << input_dimArcData.leader
       <<   "\n\tLeader Start: " << input_dimArcData.leaderStart
       <<   "\n\tLeader End  : " << input_dimArcData.leaderEnd
       <<   "\n}"                << std::endl << std::endl;

    return os;
}
