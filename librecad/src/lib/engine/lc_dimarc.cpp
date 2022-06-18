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


static const double deg45 = M_PI_2 / 2.0;
static const double deg90 = M_PI_2;
static const double deg180 = M_PI;
static const double deg270 = M_PI + M_PI_2;
static const double deg360 = M_PI * 2.0;


LC_DimArcData::LC_DimArcData()
               :
               radius     (0.0), 
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
               centre     (input_dimArcData.centre), 
               startAngle (input_dimArcData.startAngle),
               endAngle   (input_dimArcData.endAngle), 
               partial    (input_dimArcData.partial),
               leader     (input_dimArcData.leader),
               leaderStart(input_dimArcData.leaderStart),
               leaderEnd  (input_dimArcData.leaderEnd)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius,         // radius of arc being dimensioned
                             const RS_Vector& input_centre,      // coord for centre of arc being dimensioned
                             const double& input_startAngle,     // angle from centre for start of arc, radians
                             const double& input_endAngle)       // angle from centre for end of arc, radians
                                                                 // note: arc always goes counter-clockwise from start to end angle
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (false),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius, 
                             const RS_Vector& input_centre, 
                             const RS_Vector& input_startVector, // angles as unit vectors
                             const RS_Vector& input_endVector)
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startVector.angle()),
               endAngle   (input_endVector.angle()),
               partial    (false),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius, 
                             const RS_Vector& input_centre, 
                             const double& input_startAngle,     // angles as radians
                             const double& input_endAngle,
                             const bool& input_partial)          // true when dimension is for part of a larger arc
                                                                 // (changes styling of the dimension line)
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (input_partial),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius, 
                             const RS_Vector& input_centre, 
                             const RS_Vector& input_startVector, // angles as unit vectors
                             const RS_Vector& input_endVector,
                             const bool& input_partial)
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startVector.angle()),
               endAngle   (input_endVector.angle()),
               partial    (input_partial),
               leader     (false),
               leaderStart(false),
               leaderEnd  (false)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius, 
                             const RS_Vector& input_centre, 
                             const double& input_startAngle,     // angles as radians
                             const double& input_endAngle,
                             const bool& input_partial,
                             const bool& input_leader,           // true when leader line to be drawn from dimension text to the arc
                             const RS_Vector& input_leaderStart, // start and end coords for leader line
                             const RS_Vector& input_leaderEnd)
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startAngle),
               endAngle   (input_endAngle),
               partial    (input_partial),
               leader     (input_leader),
               leaderStart(input_leaderStart),
               leaderEnd  (input_leaderEnd)
{
}


LC_DimArcData::LC_DimArcData(const double& input_radius, 
                             const RS_Vector& input_centre, 
                             const RS_Vector& input_startVector, // angles as unit vectors
                             const RS_Vector& input_endVector,
                             const bool& input_partial,
                             const bool& input_leader,
                             const RS_Vector& input_leaderStart,
                             const RS_Vector& input_leaderEnd)
               :
               radius     (input_radius), 
               centre     (input_centre), 
               startAngle (input_startVector.angle()),
               endAngle   (input_endVector.angle()),
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
    LC_DimArc *cloned_dimArc_entity { new LC_DimArc(*this) };

    cloned_dimArc_entity->setOwner(isOwner());
    cloned_dimArc_entity->initId();
    cloned_dimArc_entity->detach();
    cloned_dimArc_entity->update();

    return cloned_dimArc_entity;
}


QString LC_DimArc::getMeasuredLabel()
{
    RS_Graphic* currentGraphic = getGraphic();

    QString measuredLabel;

    if (currentGraphic)
    {
        const int dimlunit { getGraphicVariableInt (QStringLiteral("$DIMLUNIT"), 2) };
        const int dimdec   { getGraphicVariableInt (QStringLiteral("$DIMDEC"),   4) };
        const int dimzin   { getGraphicVariableInt (QStringLiteral("$DIMZIN"),   1) };

        RS2::LinearFormat format = currentGraphic->getLinearFormat(dimlunit);

        measuredLabel = RS_Units::formatLinear(arcLength * getGeneralFactor(), RS2::None, format, dimdec);

        if (format == RS2::Decimal) measuredLabel = stripZerosLinear(measuredLabel, dimzin);

        if ((format == RS2::Decimal) || (format == RS2::ArchitecturalMetric))
        {
            if (getGraphicVariableInt("$DIMDSEP", 0) == 44) measuredLabel.replace(QChar('.'), QChar(','));
        }
    }
    else
    {
        measuredLabel = QString("%1").arg(arcLength * getGeneralFactor());
    }

    return measuredLabel;
}


