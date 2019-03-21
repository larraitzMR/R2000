/*
 *****************************************************************************
 *                                                                           *
 *                 IMPINJ CONFIDENTIAL AND PROPRIETARY                       *
 *                                                                           *
 * This source code is the sole property of Impinj, Inc.  Reproduction or    *
 * utilization of this source code in whole or in part is forbidden without  *
 * the prior written consent of Impinj, Inc.                                 *
 *                                                                           *
 * (c) Copyright Impinj, Inc. 2009. All rights reserved.                     *
 *                                                                           *
 *****************************************************************************
 */

/*
 *****************************************************************************
 *
 * $Id: rfid_version.h 80881 2012-05-03 17:04:06Z dciampi $
 *
 * Description:
 *
 *****************************************************************************
 */

#ifndef RFID_VERSION_H_INCLUDED
#define RFID_VERSION_H_INCLUDED

#define VER_FILESUBTYPE         0
#define VER_FILEOS              VOS_NT

#define VER_COMPANYNAME_STR     "Impinj, Inc.\0"
#define VER_LEGALCOPYRIGHT_STR  "Copyright (c) 2009, Impinj Inc.  All rights reserved.\0"

#define VER_SYSTEMNAME_STR      "\0"
#define VER_PRIVATEBLD_STR      "\0"
#define VER_SPECIALBLD_STR      "\0"
#define VER_COMMENTS_STR        "\0"

// NOTE:  This version should be kept in synch with the csharp AssemblyVersionInfo.cs property
//        file version string, until we find a way to synch these two programatically.
//
#define VER_MAJOR_NUM           2
#define VER_MINOR_NUM           6
#define VER_MAINTENANCE_NUM     0
#define VER_RELEASE_NUM         240




/* Support macros for representing the version numbers as strings */
#define STR(_X_) #_X_
#define MACRONAMETOVALUE(NAME) STR(NAME)

#define VER_MAJOR_STRING        MACRONAMETOVALUE(VER_MAJOR_NUM)
#define VER_MINOR_STRING        MACRONAMETOVALUE(VER_MINOR_NUM)
#define VER_MAINTENANCE_STRING  MACRONAMETOVALUE(VER_MAINTENANCE_NUM)
#define VER_RELEASE_STRING      MACRONAMETOVALUE(VER_RELEASE_NUM)


#define VER_LEGALTRADEMARKS_STR "\0"
#define VER_PRODUCTNAME_STR     "Indy RFID Reader SDK\0"
#define VER_FILEVERSION         VER_MAJOR_NUM,VER_MINOR_NUM,VER_MAINTENANCE_NUM,VER_RELEASE_NUM
#define VER_PRODUCTVERSION      VER_FILEVERSION

#if defined(DEBUG) || defined(_DEBUG) || defined(DBG)
    #define VER_FILEVERSION_STR     VER_MAJOR_STRING "." VER_MINOR_STRING "." VER_MAINTENANCE_STRING "." VER_RELEASE_STRING " (Debug)\0"
    #define VER_PRODUCTVERSION_STR  VER_FILEVERSION_STR
    #define VER_DEBUG               VS_FF_DEBUG
    #define VER_FILEFLAGS           VS_FF_DEBUG
    #define VER_FILEFLAGSMASK       (VS_FF_DEBUG|VS_FF_INFOINFERRED|VS_FF_PRERELEASE)
    #define VER_TESTING_STR         "Debug\0"
#else
    #define VER_FILEVERSION_STR     VER_MAJOR_STRING "." VER_MINOR_STRING "." VER_MAINTENANCE_STRING "." VER_RELEASE_STRING "\0"
    #define VER_PRODUCTVERSION_STR  VER_FILEVERSION_STR
    #define VER_DEBUG               0
    #define VER_FILEFLAGS           0x0L
    #define VER_FILEFLAGSMASK       (VS_FF_INFOINFERRED|VS_FF_PRERELEASE)
    #define VER_TESTING_STR         "\0"
#endif


#define VER_IDENT_STRING "$" "Id: v" VER_MAJOR_STRING "." VER_MINOR_STRING "." VER_MAINTENANCE_STRING \
        "." VER_RELEASE_STRING " " __DATE__  " " __TIME__ " $"

#endif
