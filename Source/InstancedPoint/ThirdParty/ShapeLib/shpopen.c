/******************************************************************************
 * $Id: shpopen.c,v 1.75 2016-12-05 12:44:05 erouault Exp $
 *
 * Project:  Shapelib
 * Purpose:  Implementation of core Shapefile read/write functions.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, 2001, Frank Warmerdam
 * Copyright (c) 2011-2013, Even Rouault <even dot rouault at mines-paris dot org>
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see COPYING).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: shpopen.c,v $
 * Revision 1.75  2016-12-05 12:44:05  erouault
 * * Major overhaul of Makefile build system to use autoconf/automake.
 *
 * * Warning fixes in contrib/
 *
 * Revision 1.74  2016-12-04 15:30:15  erouault
 * * shpopen.c, dbfopen.c, shptree.c, shapefil.h: resync with
 * GDAL Shapefile driver. Mostly cleanups. SHPObject and DBFInfo
 * structures extended with new members. New functions:
 * DBFSetLastModifiedDate, SHPOpenLLEx, SHPRestoreSHX,
 * SHPSetFastModeReadObject
 *
 * * sbnsearch.c: new file to implement original ESRI .sbn spatial
 * index reading. (no write support). New functions:
 * SBNOpenDiskTree, SBNCloseDiskTree, SBNSearchDiskTree,
 * SBNSearchDiskTreeInteger, SBNSearchFreeIds
 *
 * * Makefile, makefile.vc, CMakeLists.txt, shapelib.def: updates
 * with new file and symbols.
 *
 * * commit: helper script to cvs commit
 *
 * Revision 1.73  2012-01-24 22:33:01  fwarmerdam
 * fix memory leak on failure to open .shp (gdal #4410)
 *
 * Revision 1.72  2011-12-11 22:45:28  fwarmerdam
 * fix failure return from SHPOpenLL.
 *
 * Revision 1.71  2011-09-15 03:33:58  fwarmerdam
 * fix missing cast (#2344)
 *
 * Revision 1.70  2011-07-24 05:59:25  fwarmerdam
 * minimize use of CPLError in favor of SAHooks.Error()
 *
 * Revision 1.69  2011-07-24 03:24:22  fwarmerdam
 * fix memory leaks in error cases creating shapefiles (#2061)
 *
 * Revision 1.68  2010-08-27 23:42:52  fwarmerdam
 * add SHPAPI_CALL attribute in code
 *
 * Revision 1.67  2010-07-01 08:15:48  fwarmerdam
 * do not error out on an object with zero vertices
 *
 * Revision 1.66  2010-07-01 07:58:57  fwarmerdam
 * minor cleanup of error handling
 *
 * Revision 1.65  2010-07-01 07:27:13  fwarmerdam
 * white space formatting adjustments
 *
 * Revision 1.64  2010-01-28 11:34:34  fwarmerdam
 * handle the shape file length limits more gracefully (#3236)
 *
 * Revision 1.63  2010-01-28 04:04:40  fwarmerdam
 * improve numerical accuracy of SHPRewind() algs (gdal #3363)
 *
 * Revision 1.62  2010-01-17 05:34:13  fwarmerdam
 * Remove asserts on x/y being null (#2148).
 *
 * Revision 1.61  2010-01-16 05:07:42  fwarmerdam
 * allow 0/nulls in shpcreateobject (#2148)
 *
 * Revision 1.60  2009-09-17 20:50:02  bram
 * on Win32, define snprintf as alias to _snprintf
 *
 * Revision 1.59  2008-03-14 05:25:31  fwarmerdam
 * Correct crash on buggy geometries (gdal #2218)
 *
 * Revision 1.58  2008/01/08 23:28:26  bram
 * on line 2095, use a float instead of a double to avoid a compiler warning
 *
 * Revision 1.57  2007/12/06 07:00:25  fwarmerdam
 * dbfopen now using SAHooks for fileio
 *
 * Revision 1.56  2007/12/04 20:37:56  fwarmerdam
 * preliminary implementation of hooks api for io and errors
 *
 * Revision 1.55  2007/11/21 22:39:56  fwarmerdam
 * close shx file in readonly mode (GDAL #1956)
 *
 * Revision 1.54  2007/11/15 00:12:47  mloskot
 * Backported recent changes from GDAL (Ticket #1415) to Shapelib.
 *
 * Revision 1.53  2007/11/14 22:31:08  fwarmerdam
 * checks after mallocs to detect for corrupted/voluntary broken shapefiles.
 * http://trac.osgeo.org/gdal/ticket/1991
 *
 * Revision 1.52  2007/06/21 15:58:33  fwarmerdam
 * fix for SHPRewindObject when rings touch at one vertex (gdal #976)
 *
 * Revision 1.51  2006/09/04 15:24:01  fwarmerdam
 * Fixed up log message for 1.49.
 *
 * Revision 1.50  2006/09/04 15:21:39  fwarmerdam
 * fix of last fix
 *
 * Revision 1.49  2006/09/04 15:21:00  fwarmerdam
 * MLoskot: Added stronger test of Shapefile reading failures, e.g. truncated
 * files.  The problem was discovered by Tim Sutton and reported here
 *   https://svn.qgis.org/trac/ticket/200
 *
 * Revision 1.48  2006/01/26 15:07:32  fwarmerdam
 * add bMeasureIsUsed flag from Craig Bruce: Bug 1249
 *
 * Revision 1.47  2006/01/04 20:07:23  fwarmerdam
 * In SHPWriteObject() make sure that the record length is updated
 * when rewriting an existing record.
 *
 * Revision 1.46  2005/02/11 17:17:46  fwarmerdam
 * added panPartStart[0] validation
 *
 * Revision 1.45  2004/09/26 20:09:48  fwarmerdam
 * const correctness changes
 *
 * Revision 1.44  2003/12/29 00:18:39  fwarmerdam
 * added error checking for failed IO and optional CPL error reporting
 *
 * Revision 1.43  2003/12/01 16:20:08  warmerda
 * be careful of zero vertex shapes
 *
 * Revision 1.42  2003/12/01 14:58:27  warmerda
 * added degenerate object check in SHPRewindObject()
 *
 * Revision 1.41  2003/07/08 15:22:43  warmerda
 * avoid warning
 *
 * Revision 1.40  2003/04/21 18:30:37  warmerda
 * added header write/update public methods
 *
 * Revision 1.39  2002/08/26 06:46:56  warmerda
 * avoid c++ comments
 *
 * Revision 1.38  2002/05/07 16:43:39  warmerda
 * Removed debugging printf.
 *
 * Revision 1.37  2002/04/10 17:35:22  warmerda
 * fixed bug in ring reversal code
 *
 * Revision 1.36  2002/04/10 16:59:54  warmerda
 * added SHPRewindObject
 *
 * Revision 1.35  2001/12/07 15:10:44  warmerda
 * fix if .shx fails to open
 *
 * Revision 1.34  2001/11/01 16:29:55  warmerda
 * move pabyRec into SHPInfo for thread safety
 *
 * Revision 1.33  2001/07/03 12:18:15  warmerda
 * Improved cleanup if SHX not found, provided by Riccardo Cohen.
 *
 * Revision 1.32  2001/06/22 01:58:07  warmerda
 * be more careful about establishing initial bounds in face of NULL shapes
 *
 * Revision 1.31  2001/05/31 19:35:29  warmerda
 * added support for writing null shapes
 *
 * Revision 1.30  2001/05/28 12:46:29  warmerda
 * Add some checking on reasonableness of record count when opening.
 *
 * Revision 1.29  2001/05/23 13:36:52  warmerda
 * added use of SHPAPI_CALL
 *
 * Revision 1.28  2001/02/06 22:25:06  warmerda
 * fixed memory leaks when SHPOpen() fails
 *
 * Revision 1.27  2000/07/18 15:21:33  warmerda
 * added better enforcement of -1 for append in SHPWriteObject
 *
 * Revision 1.26  2000/02/16 16:03:51  warmerda
 * added null shape support
 *
 * Revision 1.25  1999/12/15 13:47:07  warmerda
 * Fixed record size settings in .shp file (was 4 words too long)
 * Added stdlib.h.
 *
 * Revision 1.24  1999/11/05 14:12:04  warmerda
 * updated license terms
 *
 * Revision 1.23  1999/07/27 00:53:46  warmerda
 * added support for rewriting shapes
 *
 * Revision 1.22  1999/06/11 19:19:11  warmerda
 * Cleanup pabyRec static buffer on SHPClose().
 *
 * Revision 1.21  1999/06/02 14:57:56  kshih
 * Remove unused variables
 *
 * Revision 1.20  1999/04/19 21:04:17  warmerda
 * Fixed syntax error.
 *
 * Revision 1.19  1999/04/19 21:01:57  warmerda
 * Force access string to binary in SHPOpen().
 *
 * Revision 1.18  1999/04/01 18:48:07  warmerda
 * Try upper case extensions if lower case doesn't work.
 *
 * Revision 1.17  1998/12/31 15:29:39  warmerda
 * Disable writing measure values to multipatch objects if
 * DISABLE_MULTIPATCH_MEASURE is defined.
 *
 * Revision 1.16  1998/12/16 05:14:33  warmerda
 * Added support to write MULTIPATCH.  Fixed reading Z coordinate of
 * MULTIPATCH. Fixed record size written for all feature types.
 *
 * Revision 1.15  1998/12/03 16:35:29  warmerda
 * r+b is proper binary access string, not rb+.
 *
 * Revision 1.14  1998/12/03 15:47:56  warmerda
 * Fixed setting of nVertices in SHPCreateObject().
 *
 * Revision 1.13  1998/12/03 15:33:54  warmerda
 * Made SHPCalculateExtents() separately callable.
 *
 * Revision 1.12  1998/11/11 20:01:50  warmerda
 * Fixed bug writing ArcM/Z, and PolygonM/Z for big endian machines.
 *
 * Revision 1.11  1998/11/09 20:56:44  warmerda
 * Fixed up handling of file wide bounds.
 *
 * Revision 1.10  1998/11/09 20:18:51  warmerda
 * Converted to support 3D shapefiles, and use of SHPObject.
 *
 * Revision 1.9  1998/02/24 15:09:05  warmerda
 * Fixed memory leak.
 *
 * Revision 1.8  1997/12/04 15:40:29  warmerda
 * Fixed byte swapping of record number, and record length fields in the
 * .shp file.
 *
 * Revision 1.7  1995/10/21 03:15:58  warmerda
 * Added support for binary file access, the magic cookie 9997
 * and tried to improve the int32 selection logic for 16bit systems.
 *
 * Revision 1.6  1995/09/04  04:19:41  warmerda
 * Added fix for file bounds.
 *
 * Revision 1.5  1995/08/25  15:16:44  warmerda
 * Fixed a couple of problems with big endian systems ... one with bounds
 * and the other with multipart polygons.
 *
 * Revision 1.4  1995/08/24  18:10:17  warmerda
 * Switch to use SfRealloc() to avoid problems with pre-ANSI realloc()
 * functions (such as on the Sun).
 *
 * Revision 1.3  1995/08/23  02:23:15  warmerda
 * Added support for reading bounds, and fixed up problems in setting the
 * file wide bounds.
 *
 * Revision 1.2  1995/08/04  03:16:57  warmerda
 * Added header.
 *
 */

