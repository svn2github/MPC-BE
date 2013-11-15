/*
 * (C) 2003-2006 Gabest
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

#ifndef _ARRAY_ALLOCATOR_H_
#define _ARRAY_ALLOCATOR_H_

template <class T,size_t size> class array_allocator
{
private:
	T* p;
public:
	typedef T value_type;
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef T* pointer;
	typedef const T* const_pointer;

	typedef T& reference;
	typedef const T& const_reference;

	pointer address(reference r) const {
		return &r;
	}
	const_pointer address(const_reference r) const {
		return &r;
	}

	array_allocator() throw() {}
	template <class U,size_t sz> array_allocator(const array_allocator<U,sz>& ) throw() {}
	~array_allocator() throw() {}

	pointer allocate(size_type n, const void* = 0) {
		p = ((T  *)::operator new(size * sizeof (T)));
		return p;
	}
	void deallocate(pointer p, size_type) {
		delete p;
	}

	//Use placement new to engage the constructor
	void construct(pointer p, const T& val) {
		new((void*)p) T(val);
	}
	void destroy(pointer p) {
		((T*)p)->~T();
	}

	size_type max_size() const throw() {
		return size;
	}
	template<class U> struct rebind {
		typedef array_allocator<U,size> other;
	};
};

template<class T,size_t size> struct array_vector : std::vector<T, array_allocator<T,size> > {
};

#if defined(__INTEL_COMPILER) || defined(__GNUC__) || (_MSC_VER>=1300)
template<class T,size_t a> struct allocator_traits<array_allocator<T,a> > {
	enum {is_static=true};
};
#endif

#endif

#ifndef _CHAR_T_H_
#define _CHAR_T_H_

#include <stdio.h>
#include <time.h>
#include <wchar.h>

#pragma warning(push)
#pragma warning(disable:4995 4996)

#undef _l
#ifdef UNICODE
typedef wchar_t char_t;
#define tsprintf swprintf
#define tsnprintf_s _snwprintf_s
#define tfprintf fwprintf
#define tsscanf swscanf
#define __l(x) L ## x
#define _l(x) __l(x)
#else
typedef char char_t;
#define tsprintf sprintf
#define tsnprintf_s _snprintf_s
#define tfprintf fprintf
#define tsscanf sscanf
#define _l(x) x
#endif

#ifdef __cplusplus

#ifdef __GNUC__
#ifndef __forceinline
#define __forceinline __attribute__((__always_inline__)) inline
#endif
#endif

static __forceinline errno_t strncat_s(wchar_t *a, size_t b, const wchar_t *c, size_t d)
{
	return wcsncat_s(a,b,c,d);
}
static __forceinline int vsnprintf_s(wchar_t *a, size_t b, size_t c, const wchar_t *d, va_list e)
{
	return _vsnwprintf_s(a,b,c,d,e);
}
static __forceinline errno_t _splitpath_s(const wchar_t * a,
		wchar_t * b, size_t c, wchar_t * d, size_t e,
		wchar_t * f, size_t g, wchar_t * h, size_t i)
{
	return _wsplitpath_s(a,b,c,d,e,f,g,h,i);
};
static __forceinline errno_t _makepath_s(wchar_t *a, size_t b,
		const wchar_t *c, const wchar_t *d,
		const wchar_t *e, const wchar_t *f)
{
	return _wmakepath_s(a,b,c,d,e,f);
};
static __forceinline wchar_t* ff_strncpy(wchar_t *dst, const wchar_t *src, size_t count)
{
	wcsncpy_s(dst, count, src, _TRUNCATE);
	return dst;
}
static __forceinline char* ff_strncpy(char *dst, const char *src, size_t count)
{
	strncpy_s(dst, count, src, _TRUNCATE);
	return dst;
}
static __forceinline wchar_t* strcat(wchar_t *a, const wchar_t *b)
{
	return wcscat(a,b);
}
static __forceinline int strcmp(const wchar_t *a, const wchar_t *b)
{
	return wcscmp(a,b);
}
static __forceinline int strncmp(const wchar_t *a, const wchar_t *b, size_t c)
{
	return wcsncmp(a,b,c);
}
static __forceinline int strnicmp(const wchar_t *a, const wchar_t *b,size_t c)
{
	return _wcsnicmp(a,b,c);
}
static __forceinline long strtol(const wchar_t *a, wchar_t **b, int c)
{
	return wcstol(a,b,c);
}
static __forceinline wchar_t* strchr(const wchar_t *a, wchar_t b)
{
	return (wchar_t*)wcschr(a,b);
}
static __forceinline int _strnicmp(const wchar_t *a, const wchar_t *b, size_t c)
{
	return _wcsnicmp(a,b,c);
}
static __forceinline wchar_t* strstr(const wchar_t *a, const wchar_t *b)
{
	return (wchar_t*)wcsstr(a,b);
}
static __forceinline size_t strlen(const wchar_t *a)
{
	return wcslen(a);
}
static __forceinline wchar_t* strcpy(wchar_t *a, const wchar_t *b)
{
	return wcscpy(a,b);
}
static __forceinline int atoi(const wchar_t *a)
{
	return _wtoi(a);
}
static __forceinline wchar_t* _itoa(int a, wchar_t *b, int c)
{
	return _itow(a,b,c);
}
static __forceinline int stricmp(const wchar_t *a, const wchar_t *b)
{
	return  _wcsicmp(a,b);
}
static __forceinline unsigned long strtoul(const wchar_t *a,wchar_t **b,int c)
{
	return wcstoul(a,b,c);
}
static __forceinline double strtod(const wchar_t *a, wchar_t **b)
{
	return wcstod(a,b);
}
static __forceinline int vsprintf(wchar_t *a, const wchar_t *b, va_list c)
{
	return vswprintf(a,b,c);
}
static __forceinline int _vsnprintf(wchar_t *a, size_t b, const wchar_t *c, va_list d)
{
	return _vsnwprintf(a,b,c,d);
}
static __forceinline FILE* fopen(const wchar_t *a, const wchar_t *b)
{
	return _wfopen(a,b);
}
static __forceinline int fputs(const wchar_t *a, FILE *b)
{
	return fputws(a,b);
}
static __forceinline int _stricoll(const wchar_t *a, const wchar_t *b)
{
	return _wcsicoll(a,b);
}
static __forceinline wchar_t* strrchr(const wchar_t *a, wchar_t b)
{
	return (wchar_t*)wcsrchr(a,b);
}
static __forceinline wchar_t* strupr(wchar_t *a)
{
	return _wcsupr(a);
}
static __forceinline wchar_t* strlwr(wchar_t *a)
{
	return _wcslwr(a);
}
static __forceinline wchar_t* _strtime(wchar_t *a)
{
	return _wstrtime(a);
}
static __forceinline wchar_t* memchr(wchar_t *a,wchar_t b,size_t c)
{
	return wmemchr(a,b,c);
}
static __forceinline const wchar_t* memchr(const wchar_t *a,wchar_t b,size_t c)
{
	return wmemchr(a,b,c);
}
static __forceinline wchar_t* strpbrk(const wchar_t *a, const wchar_t *b)
{
	return (wchar_t*)wcspbrk(a,b);
}
static __forceinline int64_t _strtoi64(const wchar_t *nptr, wchar_t **endptr, int base)
{
	return _wcstoi64(nptr,endptr,base);
};
template<class Tout> struct text
		// ANSI <--> UNICODE conversion.
		// in	 : input  string in char* or wchar_t*.
		// inlen  : count of characters, not byte size!! -1 for null terminated string.
		// out	: output string in char* or wchar_t*.
		// outlen : count of characters, not byte size!!
{
private:
	Tout *buf;
	bool own;
public:
	template<class Tin> inline text(const Tin *in,int code_page = CP_ACP);
	template<class Tin> inline text(const Tin *in,Tout *Ibuf,int code_page = CP_ACP);
	template<class Tin> inline text(const Tin *in,int inlen,Tout *Ibuf,size_t outlen,int code_page = CP_ACP);
	~text() {
		if (own) {
			delete []buf;
		}
	}
	operator const Tout*() const {
		return buf;
	}
};
template<> template<> inline text<char>::text(const char *in,int code_page):buf(const_cast<char*>(in)),own(false) {}
template<> template<> inline text<char>::text(const char *in,char *Ibuf,int code_page):buf(strcpy(Ibuf,in)),own(false) {}
template<> template<> inline text<char>::text(const char *in,int inlen,char *Ibuf,size_t outlen,int code_page):own(false)
{
	if (inlen!=-1) {
		buf=Ibuf;
		if ((size_t)inlen < outlen) {
			ff_strncpy(Ibuf, in, inlen + 1);
		} else {
			ff_strncpy(Ibuf, in, outlen);
		}
	} else {
		buf=Ibuf;
		ff_strncpy(Ibuf, in, outlen);
	}
}
template<> template<> inline text<wchar_t>::text(const wchar_t *in,int code_page):buf(const_cast<wchar_t*>(in)),own(false) {}
template<> template<> inline text<wchar_t>::text(const wchar_t *in,wchar_t *Ibuf,int code_page):buf(strcpy(Ibuf,in)),own(false) {}
template<> template<> inline text<wchar_t>::text(const wchar_t *in,int inlen,wchar_t *Ibuf,size_t outlen,int code_page):own(false)
{
	if (inlen!=-1) {
		buf=Ibuf;
		if ((size_t)inlen < outlen) {
			ff_strncpy(Ibuf, in, inlen + 1);
		} else {
			ff_strncpy(Ibuf, in, outlen);
		}
	} else {
		buf=Ibuf;
		ff_strncpy(Ibuf, in, outlen);
	}
}
template<> template<> inline text<wchar_t>::text(const char *in,int code_page):own(in?true:false)
{
	if (in) {
		size_t l=strlen(in);
		buf=new wchar_t[l+1];
		MultiByteToWideChar(code_page,0,in,int(l+1),buf,int(l+1));
	} else {
		buf=NULL;
	}
}
template<> template<> inline text<wchar_t>::text(const char *in,wchar_t *Ibuf,int code_page):own(false),buf(Ibuf)
{
	size_t l=strlen(in);
	MultiByteToWideChar(code_page,0,in,int(l+1),buf,int(l+1));
}
template<> template<> inline text<wchar_t>::text(const char *in,int inlen,wchar_t *Ibuf,size_t outlen,int code_page):own(false),buf(Ibuf)
{
	MultiByteToWideChar(code_page,0,in,int(inlen),buf,int(outlen));
}
template<> template<> inline text<char>::text(const wchar_t *in,int code_page):own(in?true:false)
{
	if (in) {
		size_t l=strlen(in);
		int length=WideCharToMultiByte(code_page,0,in,int(l+1),buf,0,NULL,NULL);
		buf=new char[length+1];
		WideCharToMultiByte(code_page,0,in,int(l+1),buf,int(length+1),NULL,NULL);
	} else {
		buf=NULL;
	}
}
template<> template<> inline text<char>::text(const wchar_t *in,char *Ibuf,int code_page):own(false),buf(Ibuf)
{
	size_t l=strlen(in);
	int length= WideCharToMultiByte(code_page,0,in,int(l+1),buf,0,NULL,NULL);
	WideCharToMultiByte(code_page,0,in,int(l+1),buf,length,NULL,NULL);
}
template<> template<> inline text<char>::text(const wchar_t *in,int inlen,char *Ibuf,size_t outlen,int code_page):own(false),buf(Ibuf)
{
	WideCharToMultiByte(code_page,0,in,int(inlen),buf,int(outlen),NULL,NULL);
}

#endif

#pragma warning(pop)

#endif

#ifndef _FFIMGFMT_H_
#define _FFIMGFMT_H_

#include <ffmpeg/libavutil/pixfmt.h>

//================================ ffdshow ==================================

#define FF_CSP_NULL	(0ULL)

#define FF_CSP_420P	(1ULL << 0)	// 0x0000001
#define FF_CSP_422P	(1ULL << 1)	// 0x0000002
#define FF_CSP_444P	(1ULL << 2)	// 0x0000004
#define FF_CSP_411P	(1ULL << 3)	// 0x0000008
#define FF_CSP_410P	(1ULL << 4)	// 0x0000010

#define FF_CSP_YUY2	(1ULL << 5)	// 0x0000020
#define FF_CSP_UYVY	(1ULL << 6)	// 0x0000040
#define FF_CSP_YVYU	(1ULL << 7)	// 0x0000080
#define FF_CSP_VYUY	(1ULL << 8)	// 0x0000100

#define FF_CSP_ABGR	(1ULL << 9)	// 0x0000200 [a|b|g|r]
#define FF_CSP_RGBA	(1ULL << 10)	 // 0x0000400 [r|g|b|a]
#define FF_CSP_BGR32 (1ULL << 11)	 // 0x0000800
#define FF_CSP_BGR24 (1ULL << 12)	 // 0x0001000
#define FF_CSP_BGR15 (1ULL << 13)	 // 0x0002000
#define FF_CSP_BGR16 (1ULL << 14)	 // 0x0004000
#define FF_CSP_RGB32 (1ULL << 15)	 // 0x0008000
#define FF_CSP_RGB24 (1ULL << 16)	 // 0x0010000
#define FF_CSP_RGB15 (1ULL << 17)	 // 0x0020000
#define FF_CSP_RGB16 (1ULL << 18)	 // 0x0040000

#define FF_CSP_CLJR	(1ULL << 19)	 // 0x0080000
#define FF_CSP_Y800	(1ULL << 20)	 // 0x0100000
#define FF_CSP_NV12	(1ULL << 21)	 // 0x0200000

#define FF_CSP_420P10 (1ULL << 22)	// 0x0400000
#define FF_CSP_444P10 (1ULL << 23)	// 0x0800000
#define FF_CSP_P016	 (1ULL << 24)	// 0x1000000 P016 in Media Fundation (MFVideoFormat_P016). 16bit version of NV12.
#define FF_CSP_P010	 (1ULL << 25)	// 0x2000000 P010 in Media Fundation (MFVideoFormat_P010). same as FF_CSP_P016
#define FF_CSP_422P10 (1ULL << 26)	// 0x4000000
#define FF_CSP_P210	 (1ULL << 27)	// 0x8000000
#define FF_CSP_P216	 (1ULL << 28)	// 0x10000000

#define FF_CSP_AYUV	 (1ULL << 29)	// 0x20000000
#define FF_CSP_Y416	 (1ULL << 30)	// 0x40000000

#define FF_CSP_PAL8	 (1ULL << 31)	// 0x80000000

#define FF_CSP_GBRP	 (1ULL << 32)	// 0x100000000
#define FF_CSP_GBRP9	(1ULL << 33)	// 0x200000000
#define FF_CSP_GBRP10 (1ULL << 34)	// 0x400000000

#define FF_CSP_420P9	(1ULL << 35)	// 0x800000000
#define FF_CSP_422P9	(1ULL << 36)	// 0x1000000000
#define FF_CSP_444P9	(1ULL << 37)	// 0x2000000000

// Flags
#define FF_CSP_FLAGS_YUV_JPEG	 (1ULL << 59)
#define FF_CSP_FLAGS_YUV_ORDER	(1ULL << 60) // UV ordered chroma planes (not VU as default)
#define FF_CSP_FLAGS_YUV_ADJ	(1ULL << 61) // YUV planes are stored consecutively in one memory block
#define FF_CSP_FLAGS_INTERLACED (1ULL << 62)
#define FF_CSP_FLAGS_VFLIP		(1ULL << 63) // flip mask

#define FF_CSPS_NUM 38

#define FF_CSP_UNSUPPORTED		(1ULL<<FF_CSPS_NUM)

#define FF_CSPS_MASK			(FF_CSP_UNSUPPORTED-1)
#define FF_CSPS_MASK_HIGH_BIT	 (FF_CSP_420P10|FF_CSP_422P10|FF_CSP_444P10)
#define FF_CSPS_MASK_YUV_PLANAR (FF_CSP_420P|FF_CSP_422P|FF_CSP_444P|FF_CSP_411P|FF_CSP_410P)
#define FF_CSPS_MASK_RGB_PLANAR (FF_CSP_GBRP|FF_CSP_GBRP9|FF_CSP_GBRP10)
#define FF_CSPS_MASK_YUV_PACKED (FF_CSP_YUY2|FF_CSP_UYVY|FF_CSP_YVYU|FF_CSP_VYUY)
#define FF_CSPS_MASK_RGB		(FF_CSP_RGBA|FF_CSP_RGB32|FF_CSP_RGB24|FF_CSP_RGB15|FF_CSP_RGB16)
#define FF_CSPS_MASK_BGR		(FF_CSP_ABGR|FF_CSP_BGR32|FF_CSP_BGR24|FF_CSP_BGR15|FF_CSP_BGR16)
#define FF_CSPS_MASK_FFRGB		(FF_CSP_RGB32|FF_CSP_RGB24|FF_CSP_BGR32|FF_CSP_BGR24) // ffdshow converters output color spaces. Require dst stride to be multiple of 4.

#include <stddef.h>
typedef int stride_t;

//==================================== xvid4 =====================================

#define XVID4_CSP_PLANAR	 (1<< 0) /* 4:2:0 planar */
#define XVID4_CSP_I420	 (1<< 1) /* 4:2:0 packed(planar win32) */
#define XVID4_CSP_YV12	 (1<< 2) /* 4:2:0 packed(planar win32) */
#define XVID4_CSP_YUY2	 (1<< 3) /* 4:2:2 packed */
#define XVID4_CSP_UYVY	 (1<< 4) /* 4:2:2 packed */
#define XVID4_CSP_YVYU	 (1<< 5) /* 4:2:2 packed */
#define XVID4_CSP_BGRA	 (1<< 6) /* 32-bit bgra packed */
#define XVID4_CSP_ABGR	 (1<< 7) /* 32-bit abgr packed */
#define XVID4_CSP_RGBA	 (1<< 8) /* 32-bit rgba packed */
#define XVID4_CSP_BGR		(1<< 9) /* 24-bit bgr packed */
#define XVID4_CSP_RGB555	 (1<<10) /* 16-bit rgb555 packed */
#define XVID4_CSP_RGB565	 (1<<11) /* 16-bit rgb565 packed */
#define XVID4_CSP_SLICE	(1<<12) /* decoder only: 4:2:0 planar, per slice rendering */
#define XVID4_CSP_INTERNAL (1<<13) /* decoder only: 4:2:0 planar, returns ptrs to internal buffers */
#define XVID4_CSP_NULL	 (1<<14) /* decoder only: dont output anything */
#define XVID4_CSP_VFLIP	(1<<31) /* vertical flip mask */

