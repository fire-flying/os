/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vbe.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VBE_H__
#define __VBE_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define PM_BUFF_LEN 0x8000

#define VBE_REAL_CODE_64K_NO 1

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : address: 0xc54da
 ***************************************************************/
struct pm_info_block {
    os_u8 signature[4]; /* PMID = 0x44494d50 */
    os_u16 entry_point; /* 0x1b21 */
    os_u16 pm_initialize; /* 0x54f0 */
    os_u16 bios_data_sel; /* 0x0000 */
    os_u16 a0000_sel; /* 0xa000 */
    os_u16 b0000_sel; /* 0xb000 */
    os_u16 b8000_sel; /* 0xb800 */
    os_u16 code_seg_sel; /* 0xc000 */
    os_u8 in_protect_mode; /* 0x00 */
    os_u8 checksum; /* 0x00 */
};

/***************************************************************
 extern function
 ***************************************************************/
os_void init_vbe(os_void);

#pragma pack()

#endif /* end of header */