void LC_DimArc::arrow( const RS_Vector& point, 
                       const double angle, 
                       const double direction, 
                       const RS_Pen& pen)
{
    if ((getTickSize() * getGeneralScale()) < 0.01)
    {
        double endAngle { 0.0 };
        double dimLineRadius = dimArcData.centre.distanceTo(data.definitionPoint);

        if (dimLineRadius > RS_TOLERANCE_ANGLE) endAngle = getArrowSize() / dimLineRadius;

        const RS_Vector arrowEnd = RS_Vector::polar(dimLineRadius, angle + std::copysign(endAngle, direction)) 
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
        const RS_Vector tickVector = RS_Vector::polar(getTickSize() * getGeneralScale(), angle - deg45);

        RS_Line* tick = new RS_Line(this, point - tickVector, point + tickVector);
        tick->setPen(pen);
        tick->setLayer(nullptr);
        addEntity(tick);
    }
}


void LC_DimArc::updateDim(bool autoText /* = false */)
{
    Q_UNUSED (autoText)

    clear();

    if (isUndone()) {
        return;
    }

    if ( ! dimArcData.centre.valid) {
        return;
    }

    calcDimension();

    RS_Pen pen (getExtensionLineColor(), getExtensionLineWidth(), RS2::LineByBlock);

    pen.setWidth (getDimensionLineWidth());
    pen.setColor (getDimensionLineColor());

    extLine1->setPen (pen);
    extLine2->setPen (pen);

    extLine1->setLayer (nullptr);
    extLine2->setLayer (nullptr);

    addEntity (extLine1);
    addEntity (extLine2);

    RS_Arc* refArc;
    double dimLineRadius = dimArcData.centre.distanceTo(data.definitionPoint);
    double arcAngle = RS_Math::correctAngle(dimArcData.endAngle - dimArcData.startAngle);
    if (dimArcData.partial && arcAngle < deg90) {
        double midAngle = (dimArcData.startAngle + dimArcData.endAngle) / 2;
        RS_Vector midAngleVector = RS_Vector(midAngle);
        RS_Vector offsetVector = midAngleVector * ( dimLineRadius - dimArcData.radius );

        refArc = new RS_Arc( this, 
                             RS_ArcData( dimArcData.centre + offsetVector, 
                                         dimArcData.radius, 
                                         dimArcData.startAngle, 
                                         dimArcData.endAngle, 
                                         false) 
                           );
        arrow (arrowStartPoint, midAngle, +1.0, pen);
        arrow (arrowEndPoint,   midAngle,   -1.0, pen);
    } else {
        refArc = new RS_Arc( this, 
                             RS_ArcData( dimArcData.centre, 
                                         dimLineRadius, 
                                         dimArcData.startAngle, 
                                         dimArcData.endAngle, 
                                         false) 
                           );

        arrow (arrowStartPoint, dimArcData.startAngle, +1.0, pen);
        arrow (arrowEndPoint,   dimArcData.endAngle,   -1.0, pen);
    }

    double textAngle  { 0.0 };

    RS_Vector textPos { refArc->getMiddlePoint() };

    const double textAngle_preliminary { std::trunc((textPos.angleTo(dimArcData.centre) - deg180) * 1.0E+10) * 1.0E-10 };

    if ( ! this->getInsideHorizontalText())
    {
        RS_Vector textPosOffset;

        const double degTolerance { 1.0E-3 };

        /* With regards to Quadrants #1 and #2 */
        if (((textAngle_preliminary >= -degTolerance) && (textAngle_preliminary <= (deg180 + degTolerance))) 
        ||  ((textAngle_preliminary <= -(deg180 - degTolerance)) && (textAngle_preliminary >= -(deg360 + degTolerance))))
        {
            textPosOffset.setPolar (getDimensionLineGap(), textAngle_preliminary);
            textAngle = textAngle_preliminary + deg270;
        }
        /* With regards to Quadrants #3 and #4 */
        else
        {
            textPosOffset.setPolar (getDimensionLineGap(), textAngle_preliminary + deg180);
            textAngle = textAngle_preliminary + deg90;
        }
    }

    QString dimLabel { getLabel() };

    bool ok;

    const double dummyVar = dimLabel.toDouble(&ok);

    if (dummyVar) { /* This is a dummy code, to suppress the unused variable compiler warning. */ }

    if (ok) dimLabel.prepend("∩ ");

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

    RS_MText* text { new RS_MText (this, textData) };

    text->setPen (RS_Pen (getTextColor(), RS2::WidthByBlock, RS2::SolidLine));
    text->setLayer (nullptr);
    addEntity (text);

    double halfWidth_plusGap  = (text->getUsedTextWidth() / 2.0) + getDimensionLineGap();
    double halfHeight_plusGap = (getTextHeight()          / 2.0) + getDimensionLineGap();

    text->move(-RS_Vector::polar(getTextHeight() / 2.0, textAngle + M_PI_2));

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

    for (int i = 0; i < 4; i++)
    {
        textRectCorners[i].rotate(textPos, text->getAngle());
        textRectCorners[i].x = std::trunc(textRectCorners[i].x * 1.0E+4) * 1.0E-4;
        textRectCorners[i].y = std::trunc(textRectCorners[i].y * 1.0E+4) * 1.0E-4;
    }

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

    while (((dimArc1->getEndpoint().x < cornerLeftX)   || (dimArc1->getEndpoint().x > cornerRightX) 
    ||      (dimArc1->getEndpoint().y < cornerBottomY) || (dimArc1->getEndpoint().y > cornerTopY))

    &&     (dimArc1->getAngle2() < RS_MAXDOUBLE) 
    &&     (dimArc1->getAngle2() > RS_MINDOUBLE))
    {
        dimArc1->setAngle2(dimArc1->getAngle2() + deltaOffset);
    }

    while (((dimArc2->getStartpoint().x < cornerLeftX)   || (dimArc2->getStartpoint().x > cornerRightX) 
    ||      (dimArc2->getStartpoint().y < cornerBottomY) || (dimArc2->getStartpoint().y > cornerTopY)) 

    &&     (dimArc2->getAngle1() < RS_MAXDOUBLE) 
    &&     (dimArc2->getAngle1() > RS_MINDOUBLE))
    {
        dimArc2->setAngle1(dimArc2->getAngle1() - deltaOffset);
    }

    dimArc1->setPen (pen);
    dimArc2->setPen (pen);

    dimArc1->setLayer (nullptr);
    dimArc2->setLayer (nullptr);

    addEntity (dimArc1);
    addEntity (dimArc2);

    calculateBorders();
}