static __inline uint64_t csp_xvid4_2ffdshow(int csp)
{
	switch (csp) {
		case XVID4_CSP_BGR	 :
			return FF_CSP_RGB24;
		case XVID4_CSP_YV12	:
			return FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ;
		case XVID4_CSP_YUY2	:
			return FF_CSP_YUY2;
		case XVID4_CSP_UYVY	:
			return FF_CSP_UYVY;
		case XVID4_CSP_I420	:
			return FF_CSP_420P|FF_CSP_FLAGS_YUV_ADJ|FF_CSP_FLAGS_YUV_ORDER;
		case XVID4_CSP_RGB555:
			return FF_CSP_RGB15;
		case XVID4_CSP_RGB565:
			return FF_CSP_RGB16;
		case XVID4_CSP_PLANAR:
			return FF_CSP_420P;
		case XVID4_CSP_YVYU	:
			return FF_CSP_YVYU;
		case XVID4_CSP_BGRA	:
			return FF_CSP_RGB32;
		case XVID4_CSP_ABGR	:
			return FF_CSP_ABGR;
		case XVID4_CSP_RGBA	:
			return FF_CSP_RGBA;
		default				:
			return FF_CSP_NULL;
	}
}

//================================= ffmpeg ===================================
static __inline uint64_t csp_lavc2ffdshow(enum AVPixelFormat pix_fmt)
{
	switch (pix_fmt) {
		case AV_PIX_FMT_YUV420P :
		case AV_PIX_FMT_YUVJ420P:
			return FF_CSP_420P;
		case AV_PIX_FMT_YUV422P :
		case AV_PIX_FMT_YUVJ422P:
			return FF_CSP_422P;
		case AV_PIX_FMT_YUV444P :
		case AV_PIX_FMT_YUVJ444P:
			return FF_CSP_444P;
		case AV_PIX_FMT_YUV411P :
			return FF_CSP_411P;
		case AV_PIX_FMT_YUV410P :
			return FF_CSP_410P;
		case AV_PIX_FMT_YUYV422 :
			return FF_CSP_YUY2;
		case AV_PIX_FMT_UYVY422 :
			return FF_CSP_UYVY;
		case AV_PIX_FMT_YUV420P10:
			return FF_CSP_420P10;
		case AV_PIX_FMT_YUV422P10:
			return FF_CSP_422P10;
		case AV_PIX_FMT_YUV444P10:
			return FF_CSP_444P10;
		case AV_PIX_FMT_BGR24	 :
			return FF_CSP_RGB24;
		case AV_PIX_FMT_RGB24	 :
			return FF_CSP_BGR24;
		case AV_PIX_FMT_RGB32	 :
		case AV_PIX_FMT_ARGB:
			return FF_CSP_RGB32;
		case AV_PIX_FMT_RGB555	:
			return FF_CSP_RGB15;
		case AV_PIX_FMT_RGB565	:
			return FF_CSP_RGB16;
		case AV_PIX_FMT_GRAY8	 :
			return FF_CSP_Y800;
		case AV_PIX_FMT_PAL8	:
			return FF_CSP_PAL8;
		case AV_PIX_FMT_NV12	:
			return FF_CSP_NV12;
		case AV_PIX_FMT_GBRP	:
			return FF_CSP_GBRP;
		case AV_PIX_FMT_GBRP9	 :
			return FF_CSP_GBRP9;
		case AV_PIX_FMT_GBRP10	:
			return FF_CSP_GBRP10;
		case AV_PIX_FMT_YUV420P9:
			return FF_CSP_420P9;
		case AV_PIX_FMT_YUV422P9:
			return FF_CSP_422P9;
		case AV_PIX_FMT_YUV444P9:
			return FF_CSP_444P9;
		default				:
			return FF_CSP_NULL;
	}
}
static __inline enum AVPixelFormat csp_ffdshow2lavc(uint64_t pix_fmt)
{
	switch (pix_fmt&FF_CSPS_MASK) {
		case FF_CSP_420P:
			return pix_fmt&FF_CSP_FLAGS_YUV_JPEG?AV_PIX_FMT_YUVJ420P:AV_PIX_FMT_YUV420P;
		case FF_CSP_422P:
			return pix_fmt&FF_CSP_FLAGS_YUV_JPEG?AV_PIX_FMT_YUVJ422P:AV_PIX_FMT_YUV422P;
		case FF_CSP_444P:
			return pix_fmt&FF_CSP_FLAGS_YUV_JPEG?AV_PIX_FMT_YUVJ444P:AV_PIX_FMT_YUV444P;
		case FF_CSP_411P:
			return AV_PIX_FMT_YUV411P;
		case FF_CSP_410P:
			return AV_PIX_FMT_YUV410P;
		case FF_CSP_YUY2:
			return AV_PIX_FMT_YUYV422;
		case FF_CSP_UYVY:
			return AV_PIX_FMT_UYVY422;
		case FF_CSP_420P10:
			return AV_PIX_FMT_YUV420P10;
		case FF_CSP_422P10:
			return AV_PIX_FMT_YUV422P10;
		case FF_CSP_444P10:
			return AV_PIX_FMT_YUV444P10;
		case FF_CSP_RGB24:
			return AV_PIX_FMT_BGR24;
		case FF_CSP_BGR24:
			return AV_PIX_FMT_RGB24;
		case FF_CSP_RGB32:
			return AV_PIX_FMT_RGB32;
		case FF_CSP_BGR32:
			return AV_PIX_FMT_BGR32;
		case FF_CSP_RGB15:
			return AV_PIX_FMT_RGB555;
		case FF_CSP_RGB16:
			return AV_PIX_FMT_RGB565;
		case FF_CSP_Y800:
			return AV_PIX_FMT_GRAY8;
		case FF_CSP_PAL8:
			return AV_PIX_FMT_PAL8;
		case FF_CSP_NV12:
			return AV_PIX_FMT_NV12;
		case FF_CSP_ABGR:
			return AV_PIX_FMT_ABGR;
		case FF_CSP_RGBA:
			return AV_PIX_FMT_RGBA;
		case FF_CSP_GBRP:
			return AV_PIX_FMT_GBRP;
		case FF_CSP_GBRP9:
			return AV_PIX_FMT_GBRP9;
		case FF_CSP_GBRP10:
			return AV_PIX_FMT_GBRP10;
		case FF_CSP_420P9:
			return AV_PIX_FMT_YUV420P9;
		case FF_CSP_422P9:
			return AV_PIX_FMT_YUV422P9;
		case FF_CSP_444P9:
			return AV_PIX_FMT_YUV444P9;
		default		 :
			return AV_PIX_FMT_NONE;
	}
}