#include "shapefil.h"

#include <math.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>

SHP_CVSID("$Id: shpopen.c,v 1.75 2016-12-05 12:44:05 erouault Exp $")

typedef unsigned char uchar;

#if UINT_MAX == 65535
typedef unsigned long	      int32;
#else
typedef unsigned int	      int32;
#endif

#ifndef FALSE
#  define FALSE		0
#  define TRUE		1
#endif

#define ByteCopy( a, b, c )	memcpy( b, a, c )
#ifndef MAX
#  define MIN(a,b)      ((a<b) ? a : b)
#  define MAX(a,b)      ((a>b) ? a : b)
#endif

#ifndef USE_CPL
#if defined(_MSC_VER)
# if _MSC_VER < 1900
#     define snprintf _snprintf
# endif
#elif defined(WIN32) || defined(_WIN32)
#  ifndef snprintf
#     define snprintf _snprintf
#  endif
#endif
#endif

#ifndef CPL_UNUSED
#if defined(__GNUC__) && __GNUC__ >= 4
#  define CPL_UNUSED __attribute((__unused__))
#else
#  define CPL_UNUSED
#endif
#endif  

#if defined(CPL_LSB)
#define bBigEndian FALSE
#elif defined(CPL_MSB)
#define bBigEndian TRUE
#else
static int 	bBigEndian;
#endif

/************************************************************************/
/*                              SwapWord()                              */
/*                                                                      */
/*      Swap a 2, 4 or 8 byte word.                                     */
/************************************************************************/

static void	SwapWord( int length, void * wordP )

{
    int		i;
    uchar	temp;

    for( i=0; i < length/2; i++ )
    {
	temp = ((uchar *) wordP)[i];
	((uchar *)wordP)[i] = ((uchar *) wordP)[length-i-1];
	((uchar *) wordP)[length-i-1] = temp;
    }
}

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/

static void * SfRealloc( void * pMem, int nNewSize )

{
    if( pMem == NULL )
        return( (void *) malloc(nNewSize) );
    else
        return( (void *) realloc(pMem,nNewSize) );
}

/************************************************************************/
/*                          SHPWriteHeader()                            */
/*                                                                      */
/*      Write out a header for the .shp and .shx files as well as the	*/
/*	contents of the index (.shx) file.				*/
/************************************************************************/

void SHPAPI_CALL SHPWriteHeader( SHPHandle psSHP )

{
    uchar     	abyHeader[100] = { 0 };
    int		i;
    int32	i32;
    double	dValue;
    int32	*panSHX;

    if (psSHP->fpSHX == NULL)
    {
        psSHP->sHooks.Error( "SHPWriteHeader failed : SHX file is closed");
        return;
    }

/* -------------------------------------------------------------------- */
/*      Prepare header block for .shp file.                             */
/* -------------------------------------------------------------------- */

    abyHeader[2] = 0x27;				/* magic cookie */
    abyHeader[3] = 0x0a;

    i32 = psSHP->nFileSize/2;				/* file size */
    ByteCopy( &i32, abyHeader+24, 4 );
    if( !bBigEndian ) SwapWord( 4, abyHeader+24 );

    i32 = 1000;						/* version */
    ByteCopy( &i32, abyHeader+28, 4 );
    if( bBigEndian ) SwapWord( 4, abyHeader+28 );

    i32 = psSHP->nShapeType;				/* shape type */
    ByteCopy( &i32, abyHeader+32, 4 );
    if( bBigEndian ) SwapWord( 4, abyHeader+32 );

    dValue = psSHP->adBoundsMin[0];			/* set bounds */
    ByteCopy( &dValue, abyHeader+36, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+36 );

    dValue = psSHP->adBoundsMin[1];
    ByteCopy( &dValue, abyHeader+44, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+44 );

    dValue = psSHP->adBoundsMax[0];
    ByteCopy( &dValue, abyHeader+52, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+52 );

    dValue = psSHP->adBoundsMax[1];
    ByteCopy( &dValue, abyHeader+60, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+60 );

    dValue = psSHP->adBoundsMin[2];			/* z */
    ByteCopy( &dValue, abyHeader+68, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+68 );

    dValue = psSHP->adBoundsMax[2];
    ByteCopy( &dValue, abyHeader+76, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+76 );

    dValue = psSHP->adBoundsMin[3];			/* m */
    ByteCopy( &dValue, abyHeader+84, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+84 );

    dValue = psSHP->adBoundsMax[3];
    ByteCopy( &dValue, abyHeader+92, 8 );
    if( bBigEndian ) SwapWord( 8, abyHeader+92 );

/* -------------------------------------------------------------------- */
/*      Write .shp file header.                                         */
/* -------------------------------------------------------------------- */
    if( psSHP->sHooks.FSeek( psSHP->fpSHP, 0, 0 ) != 0
        || psSHP->sHooks.FWrite( abyHeader, 100, 1, psSHP->fpSHP ) != 1 )
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                 "Failure writing .shp header: %s", strerror(errno) );
        psSHP->sHooks.Error( szError );
        return;
    }