void LC_DimArc::update()
{
    updateDim();
    RS_Dimension::update();
}


void LC_DimArc::move(const RS_Vector& offset)
{
    RS_Dimension::move (offset);

    dimArcData.centre.move (offset);
    dimArcData.leaderEnd.move (offset);
    dimArcData.leaderStart.move (offset);

    update();
}


void LC_DimArc::rotate(const RS_Vector& center, const double& angle)
{
    RS_Vector angleVector(angle);

    RS_Dimension::rotate (center, angleVector);
    dimArcData.centre.rotate (center, angleVector);
    dimArcData.leaderEnd.rotate (center, angleVector);
    dimArcData.leaderStart.rotate (center, angleVector);
    dimArcData.startAngle = RS_Math::correctAngle(dimArcData.startAngle + angle);
    dimArcData.endAngle = RS_Math::correctAngle(dimArcData.endAngle + angle);

    update();
}


void LC_DimArc::rotate(const RS_Vector& center, const RS_Vector& angleVector)
{
    double angle = angleVector.angle();

    RS_Dimension::rotate (center, angleVector);
    dimArcData.centre.rotate (center, angleVector);
    dimArcData.leaderEnd.rotate (center, angleVector);
    dimArcData.leaderStart.rotate (center, angleVector);
    dimArcData.startAngle = RS_Math::correctAngle(dimArcData.startAngle + angle);
    dimArcData.endAngle = RS_Math::correctAngle(dimArcData.endAngle + angle);

    update();
}