#define SWS_IN_CSPS \
 (					\
	FF_CSP_420P|		\
	FF_CSP_444P|		\
	FF_CSP_422P|		\
	FF_CSP_411P|		\
	FF_CSP_410P|		\
	FF_CSP_YUY2|		\
	FF_CSP_UYVY|		\
	FF_CSP_YVYU|		\
	FF_CSP_VYUY|		\
	FF_CSP_BGR32|	 \
	FF_CSP_BGR24|	 \
	FF_CSP_BGR16|	 \
	FF_CSP_BGR15|	 \
	FF_CSP_RGB32|	 \
	FF_CSP_RGB24|	 \
	FF_CSP_RGB16|	 \
	FF_CSP_RGB15|	 \
	FF_CSP_NV12|		\
	FF_CSP_420P10|	\
	FF_CSP_422P10|	\
	FF_CSP_444P10|	\
	FF_CSP_420P9|	 \
	FF_CSP_422P9|	 \
	FF_CSP_444P9|	 \
	FF_CSP_Y800		 \
 )
#define SWS_OUT_CSPS \
 (					 \
	FF_CSP_420P|		 \
	FF_CSP_444P|		 \
	FF_CSP_422P|		 \
	FF_CSP_411P|		 \
	FF_CSP_410P|		 \
	FF_CSP_YUY2|		 \
	FF_CSP_UYVY|		 \
	FF_CSP_YVYU|		 \
	FF_CSP_VYUY|		 \
	FF_CSP_RGB32|		\
	FF_CSP_RGB24|		\
	FF_CSP_RGB16|		\
	FF_CSP_RGB15|		\
	FF_CSP_BGR32|		\
	FF_CSP_BGR24|		\
	FF_CSP_BGR16|		\
	FF_CSP_BGR15|		\
	FF_CSP_NV12|		 \
	FF_CSP_420P10|	 \
	FF_CSP_422P10|	 \
	FF_CSP_444P10|	 \
	FF_CSP_420P9|		\
	FF_CSP_422P9|		\
	FF_CSP_444P9|		\
	FF_CSP_Y800		\
 )

