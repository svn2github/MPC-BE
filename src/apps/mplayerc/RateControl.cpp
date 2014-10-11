/*
 * (C) 2014 see Authors.txt
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
#include "RateControl.h"

const static double s_autorates[] = { 0.125, 0.25, 0.5, 1.0, 1.2, 1.5, 2.0, 4.0, 8.0 };
const static double s_dvdautorates[] = { -16.0, -8.0, -4.0, -2.0, -1.0, 1.0, 2.0, 4.0, 8.0, 16.0 };

double GatNextRate(double rate, double step) // 0 < step <= 1.0
{
	if (step == 0.0) {
		size_t i = 0;
		for (; i < _countof(s_autorates) - 1; i++) {
			if (s_autorates[i] > rate) {
				break;
			}
		}
		rate = s_autorates[i];
	} else {
		rate = 1.0 + (floor((rate - 0.9999) / step) + 1) * step;
		if (rate > MAXRATE) {
			rate = MAXRATE;
		}
	}

	return rate;
}

double GatPreviousRate(double rate, double step) // 0 < step <= 1.0
{
	if (step == 0.0) {
		size_t i = 1;
		for (; i < _countof(s_autorates); i++) {
			if (s_autorates[i] >= rate) {
				break;
			}
		}
		rate = s_autorates[i - 1];
	} else {
		rate = 1.0 + (ceil((rate - 1.0001) / step) - 1) * step;
		if (rate < MINRATE) {
			rate = MINRATE;
		}
	}

	return rate;
}

double GatNextDVDRate(double rate)
{
	size_t i = 0;
	for (; i < _countof(s_dvdautorates) - 1; i++) {
		if (s_dvdautorates[i] > rate) {
			break;
		}
	}

	return s_dvdautorates[i];
}

double GatPreviousDVDRate(double rate)
{
	size_t i = 1;
	for (; i < _countof(s_dvdautorates); i++) {
		if (s_dvdautorates[i] >= rate) {
			break;
		}
	}

	return s_dvdautorates[i - 1];
}

CString Rate2String(double rate)
{
	CString str;
	str.Format(L"%.3g", rate);
	if (str.Find('.') < 0) {
		str += L".0";
	}

	return str;
}
