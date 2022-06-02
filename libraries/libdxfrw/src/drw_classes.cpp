/******************************************************************************
**  libDXFrw - Library to read/write DXF files (ascii & binary)              **
**                                                                           **
**  Copyright (C) 2011-2015 José F. Soriano, rallazz@gmail.com               **
**                                                                           **
**  This library is free software, licensed under the terms of the GNU       **
**  General Public License as published by the Free Software Foundation,     **
**  either version 2 of the License, or (at your option) any later version.  **
**  You should have received a copy of the GNU General Public License        **
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.    **
******************************************************************************/

#include "drw_classes.h"
#include "intern/dxfreader.h"
#include "intern/dxfwriter.h"
#include "intern/dwgbuffer.h"
#include "intern/drw_dbg.h"


bool DRW_Class::parseCode(int code, dxfReader *reader){
    switch (code) {
    case 1:
        /* Class DXF record name; always unique */
        recName = reader->getUtf8String();
        DRW_DBG("dxf rec name: "); DRW_DBG(recName.c_str()); DRW_DBG("\n");
        break;
    case 2:
        /* C++ class name. Used to bind with software that defines object class behavior; always unique */
        className = reader->getUtf8String();
        DRW_DBG("class name: "); DRW_DBG(className.c_str()); DRW_DBG("\n");
        break;
    case 3:
        /* Application name. Posted in Alert box when a class definition listed in this section is not currently loaded */
        appName = reader->getUtf8String();
        DRW_DBG("app name: "); DRW_DBG(appName.c_str()); DRW_DBG("\n");
        break;
    case 90:
        /* Proxy capabilities flag. Bit-coded value that indicates the capabilities of this object as a proxy:
           0 = No operations allowed (0)
           1 = Erase allowed (0x1)
           2 = Transform allowed (0x2)
           4 = Color change allowed (0x4)
           8 = Layer change allowed (0x8)
           16 = Linetype change allowed (0x10)
           32 = Linetype scale change allowed (0x20)
           64 = Visibility change allowed (0x40)
           128 = Cloning allowed (0x80)
           256 = Lineweight change allowed (0x100)
           512 = Plot Style Name change allowed (0x200)
           895 = All operations except cloning allowed (0x37F)
           1023 = All operations allowed (0x3FF)
           1024 = Disables proxy warning dialog (0x400)
           32768 = R13 format proxy (0x8000)
        */
        proxyFlag = reader->getInt32();
        DRW_DBG("Proxy capabilities flag: "); DRW_DBG(proxyFlag); DRW_DBG("\n");
        break;
    case 91:
        /* Instance count for a custom class */
        instanceCount = reader->getInt32();
        DRW_DBG("Instance Count: "); DRW_DBG(instanceCount); DRW_DBG("\n");
        break;
    case 280:
        /* Was-a-proxy flag. Set to 1 if class was not loaded when this DXF file was created, and 0 otherwise */
        wasaProxyFlag = reader->getInt32() & 1;
        DRW_DBG("Proxy flag (280): "); DRW_DBG(wasaProxyFlag); DRW_DBG("\n");
        break;
    case 281:
        /* Is-an-entity flag. Set to 1 if class was derived from the AcDbEntity class and can reside 
           in the BLOCKS or ENTITIES section. If 0, instances may appear only in the OBJECTS section */
        entityFlag = reader->getInt32() & 1;
        DRW_DBG("Entity flag: "); DRW_DBGH(entityFlag); DRW_DBG("\n");
        break;
    default:
        DRW_DBG("Unexpected code "); DRW_DBG(code); DRW_DBG(" in class section\n"); DRW_DBG("\n");
        return false;
    }

    return true;
}

bool DRW_Class::parseDwg(DRW::Version version, dwgBuffer *buf, dwgBuffer *strBuf){
    DRW_DBG("\n***************************** parsing Class *********************************************\n");

    classNum = buf->getBitShort();
    DRW_DBG("Class number: "); DRW_DBG(classNum);
    proxyFlag = buf->getBitShort(); //in dwg specs says "version"

    appName = strBuf->getVariableText(version, false);
    className = strBuf->getVariableText(version, false);
    recName = strBuf->getVariableText(version, false);

    DRW_DBG("\napp name: "); DRW_DBG(appName.c_str());
    DRW_DBG("\nclass name: "); DRW_DBG(className.c_str());
    DRW_DBG("\ndxf rec name: "); DRW_DBG(recName.c_str());
    wasaProxyFlag = buf->getBit(); //in dwg says wasazombie
    entityFlag = buf->getBitShort();
    entityFlag = entityFlag == 0x1F2 ? 1: 0;

    DRW_DBG("\nProxy capabilities flag: "); DRW_DBG(proxyFlag);
    DRW_DBG(", proxy flag (280): "); DRW_DBG(wasaProxyFlag);
    DRW_DBG(", entity flag: "); DRW_DBGH(entityFlag);

    if (version > DRW::AC1015) {//2004+
        instanceCount = buf->getBitLong();
        DRW_DBG("\nInstance Count: "); DRW_DBG(instanceCount);
        duint32 dwgVersion = buf->getBitLong();
        DRW_DBG("\nDWG version: "); DRW_DBG(dwgVersion);
        DRW_DBG("\nmaintenance version: "); DRW_DBG(buf->getBitLong());
        DRW_DBG("\nunknown 1: "); DRW_DBG(buf->getBitLong());
        DRW_DBG("\nunknown 2: "); DRW_DBG(buf->getBitLong());
    }
    DRW_DBG("\n");
    toDwgType();
    return buf->isGood();
}

void DRW_Class::write(dxfWriter *writer, DRW::Version ver){
    if (ver > DRW::AC1009) {
        writer->writeString(0, "CLASS");
        writer->writeString(1, recName);
        writer->writeString(2, className);
        writer->writeString(3, appName);
        writer->writeInt32(90, proxyFlag);
        if (ver > DRW::AC1015) { //2004+
            writer->writeInt32(91, instanceCount);
        }
        writer->writeInt16(280, wasaProxyFlag);
        writer->writeInt16(281, entityFlag);
    }
}

void DRW_Class::toDwgType(){
    if (recName == "LWPOLYLINE")
        dwgType = 77;
    else if (recName == "HATCH")
        dwgType = 78;
    else if (recName == "GROUP")
        dwgType = 72;
    else if (recName == "LAYOUT")
        dwgType = 82;
    else if (recName == "IMAGE")
        dwgType = 101;
    else if (recName == "IMAGEDEF")
        dwgType = 102;
    else if (recName == "ARC_DIMENSION")
        dwgType = 103;
    else
        dwgType =0;
}
