/**
 * This file is part of Hercules.
 * http://herc.ws - http://github.com/HerculesWS/Hercules
 *
 * Copyright (C) 2013-2015  Hercules Dev Team
 * Copyright (C)  Athena Dev Teams
 *
 * Hercules is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * See sysinfo.h for a description of this file.
 *
 * Base Author: Haru @ http://herc.ws
 */
#define HERCULES_CORE

#include "sysinfo.h"

#include "common/cbasetypes.h"
#include "common/core.h"
#include "common/memmgr.h"
#include "common/strlib.h"

#include <stdio.h> // fopen
#include <stdlib.h> // atoi
#ifdef WIN32
#	include <windows.h>
#else
#	include <sys/time.h> // time constants
#	include <unistd.h>
#endif

/// Private interface fields
struct sysinfo_private {
	char *platform;
	char *osversion;
	char *cpu;
	int cpucores;
	char *arch;
	char *compiler;
	char *cflags;
	char *vcstype_name;
	int vcstype;
	char *vcsrevision_src;
	char *vcsrevision_scripts;
};

/// sysinfo.c interface source
struct sysinfo_interface sysinfo_s;
struct sysinfo_private sysinfo_p;

struct sysinfo_interface *sysinfo;

#define VCSTYPE_UNKNOWN 0
#define VCSTYPE_GIT 1
#define VCSTYPE_SVN 2
#define VCSTYPE_NONE (-1)

#ifdef WIN32
/**
 * Values to be used with GetProductInfo.
 *
 * These aren't defined in MSVC2008/WindowsXP, so we gotta define them here.
 * Values from: http://msdn.microsoft.com/en-us/library/windows/desktop/ms724358%28v=vs.85%29.aspx
 */
