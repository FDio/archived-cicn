/******************************************************************************
    QtAV Player Demo:  this file is part of QtAV examples
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>
*   This file is part of QtAV (from 2014)
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#ifndef COMMON_EXPORT_H
#define COMMON_EXPORT_H

#include <QtCore/qglobal.h>

#ifdef BUILD_COMMON_STATIC
#define COMMON_EXPORT
#else
#if defined(BUILD_COMMON_LIB)
#  undef COMMON_EXPORT
#  define COMMON_EXPORT Q_DECL_EXPORT
#else
#  undef COMMON_EXPORT
#  define COMMON_EXPORT //Q_DECL_IMPORT //only for vc? link to static lib error
#endif
#endif //BUILD_COMMON_STATIC
#endif // COMMON_EXPORT_H