static __inline uint64_t csp_supSWSin(uint64_t x)
{
	return (x&FF_CSPS_MASK)&(SWS_IN_CSPS|FF_CSPS_MASK_HIGH_BIT);
}
static __inline uint64_t csp_supSWSout(uint64_t x)
{
	return (x&FF_CSPS_MASK)&(SWS_OUT_CSPS|FF_CSPS_MASK_HIGH_BIT);
}

#endif

#if defined(__cplusplus) && !defined(FF_CSP_ONLY)

#ifndef _FFIMGFMTCPP_H_
#define _FFIMGFMTCPP_H_

struct TcspInfo {
	uint64_t id;
	const char_t *name;
	int Bpp; // Bytes per pixel for each plane.
	int bpp; // bits per pixel for all plane. (Memory usage, not the effective bit depth)
	unsigned int numPlanes;
	unsigned int shiftX[4], shiftY[4];
	unsigned int black[4];
	FOURCC fcc, fcccsp;
	const GUID *subtype;
	int packedLumaOffset, packedChromaOffset;
};
extern const TcspInfo cspInfos[];
struct TcspInfos :std::vector<const TcspInfo*,array_allocator<const TcspInfo*,FF_CSPS_NUM*2> > {
private:
	struct TsortFc {
	private:
		uint64_t csp,outPrimaryCSP;
	public:
		TsortFc(uint64_t Icsp,uint64_t IoutPrimaryCSP):csp(Icsp),outPrimaryCSP(IoutPrimaryCSP) {}
		bool operator ()(const TcspInfo* &csp1,const TcspInfo* &csp2);
	};
public:
	void sort(uint64_t csp, uint64_t outPrimaryCSP);
};