enum windows_product_type {
	msPRODUCT_UNDEFINED                           = 0x00000000, ///< An unknown product
	msPRODUCT_ULTIMATE                            = 0x00000001, ///< Ultimate
	msPRODUCT_HOME_BASIC                          = 0x00000002, ///< Home Basic
	msPRODUCT_HOME_PREMIUM                        = 0x00000003, ///< Home Premium
	msPRODUCT_ENTERPRISE                          = 0x00000004, ///< Enterprise
	msPRODUCT_HOME_BASIC_N                        = 0x00000005, ///< Home Basic N
	msPRODUCT_BUSINESS                            = 0x00000006, ///< Business
	msPRODUCT_STANDARD_SERVER                     = 0x00000007, ///< Server Standard
	msPRODUCT_DATACENTER_SERVER                   = 0x00000008, ///< Server Datacenter (full installation)
	msPRODUCT_SMALLBUSINESS_SERVER                = 0x00000009, ///< Windows Small Business Server
	msPRODUCT_ENTERPRISE_SERVER                   = 0x0000000A, ///< Server Enterprise (full installation)
	msPRODUCT_STARTER                             = 0x0000000B, ///< Starter
	msPRODUCT_DATACENTER_SERVER_CORE              = 0x0000000C, ///< Server Datacenter (core installation)
	msPRODUCT_ENTERPRISE_SERVER_CORE              = 0x0000000E, ///< Server Enterprise (core installation)
	msPRODUCT_STANDARD_SERVER_CORE                = 0x0000000D, ///< Server Standard (core installation)
	msPRODUCT_ENTERPRISE_SERVER_IA64              = 0x0000000F, ///< Server Enterprise for Itanium-based Systems
	msPRODUCT_BUSINESS_N                          = 0x00000010, ///< Business N
	msPRODUCT_WEB_SERVER                          = 0x00000011, ///< Web Server (full installation)
	msPRODUCT_CLUSTER_SERVER                      = 0x00000012, ///< HPC Edition
	msPRODUCT_HOME_SERVER                         = 0x00000013, ///< Windows Storage Server 2008 R2 Essentials
	msPRODUCT_STORAGE_EXPRESS_SERVER              = 0x00000014, ///< Storage Server Express
	msPRODUCT_STORAGE_STANDARD_SERVER             = 0x00000015, ///< Storage Server Standard
	msPRODUCT_STORAGE_WORKGROUP_SERVER            = 0x00000016, ///< Storage Server Workgroup
	msPRODUCT_STORAGE_ENTERPRISE_SERVER           = 0x00000017, ///< Storage Server Enterprise
	msPRODUCT_SERVER_FOR_SMALLBUSINESS            = 0x00000018, ///< Windows Server 2008 for Windows Essential Server Solutions
	msPRODUCT_SMALLBUSINESS_SERVER_PREMIUM        = 0x00000019, ///< Small Business Server Premium
	msPRODUCT_HOME_PREMIUM_N                      = 0x0000001A, ///< Home Premium N
	msPRODUCT_ENTERPRISE_N                        = 0x0000001B, ///< Enterprise N
	msPRODUCT_ULTIMATE_N                          = 0x0000001C, ///< Ultimate N
	msPRODUCT_WEB_SERVER_CORE                     = 0x0000001D, ///< Web Server (core installation)
	msPRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT    = 0x0000001E, ///< Windows Essential Business Server Management Server
	msPRODUCT_MEDIUMBUSINESS_SERVER_SECURITY      = 0x0000001F, ///< Windows Essential Business Server Security Server
	msPRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING     = 0x00000020, ///< Windows Essential Business Server Messaging Server
	msPRODUCT_SERVER_FOUNDATION                   = 0x00000021, ///< Server Foundation
	msPRODUCT_HOME_PREMIUM_SERVER                 = 0x00000022, ///< Windows Home Server 2011
	msPRODUCT_SERVER_FOR_SMALLBUSINESS_V          = 0x00000023, ///< Windows Server 2008 without Hyper-V for Windows Essential Server Solutions
	msPRODUCT_STANDARD_SERVER_V                   = 0x00000024, ///< Server Standard without Hyper-V
	msPRODUCT_DATACENTER_SERVER_V                 = 0x00000025, ///< Server Datacenter without Hyper-V (full installation)
	msPRODUCT_ENTERPRISE_SERVER_V                 = 0x00000026, ///< Server Enterprise without Hyper-V (full installation)
	msPRODUCT_DATACENTER_SERVER_CORE_V            = 0x00000027, ///< Server Datacenter without Hyper-V (core installation)
	msPRODUCT_STANDARD_SERVER_CORE_V              = 0x00000028, ///< Server Standard without Hyper-V (core installation)
	msPRODUCT_ENTERPRISE_SERVER_CORE_V            = 0x00000029, ///< Server Enterprise without Hyper-V (core installation)
	msPRODUCT_HYPERV                              = 0x0000002A, ///< Microsoft Hyper-V Server
	msPRODUCT_STORAGE_EXPRESS_SERVER_CORE         = 0x0000002B, ///< Storage Server Express (core installation)
	msPRODUCT_STORAGE_STANDARD_SERVER_CORE        = 0x0000002C, ///< Storage Server Standard (core installation)
	msPRODUCT_STORAGE_WORKGROUP_SERVER_CORE       = 0x0000002D, ///< Storage Server Workgroup (core installation)
	msPRODUCT_STORAGE_ENTERPRISE_SERVER_CORE      = 0x0000002E, ///< Storage Server Enterprise (core installation)
	msPRODUCT_STARTER_N                           = 0x0000002F, ///< Starter N
	msPRODUCT_PROFESSIONAL                        = 0x00000030, ///< Professional
	msPRODUCT_PROFESSIONAL_N                      = 0x00000031, ///< Professional N
	msPRODUCT_SB_SOLUTION_SERVER                  = 0x00000032, ///< Windows Small Business Server 2011 Essentials
	msPRODUCT_SERVER_FOR_SB_SOLUTIONS             = 0x00000033, ///< Server For SB Solutions
	msPRODUCT_STANDARD_SERVER_SOLUTIONS           = 0x00000034, ///< Server Solutions Premium
	msPRODUCT_STANDARD_SERVER_SOLUTIONS_CORE      = 0x00000035, ///< Server Solutions Premium (core installation)
	msPRODUCT_SB_SOLUTION_SERVER_EM               = 0x00000036, ///< Server For SB Solutions EM
	msPRODUCT_SERVER_FOR_SB_SOLUTIONS_EM          = 0x00000037, ///< Server For SB Solutions EM
	msPRODUCT_SOLUTION_EMBEDDEDSERVER             = 0x00000038, ///< Windows MultiPoint Server
	msPRODUCT_ESSENTIALBUSINESS_SERVER_MGMT       = 0x0000003B, ///< Windows Essential Server Solution Management
	msPRODUCT_ESSENTIALBUSINESS_SERVER_ADDL       = 0x0000003C, ///< Windows Essential Server Solution Additional
	msPRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC    = 0x0000003D, ///< Windows Essential Server Solution Management SVC
	msPRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC    = 0x0000003E, ///< Windows Essential Server Solution Additional SVC
	msPRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE   = 0x0000003F, ///< Small Business Server Premium (core installation)
	msPRODUCT_CLUSTER_SERVER_V                    = 0x00000040, ///< Server Hyper Core V
	msPRODUCT_STARTER_E                           = 0x00000042, ///< Not supported
	msPRODUCT_HOME_BASIC_E                        = 0x00000043, ///< Not supported
	msPRODUCT_HOME_PREMIUM_E                      = 0x00000044, ///< Not supported
	msPRODUCT_PROFESSIONAL_E                      = 0x00000045, ///< Not supported
	msPRODUCT_ENTERPRISE_E                        = 0x00000046, ///< Not supported
	msPRODUCT_ULTIMATE_E                          = 0x00000047, ///< Not supported
	msPRODUCT_ENTERPRISE_EVALUATION               = 0x00000048, ///< Server Enterprise (evaluation installation)
	msPRODUCT_MULTIPOINT_STANDARD_SERVER          = 0x0000004C, ///< Windows MultiPoint Server Standard (full installation)
	msPRODUCT_MULTIPOINT_PREMIUM_SERVER           = 0x0000004D, ///< Windows MultiPoint Server Premium (full installation)
	msPRODUCT_STANDARD_EVALUATION_SERVER          = 0x0000004F, ///< Server Standard (evaluation installation)
	msPRODUCT_DATACENTER_EVALUATION_SERVER        = 0x00000050, ///< Server Datacenter (evaluation installation)
	msPRODUCT_ENTERPRISE_N_EVALUATION             = 0x00000054, ///< Enterprise N (evaluation installation)
	msPRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER = 0x0000005F, ///< Storage Server Workgroup (evaluation installation)
	msPRODUCT_STORAGE_STANDARD_EVALUATION_SERVER  = 0x00000060, ///< Storage Server Standard (evaluation installation)
	msPRODUCT_CORE_N                              = 0x00000062, ///< Windows 8 N
	msPRODUCT_CORE_COUNTRYSPECIFIC                = 0x00000063, ///< Windows 8 China
	msPRODUCT_CORE_SINGLELANGUAGE                 = 0x00000064, ///< Windows 8 Single Language
	msPRODUCT_CORE                                = 0x00000065, ///< Windows 8
	msPRODUCT_PROFESSIONAL_WMC                    = 0x00000067, ///< Professional with Media Center
};

/**
 * Values to be used with GetSystemMetrics.
 *
 * Values from http://msdn.microsoft.com/en-us/library/windows/desktop/ms724385%28v=vs.85%29.aspx
 */
enum windows_metrics {
	msSM_SERVERR2 = 89, ///< Obtains the build number if the system is Windows Server 2003 R2; otherwise, 0.
};

/**
 * Values to be used with OSVERSIONINFOEX.wSuiteMask.
 *
 * Values from http://msdn.microsoft.com/en-us/library/windows/desktop/ms724833%28v=vs.85%29.aspx
 */