/* -------------------------------------------------------------------- */
/*      Prepare, and write .shx file header.                            */
/* -------------------------------------------------------------------- */
    i32 = (psSHP->nRecords * 2 * sizeof(int32) + 100)/2;   /* file size */
    ByteCopy( &i32, abyHeader+24, 4 );
    if( !bBigEndian ) SwapWord( 4, abyHeader+24 );

    if( psSHP->sHooks.FSeek( psSHP->fpSHX, 0, 0 ) != 0
        || psSHP->sHooks.FWrite( abyHeader, 100, 1, psSHP->fpSHX ) != 1 )
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                 "Failure writing .shx header: %s", strerror(errno) );
        psSHP->sHooks.Error( szError );

        return;
    }

/* -------------------------------------------------------------------- */
/*      Write out the .shx contents.                                    */
/* -------------------------------------------------------------------- */
    panSHX = (int32 *) malloc(sizeof(int32) * 2 * psSHP->nRecords);
    if( panSHX == NULL )
    {
        psSHP->sHooks.Error( "Failure allocatin panSHX" );
        return;
    }

    for( i = 0; i < psSHP->nRecords; i++ )
    {
        panSHX[i*2  ] = psSHP->panRecOffset[i]/2;
        panSHX[i*2+1] = psSHP->panRecSize[i]/2;
        if( !bBigEndian ) SwapWord( 4, panSHX+i*2 );
        if( !bBigEndian ) SwapWord( 4, panSHX+i*2+1 );
    }

    if( (int)psSHP->sHooks.FWrite( panSHX, sizeof(int32)*2, psSHP->nRecords, psSHP->fpSHX )
        != psSHP->nRecords )
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                 "Failure writing .shx contents: %s", strerror(errno) );
        psSHP->sHooks.Error( szError );
    }

    free( panSHX );

/* -------------------------------------------------------------------- */
/*      Flush to disk.                                                  */
/* -------------------------------------------------------------------- */
    psSHP->sHooks.FFlush( psSHP->fpSHP );
    psSHP->sHooks.FFlush( psSHP->fpSHX );
}

/************************************************************************/
/*                              SHPOpen()                               */
/************************************************************************/

SHPHandle SHPAPI_CALL
SHPOpen( const char * pszLayer, const char * pszAccess )

{
    SAHooks sHooks;

    SASetupDefaultHooks( &sHooks );

    return SHPOpenLL( pszLayer, pszAccess, &sHooks );
}

/************************************************************************/
/*                              SHPOpen()                               */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name.                                      */
/************************************************************************/

SHPHandle SHPAPI_CALL
SHPOpenLL( const char * pszLayer, const char * pszAccess, SAHooks *psHooks )

{
    char        *pszFullname, *pszBasename;
    SHPHandle       psSHP;

    uchar       *pabyBuf;
    int         i;
    double      dValue;
    int         bLazySHXLoading = FALSE;
    size_t nFullnameLen;

/* -------------------------------------------------------------------- */
/*      Ensure the access string is one of the legal ones.  We          */
/*      ensure the result string indicates binary to avoid common       */
/*      problems on Windows.                                            */
/* -------------------------------------------------------------------- */
    if( strcmp(pszAccess,"rb+") == 0 || strcmp(pszAccess,"r+b") == 0
        || strcmp(pszAccess,"r+") == 0 )
        pszAccess = "r+b";
    else
    {
        bLazySHXLoading = strchr(pszAccess, 'l') != NULL;
        pszAccess = "rb";
    }

/* -------------------------------------------------------------------- */
/*  Establish the byte order on this machine.           */
/* -------------------------------------------------------------------- */
#if !defined(bBigEndian)
    i = 1;
    if( *((uchar *) &i) == 1 )
        bBigEndian = FALSE;
    else
        bBigEndian = TRUE;
#endif

/* -------------------------------------------------------------------- */
/*  Initialize the info structure.                  */
/* -------------------------------------------------------------------- */
    psSHP = (SHPHandle) calloc(sizeof(SHPInfo),1);

    psSHP->bUpdated = FALSE;
    memcpy( &(psSHP->sHooks), psHooks, sizeof(SAHooks) );

/* -------------------------------------------------------------------- */
/*  Compute the base (layer) name.  If there is any extension   */
/*  on the passed in filename we will strip it off.         */
/* -------------------------------------------------------------------- */
    pszBasename = (char *) malloc(strlen(pszLayer)+5);
    strcpy( pszBasename, pszLayer );
    for( i = (int)strlen(pszBasename)-1;
         i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
             && pszBasename[i] != '\\';
         i-- ) {}

    if( pszBasename[i] == '.' )
        pszBasename[i] = '\0';

/* -------------------------------------------------------------------- */
/*  Open the .shp and .shx files.  Note that files pulled from  */
/*  a PC to Unix with upper case filenames won't work!      */
/* -------------------------------------------------------------------- */
    nFullnameLen = strlen(pszBasename) + 5;
    pszFullname = (char *) malloc(nFullnameLen);
    snprintf( pszFullname, nFullnameLen, "%s.shp", pszBasename ) ;
    psSHP->fpSHP = psSHP->sHooks.FOpen(pszFullname, pszAccess );
    if( psSHP->fpSHP == NULL )
    {
        snprintf( pszFullname, nFullnameLen, "%s.SHP", pszBasename );
        psSHP->fpSHP = psSHP->sHooks.FOpen(pszFullname, pszAccess );
    }

    if( psSHP->fpSHP == NULL )
    {
        size_t nMessageLen = strlen(pszBasename)*2+256;
        char *pszMessage = (char *) malloc(nMessageLen);
        snprintf( pszMessage, nMessageLen, "Unable to open %s.shp or %s.SHP.",
                  pszBasename, pszBasename );
        psHooks->Error( pszMessage );
        free( pszMessage );

        free( psSHP );
        free( pszBasename );
        free( pszFullname );

        return NULL;
    }

    snprintf( pszFullname, nFullnameLen, "%s.shx", pszBasename );
    psSHP->fpSHX =  psSHP->sHooks.FOpen(pszFullname, pszAccess );
    if( psSHP->fpSHX == NULL )
    {
        snprintf( pszFullname, nFullnameLen, "%s.SHX", pszBasename );
        psSHP->fpSHX = psSHP->sHooks.FOpen(pszFullname, pszAccess );
    }

    if( psSHP->fpSHX == NULL )
    {
        size_t nMessageLen = strlen(pszBasename)*2+256;
        char *pszMessage = (char *) malloc(nMessageLen);
        snprintf( pszMessage, nMessageLen, "Unable to open %s.shx or %s.SHX."
                  "Try --config SHAPE_RESTORE_SHX true to restore or create it",
                  pszBasename, pszBasename );
        psHooks->Error( pszMessage );
        free( pszMessage );

        psSHP->sHooks.FClose( psSHP->fpSHP );
        free( psSHP );
        free( pszBasename );
        free( pszFullname );
        return( NULL );
    }

    free( pszFullname );
    free( pszBasename );

/* -------------------------------------------------------------------- */
/*  Read the file size from the SHP file.               */
/* -------------------------------------------------------------------- */
    pabyBuf = (uchar *) malloc(100);
    psSHP->sHooks.FRead( pabyBuf, 100, 1, psSHP->fpSHP );

    psSHP->nFileSize = ((unsigned int)pabyBuf[24] * 256 * 256 * 256
                        + (unsigned int)pabyBuf[25] * 256 * 256
                        + (unsigned int)pabyBuf[26] * 256
                        + (unsigned int)pabyBuf[27]);
    if( psSHP->nFileSize < 0xFFFFFFFFU / 2 )
        psSHP->nFileSize *= 2;
    else
        psSHP->nFileSize = 0xFFFFFFFEU;

/* -------------------------------------------------------------------- */
/*  Read SHX file Header info                                           */
/* -------------------------------------------------------------------- */
    if( psSHP->sHooks.FRead( pabyBuf, 100, 1, psSHP->fpSHX ) != 1
        || pabyBuf[0] != 0
        || pabyBuf[1] != 0
        || pabyBuf[2] != 0x27
        || (pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d) )
    {
        psSHP->sHooks.Error( ".shx file is unreadable, or corrupt." );
        psSHP->sHooks.FClose( psSHP->fpSHP );
        psSHP->sHooks.FClose( psSHP->fpSHX );
        free( psSHP );

        return( NULL );
    }

    psSHP->nRecords = pabyBuf[27] + pabyBuf[26] * 256
        + pabyBuf[25] * 256 * 256 + (pabyBuf[24] & 0x7F) * 256 * 256 * 256;
    psSHP->nRecords = (psSHP->nRecords - 50) / 4;

    psSHP->nShapeType = pabyBuf[32];

    if( psSHP->nRecords < 0 || psSHP->nRecords > 256000000 )
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                 "Record count in .shp header is %d, which seems\n"
                 "unreasonable.  Assuming header is corrupt.",
                 psSHP->nRecords );
        psSHP->sHooks.Error( szError );
        psSHP->sHooks.FClose( psSHP->fpSHP );
        psSHP->sHooks.FClose( psSHP->fpSHX );
        free( psSHP );
        free(pabyBuf);

        return( NULL );
    }

    /* If a lot of records are advertized, check that the file is big enough */
    /* to hold them */
    if( psSHP->nRecords >= 1024 * 1024 )
    {
        SAOffset nFileSize;
        psSHP->sHooks.FSeek( psSHP->fpSHX, 0, 2 );
        nFileSize = psSHP->sHooks.FTell( psSHP->fpSHX );
        if( nFileSize > 100 &&
            nFileSize/2 < (SAOffset)(psSHP->nRecords * 4 + 50) )
        {
            psSHP->nRecords = (int)((nFileSize - 100) / 8);
        }
        psSHP->sHooks.FSeek( psSHP->fpSHX, 100, 0 );
    }

