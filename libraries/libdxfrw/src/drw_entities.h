/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2016-2022 A. Stebich (librecad@mail.lordofbikes.de)        **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#ifndef DRW_ENTITIES_H
#define DRW_ENTITIES_H


#include <string>
#include <vector>
#include <list>
#include <memory>
#include "drw_base.h"

class dxfReader;
class dwgBuffer;
class DRW_Polyline;

namespace DRW {

   //! Entity's type.
    enum ETYPE {
        E3DFACE,
//        E3DSOLID, //encrypted proprietary data
//        ACAD_PROXY_ENTITY,
        ARC,
//        ATTDEF,
//        ATTRIB,
        BLOCK,// and ENDBLK
//        BODY, //encrypted proprietary data
        CIRCLE,
        DIMENSION,
        DIMALIGNED,
        DIMLINEAR,
        DIMRADIAL,
        DIMDIAMETRIC,
        DIMANGULAR,
        DIMANGULAR3P,
        DIMORDINATE,
        DIMARC,
        ELLIPSE,
        HATCH,
//        HELIX,
        IMAGE,
        INSERT,
        LEADER,
//        LIGHT,
        LINE,
        LWPOLYLINE,
//        MESH,
//        MLINE,
//        MLEADERSTYLE,
//        MLEADER,
        MTEXT,
//        OLEFRAME,
//        OLE2FRAME,
        POINT,
        POLYLINE,
        RAY,
//        REGION, //encrypted proprietary data
//        SECTION,
//        SEQEND,//not needed?? used in polyline and insert/attrib and dwg
//        SHAPE,
        SOLID,
        SPLINE,
//        SUN,
//        SURFACE, //encrypted proprietary data can be four types
//        TABLE,
        TEXT,
//        TOLERANCE,
        TRACE,
        UNDERLAY,
        VERTEX,
        VIEWPORT,
//        WIPEOUT, //WIPEOUTVARIABLE
        XLINE,
        UNKNOWN
    };

}
//only in DWG: MINSERT, 5 types of vertex, 4 types of polylines: 2d, 3d, pface & mesh
//shape, dictionary, MLEADER, MLEADERSTYLE

#define SETENTFRIENDS  friend class dxfRW; \
                       friend class dwgReader;

//! Base class for entities
/*!
*  Base class for entities
*  @author Rallaz
*/
class DRW_Entity {
    SETENTFRIENDS
public:
    //initializes default values
	DRW_Entity() = default;
	virtual ~DRW_Entity() = default;

	//removed copy/move ctors
	// looks like the potential issue is the "curr" pointer is reset in previous
	// versions during copy ctor

	void reset() {
		extData.clear();
		curr.reset();
	}

    virtual void applyExtrusion() = 0;

protected:
    //parses dxf pair to read entity
    virtual bool parseCode(int code, dxfReader *reader);
    //calculates extrusion axis (normal vector)
    void calculateAxis(DRW_Coord extPoint);
    //apply extrusion to @extPoint and return data in @point
    void extrudePoint(DRW_Coord extPoint, DRW_Coord *point);
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0)=0;
    //parses dwg common start part to read entity
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer* strBuf, duint32 bs=0);
    //parses dwg common handles part to read entity
    bool parseDwgEntHandle(DRW::Version version, dwgBuffer *buf);

    //parses dxf 102 groups to read entity
    bool parseDxfGroups(int code, dxfReader *reader);

public:
	enum DRW::ETYPE eType = DRW::UNKNOWN;     /*!< enum: entity type, code 0 */
	duint32 handle = DRW::NoHandle;            /*!< entity identifier, code 5 */
    std::string subclassName;     /*!< subclass name, code 100 */
    std::list<std::list<DRW_Variant> > appData; /*!< list of application data, code 102 */
	duint32 parentHandle = DRW::NoHandle;      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
	DRW::Space space = DRW::ModelSpace;          /*!< space indicator, code 67*/
	UTF8STRING layer = "0";          /*!< layer name, code 8 */
	UTF8STRING lineType = "BYLAYER";       /*!< line type, code 6 */
	duint32 material = DRW::MaterialByLayer;          /*!< hard pointer id to material object, code 347 */
	int color = DRW::ColorByLayer;                 /*!< entity color, code 62 */
	enum DRW_LW_Conv::lineWidth lWeight = DRW_LW_Conv::widthByLayer; /*!< entity lineweight, code 370 */
	double ltypeScale = 1.0;         /*!< linetype scale, code 48 */
	bool visible = true;              /*!< entity visibility, code 60 */
	int numProxyGraph = 0;         /*!< Number of bytes in proxy graphics, code 92 */
    std::string proxyGraphics; /*!< proxy graphics bytes, code 310 */
	int color24 = -1;               /*!< 24-bit color, code 420 */
    std::string colorName;     /*!< color name, code 430 */
	int transparency = DRW::Opaque;          /*!< transparency, code 440 */
	int plotStyle = DRW::DefaultPlotStyle;             /*!< hard pointer id to plot style object, code 390 */
	DRW::ShadowMode shadow = DRW::CastAndReceieveShadows;    /*!< shadow mode, code 284 */
	bool haveExtrusion = false;        /*!< set to true if the entity have extrusion*/
	std::vector<std::shared_ptr<DRW_Variant>> extData; /*!< FIFO list of extended data, codes 1000 to 1071*/

protected: //only for read dwg
    duint8 haveNextLinks; //aka nolinks //B
    duint8 plotFlags; //presence of plot style //BB
    duint8 ltFlags; //presence of linetype handle //BB
    duint8 materialFlag; //presence of material handle //BB
    duint8 shadowFlag; //presence of shadow handle ?? (in dwg may be plotflag)//RC
    dwgHandle lTypeH;
    dwgHandle layerH;
	duint32 nextEntLink = 0;
	duint32 prevEntLink = 0;
	bool ownerHandle = false;

	duint8 xDictFlag = 0;
	dint32 numReactors = 0; //
    duint32 objSize;  //RL 32bits object data size in bits
    dint16 oType;

private:
	void init(DRW_Entity const& rhs);
	DRW_Coord extAxisX;
    DRW_Coord extAxisY;
	std::shared_ptr<DRW_Variant> curr;
};


//! Class to handle point entity
/*!
*  Class to handle point entity
*  @author Rallaz
*/
class DRW_Point : public DRW_Entity {
    SETENTFRIENDS
public:
    DRW_Point() {
        eType = DRW::POINT;
        basePoint.z = extPoint.x = extPoint.y = 0;
        extPoint.z = 1;
        thickness = 0;
    }

    virtual void applyExtrusion() override {}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    DRW_Coord basePoint;      /*!<  base point, code 10, 20 & 30 */
    double thickness;         /*!< thickness, code 39 */
    DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
    // TNick: we're not handling code 50 - Angle of the X axis for
    // the UCS in effect when the point was drawn
};