enum windows_ver_suite {
	msVER_SUITE_BLADE          = 0x00000400, ///< Windows Server 2003, Web Edition is installed.
	msVER_SUITE_STORAGE_SERVER = 0x00002000, ///< Windows Storage Server 2003 R2 or Windows Storage Server 2003 is installed.
	msVER_SUITE_COMPUTE_SERVER = 0x00004000, ///< Windows Server 2003, Compute Cluster Edition is installed.
	msVER_SUITE_WH_SERVER      = 0x00008000, ///< Windows Home Server is installed.
};

#else // not WIN32
// UNIX. Use build-time cached values
#include "sysinfo.inc"
#endif // WIN32

// Compiler detection <http://sourceforge.net/p/predef/wiki/Compilers/>
#if defined(__BORLANDC__)
#define SYSINFO_COMPILER "Borland C++"
#elif defined(__clang__)
#define SYSINFO_COMPILER "Clang v" EXPAND_AND_QUOTE(__clang_major__) "." EXPAND_AND_QUOTE(__clang_minor__) "." EXPAND_AND_QUOTE(__clang_patchlevel__)
#elif defined(__INTEL_COMPILER)
#define SYSINFO_COMPILER "Intel CC v" EXPAND_AND_QUOTE(__INTEL_COMPILER)
#elif defined(__MINGW32__)
#if defined(__MINGW64__)
#define SYSINFO_COMPILER "MinGW-w64 64 Bit v" EXPAND_AND_QUOTE(__MINGW64_VERSION_MAJOR) "." EXPAND_AND_QUOTE(__MINGW64_VERSION_MINOR) \
	" (MinGW " EXPAND_AND_QUOTE(__MINGW32_MAJOR_VERSION) "." EXPAND_AND_QUOTE(__MINGW32_MINOR_VERSION) ")"
#elif defined(__MINGW64_VERSION_MAJOR)
#define SYSINFO_COMPILER "MinGW-w64 32 Bit v" EXPAND_AND_QUOTE(__MINGW64_VERSION_MAJOR) "." EXPAND_AND_QUOTE(__MINGW64_VERSION_MINOR) \
	" (MinGW " EXPAND_AND_QUOTE(__MINGW32_MAJOR_VERSION) "." EXPAND_AND_QUOTE(__MINGW32_MINOR_VERSION) ")"
#else
#define SYSINFO_COMPILER "MinGW32 v" EXPAND_AND_QUOTE(__MINGW32_MAJOR_VERSION) "." EXPAND_AND_QUOTE(__MINGW32_MINOR_VERSION)
#endif
#elif defined(__GNUC__)
#define SYSINFO_COMPILER "GCC v" EXPAND_AND_QUOTE(__GNUC__) "." EXPAND_AND_QUOTE(__GNUC_MINOR__) "." EXPAND_AND_QUOTE(__GNUC_PATCHLEVEL__)
#elif defined(_MSC_VER)
#if _MSC_VER >= 1300 && _MSC_VER < 1310
#define SYSINFO_COMPILER "Microsoft Visual C++ 7.0 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1310 && _MSC_VER < 1400
#define SYSINFO_COMPILER "Microsoft Visual C++ 2003 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1400 && _MSC_VER < 1500
#define SYSINFO_COMPILER "Microsoft Visual C++ 2005 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1500 && _MSC_VER < 1600
#define SYSINFO_COMPILER "Microsoft Visual C++ 2008 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1600 && _MSC_VER < 1700
#define SYSINFO_COMPILER "Microsoft Visual C++ 2010 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1700 && _MSC_VER < 1800
#define SYSINFO_COMPILER "Microsoft Visual C++ 2012 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1800 && _MSC_VER < 1900
#define SYSINFO_COMPILER "Microsoft Visual C++ 2013 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#elif _MSC_VER >= 1900 && _MSC_VER < 2000
#define SYSINFO_COMPILER "Microsoft Visual C++ 2015 (v" EXPAND_AND_QUOTE(_MSC_VER) ")"
#else // < 1300 || >= 2000
#define SYSINFO_COMPILER "Microsoft Visual C++ v" EXPAND_AND_QUOTE(_MSC_VER)
#endif
#else
#define SYSINFO_COMPILER "Unknown"
#endif
// end compiler detection

/**
 * Retrieves the current SVN revision.
 *
 * @param out[out] a string pointer to return the value (to be aFree()'d.)
 * @retval true  if a revision was correctly detected.
 * @retval false if no revision was detected. out is set to NULL in this case.
 */