void LC_DimArc::scale(const RS_Vector& center, const RS_Vector& factor)
{
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
}


void LC_DimArc::mirror(const RS_Vector& axisPoint1, const RS_Vector& axisPoint2)
{
    RS_Dimension::mirror (axisPoint1, axisPoint2);

    dimArcData.centre.mirror (axisPoint1, axisPoint2);
    dimArcData.leaderEnd.mirror (axisPoint1, axisPoint2);
    dimArcData.leaderStart.mirror (axisPoint1, axisPoint2);

    double twiceMirrorAngle = axisPoint1.angleTo(axisPoint2) * 2.0;

    dimArcData.startAngle = RS_Math::correctAngle(twiceMirrorAngle - dimArcData.startAngle);
    dimArcData.endAngle = RS_Math::correctAngle(twiceMirrorAngle - dimArcData.endAngle);

    /* Arc has to be defined in CCW direction, but the mirroring has
       effectively reversed the direction. Put it right by reflecting
       angles and definition point in the bisector line of the arc.
       Reflecting the angles in the bisector line is the same as swapping
       start and end angles. Reflecting the definition point is the same
       as rotating the point around the arc centre by ( start + end - def.angle ). */
    std::swap(dimArcData.startAngle,dimArcData.endAngle);

    double defAngle = dimArcData.centre.angleTo(data.definitionPoint);
    data.definitionPoint.rotate(dimArcData.centre, dimArcData.startAngle + dimArcData.endAngle - defAngle);

    update();
}


RS_Vector LC_DimArc::truncateVector(const RS_Vector input_vector)
{
    return RS_Vector( std::trunc(input_vector.x * 1.0E+10) * 1.0E-10, 
                      std::trunc(input_vector.y * 1.0E+10) * 1.0E-10, 
                      0.0);
}