//! Class to handle line entity
/*!
*  Class to handle line entity
*  @author Rallaz
*/
class DRW_Line : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Line() {
        eType = DRW::LINE;
        secPoint.z = 0;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    DRW_Coord secPoint;        /*!< second point, code 11, 21 & 31 */
};

//! Class to handle ray entity
/*!
*  Class to handle ray entity
*  @author Rallaz
*/
class DRW_Ray : public DRW_Line {
    SETENTFRIENDS
public:
    DRW_Ray() {
        eType = DRW::RAY;
    }
protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;
};

//! Class to handle xline entity
/*!
*  Class to handle xline entity
*  @author Rallaz
*/
class DRW_Xline : public DRW_Ray {
public:
    DRW_Xline() {
        eType = DRW::XLINE;
    }
};

//! Class to handle circle entity
/*!
*  Class to handle circle entity
*  @author Rallaz
*/
class DRW_Circle : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Circle() {
        eType = DRW::CIRCLE;
    }

    virtual void applyExtrusion() override;

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    double radious;                 /*!< radius, code 40 */
};

//! Class to handle arc entity
/*!
*  Class to handle arc entity
*  @author Rallaz
*/
class DRW_Arc : public DRW_Circle {
    SETENTFRIENDS
public:
    DRW_Arc() {
        eType = DRW::ARC;
        isccw = 1;
    }

    virtual void applyExtrusion() override;

    //! center point in OCS
    const DRW_Coord & center() { return basePoint; }
    //! the radius of the circle
    double radius() { return radious; }
    //! start angle in radians
    double startAngle() { return staangle; }
    //! end angle in radians
    double endAngle() { return endangle; }
    //! thickness
    double thick() { return thickness; }
    //! extrusion
    const DRW_Coord & extrusion() { return extPoint; }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    bool parseCode(int code, dxfReader *reader) override;
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    double staangle;            /*!< start angle, code 50 in radians*/
    double endangle;            /*!< end angle, code 51 in radians */
    int isccw;                  /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle ellipse entity
/*!
*  Class to handle ellipse and elliptic arc entity
*  Note: start/end parameter are in radians for ellipse entity but
*  for hatch boundary are in degrees
*  @author Rallaz
*/
class DRW_Ellipse : public DRW_Line {
    SETENTFRIENDS
public:
    DRW_Ellipse() {
        eType = DRW::ELLIPSE;
        isccw = 1;
    }

    void toPolyline(DRW_Polyline *pol, int parts = 128);
    virtual void applyExtrusion() override;

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    bool parseCode(int code, dxfReader *reader) override;
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

private:
    void correctAxis();

public:
    double ratio;        /*!< ratio, code 40 */
    double staparam;     /*!< start parameter, code 41, 0.0 for full ellipse*/
    double endparam;     /*!< end parameter, code 42, 2*PI for full ellipse */
    int isccw;           /*!< is counter clockwise arc?, only used in hatch, code 73 */
};

//! Class to handle trace entity
/*!
*  Class to handle trace entity
*  @author Rallaz
*/
class DRW_Trace : public DRW_Line {
    SETENTFRIENDS
public:
    DRW_Trace() {
        eType = DRW::TRACE;
        thirdPoint.z = 0;
        fourPoint.z = 0;
    }

    virtual void applyExtrusion() override;

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    DRW_Coord thirdPoint;        /*!< third point, code 12, 22 & 32 */
    DRW_Coord fourPoint;        /*!< four point, code 13, 23 & 33 */
};

//! Class to handle solid entity
/*!
*  Class to handle solid entity
*  @author Rallaz
*/
class DRW_Solid : public DRW_Trace {
    SETENTFRIENDS
public:
    DRW_Solid() {
        eType = DRW::SOLID;
    }

protected:
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    //! first corner (2D)
    const DRW_Coord & firstCorner() { return basePoint; }
    //! second corner (2D)
    const DRW_Coord & secondCorner() { return secPoint; }
    //! third corner (2D)
    const DRW_Coord & thirdCorner() { return thirdPoint; }
    //! fourth corner (2D)
    const DRW_Coord & fourthCorner() { return thirdPoint; }
    //! thickness
    double thick() { return thickness; }
    //! elevation
    double elevation() { return basePoint.z; }
    //! extrusion
    const DRW_Coord & extrusion() { return extPoint; }

};

//! Class to handle 3dface entity
/*!
*  Class to handle 3dface entity
*  @author Rallaz
*/
class DRW_3Dface : public DRW_Trace {
    SETENTFRIENDS
public:
    enum InvisibleEdgeFlags {
        NoEdge = 0x00,
        FirstEdge = 0x01,
        SecodEdge = 0x02,
        ThirdEdge = 0x04,
        FourthEdge = 0x08,
        AllEdges = 0x0F
    };

    DRW_3Dface() {
        eType = DRW::E3DFACE;
        invisibleflag = 0;
    }

    //! first corner in WCS
    const DRW_Coord & firstCorner() { return basePoint; }
    //! second corner in WCS
    const DRW_Coord & secondCorner() { return secPoint; }
    //! third corner in WCS
    const DRW_Coord & thirdCorner() { return thirdPoint; }
    //! fourth corner in WCS
    const DRW_Coord & fourthCorner() { return fourPoint; }
    //! edge visibility flags
    InvisibleEdgeFlags edgeFlags() { return (InvisibleEdgeFlags)invisibleflag; }

protected:
    //! interpret code in dxf reading process or dispatch to inherited class
    bool parseCode(int code, dxfReader *reader) override;
    //! interpret dwg data (was already determined to be part of this object)
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    int invisibleflag;       /*!< invisible edge flag, code 70 */

};

//! Class to handle block entries
/*!
*  Class to handle block entries
*  @author Rallaz
*/
class DRW_Block : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Block() {
        eType = DRW::BLOCK;
        layer = "0";
        flags = 0;
        name = "*U0";
        isEnd = false;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    UTF8STRING name;             /*!< block name, code 2 */
    int flags;                   /*!< block type, code 70 */
private:
    bool isEnd; //for dwg parsing
};


//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Insert : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Insert() {
        eType = DRW::INSERT;
        xscale = 1;
        yscale = 1;
        zscale = 1;
        angle = 0;
        colcount = 1;
        rowcount = 1;
        colspace = 0;
        rowspace = 0;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    UTF8STRING name;         /*!< block name, code 2 */
    double xscale;           /*!< x scale factor, code 41 */
    double yscale;           /*!< y scale factor, code 42 */
    double zscale;           /*!< z scale factor, code 43 */
    double angle;            /*!< rotation angle in radians, code 50 */
    int colcount;            /*!< column count, code 70 */
    int rowcount;            /*!< row count, code 71 */
    double colspace;         /*!< column space, code 44 */
    double rowspace;         /*!< row space, code 45 */
public: //only for read dwg
    dwgHandle blockRecH;
    dwgHandle seqendH; //RLZ: on implement attrib remove this handle from obj list (see pline/vertex code)
};