bool sysinfo_svn_get_revision(char **out) {
	// Only include SVN support if detected it, or we're on MSVC
#if !defined(SYSINFO_VCSTYPE) || SYSINFO_VCSTYPE == VCSTYPE_SVN || SYSINFO_VCSTYPE == VCSTYPE_UNKNOWN
	FILE *fp;

	// subversion 1.7 uses a sqlite3 database
	// FIXME this is hackish at best...
	// - ignores database file structure
	// - assumes the data in NODES.dav_cache column ends with "!svn/ver/<revision>/<path>)"
	// - since it's a cache column, the data might not even exist
	if ((fp = fopen(".svn"PATHSEP_STR"wc.db", "rb")) != NULL || (fp = fopen(".."PATHSEP_STR".svn"PATHSEP_STR"wc.db", "rb")) != NULL) {

#ifndef SVNNODEPATH //not sure how to handle branches, so I'll leave this overridable define until a better solution comes up
#define SVNNODEPATH trunk
#endif // SVNNODEPATH

		const char* prefix = "!svn/ver/";
		const char* postfix = "/"EXPAND_AND_QUOTE(SVNNODEPATH)")"; // there should exist only 1 entry like this
		size_t prefix_len = strlen(prefix);
		size_t postfix_len = strlen(postfix);
		size_t i,j,flen;
		char* buffer;

		// read file to buffer
		fseek(fp, 0, SEEK_END);
		flen = ftell(fp);
		buffer = (char*)aMalloc(flen + 1);
		fseek(fp, 0, SEEK_SET);
		flen = fread(buffer, 1, flen, fp);
		buffer[flen] = '\0';
		fclose(fp);

		// parse buffer
		for (i = prefix_len + 1; i + postfix_len <= flen; ++i) {
			if (buffer[i] != postfix[0] || memcmp(buffer + i, postfix, postfix_len) != 0)
				continue; // postfix mismatch
			for (j = i; j > 0; --j) { // skip digits
				if (!ISDIGIT(buffer[j - 1]))
					break;
			}
			if (memcmp(buffer + j - prefix_len, prefix, prefix_len) != 0)
				continue; // prefix mismatch
			// done
			if (*out != NULL)
				aFree(*out);
			*out = aCalloc(1, 8);
			snprintf(*out, 8, "%d", atoi(buffer + j));
			break;
		}
		aFree(buffer);

		if (*out != NULL)
			return true;
	}

	// subversion 1.6 and older?
	if ((fp = fopen(".svn/entries", "r")) != NULL) {
		char line[1024];
		int rev;
		// Check the version
		if (fgets(line, sizeof(line), fp)) {
			if (!ISDIGIT(line[0])) {
				// XML File format
				while (fgets(line,sizeof(line),fp))
					if (strstr(line,"revision=")) break;
				if (sscanf(line," %*[^\"]\"%d%*[^\n]", &rev) == 1) {
					if (*out != NULL)
						aFree(*out);
					*out = aCalloc(1, 8);
					snprintf(*out, 8, "%d", rev);
				}
			} else {
				// Bin File format
				if (fgets(line, sizeof(line), fp) == NULL) { printf("Can't get bin name\n"); } // Get the name
				if (fgets(line, sizeof(line), fp) == NULL) { printf("Can't get entries kind\n"); } // Get the entries kind
				if (fgets(line, sizeof(line), fp)) { // Get the rev numver
					if (*out != NULL)
						aFree(*out);
					*out = aCalloc(1, 8);
					snprintf(*out, 8, "%d", atoi(line));
				}
			}
		}
		fclose(fp);

		if (*out != NULL)
			return true;
	}
#endif
	if (*out != NULL)
		aFree(*out);
	*out = NULL;
	return false;
}

/**
 * Retrieves the current Git revision.
 *
 * @param out[out] a string pointer to return the value (to be aFree()'d.)
 * @retval true  if a revision was correctly detected.
 * @retval false if no revision was detected. out is set to NULL in this case.
 */
bool sysinfo_git_get_revision(char **out) {
	// Only include Git support if we detected it, or we're on MSVC
#if !defined(SYSINFO_VCSTYPE) || SYSINFO_VCSTYPE == VCSTYPE_GIT || SYSINFO_VCSTYPE == VCSTYPE_UNKNOWN
	char ref[128], filepath[128], line[128];

	strcpy(ref, "HEAD");

	while (*ref) {
		FILE *fp;
		snprintf(filepath, sizeof(filepath), ".git/%s", ref);
		if ((fp = fopen(filepath, "r")) != NULL) {
			if (fgets(line, sizeof(line)-1, fp) == NULL) {
				fclose(fp);
				break;
			}
			fclose(fp);
			if (sscanf(line, "ref: %127[^\n]", ref) == 1) {
				continue;
			} else if (sscanf(line, "%127[a-f0-9]", ref) == 1 && strlen(ref) == 40) {
				if (*out != NULL)
					aFree(*out);
				*out = aStrdup(ref);
			}
		}
		break;
	}
	if (*out != NULL)
		return true;
#else
	if (*out != NULL)
		aFree(*out);
	*out = NULL;
#endif
	return false;
}

#ifdef WIN32

/// Windows-specific runtime detection functions.

typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
/**
 * Retrieves the Operating System version (Windows only).
 *
 * Once retrieved, the version string is stored into sysinfo->p->osversion.
 */
