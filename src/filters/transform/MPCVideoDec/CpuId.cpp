/*
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
#include <intrin.h>
#include "CpuId.h"

#define CPUID_MMX      (1 << 23)
#define CPUID_SSE      (1 << 25)
#define CPUID_SSE2     (1 << 26)
#define CPUID_SSE3     (1 << 0)
#define CPUID_SSE41    (1 << 19)
#define CPUID_SSE42    (1 << 20)
#define CPUID_AVX      ((1 << 27) | (1 << 28))

// Intel specifics
#define CPUID_SSSE3    (1 << 9)

// AMD specifics
#define CPUID_3DNOW    (1 << 31)
#define CPUID_3DNOWEXT (1 << 30)
#define CPUID_MMXEXT   (1 << 22)

CCpuId::CCpuId(void)
{
	unsigned	nHighestFeature;
	unsigned	nHighestFeatureEx;
	int			nBuff[4];
	char		szMan[13];
	//char		szFeatures[256];

	// Get CPU manufacturer and highest CPUID
	__cpuid(nBuff, 0);
	nHighestFeature = (unsigned)nBuff[0];
	*(int*)&szMan[0] = nBuff[1];
	*(int*)&szMan[4] = nBuff[3];
	*(int*)&szMan[8] = nBuff[2];
	szMan[12] = 0;
	if (strcmp(szMan, "AuthenticAMD") == 0)
		m_nType = PROCESSOR_AMD;
	else if (strcmp(szMan, "GenuineIntel") == 0)
		m_nType = PROCESSOR_INTEL;
	else
		m_nType = PROCESSOR_UNKNOWN;

	// Get highest extended feature
	__cpuid(nBuff, 0x80000000);
	nHighestFeatureEx = (unsigned)nBuff[0];

	// Get processor brand name
	/*
	if (nHighestFeatureEx >= 0x80000004)
	{
		char szCPUName[49];
		szCPUName[0] = 0;
		__cpuid((int*)&szCPUName[0],  0x80000002);
		__cpuid((int*)&szCPUName[16], 0x80000003);
		__cpuid((int*)&szCPUName[32], 0x80000004);
		szCPUName[48] = 0;
		for (int i = (int)strlen(szCPUName) - 1; i >= 0; --i)
		{
			if (szCPUName[i] == ' ')
				szCPUName[i] = '\0';
			else
				break;
		}

		ELog::Get().SystemFormat(L"PERF    : CPU: %S (%S)\n", szCPUName, szMan);
	}
	else
		ELog::Get().SystemFormat(L"PERF    : CPU: %S\n", szMan);
	*/

	// Get CPU features
	m_nCPUFeatures = 0;
	//szFeatures[0]  = 0;
	if (nHighestFeature >= 1)
	{
		__cpuid(nBuff, 1);
		if (nBuff[3] & CPUID_MMX)  m_nCPUFeatures |= MPC_MM_MMX;
		if (nBuff[3] & CPUID_SSE)  m_nCPUFeatures |= MPC_MM_SSE;
		if (nBuff[3] & CPUID_SSE2) m_nCPUFeatures |= MPC_MM_SSE2;
		if (nBuff[2] & CPUID_SSE3) m_nCPUFeatures |= MPC_MM_SSE3;
		if (nBuff[2] & CPUID_SSE41) m_nCPUFeatures |= MPC_MM_SSE4;
		if (nBuff[2] & CPUID_SSE42) m_nCPUFeatures |= MPC_MM_SSE42;

		// Intel specific:
		if (m_nType == PROCESSOR_INTEL)
		{
			if (nBuff[2] & CPUID_SSSE3) m_nCPUFeatures |= MPC_MM_SSSE3;
			//if (nBuff[2] & CPUID_EST) strcat(szFeatures, "EST ");
			if ((nBuff[2] & CPUID_AVX) == CPUID_AVX) {
				// Check for OS support
				unsigned long long xcrFeatureMask = _xgetbv(_XCR_XFEATURE_ENABLED_MASK);
				if ((xcrFeatureMask & 0x6) == 0x6) {
					m_nCPUFeatures |= MPC_MM_AVX;
				}
			}
		}

		//if(nBuff[3] & CPUID_HTT)
		//	strcat(szFeatures, "HTT ");
	}

	// AMD specific:
	if (m_nType == PROCESSOR_AMD)
	{
		// Get extended features
		__cpuid(nBuff, 0x80000000);
		if (nHighestFeatureEx >= 0x80000001)
		{
			__cpuid(nBuff, 0x80000001);
			if (nBuff[3] & CPUID_3DNOW)    m_nCPUFeatures |= MPC_MM_3DNOW;
			//if (nBuff[3] & CPUID_3DNOWEXT) strcat(szFeatures, "Ex3DNow! ");
			if (nBuff[3] & CPUID_MMXEXT)   m_nCPUFeatures |= MPC_MM_MMXEXT;
		}

		// Get level 1 cache size
		//if (nHighestFeatureEx >= 0x80000005)
		//{
		//	__cpuid(nBuff, 0x80000005);
		//	ELog::Get().SystemFormat(L"PERF    : L1 cache size: %dK\n", ((unsigned)nBuff[2])>>24);
		//}
	}

	/*
	// Get cache size
	if (nHighestFeatureEx >= 0x80000006)
	{
		__cpuid(nBuff, 0x80000006);
		ELog::Get().SystemFormat(L"PERF    : L2 cache size: %dK\n", ((unsigned)nBuff[2])>>16);
	}

	// Log features
	ELog::Get().SystemFormat(L"PERF    : CPU Features: %S\n", szFeatures);

	// Get misc system info
	SYSTEM_INFO theInfo;
	GetSystemInfo(&theInfo);

	// Log number of CPUs and speeds
	ELog::Get().SystemFormat(L"PERF    : Number of CPUs: %d\n", theInfo.dwNumberOfProcessors);
	for (DWORD i = 0; i < theInfo.dwNumberOfProcessors; ++i)
	{
		DWORD dwCPUSpeed = ReadCPUSpeedFromRegistry(i);
		ELog::Get().SystemFormat(L"PERF    : * CPU %d speed: ~%dMHz\n", i, dwCPUSpeed);
	}
	*/

	/*
	unsigned nHighestFeature;
	unsigned nHighestFeatureEx;

	m_CPUInfo[0]    = -1;
	m_CPUInfoEx[0]  = -1;
	m_CPUAMDInfo[0] = -1;

	__cpuid(m_CPUInfo, 0);
	nHighestFeature = (unsigned)m_CPUInfo[0];

	__cpuid(nBuff, 0x80000000);
	nHighestFeatureEx = (unsigned)nBuff[0];

	m_nDSPFlags = 0;
	if (m_CPUInfo[3] & CPUID_MMX)  m_nDSPFlags |= MPC_MM_MMX;
	if (m_CPUInfo[3] & CPUID_SSE)  m_nDSPFlags |= MPC_MM_SSE;
	if (m_CPUInfo[3] & CPUID_SSE2) m_nDSPFlags |= MPC_MM_SSE2;
	if (m_CPUInfo[2] & CPUID_SSE3) m_nDSPFlags |= MPC_MM_SSE3;

	switch (m_nType)
	{
	case PROCESSOR_AMD :
		__cpuid(m_CPUAMDInfo, 0x80000000);
		if (nHighestFeatureEx >= 0x80000001)
		{
			if (m_CPUAMDInfo[3] & CPUID_MMXEXT) m_nDSPFlags |= MPC_MM_MMXEXT;
			if (m_CPUAMDInfo[3] & CPUID_3DNOW)  m_nDSPFlags |= MPC_MM_3DNOW;
		}
		break;
	case PROCESSOR_INTEL :
		if (m_CPUInfo[2] & CPUID_SSSE3) m_nDSPFlags |= MPC_MM_SSSE3;
		break;
	}
	*/
}

int CCpuId::GetProcessorNumber()
{
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	return SystemInfo.dwNumberOfProcessors;
}