//! Class to handle lwpolyline entity
/*!
*  Class to handle lwpolyline entity
*  @author Rallaz
*/
class DRW_LWPolyline : public DRW_Entity {
    SETENTFRIENDS
public:
    DRW_LWPolyline() {
        eType = DRW::LWPOLYLINE;
        elevation = thickness = width = 0.0;
        flags = 0;
        extPoint.x = extPoint.y = 0;
		extPoint.z = 1;
    }
    
    DRW_LWPolyline(const DRW_LWPolyline& p):DRW_Entity(p){
        this->eType = DRW::LWPOLYLINE;
        this->elevation = p.elevation;
        this->thickness = p.thickness;
        this->width = p.width;
        this->flags = p.flags;
		this->extPoint = p.extPoint;
        for (unsigned i=0; i<p.vertlist.size(); i++)// RLZ ok or new
		  this->vertlist.push_back(
					std::make_shared<DRW_Vertex2D>(*p.vertlist.at(i))
					);
    }
	// TODO rule of 5

    virtual void applyExtrusion() override;
    void addVertex (DRW_Vertex2D v) {
		std::shared_ptr<DRW_Vertex2D> vert = std::make_shared<DRW_Vertex2D>(v);
        vertlist.push_back(vert);
    }
	std::shared_ptr<DRW_Vertex2D> addVertex () {
		std::shared_ptr<DRW_Vertex2D> vert = std::make_shared<DRW_Vertex2D>();
        vert->stawidth = 0;
        vert->endwidth = 0;
        vert->bulge = 0;
        vertlist.push_back(vert);
        return vert;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version v, dwgBuffer *buf, duint32 bs=0) override;

public:
    int vertexnum;            /*!< number of vertex, code 90 */
    int flags;                /*!< polyline flag, code 70, default 0 */
    double width;             /*!< constant width, code 43 */
    double elevation;         /*!< elevation, code 38 */
    double thickness;         /*!< thickness, code 39 */
    DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
	std::shared_ptr<DRW_Vertex2D> vertex;       /*!< current vertex to add data */
	std::vector<std::shared_ptr<DRW_Vertex2D>> vertlist;  /*!< vertex list */
};

//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_Text : public DRW_Line {
    SETENTFRIENDS
public:
    //! Vertical alignments.
        enum VAlign {
            VBaseLine = 0,  /*!< Top = 0 */
            VBottom,        /*!< Bottom = 1 */
            VMiddle,        /*!< Middle = 2 */
            VTop            /*!< Top = 3 */
        };

    //! Horizontal alignments.
        enum HAlign {
            HLeft = 0,     /*!< Left = 0 */
            HCenter,       /*!< Centered = 1 */
            HRight,        /*!< Right = 2 */
            HAligned,      /*!< Aligned = 3 (if VAlign==0) */
            HMiddle,       /*!< middle = 4 (if VAlign==0) */
            HFit           /*!< fit into point = 5 (if VAlign==0) */
        };

    DRW_Text() {
        eType = DRW::TEXT;
        angle = 0;
        widthscale = 1;
        oblique = 0;
        style = "STANDARD";
        textgen = 0;
        alignH = HLeft;
        alignV = VBaseLine;
    }

    virtual void applyExtrusion() override {} //RLZ TODO

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    double height;             /*!< height text, code 40 */
    UTF8STRING text;           /*!< text string, code 1 */
    double angle;              /*!< rotation angle in degrees (360), code 50 */
    double widthscale;         /*!< width factor, code 41 */
    double oblique;            /*!< oblique angle, code 51 */
    UTF8STRING style;          /*!< style name, code 7 */
    int textgen;               /*!< text generation, code 71 */
    enum HAlign alignH;        /*!< horizontal align, code 72 */
    enum VAlign alignV;        /*!< vertical align, code 73 */
    dwgHandle styleH;          /*!< handle for text style */
};

//! Class to handle insert entries
/*!
*  Class to handle insert entries
*  @author Rallaz
*/
class DRW_MText : public DRW_Text {
    SETENTFRIENDS
public:
    //! Attachments.
    enum Attach {
        TopLeft = 1,
        TopCenter,
        TopRight,
        MiddleLeft,
        MiddleCenter,
        MiddleRight,
        BottomLeft,
        BottomCenter,
        BottomRight
    };

    DRW_MText() {
        eType = DRW::MTEXT;
        interlin = 1;
        alignV = (VAlign)TopLeft;
        textgen = 1;
        hasXAxisVec = false; // if true need to calculate angle from secPoint vector
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    void updateAngle();    // recalculate angle if 'hasXAxisVec' is true
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    double interlin;     /*!< width factor, code 44 */
private:
    bool hasXAxisVec; /* renamed by djm for better description */
};

//! Class to handle vertex
/*!
*  Class to handle vertex  for polyline entity
*  @author Rallaz
*/
class DRW_Vertex : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Vertex() {
        eType = DRW::VERTEX;
        stawidth = endwidth = bulge = 0;
        vindex1 = vindex2 = vindex3 = vindex4 = 0;
        flags = identifier = 0;
    }
    DRW_Vertex(double sx, double sy, double sz, double b) {
        stawidth = endwidth = 0;
        vindex1 = vindex2 = vindex3 = vindex4 = 0;
        flags = identifier = 0;
        basePoint.x = sx;
        basePoint.y =sy;
        basePoint.z =sz;
        bulge = b;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    using DRW_Point::parseDwg;
    bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0, double el=0);

public:
    double stawidth;          /*!< Start width, code 40 */
    double endwidth;          /*!< End width, code 41 */
    double bulge;             /*!< bulge, code 42 */

    int flags;                 /*!< vertex flag, code 70, default 0 */
    double tgdir;           /*!< curve fit tangent direction, code 50 */
    int vindex1;             /*!< polyface mesh vertex index, code 71, default 0 */
    int vindex2;             /*!< polyface mesh vertex index, code 72, default 0 */
    int vindex3;             /*!< polyface mesh vertex index, code 73, default 0 */
    int vindex4;             /*!< polyface mesh vertex index, code 74, default 0 */
    int identifier;           /*!< vertex identifier, code 91, default 0 */
};