void sysinfo_osversion_retrieve(void) {
	OSVERSIONINFOEX osvi;
	StringBuf buf;
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	StrBuf->Init(&buf);

	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

	if (sysinfo->p->osversion != NULL) {
		aFree(sysinfo->p->osversion);
		sysinfo->p->osversion = NULL;
	}

	/*
	 * #pragma rantmode (on)
	 * Some engineer at Microsoft moronically decided that, since some applications use this information to do version checks and refuse to
	 * run if they detect a new, unknown version of Windows, now nobody will be able to rely on this information anymore, not even those who
	 * need it for reporting or logging.
	 * The correct fix was to let those applications break, and their developer fix them (and in the meanwhile let the users use the
	 * Compatibility settings to run them) but no, they decided they'd deprecate the API, and make it lie for those who use it, reporting
	 * windows 8 even if they're running on 8.1 or newer.
	 * The API wasn't broken, applications were. Now we have broken applications, and a broken API. Great move, Microsoft.  Oh right,
	 * there's the Version API helper functions. Or maybe not, since you can only do 'are we running on at least version X?' checks with
	 * those, it's not what we need.
	 * You know what? I'll just silence your deprecation warning for the time being. Maybe by the time you release the next version of
	 * Windows, you'll have provided a less crippled API or something.
	 * #pragma rantmode (off)
	 */
#pragma warning (push)
#pragma warning (disable : 4996)
	if (!GetVersionEx((OSVERSIONINFO*) &osvi)) {
		sysinfo->p->osversion = aStrdup("Unknown Version");
		return;
	}
#pragma warning (pop)

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId // Windows NT Family
	 && ((osvi.dwMajorVersion > 4 && osvi.dwMajorVersion < 6) || (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion <= 3)) // Between XP and 8.1
	) {
		if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion <= 3) { // Between Vista and 8.1
			PGPI pGPI;
			DWORD dwType;
			if (osvi.dwMinorVersion == 0) {
				StrBuf->AppendStr(&buf, osvi.wProductType == VER_NT_WORKSTATION ? "Windows Vista" : "Windows Server 2008");
			} else if (osvi.dwMinorVersion == 1) {
				StrBuf->AppendStr(&buf, osvi.wProductType == VER_NT_WORKSTATION ? "Windows 7" : "Windows Server 2008 R2");
			} else {
				// If it's 2, it can be Windows 8, or any newer version (8.1 at the time of writing this) -- see above for the reason.
				switch (osvi.dwMinorVersion) {
				case 2:
					{
						ULONGLONG mask = 0;
						OSVERSIONINFOEX osvi2;
						ZeroMemory(&osvi2, sizeof(OSVERSIONINFOEX));
						osvi2.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
						osvi2.dwMajorVersion = 6;
						osvi2.dwMinorVersion = 2;
						VER_SET_CONDITION(mask, VER_MAJORVERSION, VER_LESS_EQUAL);
						VER_SET_CONDITION(mask, VER_MINORVERSION, VER_LESS_EQUAL);
						if (VerifyVersionInfo(&osvi2, VER_MAJORVERSION | VER_MINORVERSION, mask)) {
							StrBuf->AppendStr(&buf, osvi.wProductType == VER_NT_WORKSTATION ? "Windows 8" : "Windows Server 2012");
							break;
						}
					}
				case 3:
					StrBuf->AppendStr(&buf, osvi.wProductType == VER_NT_WORKSTATION ? "Windows 8.1" : "Windows Server 2012 R2");
				}
			}

			pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");

			pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType);

			switch (dwType) {
				case msPRODUCT_ULTIMATE:
				case msPRODUCT_ULTIMATE_N:
					StrBuf->AppendStr(&buf, " Ultimate");
					break;
				case msPRODUCT_PROFESSIONAL:
				case msPRODUCT_PROFESSIONAL_N:
				case msPRODUCT_PROFESSIONAL_WMC:
					StrBuf->AppendStr(&buf, " Professional");
					break;
				case msPRODUCT_HOME_PREMIUM:
				case msPRODUCT_HOME_PREMIUM_N:
					StrBuf->AppendStr(&buf, " Home Premium");
					break;
				case msPRODUCT_HOME_BASIC:
				case msPRODUCT_HOME_BASIC_N:
					StrBuf->AppendStr(&buf, " Home Basic");
					break;
				case msPRODUCT_ENTERPRISE:
				case msPRODUCT_ENTERPRISE_N:
				case msPRODUCT_ENTERPRISE_SERVER:
				case msPRODUCT_ENTERPRISE_SERVER_CORE:
				case msPRODUCT_ENTERPRISE_SERVER_IA64:
				case msPRODUCT_ENTERPRISE_SERVER_V:
				case msPRODUCT_ENTERPRISE_SERVER_CORE_V:
				case msPRODUCT_ENTERPRISE_EVALUATION:
				case msPRODUCT_ENTERPRISE_N_EVALUATION:
					StrBuf->AppendStr(&buf, " Enterprise");
					break;
				case msPRODUCT_BUSINESS:
				case msPRODUCT_BUSINESS_N:
					StrBuf->AppendStr(&buf, " Business");
					break;
				case msPRODUCT_STARTER:
				case msPRODUCT_STARTER_N:
					StrBuf->AppendStr(&buf, " Starter");
					break;
				case msPRODUCT_CLUSTER_SERVER:
				case msPRODUCT_CLUSTER_SERVER_V:
					StrBuf->AppendStr(&buf, " Cluster Server");
					break;
				case msPRODUCT_DATACENTER_SERVER:
				case msPRODUCT_DATACENTER_SERVER_CORE:
				case msPRODUCT_DATACENTER_SERVER_V:
				case msPRODUCT_DATACENTER_SERVER_CORE_V:
				case msPRODUCT_DATACENTER_EVALUATION_SERVER:
					StrBuf->AppendStr(&buf, " Datacenter");
					break;
				case msPRODUCT_SMALLBUSINESS_SERVER:
				case msPRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
				case msPRODUCT_SMALLBUSINESS_SERVER_PREMIUM_CORE:
					StrBuf->AppendStr(&buf, " Small Business Server");
					break;
				case PRODUCT_STANDARD_SERVER:
				case PRODUCT_STANDARD_SERVER_CORE:
				case msPRODUCT_STANDARD_SERVER_V:
				case msPRODUCT_STANDARD_SERVER_CORE_V:
				case msPRODUCT_STANDARD_EVALUATION_SERVER:
					StrBuf->AppendStr(&buf, " Standard");
					break;
				case msPRODUCT_WEB_SERVER:
				case msPRODUCT_WEB_SERVER_CORE:
					StrBuf->AppendStr(&buf, " Web Server");
					break;
				case msPRODUCT_STORAGE_EXPRESS_SERVER:
				case msPRODUCT_STORAGE_STANDARD_SERVER:
				case msPRODUCT_STORAGE_WORKGROUP_SERVER:
				case msPRODUCT_STORAGE_ENTERPRISE_SERVER:
				case msPRODUCT_STORAGE_EXPRESS_SERVER_CORE:
				case msPRODUCT_STORAGE_STANDARD_SERVER_CORE:
				case msPRODUCT_STORAGE_WORKGROUP_SERVER_CORE:
				case msPRODUCT_STORAGE_ENTERPRISE_SERVER_CORE:
				case msPRODUCT_STORAGE_WORKGROUP_EVALUATION_SERVER:
				case msPRODUCT_STORAGE_STANDARD_EVALUATION_SERVER:
					StrBuf->AppendStr(&buf, " Storage Server");
					break;
				case msPRODUCT_HOME_SERVER:
				case msPRODUCT_SERVER_FOR_SMALLBUSINESS:
				case msPRODUCT_MEDIUMBUSINESS_SERVER_MANAGEMENT:
				case msPRODUCT_MEDIUMBUSINESS_SERVER_SECURITY:
				case msPRODUCT_MEDIUMBUSINESS_SERVER_MESSAGING:
				case msPRODUCT_SERVER_FOR_SMALLBUSINESS_V:
				case msPRODUCT_SERVER_FOUNDATION:
				case msPRODUCT_HOME_PREMIUM_SERVER:
				case msPRODUCT_HYPERV:
				case msPRODUCT_SB_SOLUTION_SERVER:
				case msPRODUCT_SERVER_FOR_SB_SOLUTIONS:
				case msPRODUCT_STANDARD_SERVER_SOLUTIONS:
				case msPRODUCT_STANDARD_SERVER_SOLUTIONS_CORE:
				case msPRODUCT_SB_SOLUTION_SERVER_EM:
				case msPRODUCT_SERVER_FOR_SB_SOLUTIONS_EM:
				case msPRODUCT_SOLUTION_EMBEDDEDSERVER:
				case msPRODUCT_ESSENTIALBUSINESS_SERVER_MGMT:
				case msPRODUCT_ESSENTIALBUSINESS_SERVER_ADDL:
				case msPRODUCT_ESSENTIALBUSINESS_SERVER_MGMTSVC:
				case msPRODUCT_ESSENTIALBUSINESS_SERVER_ADDLSVC:
				case msPRODUCT_MULTIPOINT_STANDARD_SERVER:
				case msPRODUCT_MULTIPOINT_PREMIUM_SERVER:
					StrBuf->AppendStr(&buf, " Server (other)");
					break;
				case msPRODUCT_CORE_N:
				case msPRODUCT_CORE_COUNTRYSPECIFIC:
				case msPRODUCT_CORE_SINGLELANGUAGE:
				case msPRODUCT_CORE:
					StrBuf->AppendStr(&buf, " Workstation (other)");
					break;
			}

		} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2) { // XP x64 and Server 2003
			if (osvi.wProductType == VER_NT_WORKSTATION) {
				StrBuf->AppendStr(&buf, "Windows XP Professional");
			} else {
				if (GetSystemMetrics(msSM_SERVERR2))
					StrBuf->AppendStr(&buf, "Windows Server 2003 R2");
				else if (osvi.wSuiteMask & msVER_SUITE_STORAGE_SERVER)
					StrBuf->AppendStr(&buf, "Windows Storage Server 2003");
				else if (osvi.wSuiteMask & msVER_SUITE_WH_SERVER)
					StrBuf->AppendStr(&buf, "Windows Home Server");
				else
					StrBuf->AppendStr(&buf, "Windows Server 2003");

				// Test for the server type.
				if (osvi.wSuiteMask & msVER_SUITE_COMPUTE_SERVER)
					StrBuf->AppendStr(&buf, " Compute Cluster");
				else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
					StrBuf->AppendStr(&buf, " Datacenter");
				else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
					StrBuf->AppendStr(&buf, " Enterprise");
				else if (osvi.wSuiteMask & msVER_SUITE_BLADE)
					StrBuf->AppendStr(&buf, " Web");
				else
					StrBuf->AppendStr(&buf, " Standard");
			}
		} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1) { // XP
			StrBuf->AppendStr(&buf, "Windows XP");
			if (osvi.wSuiteMask & VER_SUITE_EMBEDDEDNT)
				StrBuf->AppendStr(&buf, " Embedded");
			else if (osvi.wSuiteMask & VER_SUITE_PERSONAL)
				StrBuf->AppendStr(&buf, " Home");
			else
				StrBuf->AppendStr(&buf, " Professional");
		} else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0) { // 2000
			StrBuf->AppendStr(&buf, "Windows 2000");

			if (osvi.wProductType == VER_NT_WORKSTATION)
				StrBuf->AppendStr(&buf, " Professional");
			else if (osvi.wSuiteMask & VER_SUITE_DATACENTER)
				StrBuf->AppendStr(&buf, " Datacenter Server");
			else if (osvi.wSuiteMask & VER_SUITE_ENTERPRISE)
				StrBuf->AppendStr(&buf, " Advanced Server");
			else
				StrBuf->AppendStr(&buf, " Server");
		} else {
			StrBuf->Printf(&buf, "Unknown Windows version %d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion);
		}
	}

	// Include service pack (if any) and build number.

	if (osvi.szCSDVersion[0] != '\0') {
		StrBuf->Printf(&buf, " %s", osvi.szCSDVersion);
	}

	StrBuf->Printf(&buf, " (build %d)", osvi.dwBuildNumber);

	sysinfo->p->osversion = aStrdup(StrBuf->Value(&buf));

	StrBuf->Destroy(&buf);
	return;
}

typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

/**
 * Retrieves SYSTEM_INFO (Windows only)
 * System info is not stored anywhere after retrieval
 * @see http://msdn.microsoft.com/en-us/library/windows/desktop/ms724958(v=vs.85).aspx
 **/
void sysinfo_systeminfo_retrieve( LPSYSTEM_INFO info ) {
	PGNSI pGNSI;

	// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
	pGNSI = (PGNSI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
	if (NULL != pGNSI)
		pGNSI(info);
	else
		GetSystemInfo(info);

	return;
}

/**
 * Returns number of bytes in a memory page
 * Only needed when compiling with MSVC
 **/
long sysinfo_getpagesize( void ) {
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));

	sysinfo_systeminfo_retrieve(&si);
	return si.dwPageSize;
}

/**
 * Retrieves the CPU type (Windows only).
 *
 * Once retrieved, the name is stored into sysinfo->p->cpu and the
 * number of cores in sysinfo->p->cpucores.
 */
void sysinfo_cpu_retrieve(void) {
	StringBuf buf;
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	StrBuf->Init(&buf);

	if (sysinfo->p->cpu != NULL) {
		aFree(sysinfo->p->cpu);
		sysinfo->p->cpu = NULL;
	}

	sysinfo_systeminfo_retrieve(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL
	 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64
	) {
		StrBuf->Printf(&buf, "%s CPU, Family %d, Model %d, Stepping %d",
		               si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ? "x86" : "x86_64",
		               si.wProcessorLevel,
		               (si.wProcessorRevision&0xff00)>>8,
		               (si.wProcessorRevision&0xff));
	} else {
		StrBuf->AppendStr(&buf, "Unknown");
	}

	sysinfo->p->cpu = aStrdup(StrBuf->Value(&buf));
	sysinfo->p->cpucores = si.dwNumberOfProcessors;

	StrBuf->Destroy(&buf);
}

/**
 * Retrieves the OS architecture (Windows only).
 *
 * Once retrieved, the name is stored into sysinfo->p->arch.
 */
