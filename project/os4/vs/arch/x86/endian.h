/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : endian.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __ENDIAN_H__
#define __ENDIAN_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <swab.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

#define LITTLE_ENDIAN

#define net_host_16(x) ___constant_swab16(x)
#define net_host_32(x) ___constant_swab32(x)
#define net_host_64(x) ___constant_swab64(x)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

