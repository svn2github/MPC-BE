/*
 * $Id$
 *
 * (C) 2009-2012 see Authors.txt
 *
 * This file is part of MPC-HC.
 *
 * MPC-HC is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-HC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#pragma once

#pragma warning(disable:4244)
#ifdef _WIN64
#pragma warning(disable:4267)
#endif
#pragma warning(disable:4995)
#ifdef _DEBUG
// Remove this if you want to see all the "unsafe" functions used
// For Release builds _CRT_SECURE_NO_WARNINGS is defined
#pragma warning(disable:4996)
#endif

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC // include Microsoft memory leak detection procedures

#if 0
#include <crtdbg.h>
#define DNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DNew new(__FILE__, __LINE__)
#endif

#else

#define DNew new

#endif