void sysinfo_arch_retrieve(void) {
	SYSTEM_INFO si;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));

	if (sysinfo->p->arch != NULL) {
		aFree(sysinfo->p->arch);
		sysinfo->p->arch = NULL;
	}

	sysinfo_systeminfo_retrieve(&si);

	if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) // x64
		sysinfo->p->arch = aStrdup("x86_64");
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) // x32
		sysinfo->p->arch = aStrdup("x86");
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM) // ARM
		sysinfo->p->arch = aStrdup("ARM");
	else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) // Itanium
		sysinfo->p->arch = aStrdup("IA-64");
	else
		sysinfo->p->arch = aStrdup("Unknown");
}

/**
 * Retrieves the startup-time VCS revision information.
 *
 * Once retrieved, the value is stored in sysinfo->p->vcsrevision_src.
 */
void sysinfo_vcsrevision_src_retrieve(void) {
	if (sysinfo->p->vcsrevision_src != NULL) {
		aFree(sysinfo->p->vcsrevision_src);
		sysinfo->p->vcsrevision_src = NULL;
	}
	// Try Git, then SVN
	if (sysinfo_git_get_revision(&sysinfo->p->vcsrevision_src)) {
		sysinfo->p->vcstype = VCSTYPE_GIT;
		return;
	}
	if (sysinfo_svn_get_revision(&sysinfo->p->vcsrevision_src)) {
		sysinfo->p->vcstype = VCSTYPE_SVN;
		return;
	}
	sysinfo->p->vcstype = VCSTYPE_NONE;
	sysinfo->p->vcsrevision_src = aStrdup("Unknown");
}
#endif // WIN32

/**
 * Retrieves the VCS type name.
 *
 * Once retrieved, the value is stored in sysinfo->p->vcstype_name.
 */
void sysinfo_vcstype_name_retrieve(void) {
	if (sysinfo->p->vcstype_name != NULL) {
		aFree(sysinfo->p->vcstype_name);
		sysinfo->p->vcstype_name = NULL;
	}
	switch (sysinfo->p->vcstype) {
		case VCSTYPE_GIT:
			sysinfo->p->vcstype_name = aStrdup("Git");
			break;
		case VCSTYPE_SVN:
			sysinfo->p->vcstype_name = aStrdup("SVN");
			break;
		default:
			sysinfo->p->vcstype_name = aStrdup("Exported");
			break;
	}
}

/**
 * Returns the platform (OS type) this application is running on.
 *
 * This information is cached at compile time, since it's unlikely to change.
 *
 * @return the OS platform name.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "Linux", "Darwin", "Windows", etc.
 */
const char *sysinfo_platform(void) {
	return sysinfo->p->platform;
}

/**
 * Returns the Operating System version the application is running on.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time, since it is uncommon that an application is compiled and runs
 *   on different machines.
 *
 * @return the OS name.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "Windows 2008 Small Business Server", "OS X 10.8 Mountain Lion",
 *   "Gentoo Base System Release 2.2", "Debian GNU/Linux 6.0.6 (squeeze)", etc.
 */
const char *sysinfo_osversion(void) {
	return sysinfo->p->osversion;
}

/**
 * Returns the CPU model the application is running on.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time, since it is uncommon that an application is compiled and runs
 *   on different machines.
 *
 * @return the CPU model name.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "Intel(R) Atom(TM) CPU D2500   @ 1.86GHz",
 *   "Intel(R) Xeon(R) CPU E5-1650 0 @ 3.20GHz", "Intel Core i7",
 *   "x86 CPU, Family 6, Model 54, Stepping 1", etc.
 */
const char *sysinfo_cpu(void) {
	return sysinfo->p->cpu;
}

/**
 * Returns the number of CPU cores available.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time, since it is uncommon that an application is compiled and runs
 *   on different machines.
 *
 * @return the number of CPU cores.
 */
int sysinfo_cpucores(void) {
	return sysinfo->p->cpucores;
}

/**
 * Returns the CPU architecture the application was compiled for.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time, since it is uncommon that an application is compiled and runs
 *   on different machines.
 *
 * @return the CPU architecture name.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "x86", "x86_64", "IA-64", "ARM", etc.
 */
const char *sysinfo_arch(void) {
	return sysinfo->p->arch;
}

/**
 * Returns info about the 32 or 64 bit build of Hercules.
 *
 * @retval true  if this is a 64 bit build.
 * @retval false if this isn't a 64 bit build (i.e. it is a 32 bit build).
 */
bool sysinfo_is64bit(void) {
#ifdef _LP64
	return true;
#else
	return false;
#endif
}

/**
 * Returns the compiler the application was compiled with.
 *
 * @return the compiler name.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "Microsoft Visual C++ 2012 (v170050727)",
 *   "Clang v5.0.0", "MinGW32 v3.20", "GCC v4.7.3", etc.
 */
const char *sysinfo_compiler(void) {
	return sysinfo->p->compiler;
}

/**
 * Returns the compiler flags the application was compiled with.
 *
 * On Windows (MSVC), an empty string is returned instead.
 *
 * @return the compiler flags.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "-ggdb -O2 -flto -pipe -ffast-math ..."
 */
const char *sysinfo_cflags(void) {
	return sysinfo->p->cflags;
}

/**
 * Returns the Version Control System the application was downloaded with.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time. On Windows (MSVC), it is cached when the function is first
 *   called (most likely on server startup).
 *
 * @return the VCS type (numerical).
 *
 * @see VCSTYPE_NONE, VCSTYPE_GIT, VCSTYPE_SVN, VCSTYPE_UNKNOWN
 */
int sysinfo_vcstypeid(void) {
	return sysinfo->p->vcstype;
}

/**
 * Returns the Version Control System the application was downloaded with.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time. On Windows (MSVC), it is cached when the function is first
 *   called (most likely on server startup).
 *
 * @return the VCS type.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: "Git", "SVN", "Exported"
 */
const char *sysinfo_vcstype(void) {
	return sysinfo->p->vcstype_name;
}