/* -------------------------------------------------------------------- */
/*      Read the bounds.                                                */
/* -------------------------------------------------------------------- */
    if( bBigEndian ) SwapWord( 8, pabyBuf+36 );
    memcpy( &dValue, pabyBuf+36, 8 );
    psSHP->adBoundsMin[0] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+44 );
    memcpy( &dValue, pabyBuf+44, 8 );
    psSHP->adBoundsMin[1] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+52 );
    memcpy( &dValue, pabyBuf+52, 8 );
    psSHP->adBoundsMax[0] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+60 );
    memcpy( &dValue, pabyBuf+60, 8 );
    psSHP->adBoundsMax[1] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+68 );     /* z */
    memcpy( &dValue, pabyBuf+68, 8 );
    psSHP->adBoundsMin[2] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+76 );
    memcpy( &dValue, pabyBuf+76, 8 );
    psSHP->adBoundsMax[2] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+84 );     /* z */
    memcpy( &dValue, pabyBuf+84, 8 );
    psSHP->adBoundsMin[3] = dValue;

    if( bBigEndian ) SwapWord( 8, pabyBuf+92 );
    memcpy( &dValue, pabyBuf+92, 8 );
    psSHP->adBoundsMax[3] = dValue;

    free( pabyBuf );

/* -------------------------------------------------------------------- */
/*  Read the .shx file to get the offsets to each record in     */
/*  the .shp file.                          */
/* -------------------------------------------------------------------- */
    psSHP->nMaxRecords = psSHP->nRecords;

    psSHP->panRecOffset = (unsigned int *)
        malloc(sizeof(unsigned int) * MAX(1,psSHP->nMaxRecords) );
    psSHP->panRecSize = (unsigned int *)
        malloc(sizeof(unsigned int) * MAX(1,psSHP->nMaxRecords) );
    if( bLazySHXLoading )
        pabyBuf = NULL;
    else
        pabyBuf = (uchar *) malloc(8 * MAX(1,psSHP->nRecords) );

    if (psSHP->panRecOffset == NULL ||
        psSHP->panRecSize == NULL ||
        (!bLazySHXLoading && pabyBuf == NULL))
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                "Not enough memory to allocate requested memory (nRecords=%d).\n"
                "Probably broken SHP file",
                psSHP->nRecords );
        psSHP->sHooks.Error( szError );
        psSHP->sHooks.FClose( psSHP->fpSHP );
        psSHP->sHooks.FClose( psSHP->fpSHX );
        if (psSHP->panRecOffset) free( psSHP->panRecOffset );
        if (psSHP->panRecSize) free( psSHP->panRecSize );
        if (pabyBuf) free( pabyBuf );
        free( psSHP );
        return( NULL );
    }

    if( bLazySHXLoading )
    {
        memset(psSHP->panRecOffset, 0, sizeof(unsigned int) * MAX(1,psSHP->nMaxRecords) );
        memset(psSHP->panRecSize, 0, sizeof(unsigned int) * MAX(1,psSHP->nMaxRecords) );
        return( psSHP );
    }

    if( (int) psSHP->sHooks.FRead( pabyBuf, 8, psSHP->nRecords, psSHP->fpSHX )
        != psSHP->nRecords )
    {
        char szError[200];

        snprintf( szError, sizeof(szError),
                 "Failed to read all values for %d records in .shx file: %s.",
                 psSHP->nRecords, strerror(errno) );
        psSHP->sHooks.Error( szError );

        /* SHX is short or unreadable for some reason. */
        psSHP->sHooks.FClose( psSHP->fpSHP );
        psSHP->sHooks.FClose( psSHP->fpSHX );
        free( psSHP->panRecOffset );
        free( psSHP->panRecSize );
        free( pabyBuf );
        free( psSHP );

        return( NULL );
    }

    /* In read-only mode, we can close the SHX now */
    if (strcmp(pszAccess, "rb") == 0)
    {
        psSHP->sHooks.FClose( psSHP->fpSHX );
        psSHP->fpSHX = NULL;
    }

    for( i = 0; i < psSHP->nRecords; i++ )
    {
        unsigned int        nOffset, nLength;

        memcpy( &nOffset, pabyBuf + i * 8, 4 );
        if( !bBigEndian ) SwapWord( 4, &nOffset );

        memcpy( &nLength, pabyBuf + i * 8 + 4, 4 );
        if( !bBigEndian ) SwapWord( 4, &nLength );

        if( nOffset > (unsigned int)INT_MAX )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Invalid offset for entity %d", i);

            psSHP->sHooks.Error( str );
            SHPClose(psSHP);
            free( pabyBuf );
            return NULL;
        }
        if( nLength > (unsigned int)(INT_MAX / 2 - 4) )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Invalid length for entity %d", i);

            psSHP->sHooks.Error( str );
            SHPClose(psSHP);
            free( pabyBuf );
            return NULL;
        }
        psSHP->panRecOffset[i] = nOffset*2;
        psSHP->panRecSize[i] = nLength*2;
    }
    free( pabyBuf );

    return( psSHP );
}

/************************************************************************/
/*                              SHPOpenLLEx()                           */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name. It generally invokes SHPRestoreSHX() */
/*      in case when bRestoreSHX equals true.                           */
/************************************************************************/

SHPHandle SHPAPI_CALL
SHPOpenLLEx( const char * pszLayer, const char * pszAccess, SAHooks *psHooks,
            int bRestoreSHX )

{
    if ( !bRestoreSHX ) return SHPOpenLL ( pszLayer, pszAccess, psHooks );
    else
    {
        if ( SHPRestoreSHX ( pszLayer, pszAccess, psHooks ) )
        {
            return SHPOpenLL ( pszLayer, pszAccess, psHooks );
        }
    }

    return( NULL );
}

/************************************************************************/
/*                              SHPRestoreSHX()                         */
/*                                                                      */
/*      Restore .SHX file using associated .SHP file.                   */
/*                                                                      */
/************************************************************************/

int       SHPAPI_CALL
SHPRestoreSHX ( const char * pszLayer, const char * pszAccess, SAHooks *psHooks )