//! Class to handle polyline entity
/*!
*  Class to handle polyline entity
*  @author Rallaz
*/
class DRW_Polyline : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Polyline() {
        eType = DRW::POLYLINE;
        defstawidth = defendwidth = 0.0;
        basePoint.x = basePoint.y = 0.0;
        flags = vertexcount = facecount = 0;
        smoothM = smoothN = curvetype = 0;
    }
    void addVertex (DRW_Vertex v) {
        std::shared_ptr<DRW_Vertex> vert = std::make_shared<DRW_Vertex>();
        vert->basePoint.x = v.basePoint.x;
        vert->basePoint.y = v.basePoint.y;
        vert->basePoint.z = v.basePoint.z;
        vert->stawidth = v.stawidth;
        vert->endwidth = v.endwidth;
        vert->bulge = v.bulge;
        vertlist.push_back(vert);
    }
    void appendVertex (std::shared_ptr<DRW_Vertex> const& v) {
        vertlist.push_back(v);
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    int flags;               /*!< polyline flag, code 70, default 0 */
    double defstawidth;      /*!< Start width, code 40, default 0 */
    double defendwidth;      /*!< End width, code 41, default 0 */
    int vertexcount;         /*!< polygon mesh M vertex or  polyface vertex num, code 71, default 0 */
    int facecount;           /*!< polygon mesh N vertex or  polyface face num, code 72, default 0 */
    int smoothM;             /*!< smooth surface M density, code 73, default 0 */
    int smoothN;             /*!< smooth surface M density, code 74, default 0 */
    int curvetype;           /*!< curves & smooth surface type, code 75, default 0 */

    std::vector<std::shared_ptr<DRW_Vertex>> vertlist;  /*!< vertex list */

private:
    std::list<duint32>hadlesList; //list of handles, only in 2004+
    duint32 firstEH;      //handle of first entity, only in pre-2004
    duint32 lastEH;       //handle of last entity, only in pre-2004
    dwgHandle seqEndH;    //handle of SEQEND entity
};


//! Class to handle spline entity
/*!
*  Class to handle spline entity
*  @author Rallaz
*/
class DRW_Spline : public DRW_Entity {
    SETENTFRIENDS
public:
    DRW_Spline() {
        eType = DRW::SPLINE;
        flags = nknots = ncontrol = nfit = 0;
        tolknot = tolcontrol = tolfit = 0.0000001;

    }
    virtual void applyExtrusion() override {}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
//    double ex;                /*!< normal vector x coordinate, code 210 */
//    double ey;                /*!< normal vector y coordinate, code 220 */
//    double ez;                /*!< normal vector z coordinate, code 230 */
    DRW_Coord normalVec;      /*!< normal vector, code 210, 220, 230 */
    DRW_Coord tgStart;        /*!< start tangent, code 12, 22, 32 */
//    double tgsx;              /*!< start tangent x coordinate, code 12 */
//    double tgsy;              /*!< start tangent y coordinate, code 22 */
//    double tgsz;              /*!< start tangent z coordinate, code 32 */
    DRW_Coord tgEnd;          /*!< end tangent, code 13, 23, 33 */
//    double tgex;              /*!< end tangent x coordinate, code 13 */
//    double tgey;              /*!< end tangent y coordinate, code 23 */
//    double tgez;              /*!< end tangent z coordinate, code 33 */
    int flags;                /*!< spline flag, code 70 */
    int degree;               /*!< degree of the spline, code 71 */
    dint32 nknots;            /*!< number of knots, code 72, default 0 */
    dint32 ncontrol;          /*!< number of control points, code 73, default 0 */
    dint32 nfit;              /*!< number of fit points, code 74, default 0 */
    double tolknot;           /*!< knot tolerance, code 42, default 0.0000001 */
    double tolcontrol;        /*!< control point tolerance, code 43, default 0.0000001 */
    double tolfit;            /*!< fit point tolerance, code 44, default 0.0000001 */

    std::vector<double> knotslist;           /*!< knots list, code 40 */
    std::vector<double> weightlist;          /*!< weight list, code 41 */
    std::vector<std::shared_ptr<DRW_Coord>> controllist;  /*!< control points list, code 10, 20 & 30 */
    std::vector<std::shared_ptr<DRW_Coord>> fitlist;      /*!< fit points list, code 11, 21 & 31 */

private:
    std::shared_ptr<DRW_Coord> controlpoint;   /*!< current control point to add data */
    std::shared_ptr<DRW_Coord> fitpoint;       /*!< current fit point to add data */
};

//! Class to handle hatch loop
/*!
*  Class to handle hatch loop
*  @author Rallaz
*/
class DRW_HatchLoop {
public:
    DRW_HatchLoop(int t) {
        type = t;
        numedges = 0;
    }

    void update() {
        numedges = objlist.size();
    }

public:
    int type;               /*!< boundary path type, code 92, polyline=2, default=0 */
    int numedges;           /*!< number of edges (if not a polyline), code 93 */
//TODO: store lwpolylines as entities
//    std::vector<DRW_LWPolyline *> pollist;  /*!< polyline list */
    std::vector<std::shared_ptr<DRW_Entity>> objlist;      /*!< entities list */
};

//! Class to handle hatch entity
/*!
*  Class to handle hatch entity
*  @author Rallaz
*/
//TODO: handle lwpolylines, splines and ellipses
class DRW_Hatch : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Hatch() {
        eType = DRW::HATCH;
        angle = scale = 0.0;
        basePoint.x = basePoint.y = basePoint.z = 0.0;
        loopsnum = hstyle = associative = 0;
        solid = hpattern = 1;
        deflines = doubleflag = 0;
        clearEntities();
    }

    void appendLoop (std::shared_ptr<DRW_HatchLoop> const& v) {
        looplist.push_back(v);
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    UTF8STRING name;           /*!< hatch pattern name, code 2 */
    int solid;                 /*!< solid fill flag, code 70, solid=1, pattern=0 */
    int associative;           /*!< associativity, code 71, associatve=1, non-assoc.=0 */
    int hstyle;                /*!< hatch style, code 75 */
    int hpattern;              /*!< hatch pattern type, code 76 */
    int doubleflag;            /*!< hatch pattern double flag, code 77, double=1, single=0 */
    int loopsnum;              /*!< namber of boundary paths (loops), code 91 */
    double angle;              /*!< hatch pattern angle, code 52 */
    double scale;              /*!< hatch pattern scale, code 41 */
    int deflines;              /*!< number of pattern definition lines, code 78 */

    std::vector<std::shared_ptr<DRW_HatchLoop>> looplist;  /*!< polyline list */

private:
    void clearEntities(){
        pt.reset();
        line.reset();
        pline.reset();
        arc.reset();
        ellipse.reset();
        spline.reset();
        plvert.reset();
    }

    void addLine() {
        clearEntities();
        if (loop) {
            pt = line = std::make_shared<DRW_Line>();
            loop->objlist.push_back(line);
        }
    }

    void addArc() {
        clearEntities();
        if (loop) {
            pt = arc = std::make_shared<DRW_Arc>();
            loop->objlist.push_back(arc);
        }
    }

    void addEllipse() {
        clearEntities();
        if (loop) {
            pt = ellipse = std::make_shared<DRW_Ellipse>();
            loop->objlist.push_back(ellipse);
        }
    }

    void addSpline() {
        clearEntities();
        if (loop) {
            pt.reset();
            spline = std::make_shared<DRW_Spline>();
            loop->objlist.push_back(spline);
        }
    }

    std::shared_ptr<DRW_HatchLoop> loop;       /*!< current loop to add data */
    std::shared_ptr<DRW_Line> line;
    std::shared_ptr<DRW_Arc> arc;
    std::shared_ptr<DRW_Ellipse> ellipse;
    std::shared_ptr<DRW_Spline> spline;
    std::shared_ptr<DRW_LWPolyline> pline;
    std::shared_ptr<DRW_Point> pt;
    std::shared_ptr<DRW_Vertex2D> plvert;
    bool ispol;
};

