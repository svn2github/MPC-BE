/*
 * (C) 2003-2006 Gabest
 * (C) 2006-2014 see Authors.txt
 *
 * This file is part of MPC-BE.
 *
 * MPC-BE is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * MPC-BE is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "stdafx.h"

extern "C" {
	// Hack to use MinGW64 from 2.x branch
	void __mingw_raise_matherr(int typ, const char *name, double a1, double a2, double rslt) {}
	void __mingw_get_msvcrt_handle() {}
}

#ifdef REGISTER_FILTER
	void *__imp_time64				= _time64;
	void *__imp__aligned_malloc		= _aligned_malloc;
	void *__imp__aligned_realloc	= _aligned_realloc;
	void *__imp__aligned_free		= _aligned_free;
#endif
