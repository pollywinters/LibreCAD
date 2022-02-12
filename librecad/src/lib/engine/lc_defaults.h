/****************************************************************************
**
** This file is part of the LibreCAD project, a 2D CAD program
**
** Copyright (C) 2022 P. Winters (polly.winters1@gmail.com)
**
**
** This file is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
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


#ifndef LC_DEFAULTS_H
#define LC_DEFAULTS_H

#include <string>

#include "dxf_format.h"


/**
 * Classes containing default values for system settings etc within the
 * LibreCAD program. 
 * Eg. Application preferences default values, current drawing preferences
 * default values, 
 *
 * @author Polly Winters
 */

class LC_Defaults {
public:
//	LC_Defaults();
//	~LC_Defaults();

	/*	Point display mode and size  */
	static inline constexpr int def_PDMode = DXF_Format::PDMode_CentreDot;
	static inline constexpr int def_PDSize = 0.0;

} ;


#endif