//! Class to handle image entity
/*!
*  Class to handle image entity
*  @author Rallaz
*/
class DRW_Image : public DRW_Line {
    SETENTFRIENDS
public:
    DRW_Image() {
        eType = DRW::IMAGE;
        fade = clip = 0;
        brightness = contrast = 50;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    duint32 ref;               /*!< Hard reference to imagedef object, code 340 */
    DRW_Coord vVector;         /*!< V-vector of single pixel, x coordinate, code 12, 22 & 32 */
//    double vx;                 /*!< V-vector of single pixel, x coordinate, code 12 */
//    double vy;                 /*!< V-vector of single pixel, y coordinate, code 22 */
//    double vz;                 /*!< V-vector of single pixel, z coordinate, code 32 */
    double sizeu;              /*!< image size in pixels, U value, code 13 */
    double sizev;              /*!< image size in pixels, V value, code 23 */
    double dz;                 /*!< z coordinate, code 33 */
    int clip;                  /*!< Clipping state, code 280, 0=off 1=on */
    int brightness;            /*!< Brightness value, code 281, (0-100) default 50 */
    int contrast;              /*!< Brightness value, code 282, (0-100) default 50 */
    int fade;                  /*!< Brightness value, code 283, (0-100) default 0 */

};


//! Base class for dimension entity
/*!
*  Base class for dimension entity
*  @author Rallaz
*/
class DRW_Dimension : public DRW_Entity {
    SETENTFRIENDS

public:
    DRW_Dimension() {
        eType = DRW::DIMENSION;
        dimType = 0;
        haveActvalue = false;
        dimVersion = 0;
        styleName = "STANDARD";
        textAttach = 5;
        textLinestyle = 1;
        textLinefactor = 1.0;
        dimActvalue = textRotation = 0;
        extrusionVec.z = 1.0;
        dimHdir = 0;
    }

    DRW_Dimension(const DRW_Dimension& d): DRW_Entity(d) {
        eType = DRW::DIMENSION;
        dimType = d.dimType;
        haveActvalue = d.haveActvalue;
        dimVersion = d.dimVersion;
        blockName = d.blockName;
        pt0 = d.pt0;
        pt1 = d.pt1;
        pt2 = d.pt2;
        pt3 = d.pt3;
        pt4 = d.pt4;
        pt5 = d.pt5;
        pt6 = d.pt6;
        pt7 = d.pt7;
        dimText = d.dimText;
        styleName = d.styleName;
        textAttach = d.textAttach;
        textLinestyle = d.textLinestyle;
        textLinefactor = d.textLinefactor;
        dimActvalue = d.dimActvalue;
        haveActvalue = d.haveActvalue;
        textRotation = d.textRotation;
        extrusionVec = d.extrusionVec;
        dimHdir = d.dimHdir;
    }

    virtual ~DRW_Dimension() {}

    virtual void applyExtrusion() override {}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *sBuf, duint32 bs = 0) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer* buf, duint32 bs=0) override;

    /* Accessor methods for class members corresponding to DXF codes data
       (common to all dimension types).                                   */
protected:
    DRW_Coord getPt0() const {return pt0;}
    void setPt0(const DRW_Coord p) {pt0 = p;}
    void setPt0(double x, double y, double z) {pt0.x = x; pt0.y = y; pt0.z = z;}

    DRW_Coord getPt1() const {return pt1;}
    void setPt1(const DRW_Coord p) {pt1 = p;}
    void setPt1(double x, double y, double z) {pt1.x = x; pt1.y = y; pt1.z = z;}

    DRW_Coord getPt2() const {return pt2;}
    void setPt2(const DRW_Coord p) {pt2 = p;}
    void setPt2(double x, double y, double z) {pt2.x = x; pt2.y = y; pt2.z = z;}

    DRW_Coord getPt3() const {return pt3;}
    void setPt3(const DRW_Coord p) {pt3 = p;}
    void setPt3(double x, double y, double z) {pt3.x = x; pt3.y = y; pt3.z = z;}

    DRW_Coord getPt4() const {return pt4;}
    void setPt4(const DRW_Coord p) {pt4 = p;}
    void setPt4(double x, double y, double z) {pt4.x = x; pt4.y = y; pt4.z = z;}

    DRW_Coord getPt5() const {return pt5;}
    void setPt5(const DRW_Coord p) {pt5 = p;}
    void setPt5(double x, double y, double z) {pt5.x = x; pt5.y = y; pt5.z = z;}

    DRW_Coord getPt6() const {return pt6;}
    void setPt6(const DRW_Coord p) {pt6 = p;}
    void setPt6(double x, double y, double z) {pt6.x = x; pt6.y = y; pt6.z = z;}

    DRW_Coord getPt7() const {return pt7;}
    void setPt7(const DRW_Coord p) {pt7 = p;}
    void setPt7(double x, double y, double z) {pt7.x = x; pt7.y = y; pt7.z = z;}

public:
    DRW_Coord getDimLinePoint() const {return getPt0();}         /*!< Definition point for dimension line = pt0 */
    void setDimLinePoint(const DRW_Coord p) {setPt0(p);}
    void setDimLinePoint(double x, double y, double z) {setPt0(x,y,z);}

    DRW_Coord getTextPoint() const {return getPt1();}            /*!< Middle point of dimText = pt1 */
    void setTextPoint(const DRW_Coord p) {setPt1(p);}
    void setTextPoint(double x, double y, double z) {setPt1(x,y,z);}

    int getDimVersion(){return dimVersion;}                      /*!< Dimension version number */
    void setDimVersion(const int v) {dimVersion = v;}

    std::string getName() {return blockName;}                    /*!< Name of the block that contains the entities */
    void setName(const std::string s) {blockName = s;}

    int getType() {return dimType;}                              /*!< Dimension dimType (rotated/aligned/angular/diameter/radius etc) */
    void setType(const int t) {dimType = t;}

    int getAlign() const {return textAttach;}                    /*!< attachment point, code 71 */
    void setAlign(const int a) {textAttach = a;}

    std::string getStyle() const {return styleName;}             /*!< Dimension styleName, code 3 */
    void setStyle(const std::string s) {styleName = s;}

    int getTextLineStyle() const {return textLinestyle;}         /*!< Dimension text line spacing style, code 72, default 1 */
    void setTextLineStyle(const int l) {textLinestyle = l;}

    std::string getDimText() const {return dimText;}             /*!< Dimension dimText explicitly entered by the user, code 1 */
    void setDimText(const std::string t) {dimText = t;}

    double getTextLineFactor() const {return textLinefactor;}    /*!< Dimension text line spacing factor, code 41 */
    void setTextLineFactor(const double l) {textLinefactor = l;}

    double getTextRotation() const {return textRotation;}        /*!< rotation angle of the dimension dimText, code 53 (optional) default 0 */
    void setTextRotation(const double d) {textRotation = d;}

    DRW_Coord getExtrusion() const {return extrusionVec;}        /*!< extrusion vector, code 210, 220 & 230 */
    void setExtrusion(const DRW_Coord p) {extrusionVec = p;}

    double getActValue() const {return dimActvalue;}             /*!< Actual measurement, code 42 (optional, read-only) */
    void setActValue(const double d) {dimActvalue = d; haveActvalue = true;}

    /* Following all correspond to common dimension group code values within the
       DXF. Values are within the "AcDbDimension" and other dimension sub-classes
       in the DXF.                                                                */