static __inline const TcspInfo* csp_getInfo(uint64_t csp)
{
	switch (csp&(FF_CSPS_MASK|FF_CSP_FLAGS_YUV_ORDER)) {
		case FF_CSP_420P|FF_CSP_FLAGS_YUV_ORDER: {
			extern TcspInfo cspInfoIYUV;
			return &cspInfoIYUV;
		}
		default:
			csp&=FF_CSPS_MASK;
			if (csp==0) {
				return NULL;
			}
			int i=0;
			while (csp>>=1) {
				i++;
			}
			if (i<=FF_CSPS_NUM) {
				return &cspInfos[i];
			} else {
				return NULL;
			}
	}
}
const TcspInfo* csp_getInfoFcc(FOURCC fcc);

static __inline uint64_t csp_isYUVplanar(uint64_t x)
{
	return x&FF_CSPS_MASK&FF_CSPS_MASK_YUV_PLANAR;
}
static __inline uint64_t csp_isRGBplanar(uint64_t x)
{
	return x&FF_CSPS_MASK&FF_CSPS_MASK_RGB_PLANAR;
}
static __inline uint64_t csp_isYUVplanarHighBit(uint64_t x)
{
	return x & FF_CSPS_MASK & FF_CSPS_MASK_HIGH_BIT;
}
static __inline uint64_t csp_isYUVpacked(uint64_t x)
{
	return x&FF_CSPS_MASK&FF_CSPS_MASK_YUV_PACKED;
}
static __inline uint64_t csp_isYUV(uint64_t x)
{
	return csp_isYUVpacked(x)|csp_isYUVplanar(x);
}
static __inline uint64_t csp_isYUV_NV(uint64_t x)
{
	return csp_isYUVpacked(x)|csp_isYUVplanar(x)|(x & (FF_CSP_NV12|FF_CSP_P016|FF_CSP_P010|FF_CSP_P210|FF_CSP_P216));
}
static __inline uint64_t csp_isRGB_RGB(uint64_t x)
{
	return x&FF_CSPS_MASK&FF_CSPS_MASK_RGB;
}
static __inline uint64_t csp_isRGB_BGR(uint64_t x)
{
	return x&FF_CSPS_MASK&FF_CSPS_MASK_BGR;
}
static __inline uint64_t csp_isRGB(uint64_t x)
{
	return csp_isRGB_RGB(x)|csp_isRGB_BGR(x);
}
static __inline uint64_t csp_supXvid(uint64_t x)
{
	return (x&FF_CSPS_MASK)&(FF_CSP_RGB24|FF_CSP_420P|FF_CSP_YUY2|FF_CSP_UYVY|FF_CSP_YVYU|FF_CSP_VYUY|FF_CSP_RGB15|FF_CSP_RGB16|FF_CSP_RGB32|FF_CSP_ABGR|FF_CSP_RGBA|FF_CSP_BGR24);
}