/**
 * Returns the Version Control System revision.
 *
 * On platforms other than Windows (MSVC), this information is cached at
 *   compile time for better reliability. On Windows (MSVC), it is cached when
 *   the function is first called (most likely on server startup), so it may
 *   diverge from the actual revision that was compiled.
 *
 * @return the VCS revision.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: Git: "9128feccf3bddda94a7f8a170305565416815b40", SVN: "17546"
 */
const char *sysinfo_vcsrevision_src(void) {
	return sysinfo->p->vcsrevision_src;
}

/**
 * Returns the Version Control System revision.
 *
 * This information is cached during a script reload, so that it matches the
 *   version of the loaded scripts.
 *
 * @return the VCS revision.
 *
 * Note: Ownership is NOT transferred, the value should not be freed.
 *
 * Output example: Git: "9128feccf3bddda94a7f8a170305565416815b40", SVN: "17546"
 */
const char *sysinfo_vcsrevision_scripts(void) {
	return sysinfo->p->vcsrevision_scripts;
}

/**
 * Reloads the run-time (scripts) VCS revision information. To be used during
 *   script reloads to refresh the cached version.
 */
void sysinfo_vcsrevision_reload(void) {
	if (sysinfo->p->vcsrevision_scripts != NULL) {
		aFree(sysinfo->p->vcsrevision_scripts);
		sysinfo->p->vcsrevision_scripts = NULL;
	}
	// Try Git, then SVN
	if (sysinfo_git_get_revision(&sysinfo->p->vcsrevision_scripts)) {
		return;
	}
	if (sysinfo_svn_get_revision(&sysinfo->p->vcsrevision_scripts)) {
		return;
	}
	sysinfo->p->vcsrevision_scripts = aStrdup("Unknown");
}

/**
 * Checks if we're running (unnecessarily) as superuser.
 *
 * @retval true  if the current process is running as UNIX super-user.
 * @retval false if the current process is running as regular user, or
 *               in any case under Windows.
 */
bool sysinfo_is_superuser(void) {
#ifndef _WIN32
	if (geteuid() == 0)
		return true;
#endif
	return false;
}

/**
 * Interface runtime initialization.
 */
void sysinfo_init(void) {
	sysinfo->p->compiler = SYSINFO_COMPILER;
#ifdef WIN32
	sysinfo->p->platform = "Windows";
	sysinfo->p->cflags = "N/A";
	sysinfo_osversion_retrieve();
	sysinfo_cpu_retrieve();
	sysinfo_arch_retrieve();
	sysinfo_vcsrevision_src_retrieve();
#else
	sysinfo->p->platform = SYSINFO_PLATFORM;
	sysinfo->p->osversion = SYSINFO_OSVERSION;
	sysinfo->p->cpucores = SYSINFO_CPUCORES;
	sysinfo->p->cpu = SYSINFO_CPU;
	sysinfo->p->arch = SYSINFO_ARCH;
	sysinfo->p->cflags = SYSINFO_CFLAGS;
	sysinfo->p->vcstype = SYSINFO_VCSTYPE;
	sysinfo->p->vcsrevision_src = SYSINFO_VCSREV;
#endif
	sysinfo->vcsrevision_reload();
	sysinfo_vcstype_name_retrieve(); // Must be called after setting vcstype
}

/**
 * Interface shutdown cleanup.
 */
void sysinfo_final(void) {
#ifdef WIN32
	// Only need to be free'd in win32, they're #defined elsewhere
	if (sysinfo->p->osversion)
		aFree(sysinfo->p->osversion);
	if (sysinfo->p->cpu)
		aFree(sysinfo->p->cpu);
	if (sysinfo->p->arch)
		aFree(sysinfo->p->arch);
	if (sysinfo->p->vcsrevision_src)
		aFree(sysinfo->p->vcsrevision_src);
#endif
	sysinfo->p->platform = NULL;
	sysinfo->p->osversion = NULL;
	sysinfo->p->cpu = NULL;
	sysinfo->p->arch = NULL;
	sysinfo->p->vcsrevision_src = NULL;
	sysinfo->p->cflags = NULL;
	if (sysinfo->p->vcsrevision_scripts)
		aFree(sysinfo->p->vcsrevision_scripts);
	sysinfo->p->vcsrevision_scripts = NULL;
	if (sysinfo->p->vcstype_name)
		aFree(sysinfo->p->vcstype_name);
	sysinfo->p->vcstype_name = NULL;
}

static const char *sysinfo_time(void)
{
#if defined(WIN32)
	return "ticks count";
#elif defined(ENABLE_RDTSC)
	return "rdtsc";
#elif defined(HAVE_MONOTONIC_CLOCK)
	return "monotonic clock";
#else
	return "time of day";
#endif
}

/**
 * Interface default values initialization.
 */
void sysinfo_defaults(void) {
	sysinfo = &sysinfo_s;
	memset(&sysinfo_p, '\0', sizeof(sysinfo_p));
	sysinfo->p = &sysinfo_p;
#if defined(WIN32) && !defined(__CYGWIN__)
	sysinfo->getpagesize = sysinfo_getpagesize;
#else
	sysinfo->getpagesize = getpagesize;
#endif
	sysinfo->platform = sysinfo_platform;
	sysinfo->osversion = sysinfo_osversion;
	sysinfo->cpu = sysinfo_cpu;
	sysinfo->cpucores = sysinfo_cpucores;
	sysinfo->arch = sysinfo_arch;
	sysinfo->is64bit = sysinfo_is64bit;
	sysinfo->compiler = sysinfo_compiler;
	sysinfo->cflags = sysinfo_cflags;
	sysinfo->time = sysinfo_time;
	sysinfo->vcstype = sysinfo_vcstype;
	sysinfo->vcstypeid = sysinfo_vcstypeid;
	sysinfo->vcsrevision_src = sysinfo_vcsrevision_src;
	sysinfo->vcsrevision_scripts = sysinfo_vcsrevision_scripts;
	sysinfo->vcsrevision_reload = sysinfo_vcsrevision_reload;
	sysinfo->is_superuser = sysinfo_is_superuser;
	sysinfo->init = sysinfo_init;
	sysinfo->final = sysinfo_final;
}