protected:
    UTF8STRING dimText;        /*!< Code 1 - Dimension dimText explicitly entered by the user */
    std::string blockName;     /*!< Code 2 - Name of the block that contains the entities */
    UTF8STRING styleName;      /*!< Code 3 - Dimension styleName name */

    DRW_Coord pt0;             /*!< Code 10/20/30 coordinate - definition point for the dimension line (WCS) */
    DRW_Coord pt1;             /*!< Code 11/21/31 coordinate - mid point for the dimension dimText (OCS) */
    DRW_Coord pt2;             /*!< Code 12/22/32 coordinate - usage depends on dimension dimType (OCS) */
    DRW_Coord pt3;             /*!< Code 13/23/33 coordinate - usage depends on dimension dimType (WCS) */
    DRW_Coord pt4;             /*!< Code 14/24/34 coordinate - usage depends on dimension dimType (WCS) */
    DRW_Coord pt5;             /*!< Code 15/25/35 coordinate - usage depends on dimension dimType (WCS) */
    DRW_Coord pt6;             /*!< Code 16/26/36 coordinate - usage depends on dimension dimType (OCS) */
    DRW_Coord pt7;             /*!< Code 17/27/37 coordinate - usage depends on dimension dimType (OCS) */

    double textLinefactor;     /*!< Code 41 - Dimension dimText line spacing factor, default 1 (value range 0.25 to 4.00 */
    double dimActvalue;        /*!< Code 42 - Actual measurement (optional, read-only) */
    double dimHdir;            /*!< Code 51 - Horizontal direction for the dimension entity, radians */
    double textRotation;       /*!< Code 53 - Rotation angle of the dimension dimText, radians */

    int dimType;               /*!< Code 70 - Dimension dimType */
    int textAttach;            /*!< Code 71 - Dimension dimText attachment specifier, left/centre/right & top/middle/bottom */
    int textLinestyle;         /*!< Code 72 - Dimension dimText line spacing styleName, default 1 */
    DRW_Coord extrusionVec;    /*!< Code 210/220/230 - Extrusion normal vector, default (0,0,1) */
    int dimVersion;            /*!< Code 280 - Dimension version number */

    /* Internal data of the class */
protected:
    bool haveActvalue;         /*!< Set true when actual measurement value is present */
    dwgHandle dimStyleH;
    dwgHandle blockH;
};


//! Class to handle aligned dimension entity
/*!
*  Class to handle aligned dimension entity
*  @author Rallaz
*/
class DRW_DimAligned : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimAligned(){
        eType = DRW::DIMALIGNED;
        oblique = 0.0;
    }
    DRW_DimAligned(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMALIGNED;
    }

    DRW_Coord getClonepoint() const {return getPt2();}      /*!< Insertion point for clones (Baseline & Continue) = pt2 */
    void setClonePoint(DRW_Coord c){setPt2(c);}
    void setClonePoint(double x, double y, double z) {setPt2(x,y,z);}

    DRW_Coord getDefPoint1() const {return getPt3();}       /*!< Definition point 1 (point to measure from) = pt3 */
    void setDefPoint1(const DRW_Coord p) {setPt3(p);}
    void setDefPoint1(double x, double y, double z) {setPt3(x,y,z);}

    DRW_Coord getDefPoint2() const {return getPt4();}       /*!< Definition point 2 (point to measure to) = pt4 */
    void setDefPoint2(const DRW_Coord p) {setPt4(p);}
    void setDefPoint2(double x, double y, double z) {setPt4(x,y,z);}

    double getOblique() const {return oblique;}             /*!< Oblique angle for extension lines = code 50, degrees? */
    void setOblique(const double d) {oblique = d;}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

private:
    double oblique;            /*!< Code 52 - Oblique angle for extension lines, degrees? */
};


//! Class to handle linear or rotated dimension entity
/*!
*  Class to handle linear or rotated dimension entity
*  (superclass of aligned dimension entity)
*  @author Rallaz
*/
class DRW_DimLinear : public DRW_DimAligned {
    SETENTFRIENDS

public:
    DRW_DimLinear() {
        eType = DRW::DIMLINEAR;
        angle = 0.0;
    }
    DRW_DimLinear(const DRW_Dimension& d): DRW_DimAligned(d) {
        eType = DRW::DIMLINEAR;
    }

    double getAngle() const {return angle;}          /*!< Angle of rotated, horizontal, or vertical dimensions (DXF code 50), degrees? */
    void setAngle(const double d) {angle = d;}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

private:
    double angle;              /*!< Code 50 - Angle of rotated, horizontal, or vertical dimensions, degrees? */
};


//! Class to handle radial dimension entity
/*!
*  Class to handle radial dimension entity
*  @author Rallaz
*/
class DRW_DimRadial : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimRadial() {
        eType = DRW::DIMRADIAL;
        length = 0;
    }
    DRW_DimRadial(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMRADIAL;
    }

    DRW_Coord getDefPoint1() const {return getPt0();}      /*!< Definition point 1 (point to measure from), centre of circle/arc = pt0 */
    void setDefPoint1(const DRW_Coord p) {setPt0(p);}
    void setDefPoint1(double x, double y, double z) {setPt0(x,y,z);}

    DRW_Coord getDefPoint2() const {return getPt5();}      /*!< Definition point 2 (point to measure to), on circle/arc = pt5 */
    void setDefPoint2(const DRW_Coord p){setPt5(p);}
    void setDefPoint2(double x, double y, double z) {setPt5(x,y,z);}

    double getLeaderLength() const {return length;}        /*!< Leader line length */
    void setLeaderLength(const double d) {length = d;}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

private:
    double length;             /*!< Leader length, code 40 */
};


