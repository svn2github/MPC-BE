/*
 * (C) 2006-2013 see Authors.txt
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

#pragma once

class CCpuId
{
public:

	typedef enum {
		PROCESSOR_AMD,
		PROCESSOR_INTEL,
		PROCESSOR_UNKNOWN
	} PROCESSOR_TYPE;

	// Enum codes identicals to FFMpeg cpu features define !
	typedef enum {
		MPC_MM_MMX      = 0x0001, /* standard MMX */
		MPC_MM_3DNOW    = 0x0004, /* AMD 3DNOW */
		MPC_MM_MMXEXT   = 0x0002, /* SSE integer functions or AMD MMX ext */
		MPC_MM_SSE      = 0x0008, /* SSE functions */
		MPC_MM_SSE2     = 0x0010, /* PIV SSE2 functions */
		MPC_MM_3DNOWEXT = 0x0020, /* AMD 3DNowExt */
		MPC_MM_SSE3     = 0x0040, /* AMD64 & PIV SSE3 functions*/
		MPC_MM_SSSE3    = 0x0080, /* PIV Core 2 SSSE3 functions*/
		MPC_MM_SSE4     = 0x0100,
		MPC_MM_SSE42    = 0x0200,
		MPC_MM_AVX      = 0x4000
	} PROCESSOR_FEATURES;

	CCpuId();

	int GetFeatures()   const {return m_nCPUFeatures;};
	PROCESSOR_TYPE      GetType() const {return m_nType;};
	int                 GetProcessorNumber();

private:
	int             m_nCPUFeatures;
	PROCESSOR_TYPE  m_nType;
};