void LC_DimArc::calcDimension()
{
    double dimLineRadius = dimArcData.centre.distanceTo(data.definitionPoint);

    const double entityRadius  = dimArcData.radius;
//    std::cout << "entityRadius " << entityRadius << "\n";
//    std::cout << "dimLineRadius " << dimLineRadius << "\n";

    RS_Vector startAngleVector = RS_Vector(dimArcData.startAngle);
    RS_Vector endAngleVector   = RS_Vector(dimArcData.endAngle);
//    std::cout << "startAngleVector " << startAngleVector << "\n";
//    std::cout << "endAngleVector " << endAngleVector << "\n";

    RS_Vector entityStartPoint = truncateVector(dimArcData.centre + startAngleVector * entityRadius);
//    std::cout << "entityStartPoint " << entityStartPoint << "\n";

    RS_Vector entityEndPoint   = truncateVector(dimArcData.centre + endAngleVector * entityRadius);
//    std::cout << "entityEndPoint " << entityEndPoint << "\n";

    double arcAngle = RS_Math::correctAngle(dimArcData.endAngle - dimArcData.startAngle);

    arcLength = dimArcData.radius * arcAngle;
//    std::cout << "arcLength " << arcLength << "\n";

    if (dimArcData.partial && arcAngle < deg90) {
        RS_Vector midAngleVector = RS_Vector( (dimArcData.startAngle + dimArcData.endAngle) / 2);
        RS_Vector offsetVector = midAngleVector * ( dimLineRadius - dimArcData.radius );
        dimStartPoint = entityStartPoint + offsetVector;
        dimEndPoint   = entityEndPoint + offsetVector;
        dimArc1 = new RS_Arc (this, RS_ArcData(dimArcData.centre + offsetVector, dimArcData.radius, dimArcData.startAngle, dimArcData.startAngle, false));
        dimArc2 = new RS_Arc (this, RS_ArcData(dimArcData.centre + offsetVector, dimArcData.radius, dimArcData.endAngle,   dimArcData.endAngle, false));
    } else {
        dimStartPoint = dimArcData.centre + startAngleVector * dimLineRadius;
        dimEndPoint   = dimArcData.centre + endAngleVector * dimLineRadius;
        dimArc1 = new RS_Arc (this, RS_ArcData(dimArcData.centre, dimLineRadius, dimArcData.startAngle, dimArcData.startAngle, false));
        dimArc2 = new RS_Arc (this, RS_ArcData(dimArcData.centre, dimLineRadius, dimArcData.endAngle,   dimArcData.endAngle, false));
    }

    arrowStartPoint = dimStartPoint;
    arrowEndPoint   = dimEndPoint;
//    std::cout << "arrowStartPoint " << arrowStartPoint << "\n";
//    std::cout << "arrowEndPoint " << arrowEndPoint << "\n";

    RS_Vector extLine1from = entityStartPoint + RS_Vector::polar(getExtensionLineOffset(), entityStartPoint.angleTo(dimStartPoint));
    RS_Vector extLine1to = dimStartPoint + RS_Vector::polar(getExtensionLineExtension(), entityStartPoint.angleTo(dimStartPoint));

    RS_Vector extLine2from = entityEndPoint + RS_Vector::polar(getExtensionLineOffset(), entityEndPoint.angleTo(dimEndPoint));
    RS_Vector extLine2to = dimEndPoint + RS_Vector::polar(getExtensionLineExtension(), entityEndPoint.angleTo(dimEndPoint));

    extLine1 = new RS_Line (this, extLine1from, extLine1to);
    extLine2 = new RS_Line (this, extLine2from, extLine2to);
//    std::cout << "extLine1 " << extLine1from << " to " << extLine1to << "\n";
//    std::cout << "extLine2 " << extLine2from << " to " << extLine2to << "\n";


    /* RS_DEBUG->setLevel(RS_Debug::D_INFORMATIONAL); */

    RS_DEBUG->print( RS_Debug::D_INFORMATIONAL, 
                     "\n LC_DimArc::calcDimension: Start / end angles : %lf / %lf\n", 
                     dimArcData.startAngle, dimArcData.endAngle);

    RS_DEBUG->print( RS_Debug::D_INFORMATIONAL, 
                     "\n LC_DimArc::calcDimension: Dimension / entity radii : %lf / %lf\n", 
                     dimLineRadius, entityRadius);

    if (RS_DEBUG->getLevel() == RS_Debug::D_INFORMATIONAL)
    {
        std::cout << std::endl 
                  << " LC_DimArc::calcDimension: Start Points : " << extLine1from << " to " << extLine1to 
                  << std::endl;

        std::cout << std::endl 
                  << " LC_DimArc::calcDimension: End Points : " << extLine2from << " to " << extLine2to 
                  << std::endl;
    }
}


std::ostream& operator << (std::ostream& os, const LC_DimArc& input_dimArc)
{
    os << "DimArc Information : \n" 
       << input_dimArc.getData() << std::endl << std::endl;

    return os;
}


std::ostream& operator << (std::ostream& os, const LC_DimArcData& input_dimArcData)
{
    os << "{\n\tCentre      : " << input_dimArcData.centre 
       <<  "\n\tRadius      : " << input_dimArcData.radius 
       <<  "\n\tStart Angle : " << input_dimArcData.startAngle 
       <<  "\n\tEnd Angle   : " << input_dimArcData.endAngle 
       <<  "\n\tPartial     : " << input_dimArcData.partial
       <<  "\n\tLeader      : " << input_dimArcData.leader
       <<  "\n\tLeader Start: " << input_dimArcData.leaderStart
       <<  "\n\tLeader End  : " << input_dimArcData.leaderEnd
       <<  "\n}"                << std::endl << std::endl;

    return os;
}