//! Class to handle diameter dimension entity
/*!
*  Class to handle diameter dimension entity
*  @author Rallaz
*/
class DRW_DimDiametric : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimDiametric() {
        eType = DRW::DIMDIAMETRIC;
        length = 0;
    }
    DRW_DimDiametric(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMDIAMETRIC;
    }

    DRW_Coord getDefPoint1() const {return getPt0();}      /*!< Definition point 1 (point to measure from), on circle = pt0 */
    void setDefPoint1(const DRW_Coord p) {setPt0(p);}
    void setDefPoint1(double x, double y, double z) {setPt0(x,y,z);}

    DRW_Coord getDefPoint2() const {return getPt5();}      /*!< Definition point 2 (point to measure to), opposite side of circle = pt5 */
    void setDefPoint2(const DRW_Coord p){setPt5(p);}
    void setDefPoint2(double x, double y, double z) {setPt5(x,y,z);}

    double getLeaderLength() const {return length;}        /*!< Leader line length */
    void setLeaderLength(const double d) {length = d;}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

private:
    double length;             /*!< Leader length, code 40 */
};


//! Class to handle angular dimension entity
/*!
*  Class to handle angular dimension entity
*  @author Rallaz
*/
class DRW_DimAngular : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimAngular() {
        eType = DRW::DIMANGULAR;
    }
    DRW_DimAngular(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMANGULAR;
    }

    DRW_Coord getDimLinePoint() const {return getPt6();}      /*!< Definition point for dimension line = pt6 (for angular dimension only!) */
    void setDimLinePoint(const DRW_Coord p) {setPt6(p);}
    void setDimLinePoint(double x, double y, double z) {setPt6(x,y,z);}

    DRW_Coord getFirstLine1() const {return getPt3();}        /*!< First definition line, point 1 (closer to angle vertex) = pt3 */
    void setFirstLine1(const DRW_Coord p) {setPt3(p);}
    void setFirstLine1(double x, double y, double z) {setPt3(x,y,z);}

    DRW_Coord getFirstLine2() const {return getPt4();}        /*!< First definition line, point 2 (further from angle vertex) = pt4 */
    void setFirstLine2(const DRW_Coord p) {setPt4(p);}
    void setFirstLine2(double x, double y, double z) {setPt4(x,y,z);}

    DRW_Coord getSecondLine1() const {return getPt5();}       /*!< Second definition line, point 1 (closer to angle vertex) = pt5 */
    void setSecondLine1(const DRW_Coord p) {setPt5(p);}
    void setSecondLine1(double x, double y, double z) {setPt5(x,y,z);}

    DRW_Coord getSecondLine2() const {return getPt0();}       /*!< Second definition line, point 2 (further from angle vertex) = pt0 (for angular dimension only!) */
    void setSecondLine2(const DRW_Coord p) {setPt0(p);}
    void setSecondLine2(double x, double y, double z) {setPt0(x,y,z);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;
};


//! Class to handle angular 3 point dimension entity
/*!
*  Class to handle angular 3 point dimension entity
*  @author Rallaz
*/
class DRW_DimAngular3p : public DRW_Dimension {
    SETENTFRIENDS
public:
    DRW_DimAngular3p() {
        eType = DRW::DIMANGULAR3P;
    }
    DRW_DimAngular3p(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMANGULAR3P;
    }

    DRW_Coord getFirstLine() const {return getPt3();}        /*!< First definition line point = pt3 */
    void setFirstLine(const DRW_Coord p) {setPt3(p);}
    void setFirstLine(double x, double y, double z) {setPt3(x,y,z);}

    DRW_Coord getSecondLine() const {return getPt4();}       /*!< Second definition line point = pt4 */
    void setSecondLine(const DRW_Coord p) {setPt4(p);}
    void setSecondLine(double x, double y, double z) {setPt4(x,y,z);}

    DRW_Coord getVertexPoint() const {return getPt5();}      /*!< Angle vertex point = pt5 */
    void SetVertexPoint(const DRW_Coord p) {setPt5(p);}
    void SetVertexPoint(double x, double y, double z) {setPt5(x,y,z);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;
};


//! Class to handle ordinate dimension entity
/*!
*  Class to handle ordinate dimension entity
*  @author Rallaz
*/
class DRW_DimOrdinate : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimOrdinate() {
        eType = DRW::DIMORDINATE;
    }
    DRW_DimOrdinate(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMORDINATE;
    }

    DRW_Coord getOriginPoint() const {return getPt0();}        /*!< Origin definition point (ie. point to measure ordinate values from) = pt0 */
    void setOriginPoint(const DRW_Coord p) {setPt0(p);}
    void setOriginPoint(double x, double y, double z) {setPt0(x,y,z);}

    DRW_Coord getDefPoint() const {return getPt3();}           /*!< Definition point (ie. point to give the ordinates of) = pt3 */
    void setDefPoint(const DRW_Coord p) {setPt3(p);}
    void setDefPoint(double x, double y, double z) {setPt3(x,y,z);}

    DRW_Coord getLeaderPoint() const {return getPt4();}        /*!< Leader line point = pt4 */
    void setLeaderPoint(const DRW_Coord p) {setPt4(p);}
    void setLeaderPoint(double x, double y, double z) {setPt4(x,y,z);}

protected:
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;
};


//! Class to handle arc dimension entity
/*!
*  Class to handle arc dimension entity
*  @author Pwinters
*/
class DRW_DimArc : public DRW_Dimension {
    SETENTFRIENDS

public:
    DRW_DimArc() {
        eType = DRW::DIMARC;
    }
    DRW_DimArc(const DRW_Dimension& d): DRW_Dimension(d) {
        eType = DRW::DIMARC;
    }

    DRW_Coord getDefPoint1() const {return getPt3();}       /*!< Definition point 1 (arc point to measure from) = pt3 */
    void setDefPoint1(const DRW_Coord p) {setPt3(p);}
    void setDefPoint1(double x, double y, double z) {setPt3(x,y,z);}

    DRW_Coord getDefPoint2() const {return getPt4();}       /*!< Definition point 2 (arc point to measure to) = pt4 */
    void setDefPoint2(const DRW_Coord p) {setPt4(p);}
    void setDefPoint2(double x, double y, double z) {setPt4(x,y,z);}

    DRW_Coord getVertexPoint() const {return getPt5();}      /*!< Arc vertex point = pt5 */
    void setVertexPoint(const DRW_Coord p) {setPt5(p);}
    void setVertexPoint(double x, double y, double z) {setPt5(x,y,z);}

    DRW_Coord getLeaderStart() const {return getPt6();}      /*!< Leader line start point = pt6 */
    void setLeaderStart(const DRW_Coord p) {setPt6(p);}
    void setLeaderStart(double x, double y, double z) {setPt6(x,y,z);}

    DRW_Coord getLeaderEnd() const {return getPt7();}        /*!< Leader line end point = pt7 */
    void setLeaderEnd(const DRW_Coord p) {setPt7(p);}
    void setLeaderEnd(double x, double y, double z) {setPt7(x,y,z);}

