// Copyright (c) 2009-2015 The Bitcoin Core developers
// Copyright (c) 2018-2022 The Documentchain developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_CLIENTVERSION_H
#define BITCOIN_CLIENTVERSION_H

#if defined(HAVE_CONFIG_H)
#include <config/dms-config.h>
#endif //HAVE_CONFIG_H

// Check that required client information is defined
#if !defined(CLIENT_VERSION_MAJOR) || !defined(CLIENT_VERSION_MINOR) || !defined(CLIENT_VERSION_REVISION) || !defined(CLIENT_VERSION_BUILD) || !defined(CLIENT_VERSION_IS_RELEASE) || !defined(COPYRIGHT_YEAR)
#error Client version information missing: version is not defined by dms-config.h or in any other way
#endif

/**
 Release code names 
  1.  Alice    0.12.9   26 Aug 2018
  2.  Bob      0.12.10  13 Nov 2018
  3.  Carol    0.12.11   4 Jan 2019
  4.  Dave     0.12.12  15 Apr 2019
  5.  Erin     0.12.13  16 Aug 2019
  6.  Faythe   0.12.14  29 Nov 2019
  7.  Grace    0.12.15  17 Apr 2020
  8.  Heidi    0.12.16   8 Oct 2020
  9.  Ivan     0.13.3    3 May 2021
  10. Judy     0.13.4    1 Feb 2022
  11. Kenji    0.17.0   14 May 2022
*/
#define RELEASE_CODE_NAME "Kenji"

/**
 * Converts the parameter X to a string after macro replacement on X has been performed.
 * Don't merge these into one macro!
 */
#define STRINGIZE(X) DO_STRINGIZE(X)
#define DO_STRINGIZE(X) #X

//! Copyright string used in Windows .rc files
#define COPYRIGHT_STR "2009-" STRINGIZE(COPYRIGHT_YEAR) " The Bitcoin Core Developers, 2014-" STRINGIZE(COPYRIGHT_YEAR) " " COPYRIGHT_HOLDERS_FINAL

/**
 * dmsd-res.rc includes this file, but it cannot cope with real c++ code.
 * WINDRES_PREPROC is defined to indicate that its pre-processor is running.
 * Anything other than a define should be guarded below.
 */

#if !defined(WINDRES_PREPROC)

#include <string>
#include <vector>

static const int CLIENT_VERSION =
                           1000000 * CLIENT_VERSION_MAJOR
                         +   10000 * CLIENT_VERSION_MINOR
                         +     100 * CLIENT_VERSION_REVISION
                         +       1 * CLIENT_VERSION_BUILD;

extern const std::string CLIENT_NAME;
extern const std::string CLIENT_BUILD;

std::string FormatVersion(int nVersion);
std::string FormatFullVersion();
std::string FormatSubVersion(const std::string& name, int nClientVersion, const std::vector<std::string>& comments);

#endif // WINDRES_PREPROC

#endif // BITCOIN_CLIENTVERSION_H