{
    char            *pszFullname, *pszBasename;
    SAFile          fpSHP, fpSHX;


    uchar           *pabyBuf;
    int             i;
    size_t          nFullnameLen;
    unsigned int    nSHPFilesize;

    size_t          nMessageLen;
    char            *pszMessage;

    unsigned int    nCurrentRecordOffset = 0;
    unsigned int    nCurrentSHPOffset = 100;
    size_t          nRealSHXContentSize = 100;

    const char      pszSHXAccess[] = "w+b";
    char            *pabySHXHeader;
    char            abyReadedRecord[8];
    unsigned int    niRecord = 0;
    unsigned int    nRecordLength = 0;
    unsigned int    nRecordOffset = 50;

/* -------------------------------------------------------------------- */
/*      Ensure the access string is one of the legal ones.  We          */
/*      ensure the result string indicates binary to avoid common       */
/*      problems on Windows.                                            */
/* -------------------------------------------------------------------- */
    if( strcmp(pszAccess,"rb+") == 0 || strcmp(pszAccess,"r+b") == 0
        || strcmp(pszAccess,"r+") == 0 )
        pszAccess = "r+b";
    else
    {
        pszAccess = "rb";
    }

/* -------------------------------------------------------------------- */
/*  Establish the byte order on this machine.                           */
/* -------------------------------------------------------------------- */
#if !defined(bBigEndian)
    i = 1;
    if( *((uchar *) &i) == 1 )
        bBigEndian = FALSE;
    else
        bBigEndian = TRUE;
#endif

/* -------------------------------------------------------------------- */
/*  Compute the base (layer) name.  If there is any extension           */
/*  on the passed in filename we will strip it off.                     */
/* -------------------------------------------------------------------- */
    pszBasename = (char *) malloc(strlen(pszLayer)+5);
    strcpy( pszBasename, pszLayer );
    for( i = (int)strlen(pszBasename)-1;
         i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
             && pszBasename[i] != '\\';
         i-- ) {}

    if( pszBasename[i] == '.' )
        pszBasename[i] = '\0';

/* -------------------------------------------------------------------- */
/*  Open the .shp file.  Note that files pulled from                    */
/*  a PC to Unix with upper case filenames won't work!                  */
/* -------------------------------------------------------------------- */
    nFullnameLen = strlen(pszBasename) + 5;
    pszFullname = (char *) malloc(nFullnameLen);
    snprintf( pszFullname, nFullnameLen, "%s.shp", pszBasename ) ;
    fpSHP = psHooks->FOpen(pszFullname, pszAccess );
    if( fpSHP == NULL )
    {
        snprintf( pszFullname, nFullnameLen, "%s.SHP", pszBasename );
        fpSHP = psHooks->FOpen(pszFullname, pszAccess );
    }

    if( fpSHP == NULL )
    {
        nMessageLen = strlen(pszBasename)*2+256;
        pszMessage = (char *) malloc(nMessageLen);
        snprintf( pszMessage, nMessageLen, "Unable to open %s.shp or %s.SHP.",
                  pszBasename, pszBasename );
        psHooks->Error( pszMessage );
        free( pszMessage );

        free( pszBasename );
        free( pszFullname );

        return( 0 );
    }

/* -------------------------------------------------------------------- */
/*  Read the file size from the SHP file.                               */
/* -------------------------------------------------------------------- */
    pabyBuf = (uchar *) malloc(100);
    psHooks->FRead( pabyBuf, 100, 1, fpSHP );

    nSHPFilesize = ((unsigned int)pabyBuf[24] * 256 * 256 * 256
                        + (unsigned int)pabyBuf[25] * 256 * 256
                        + (unsigned int)pabyBuf[26] * 256
                        + (unsigned int)pabyBuf[27]);
    if( nSHPFilesize < 0xFFFFFFFFU / 2 )
        nSHPFilesize *= 2;
    else
        nSHPFilesize = 0xFFFFFFFEU;

    snprintf( pszFullname, nFullnameLen, "%s.shx", pszBasename );
    fpSHX = psHooks->FOpen( pszFullname, pszSHXAccess );

    if( fpSHX == NULL )
    {
        nMessageLen = strlen( pszBasename ) * 2 + 256;
        pszMessage = (char *) malloc( nMessageLen );
        snprintf( pszMessage, nMessageLen, "Error opening file %s.shx for writing",
                 pszBasename );
        psHooks->Error( pszMessage );
        free( pszMessage );

        psHooks->FClose( fpSHX );

        free( pabyBuf );
        free( pszBasename );
        free( pszFullname );

        return( 0 );
    }

/* -------------------------------------------------------------------- */
/*  Open SHX and create it using SHP file content.                      */
/* -------------------------------------------------------------------- */
    psHooks->FSeek( fpSHP, 100, 0 );
    pabySHXHeader = (char *) malloc ( 100 );
    memcpy( pabySHXHeader, pabyBuf, 100 );
    psHooks->FWrite( pabySHXHeader, 100, 1, fpSHX );

    while( nCurrentSHPOffset < nSHPFilesize )
    {
        if( psHooks->FRead( &niRecord, 4, 1, fpSHP ) == 1 &&
            psHooks->FRead( &nRecordLength, 4, 1, fpSHP ) == 1)
        {
            if( !bBigEndian ) SwapWord( 4, &nRecordOffset );
            memcpy( abyReadedRecord, &nRecordOffset, 4 );
            memcpy( abyReadedRecord + 4, &nRecordLength, 4 );

            psHooks->FWrite( abyReadedRecord, 8, 1, fpSHX );

            if ( !bBigEndian ) SwapWord( 4, &nRecordOffset );
            if ( !bBigEndian ) SwapWord( 4, &nRecordLength );
            nRecordOffset += nRecordLength + 4;
            nCurrentRecordOffset += 8;
            nCurrentSHPOffset += 8 + nRecordLength * 2;

            psHooks->FSeek( fpSHP, nCurrentSHPOffset, 0 );
            nRealSHXContentSize += 8;
        }
        else
        {
            nMessageLen = strlen( pszBasename ) * 2 + 256;
            pszMessage = (char *) malloc( nMessageLen );
            snprintf( pszMessage, nMessageLen, "Error parsing .shp to restore .shx" );
            psHooks->Error( pszMessage );
            free( pszMessage );

            psHooks->FClose( fpSHX );
            psHooks->FClose( fpSHP );

            free( pabySHXHeader );
            free( pszBasename );
            free( pszFullname );

            return( 0 );
        }
    }

    nRealSHXContentSize /= 2; // Bytes counted -> WORDs
    if( !bBigEndian ) SwapWord( 4, &nRealSHXContentSize );
    psHooks->FSeek( fpSHX, 24, 0 );
    psHooks->FWrite( &nRealSHXContentSize, 4, 1, fpSHX );

    psHooks->FClose( fpSHP );
    psHooks->FClose( fpSHX );

    free ( pabyBuf );
    free ( pszFullname );
    free ( pszBasename );
    free ( pabySHXHeader );

    return( 1 );
}

/************************************************************************/
/*                              SHPClose()                              */
/*								       	*/
/*	Close the .shp and .shx files.					*/
/************************************************************************/

void SHPAPI_CALL
SHPClose(SHPHandle psSHP )

{
    if( psSHP == NULL )
        return;

/* -------------------------------------------------------------------- */
/*	Update the header if we have modified anything.			*/
/* -------------------------------------------------------------------- */
    if( psSHP->bUpdated )
	SHPWriteHeader( psSHP );

/* -------------------------------------------------------------------- */
/*      Free all resources, and close files.                            */
/* -------------------------------------------------------------------- */
    free( psSHP->panRecOffset );
    free( psSHP->panRecSize );

    if ( psSHP->fpSHX != NULL)
        psSHP->sHooks.FClose( psSHP->fpSHX );
    psSHP->sHooks.FClose( psSHP->fpSHP );

    if( psSHP->pabyRec != NULL )
    {
        free( psSHP->pabyRec );
    }

    if( psSHP->pabyObjectBuf != NULL )
    {
        free( psSHP->pabyObjectBuf );
    }
    if( psSHP->psCachedObject != NULL )
    {
        free( psSHP->psCachedObject );
    }

    free( psSHP );
}

/************************************************************************/
/*                             SHPGetInfo()                             */
/*                                                                      */
/*      Fetch general information about the shape file.                 */
/************************************************************************/

void SHPAPI_CALL
SHPGetInfo(SHPHandle psSHP, int * pnEntities, int * pnShapeType,
           double * padfMinBound, double * padfMaxBound )

{
    int		i;

    if( psSHP == NULL )
        return;

    if( pnEntities != NULL )
        *pnEntities = psSHP->nRecords;

    if( pnShapeType != NULL )
        *pnShapeType = psSHP->nShapeType;

    for( i = 0; i < 4; i++ )
    {
        if( padfMinBound != NULL )
            padfMinBound[i] = psSHP->adBoundsMin[i];
        if( padfMaxBound != NULL )
            padfMaxBound[i] = psSHP->adBoundsMax[i];
    }
}

/************************************************************************/
/*                         SHPAllocBuffer()                             */
/************************************************************************/