    double getStartAngle() const {return staangle;}          /*!< Start angle (radians) */
    void setStartAngle(const double d) {staangle = d;}

    double getEndAngle() const {return endangle;}            /*!< End angle (radians) */
    void setEndAngle(const double d) {endangle = d;}

    bool getPartial() const {return partial;}                /*!< Draw as partial arc dimension */
    void setPartial(const bool b) {partial = b;}

    bool getLeader() const {return leader;}                  /*!< Draw with leader line from text to arc */
    void setLeader(const bool b) {leader = b;}

    friend std::ostream& operator << (std::ostream&, const DRW_DimArc& dimArc);

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

private:
	/* Values for these within "AcDbArcDimension" sub-class in the DXF */
	double staangle;                  /*!< Start point angle ref centre point, radians, code 40 */
	double endangle;                  /*!< End point angle ref centre point, radians, code 41 */
	bool partial = false;             /*!< Draw as partial arc dimension, code 70 */
	bool leader = false;              /*!< Draw with leader line from text to arc, code 71 */
};


//! Class to handle leader entity
/*!
*  Class to handle leader entity
*  @author Rallaz
*/
class DRW_Leader : public DRW_Entity {
    SETENTFRIENDS
public:
    DRW_Leader() {
        eType = DRW::LEADER;
        flag = 3;
        hookflag = vertnum = leadertype = 0;
        extrusionPoint.x = extrusionPoint.y = 0.0;
        arrow = 1;
        extrusionPoint.z = 1.0;
    }

    virtual void applyExtrusion() override {}

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    UTF8STRING style;          /*!< Dimension style name, code 3 */
    int arrow;                 /*!< Arrowhead flag, code 71, 0=Disabled; 1=Enabled */
    int leadertype;            /*!< Leader path type, code 72, 0=Straight line segments; 1=Spline */
    int flag;                  /*!< Leader creation flag, code 73, default 3 */
    int hookline;              /*!< Hook line direction flag, code 74, default 1 */
    int hookflag;              /*!< Hook line flag, code 75 */
    double textheight;         /*!< Text annotation height, code 40 */
    double textwidth;          /*!< Text annotation width, code 41 */
    int vertnum;               /*!< Number of vertices, code 76 */
    int coloruse;              /*!< Color to use if leader's DIMCLRD = BYBLOCK, code 77 */
    duint32 annotHandle;       /*!< Hard reference to associated annotation, code 340 */
    DRW_Coord extrusionPoint;  /*!< Normal vector, code 210, 220 & 230 */
    DRW_Coord horizdir;        /*!< "Horizontal" direction for leader, code 211, 221 & 231 */
    DRW_Coord offsetblock;     /*!< Offset of last leader vertex from block, code 212, 222 & 232 */
    DRW_Coord offsettext;      /*!< Offset of last leader vertex from annotation, code 213, 223 & 233 */

    std::vector<std::shared_ptr<DRW_Coord>> vertexlist;  /*!< vertex points list, code 10, 20 & 30 */

private:
    std::shared_ptr<DRW_Coord> vertexpoint;   /*!< current control point to add data */
    dwgHandle dimStyleH;
    dwgHandle AnnotH;
};

//! Class to handle viewport entity
/*!
*  Class to handle viewport entity
*  @author Rallaz
*/
class DRW_Viewport : public DRW_Point {
    SETENTFRIENDS
public:
    DRW_Viewport() {
        eType = DRW::VIEWPORT;
        vpstatus = 0;
        pswidth = 205;
        psheight = 156;
        centerPX = 128.5;
        centerPY = 97.5;
    }

protected:
    bool parseCode(int code, dxfReader *reader) override;
    virtual bool parseDwg(DRW::Version version, dwgBuffer *buf, duint32 bs=0) override;

public:
    double pswidth;           /*!< Width in paper space units, code 40 */
    double psheight;          /*!< Height in paper space units, code 41 */
    int vpstatus;             /*!< Viewport status, code 68 */
    int vpID;                 /*!< Viewport ID, code 69 */
    double centerPX;          /*!< view center point X, code 12 */
    double centerPY;          /*!< view center point Y, code 22 */
    double snapPX;          /*!< Snap base point X, code 13 */
    double snapPY;          /*!< Snap base point Y, code 23 */
    double snapSpPX;          /*!< Snap spacing X, code 14 */
    double snapSpPY;          /*!< Snap spacing Y, code 24 */
    //TODO: complete in dxf
    DRW_Coord viewDir;        /*!< View direction vector, code 16, 26 & 36 */
    DRW_Coord viewTarget;     /*!< View target point, code 17, 27, 37 */
    double viewLength;        /*!< Perspective lens length, code 42 */
    double frontClip;         /*!< Front clip plane Z value, code 43 */
    double backClip;          /*!< Back clip plane Z value, code 44 */
    double viewHeight;        /*!< View height in model space units, code 45 */
    double snapAngle;         /*!< Snap angle, code 50 */
    double twistAngle;        /*!< view twist angle, code 51 */

private:
    duint32 frozenLyCount;
};//RLZ: missing 15,25, 72, 331, 90, 340, 1, 281, 71, 74, 110, 120, 130, 111, 121,131, 112,122, 132, 345,346, and more...

//used  //DRW_Coord basePoint;      /*!<  base point, code 10, 20 & 30 */

//double thickness;         /*!< thickness, code 39 */
//DRW_Coord extPoint;       /*!<  Dir extrusion normal vector, code 210, 220 & 230 */
//enum DRW::ETYPE eType;     /*!< enum: entity type, code 0 */
//duint32 handle;            /*!< entity identifier, code 5 */
//std::list<std::list<DRW_Variant> > appData; /*!< list of application data, code 102 */
//duint32 parentHandle;      /*!< Soft-pointer ID/handle to owner BLOCK_RECORD object, code 330 */
//DRW::Space space;          /*!< space indicator, code 67*/
//UTF8STRING layer;          /*!< layer name, code 8 */
//UTF8STRING lineType;       /*!< line type, code 6 */
//duint32 material;          /*!< hard pointer id to material object, code 347 */
//int color;                 /*!< entity color, code 62 */
//enum DRW_LW_Conv::lineWidth lWeight; /*!< entity lineweight, code 370 */
//double ltypeScale;         /*!< linetype scale, code 48 */
//bool visible;              /*!< entity visibility, code 60 */
//int numProxyGraph;         /*!< Number of bytes in proxy graphics, code 92 */
//std::string proxyGraphics; /*!< proxy graphics bytes, code 310 */
//int color24;               /*!< 24-bit color, code 420 */
//std::string colorName;     /*!< color name, code 430 */
//int transparency;          /*!< transparency, code 440 */
//int plotStyle;             /*!< hard pointer id to plot style object, code 390 */
//DRW::ShadowMode shadow;    /*!< shadow mode, code 284 */
//bool haveExtrusion;        /*!< set to true if the entity have extrusion*/

#endif

// EOF