bool csp_inFOURCCmask(uint64_t x,FOURCC fcc);

extern char_t* csp_getName2(const TcspInfo *cspInfo,uint64_t csp,char_t *buf,size_t len);
extern char_t* csp_getName(uint64_t csp,char_t *buf,size_t len);
extern uint64_t csp_bestMatch(uint64_t inCSP,uint64_t wantedCSPS,int *rank=NULL, uint64_t outPrimaryCSP=0);

static __inline void csp_yuv_adj_to_plane(uint64_t &csp,const TcspInfo *cspInfo,unsigned int dy,unsigned char *data[4],stride_t stride[4])
{
	if (csp_isYUVplanar(csp) && (csp & FF_CSP_FLAGS_YUV_ADJ)) {
		csp&=~FF_CSP_FLAGS_YUV_ADJ;
		data[2]=data[0]+stride[0]*(dy>>cspInfo->shiftY[0]);
		stride[1]=stride[0]>>cspInfo->shiftX[1];
		data[1]=data[2]+stride[1]*(dy>>cspInfo->shiftY[1]);
		stride[2]=stride[0]>>cspInfo->shiftX[2];
	} else if ((csp & (FF_CSP_NV12|FF_CSP_P016|FF_CSP_P010|FF_CSP_P210|FF_CSP_P216)) && (csp & FF_CSP_FLAGS_YUV_ADJ)) {
		csp&=~FF_CSP_FLAGS_YUV_ADJ;
		data[1] = data[0] + stride[0] *dy;
		stride[1] = stride[0];
	}

}
static __inline void csp_yuv_order(uint64_t &csp,unsigned char *data[4],stride_t stride[4])
{
	if (csp_isYUVplanar(csp) && (csp&FF_CSP_FLAGS_YUV_ORDER)) {
		csp&=~FF_CSP_FLAGS_YUV_ORDER;
		std::swap(data[1],data[2]);
		std::swap(stride[1],stride[2]);
	}
}
static __inline void csp_vflip(uint64_t &csp,const TcspInfo *cspInfo,unsigned char *data[],stride_t stride[],unsigned int dy)
{
	if (csp&FF_CSP_FLAGS_VFLIP) {
		csp&=~FF_CSP_FLAGS_VFLIP;
		for (unsigned int i=0; i<cspInfo->numPlanes; i++) {
			data[i]+=stride[i]*((dy>>cspInfo->shiftY[i])-1);
			stride[i]*=-1;
		}
	}
}

uint64_t getBMPcolorspace(const BITMAPINFOHEADER *hdr,const TcspInfos &forcedCsps);

struct TcspFcc {
	const char_t *name;
	FOURCC fcc;
	uint64_t csp;
	bool flip;
	bool supEnc;
};
extern const TcspFcc cspFccs[];

#endif

#endif