static void* SHPAllocBuffer(unsigned char** pBuffer, int nSize)
{
    unsigned char* pRet;

    if( pBuffer == NULL )
        return calloc(1, nSize);

    pRet = *pBuffer;
    if( pRet == NULL )
        return NULL;

    (*pBuffer) += nSize;
    return pRet;
}

/************************************************************************/
/*                    SHPReallocObjectBufIfNecessary()                  */
/************************************************************************/

static unsigned char* SHPReallocObjectBufIfNecessary ( SHPHandle psSHP,
                                                       int nObjectBufSize )
{
    unsigned char* pBuffer;
    if( nObjectBufSize == 0 )
    {
        nObjectBufSize = 4 * sizeof(double);
    }
    if( nObjectBufSize > psSHP->nObjectBufSize )
    {
        pBuffer = (unsigned char*) realloc( psSHP->pabyObjectBuf, nObjectBufSize );
        if( pBuffer != NULL )
        {
            psSHP->pabyObjectBuf = pBuffer;
            psSHP->nObjectBufSize = nObjectBufSize;
        }
    }
    else
        pBuffer = psSHP->pabyObjectBuf;
    return pBuffer;
}

/************************************************************************/
/*                          SHPReadObject()                             */
/*                                                                      */
/*      Read the vertices, parts, and other non-attribute information	*/
/*	for one shape.							*/
/************************************************************************/

SHPObject SHPAPI_CALL1(*)
SHPReadObject( SHPHandle psSHP, int hEntity )

{
    int                  nEntitySize, nRequiredSize;
    SHPObject           *psShape;
    char                 szErrorMsg[128];
    int                  nSHPType;
    int                  nBytesRead;

/* -------------------------------------------------------------------- */
/*      Validate the record/entity number.                              */
/* -------------------------------------------------------------------- */
    if( hEntity < 0 || hEntity >= psSHP->nRecords )
        return( NULL );

/* -------------------------------------------------------------------- */
/*      Read offset/length from SHX loading if necessary.               */
/* -------------------------------------------------------------------- */
    if( psSHP->panRecOffset[hEntity] == 0 && psSHP->fpSHX != NULL )
    {
        unsigned int       nOffset, nLength;

        if( psSHP->sHooks.FSeek( psSHP->fpSHX, 100 + 8 * hEntity, 0 ) != 0 ||
            psSHP->sHooks.FRead( &nOffset, 1, 4, psSHP->fpSHX ) != 4 ||
            psSHP->sHooks.FRead( &nLength, 1, 4, psSHP->fpSHX ) != 4 )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Error in fseek()/fread() reading object from .shx file at offset %d",
                    100 + 8 * hEntity);

            psSHP->sHooks.Error( str );
            return NULL;
        }
        if( !bBigEndian ) SwapWord( 4, &nOffset );
        if( !bBigEndian ) SwapWord( 4, &nLength );

        if( nOffset > (unsigned int)INT_MAX )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Invalid offset for entity %d", hEntity);

            psSHP->sHooks.Error( str );
            return NULL;
        }
        if( nLength > (unsigned int)(INT_MAX / 2 - 4) )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Invalid length for entity %d", hEntity);

            psSHP->sHooks.Error( str );
            return NULL;
        }

        psSHP->panRecOffset[hEntity] = nOffset*2;
        psSHP->panRecSize[hEntity] = nLength*2;
    }

/* -------------------------------------------------------------------- */
/*      Ensure our record buffer is large enough.                       */
/* -------------------------------------------------------------------- */
    nEntitySize = psSHP->panRecSize[hEntity]+8;
    if( nEntitySize > psSHP->nBufSize )
    {
        uchar* pabyRecNew;
        int nNewBufSize = nEntitySize;
        if( nNewBufSize < INT_MAX - nNewBufSize / 3 )
            nNewBufSize += nNewBufSize / 3;
        else
            nNewBufSize = INT_MAX;

        /* Before allocating too much memory, check that the file is big enough */
        if( nEntitySize >= 10 * 1024 * 1024 &&
            (psSHP->panRecOffset[hEntity] >= psSHP->nFileSize ||
             (unsigned int)nEntitySize > psSHP->nFileSize - psSHP->panRecOffset[hEntity]) )
        {
            /* We do as is we didn't trust the file size in the header */
            SAOffset nFileSize;
            psSHP->sHooks.FSeek( psSHP->fpSHP, 0, 2 );
            nFileSize = psSHP->sHooks.FTell(psSHP->fpSHP);
            if( nFileSize >= 0xFFFFFFFFU )
                psSHP->nFileSize = 0xFFFFFFFFU;
            else
                psSHP->nFileSize = (unsigned int)nFileSize;

            if( psSHP->panRecOffset[hEntity] >= psSHP->nFileSize ||
                (unsigned int)nEntitySize > psSHP->nFileSize - psSHP->panRecOffset[hEntity] )
            {
                char str[128];
                snprintf( str, sizeof(str),
                         "Error in fread() reading object of size %d at offset %u from .shp file",
                         nEntitySize, psSHP->panRecOffset[hEntity] );

                psSHP->sHooks.Error( str );
                return NULL;
            }
        }

        pabyRecNew = (uchar *) SfRealloc(psSHP->pabyRec,nNewBufSize);
        if (pabyRecNew == NULL)
        {
            char szError[200];

            snprintf( szError, sizeof(szError),
                     "Not enough memory to allocate requested memory (nNewBufSize=%d). "
                     "Probably broken SHP file", nNewBufSize);
            psSHP->sHooks.Error( szError );
            return NULL;
        }

        /* Only set new buffer size after successful alloc */
        psSHP->pabyRec = pabyRecNew;
        psSHP->nBufSize = nNewBufSize;
    }

    /* In case we were not able to reallocate the buffer on a previous step */
    if (psSHP->pabyRec == NULL)
    {
        return NULL;
    }

/* -------------------------------------------------------------------- */
/*      Read the record.                                                */
/* -------------------------------------------------------------------- */
    if( psSHP->sHooks.FSeek( psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0 ) != 0 )
    {
        /*
         * TODO - mloskot: Consider detailed diagnostics of shape file,
         * for example to detect if file is truncated.
         */
        char str[128];
        snprintf( str, sizeof(str),
                 "Error in fseek() reading object from .shp file at offset %u",
                 psSHP->panRecOffset[hEntity]);

        psSHP->sHooks.Error( str );
        return NULL;
    }

    nBytesRead = (int)psSHP->sHooks.FRead( psSHP->pabyRec, 1, nEntitySize, psSHP->fpSHP );

    /* Special case for a shapefile whose .shx content length field is not equal */
    /* to the content length field of the .shp, which is a violation of "The */
    /* content length stored in the index record is the same as the value stored in the main */
    /* file record header." (http://www.esri.com/library/whitepapers/pdfs/shapefile.pdf, page 24) */
    /* Actually in that case the .shx content length is equal to the .shp content length + */
    /* 4 (16 bit words), representing the 8 bytes of the record header... */
    if( nBytesRead >= 8 && nBytesRead == nEntitySize - 8 )
    {
        /* Do a sanity check */
        int nSHPContentLength;
        memcpy( &nSHPContentLength, psSHP->pabyRec + 4, 4 );
        if( !bBigEndian ) SwapWord( 4, &(nSHPContentLength) );
        if( nSHPContentLength < 0 ||
            nSHPContentLength > INT_MAX / 2 - 4 ||
            2 * nSHPContentLength + 8 != nBytesRead )
        {
            char str[128];
            snprintf( str, sizeof(str),
                    "Sanity check failed when trying to recover from inconsistent .shx/.shp with shape %d",
                    hEntity );

            psSHP->sHooks.Error( str );
            return NULL;
        }
    }
    else if( nBytesRead != nEntitySize )
    {
        /*
         * TODO - mloskot: Consider detailed diagnostics of shape file,
         * for example to detect if file is truncated.
         */
        char str[128];
        snprintf( str, sizeof(str),
                 "Error in fread() reading object of size %d at offset %u from .shp file",
                 nEntitySize, psSHP->panRecOffset[hEntity] );

        psSHP->sHooks.Error( str );
        return NULL;
    }

    if ( 8 + 4 > nEntitySize )
    {
        snprintf(szErrorMsg, sizeof(szErrorMsg),
                 "Corrupted .shp file : shape %d : nEntitySize = %d",
                 hEntity, nEntitySize);
        psSHP->sHooks.Error( szErrorMsg );
        return NULL;
    }
    memcpy( &nSHPType, psSHP->pabyRec + 8, 4 );

    if( bBigEndian ) SwapWord( 4, &(nSHPType) );

