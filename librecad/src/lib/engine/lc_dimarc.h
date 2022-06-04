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


#pragma once


#include "rs_dimension.h"


struct LC_DimArcData
{
    LC_DimArcData();

    LC_DimArcData(const LC_DimArcData& input_dimArcData);

    LC_DimArcData(const double& input_radius, 
                  const double& input_arcLength,
                  const RS_Vector& input_centre, 
                  const double& input_startAngle, 
                  const double& input_endAngle);

    LC_DimArcData(const double& input_radius, 
                  const double& input_arcLength,
                  const RS_Vector& input_centre, 
                  const double& input_startAngle, 
                  const double& input_endAngle,
                  const bool& input_partial);

    LC_DimArcData(const double& input_radius, 
                  const double& input_arcLength,
                  const RS_Vector& input_centre, 
                  const double& input_startAngle, 
                  const double& input_endAngle,
                  const bool& input_partial,
                  const bool& input_leader,
                  const RS_Vector& input_leaderStart,
                  const RS_Vector& input_leaderEnd);

    double radius;           /*! Radius of the arc */
    double arcLength;        /*! Length value for the arc */

    RS_Vector centre;        /*! Coordinate of arc centre point */
    double startAngle;       /*! Angle centre to arc start point, radians */
    double endAngle;         /*! Angle centre to arc end point, radians */

    bool partial;            /*! Display as a partial arc dimension */
    bool leader;             /*! Display with leader line from text to the arc */

    RS_Vector leaderStart;   /*! Coordinate of leader line start point */
    RS_Vector leaderEnd;     /*! Coordinate of leader line start point */
};


std::ostream& operator << (std::ostream& os, const LC_DimArcData& input_dimArcData);


class LC_DimArc : public RS_Dimension
{
    friend std::ostream& operator << (std::ostream& os, const LC_DimArc& input_dimArc);


    public:

        LC_DimArc( RS_EntityContainer* parent, 
                   const RS_DimensionData& input_commonDimData, 
                   const LC_DimArcData& input_dimArcData);

        RS_Entity* clone() const override;

        RS2::EntityType rtti() const override
        {
            return RS2::EntityDimArc;
        }

        LC_DimArcData getData() const
        {
            return dimArcData;
        }

        double getRadius() const
        {
            return dimArcData.radius;
        }

        double getArcLength() const
        {
            return dimArcData.arcLength;
        }

        double getStartAngle() const
        {
            return dimArcData.startAngle;
        }

        double getEndAngle() const
        {
            return dimArcData.endAngle;
        }

        RS_Vector getCenter() const
        {
            return dimArcData.centre;
        }

        bool getPartial() const
        {
            return dimArcData.partial;
        }

        bool getLeader() const
        {
            return dimArcData.leader;
        }

        RS_Vector getLeaderStart() const
        {
            return dimArcData.leaderStart;
        }

        RS_Vector getLeaderEnd() const
        {
            return dimArcData.leaderEnd;
        }


        QString getMeasuredLabel() override;

        void updateDim(bool autoText = false) override;

        void update() override;

        void move   (const RS_Vector& offset)                                   override;
        void rotate (const RS_Vector& center,     const double& angle)          override;
        void rotate (const RS_Vector& center,     const RS_Vector& angleVector) override;
        void scale  (const RS_Vector& center,     const RS_Vector& factor)      override;
        void mirror (const RS_Vector& axisPoint1, const RS_Vector& axisPoint2)  override;


    protected:

        LC_DimArcData dimArcData;


    private:

        RS_Vector arrowStartPoint;
        RS_Vector arrowEndPoint;

        RS_Vector dimStartPoint;
        RS_Vector dimEndPoint;

        RS_Line* extLine1;
        RS_Line* extLine2;
        RS_Arc*  dimArc1;
        RS_Arc*  dimArc2;

        void calcDimension();

        RS_Vector truncateVector(const RS_Vector input_vector);

        void arrow( const RS_Vector& point, 
                    const double angle, 
                    const double direction, 
                    const RS_Pen& pen);
};