/* -------------------------------------------------------------------- */
/*	Allocate and minimally initialize the object.			*/
/* -------------------------------------------------------------------- */
    if( psSHP->bFastModeReadObject )
    {
        if( psSHP->psCachedObject->bFastModeReadObject )
        {
            psSHP->sHooks.Error( "Invalid read pattern in fast read mode. "
                                 "SHPDestroyObject() should be called." );
            return NULL;
        }

        psShape = psSHP->psCachedObject;
        memset(psShape, 0, sizeof(SHPObject));
    }
    else
        psShape = (SHPObject *) calloc(1,sizeof(SHPObject));
    psShape->nShapeId = hEntity;
    psShape->nSHPType = nSHPType;
    psShape->bMeasureIsUsed = FALSE;
    psShape->bFastModeReadObject = psSHP->bFastModeReadObject;

/* ==================================================================== */
/*  Extract vertices for a Polygon or Arc.				*/
/* ==================================================================== */
    if( psShape->nSHPType == SHPT_POLYGON || psShape->nSHPType == SHPT_ARC
        || psShape->nSHPType == SHPT_POLYGONZ
        || psShape->nSHPType == SHPT_POLYGONM
        || psShape->nSHPType == SHPT_ARCZ
        || psShape->nSHPType == SHPT_ARCM
        || psShape->nSHPType == SHPT_MULTIPATCH )
    {
        int32		nPoints, nParts;
        int    		i, nOffset;
        unsigned char* pBuffer = NULL;
        unsigned char** ppBuffer = NULL;

        if ( 40 + 8 + 4 > nEntitySize )
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }
/* -------------------------------------------------------------------- */
/*	Get the X/Y bounds.						*/
/* -------------------------------------------------------------------- */
        memcpy( &(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8 );
        memcpy( &(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8 );
        memcpy( &(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8 );
        memcpy( &(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8 );

        if( bBigEndian ) SwapWord( 8, &(psShape->dfXMin) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfYMin) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfXMax) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfYMax) );

/* -------------------------------------------------------------------- */
/*      Extract part/point count, and build vertex and part arrays      */
/*      to proper size.                                                 */
/* -------------------------------------------------------------------- */
        memcpy( &nPoints, psSHP->pabyRec + 40 + 8, 4 );
        memcpy( &nParts, psSHP->pabyRec + 36 + 8, 4 );

        if( bBigEndian ) SwapWord( 4, &nPoints );
        if( bBigEndian ) SwapWord( 4, &nParts );

        /* nPoints and nParts are unsigned */
        if (/* nPoints < 0 || nParts < 0 || */
            nPoints > 50 * 1000 * 1000 || nParts > 10 * 1000 * 1000)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d, nPoints=%u, nParts=%u.",
                     hEntity, nPoints, nParts);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        /* With the previous checks on nPoints and nParts, */
        /* we should not overflow here and after */
        /* since 50 M * (16 + 8 + 8) = 1 600 MB */
        nRequiredSize = 44 + 8 + 4 * nParts + 16 * nPoints;
        if ( psShape->nSHPType == SHPT_POLYGONZ
             || psShape->nSHPType == SHPT_ARCZ
             || psShape->nSHPType == SHPT_MULTIPATCH )
        {
            nRequiredSize += 16 + 8 * nPoints;
        }
        if( psShape->nSHPType == SHPT_MULTIPATCH )
        {
            nRequiredSize += 4 * nParts;
        }
        if (nRequiredSize > nEntitySize)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d, nPoints=%u, nParts=%u, nEntitySize=%d.",
                     hEntity, nPoints, nParts, nEntitySize);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        if( psShape->bFastModeReadObject )
        {
            int nObjectBufSize = 4 * sizeof(double) * nPoints + 2 * sizeof(int) * nParts;
            pBuffer = SHPReallocObjectBufIfNecessary(psSHP, nObjectBufSize);
            ppBuffer = &pBuffer;
        }

        psShape->nVertices = nPoints;
        psShape->padfX = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfY = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfZ = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfM = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);

        psShape->nParts = nParts;
        psShape->panPartStart = (int *) SHPAllocBuffer(ppBuffer, nParts * sizeof(int));
        psShape->panPartType = (int *) SHPAllocBuffer(ppBuffer, nParts * sizeof(int));

        if (psShape->padfX == NULL ||
            psShape->padfY == NULL ||
            psShape->padfZ == NULL ||
            psShape->padfM == NULL ||
            psShape->panPartStart == NULL ||
            psShape->panPartType == NULL)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                    "Not enough memory to allocate requested memory (nPoints=%u, nParts=%u) for shape %d. "
                    "Probably broken SHP file", nPoints, nParts, hEntity );
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        for( i = 0; (int32)i < nParts; i++ )
            psShape->panPartType[i] = SHPP_RING;

/* -------------------------------------------------------------------- */
/*      Copy out the part array from the record.                        */
/* -------------------------------------------------------------------- */
        memcpy( psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts );
        for( i = 0; (int32)i < nParts; i++ )
        {
            if( bBigEndian ) SwapWord( 4, psShape->panPartStart+i );

            /* We check that the offset is inside the vertex array */
            if (psShape->panPartStart[i] < 0
                || (psShape->panPartStart[i] >= psShape->nVertices
                    && psShape->nVertices > 0)
                || (psShape->panPartStart[i] > 0 && psShape->nVertices == 0) )
            {
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Corrupted .shp file : shape %d : panPartStart[%d] = %d, nVertices = %d",
                         hEntity, i, psShape->panPartStart[i], psShape->nVertices);
                psSHP->sHooks.Error( szErrorMsg );
                SHPDestroyObject(psShape);
                return NULL;
            }
            if (i > 0 && psShape->panPartStart[i] <= psShape->panPartStart[i-1])
            {
                snprintf(szErrorMsg, sizeof(szErrorMsg),
                         "Corrupted .shp file : shape %d : panPartStart[%d] = %d, panPartStart[%d] = %d",
                         hEntity, i, psShape->panPartStart[i], i - 1, psShape->panPartStart[i - 1]);
                psSHP->sHooks.Error( szErrorMsg );
                SHPDestroyObject(psShape);
                return NULL;
            }
        }

        nOffset = 44 + 8 + 4*nParts;

/* -------------------------------------------------------------------- */
/*      If this is a multipatch, we will also have parts types.         */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_MULTIPATCH )
        {
            memcpy( psShape->panPartType, psSHP->pabyRec + nOffset, 4*nParts );
            for( i = 0; (int32)i < nParts; i++ )
            {
                if( bBigEndian ) SwapWord( 4, psShape->panPartType+i );
            }

            nOffset += 4*nParts;
        }

/* -------------------------------------------------------------------- */
/*      Copy out the vertices from the record.                          */
/* -------------------------------------------------------------------- */
        for( i = 0; (int32)i < nPoints; i++ )
        {
            memcpy(psShape->padfX + i,
                   psSHP->pabyRec + nOffset + i * 16,
                   8 );

            memcpy(psShape->padfY + i,
                   psSHP->pabyRec + nOffset + i * 16 + 8,
                   8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfX + i );
            if( bBigEndian ) SwapWord( 8, psShape->padfY + i );
        }

        nOffset += 16*nPoints;

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_POLYGONZ
            || psShape->nSHPType == SHPT_ARCZ
            || psShape->nSHPType == SHPT_MULTIPATCH )
        {
            memcpy( &(psShape->dfZMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMax) );

            for( i = 0; (int32)i < nPoints; i++ )
            {
                memcpy( psShape->padfZ + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
            }

            nOffset += 16 + 8*nPoints;
        }
        else if( psShape->bFastModeReadObject )
        {
            psShape->padfZ = NULL;
        }

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( nEntitySize >= (int)(nOffset + 16 + 8*nPoints) )
        {
            memcpy( &(psShape->dfMMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMax) );

            for( i = 0; (int32)i < nPoints; i++ )
            {
                memcpy( psShape->padfM + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfM + i );
            }
            psShape->bMeasureIsUsed = TRUE;
        }
        else if( psShape->bFastModeReadObject )
        {
            psShape->padfM = NULL;
        }
    }

/* ==================================================================== */
/*  Extract vertices for a MultiPoint.					*/
/* ==================================================================== */
    else if( psShape->nSHPType == SHPT_MULTIPOINT
             || psShape->nSHPType == SHPT_MULTIPOINTM
             || psShape->nSHPType == SHPT_MULTIPOINTZ )
    {
        int32		nPoints;
        int    		i, nOffset;
        unsigned char* pBuffer = NULL;
        unsigned char** ppBuffer = NULL;

        if ( 44 + 4 > nEntitySize )
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }
        memcpy( &nPoints, psSHP->pabyRec + 44, 4 );

        if( bBigEndian ) SwapWord( 4, &nPoints );

        /* nPoints is unsigned */
        if (/* nPoints < 0 || */ nPoints > 50 * 1000 * 1000)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nPoints = %u",
                     hEntity, nPoints);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        nRequiredSize = 48 + nPoints * 16;
        if( psShape->nSHPType == SHPT_MULTIPOINTZ )
        {
            nRequiredSize += 16 + nPoints * 8;
        }
        if (nRequiredSize > nEntitySize)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nPoints = %u, nEntitySize = %d",
                     hEntity, nPoints, nEntitySize);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        if( psShape->bFastModeReadObject )
        {
            int nObjectBufSize = 4 * sizeof(double) * nPoints;
            pBuffer = SHPReallocObjectBufIfNecessary(psSHP, nObjectBufSize);
            ppBuffer = &pBuffer;
        }

        psShape->nVertices = nPoints;

        psShape->padfX = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfY = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfZ = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);
        psShape->padfM = (double *) SHPAllocBuffer(ppBuffer, sizeof(double) * nPoints);

        if (psShape->padfX == NULL ||
            psShape->padfY == NULL ||
            psShape->padfZ == NULL ||
            psShape->padfM == NULL)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Not enough memory to allocate requested memory (nPoints=%u) for shape %d. "
                     "Probably broken SHP file", nPoints, hEntity );
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }

        for( i = 0; (int32)i < nPoints; i++ )
        {
            memcpy(psShape->padfX+i, psSHP->pabyRec + 48 + 16 * i, 8 );
            memcpy(psShape->padfY+i, psSHP->pabyRec + 48 + 16 * i + 8, 8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfX + i );
            if( bBigEndian ) SwapWord( 8, psShape->padfY + i );
        }

        nOffset = 48 + 16*nPoints;

/* -------------------------------------------------------------------- */
/*	Get the X/Y bounds.						*/
/* -------------------------------------------------------------------- */
        memcpy( &(psShape->dfXMin), psSHP->pabyRec + 8 +  4, 8 );
        memcpy( &(psShape->dfYMin), psSHP->pabyRec + 8 + 12, 8 );
        memcpy( &(psShape->dfXMax), psSHP->pabyRec + 8 + 20, 8 );
        memcpy( &(psShape->dfYMax), psSHP->pabyRec + 8 + 28, 8 );

        if( bBigEndian ) SwapWord( 8, &(psShape->dfXMin) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfYMin) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfXMax) );
        if( bBigEndian ) SwapWord( 8, &(psShape->dfYMax) );

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_MULTIPOINTZ )
        {
            memcpy( &(psShape->dfZMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfZMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfZMax) );

            for( i = 0; (int32)i < nPoints; i++ )
            {
                memcpy( psShape->padfZ + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
            }

            nOffset += 16 + 8*nPoints;
        }
        else if( psShape->bFastModeReadObject )
            psShape->padfZ = NULL;

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( nEntitySize >= (int)(nOffset + 16 + 8*nPoints) )
        {
            memcpy( &(psShape->dfMMin), psSHP->pabyRec + nOffset, 8 );
            memcpy( &(psShape->dfMMax), psSHP->pabyRec + nOffset + 8, 8 );

            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMin) );
            if( bBigEndian ) SwapWord( 8, &(psShape->dfMMax) );

            for( i = 0; (int32)i < nPoints; i++ )
            {
                memcpy( psShape->padfM + i,
                        psSHP->pabyRec + nOffset + 16 + i*8, 8 );
                if( bBigEndian ) SwapWord( 8, psShape->padfM + i );
            }
            psShape->bMeasureIsUsed = TRUE;
        }
        else if( psShape->bFastModeReadObject )
            psShape->padfM = NULL;
    }

/* ==================================================================== */
/*      Extract vertices for a point.                                   */
/* ==================================================================== */
    else if( psShape->nSHPType == SHPT_POINT
             || psShape->nSHPType == SHPT_POINTM
             || psShape->nSHPType == SHPT_POINTZ )
    {
        int	nOffset;

        psShape->nVertices = 1;
        if( psShape->bFastModeReadObject )
        {
            psShape->padfX = &(psShape->dfXMin);
            psShape->padfY = &(psShape->dfYMin);
            psShape->padfZ = &(psShape->dfZMin);
            psShape->padfM = &(psShape->dfMMin);
            psShape->padfZ[0] = 0.0;
            psShape->padfM[0] = 0.0;
        }
        else
        {
            psShape->padfX = (double *) calloc(1,sizeof(double));
            psShape->padfY = (double *) calloc(1,sizeof(double));
            psShape->padfZ = (double *) calloc(1,sizeof(double));
            psShape->padfM = (double *) calloc(1,sizeof(double));
        }

        if (20 + 8 + (( psShape->nSHPType == SHPT_POINTZ ) ? 8 : 0)> nEntitySize)
        {
            snprintf(szErrorMsg, sizeof(szErrorMsg),
                     "Corrupted .shp file : shape %d : nEntitySize = %d",
                     hEntity, nEntitySize);
            psSHP->sHooks.Error( szErrorMsg );
            SHPDestroyObject(psShape);
            return NULL;
        }
        memcpy( psShape->padfX, psSHP->pabyRec + 12, 8 );
        memcpy( psShape->padfY, psSHP->pabyRec + 20, 8 );

        if( bBigEndian ) SwapWord( 8, psShape->padfX );
        if( bBigEndian ) SwapWord( 8, psShape->padfY );

        nOffset = 20 + 8;

/* -------------------------------------------------------------------- */
/*      If we have a Z coordinate, collect that now.                    */
/* -------------------------------------------------------------------- */
        if( psShape->nSHPType == SHPT_POINTZ )
        {
            memcpy( psShape->padfZ, psSHP->pabyRec + nOffset, 8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfZ );

            nOffset += 8;
        }

/* -------------------------------------------------------------------- */
/*      If we have a M measure value, then read it now.  We assume      */
/*      that the measure can be present for any shape if the size is    */
/*      big enough, but really it will only occur for the Z shapes      */
/*      (options), and the M shapes.                                    */
/* -------------------------------------------------------------------- */
        if( nEntitySize >= nOffset + 8 )
        {
            memcpy( psShape->padfM, psSHP->pabyRec + nOffset, 8 );

            if( bBigEndian ) SwapWord( 8, psShape->padfM );
            psShape->bMeasureIsUsed = TRUE;
        }

/* -------------------------------------------------------------------- */
/*      Since no extents are supplied in the record, we will apply      */
/*      them from the single vertex.                                    */
/* -------------------------------------------------------------------- */
        psShape->dfXMin = psShape->dfXMax = psShape->padfX[0];
        psShape->dfYMin = psShape->dfYMax = psShape->padfY[0];
        psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
        psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
    }

    return( psShape );
}

/************************************************************************/
/*                          SHPDestroyObject()                          */
/************************************************************************/

void SHPAPI_CALL
SHPDestroyObject( SHPObject * psShape )

{
    if( psShape == NULL )
        return;

    if( psShape->bFastModeReadObject )
    {
        psShape->bFastModeReadObject = FALSE;
        return;
    }

    if( psShape->padfX != NULL )
        free( psShape->padfX );
    if( psShape->padfY != NULL )
        free( psShape->padfY );
    if( psShape->padfZ != NULL )
        free( psShape->padfZ );
    if( psShape->padfM != NULL )
        free( psShape->padfM );

    if( psShape->panPartStart != NULL )
        free( psShape->panPartStart );
    if( psShape->panPartType != NULL )
        free( psShape->panPartType );

    free( psShape );
}
