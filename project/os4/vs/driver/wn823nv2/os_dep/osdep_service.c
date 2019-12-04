/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/


#define _OSDEP_SERVICE_C_

#include <drv_types.h>

#define RT_TAG	'1178'

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
atomic_t _malloc_cnt = ATOMIC_INIT(0);
atomic_t _malloc_size = ATOMIC_INIT(0);
#endif
#endif /* DBG_MEMORY_LEAK */

void sema_init(_sema	*sema, int init_val);

#if defined(PLATFORM_LINUX)
/*
* Translate the OS dependent @param error_code to OS independent RTW_STATUS_CODE
* @return: one of RTW_STATUS_CODE
*/
inline int RTW_STATUS_CODE(int error_code){
	if(error_code >=0)
		return _SUCCESS;

	switch(error_code) {
		//case -ETIMEDOUT:
		//	return RTW_STATUS_TIMEDOUT;
		default:
			return _FAIL;
	}
}
#else
inline int RTW_STATUS_CODE(int error_code){
	return error_code;
}
#endif

u32 rtw_atoi(u8* s)
{

	int num=0,flag=0;
	int i;
	for(i=0;i<=str_len(s);i++)
	{
	  if(s[i] >= '0' && s[i] <= '9')
		 num = num * 10 + s[i] -'0';
	  else if(s[0] == '-' && i==0)
		 flag =1;
	  else
		  break;
	 }

	if(flag == 1)
	   num = num * -1;

	 return(num);

}

u8* _rtw_vmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX
	pbuf = kmalloc(sz);
    if (NULL == pbuf) {
        flog("_rtw_vmalloc failed, sz=%d\n", sz);
    }
#endif
#ifdef PLATFORM_FREEBSD
	pbuf = malloc(sz,M_DEVBUF,M_NOWAIT);
#endif

#ifdef PLATFORM_WINDOWS
	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);
#endif

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif
#endif /* DBG_MEMORY_LEAK */

	return pbuf;
}

u8* _rtw_zvmalloc(u32 sz)
{
	u8 	*pbuf;
#ifdef PLATFORM_LINUX
	pbuf = _rtw_vmalloc(sz);
	if (pbuf != NULL)
		mem_set(pbuf, 0, sz);
#endif
#ifdef PLATFORM_FREEBSD
	pbuf = malloc(sz,M_DEVBUF,M_ZERO|M_NOWAIT);
#endif
#ifdef PLATFORM_WINDOWS
	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);
	if (pbuf != NULL)
		NdisFillMemory(pbuf, sz, 0);
#endif

	return pbuf;
}

void _rtw_vmfree(u8 *pbuf, u32 sz)
{
#ifdef	PLATFORM_LINUX
	kfree(pbuf);
#endif
#ifdef PLATFORM_FREEBSD
	free(pbuf,M_DEVBUF);
#endif
#ifdef PLATFORM_WINDOWS
	NdisFreeMemory(pbuf,sz, 0);
#endif

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif
#endif /* DBG_MEMORY_LEAK */
}

u8* _rtw_malloc0(u32 sz)
{

	u8 	*pbuf=NULL;

#ifdef PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		pbuf = (u8 *)dvr_malloc(sz);
	else
#endif
		pbuf = kmalloc(sz);

#endif
#ifdef PLATFORM_FREEBSD
	pbuf = malloc(sz,M_DEVBUF,M_NOWAIT);
#endif
#ifdef PLATFORM_WINDOWS

	NdisAllocateMemoryWithTag(&pbuf,sz, RT_TAG);

#endif

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	if ( pbuf != NULL) {
		atomic_inc(&_malloc_cnt);
		atomic_add(sz, &_malloc_size);
	}
#endif
#endif /* DBG_MEMORY_LEAK */

	return pbuf;

}

u8* _rtw_zmalloc(u32 sz)
{
#ifdef PLATFORM_FREEBSD
	return malloc(sz,M_DEVBUF,M_ZERO|M_NOWAIT);
#else // PLATFORM_FREEBSD
	u8 	*pbuf = _rtw_malloc(sz);

	if (pbuf != NULL) {

#ifdef PLATFORM_LINUX
		mem_set(pbuf, 0, sz);
#endif

#ifdef PLATFORM_WINDOWS
		NdisFillMemory(pbuf, sz, 0);
#endif

	}

	return pbuf;
#endif // PLATFORM_FREEBSD
}

void	_rtw_mfree(u8 *pbuf, u32 sz)
{

#ifdef	PLATFORM_LINUX
#ifdef RTK_DMP_PLATFORM
	if(sz > 0x4000)
		dvr_free(pbuf);
	else
#endif
		kfree(pbuf);

#endif
#ifdef PLATFORM_FREEBSD
	free(pbuf,M_DEVBUF);
#endif
#ifdef PLATFORM_WINDOWS

	NdisFreeMemory(pbuf,sz, 0);

#endif

#ifdef DBG_MEMORY_LEAK
#ifdef PLATFORM_LINUX
	atomic_dec(&_malloc_cnt);
	atomic_sub(sz, &_malloc_size);
#endif
#endif /* DBG_MEMORY_LEAK */

}

// alic #ifdef PLATFORM_FREEBSD
//review again
struct sk_buff * dev_alloc_skb(unsigned int size)
{
	struct sk_buff *skb=NULL;
    	u8 *data=NULL;

	//skb = (struct sk_buff *)_rtw_zmalloc(sizeof(struct sk_buff)); // for skb->len, etc.
	skb = (struct sk_buff *)_rtw_malloc(sizeof(struct sk_buff));
	if(!skb)
		goto out;
	data = _rtw_malloc(size);
	if(!data)
		goto nodata;

	skb->head = (unsigned char*)data;
	skb->data = (unsigned char*)data;
	skb->tail = (unsigned char*)data;
	skb->end = (unsigned char*)data + size;
	skb->len = 0;
	//printf("%s()-%d: skb=%p, skb->head = %p\n", __FUNCTION__, __LINE__, skb, skb->head);
	//if (trace_is_ok() && (skb_to == skb)) print("skb(%x,%x)", skb, skb->head);

out:
	return skb;
nodata:
	_rtw_mfree((u8 *)skb, sizeof(struct sk_buff));
	skb = NULL;
goto out;

}

void dev_kfree_skb_any(struct sk_buff *skb)
{
//if (trace_is_ok() && (skb_to == skb)) {print("free(%x, %x)", skb, skb->head);dump_stack(print);}
	//printf("%s()-%d: skb->head = %p\n", __FUNCTION__, __LINE__, skb->head);
	if(skb->head)
		_rtw_mfree(skb->head, 0);
	//printf("%s()-%d: skb = %p\n", __FUNCTION__, __LINE__, skb);
	if(skb)
		_rtw_mfree((u8 *)skb, 0);
}
struct sk_buff *skb_clone(const struct sk_buff *skb)
{
	return NULL;
}

//#endif /* PLATFORM_FREEBSD */

struct sk_buff *_rtw_skb_alloc(u32 sz)
{
#ifdef PLATFORM_LINUX
// alic	return __dev_alloc_skb(sz, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
return dev_alloc_skb(sz);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	return dev_alloc_skb(sz);
#endif /* PLATFORM_FREEBSD */
}

void _rtw_skb_free(struct sk_buff *skb)
{
	dev_kfree_skb_any(skb);
}

struct sk_buff *_rtw_skb_copy(struct sk_buff *skb)
{
#ifdef PLATFORM_LINUX
// alic	return skb_copy(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
return NULL;
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	return NULL;
#endif /* PLATFORM_FREEBSD */
}

struct sk_buff *_rtw_skb_clone(struct sk_buff *skb)
{
#ifdef PLATFORM_LINUX
// alic	return skb_clone(skb, in_interrupt() ? GFP_ATOMIC : GFP_KERNEL);
return skb_clone(skb);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	return skb_clone(skb);
#endif /* PLATFORM_FREEBSD */
}

int _rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb)
{
#if 0 // alic
#ifdef PLATFORM_LINUX
	skb->dev = ndev;
	return netif_rx(skb);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	return (*ndev->if_input)(ndev, skb);
#endif /* PLATFORM_FREEBSD */
#endif
}

void _rtw_skb_queue_purge(struct sk_buff_head *list)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		_rtw_skb_free(skb);
}

#ifdef CONFIG_USB_HCI
inline void *_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma)
{
#ifdef PLATFORM_LINUX
#if 0 // alic
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	return usb_alloc_coherent(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#else
	return usb_buffer_alloc(dev, size, (in_interrupt() ? GFP_ATOMIC : GFP_KERNEL), dma);
#endif
#endif
return kmalloc(size);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	return (malloc(size, M_USBDEV, M_NOWAIT | M_ZERO));
#endif /* PLATFORM_FREEBSD */
}
inline void _rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma)
{
#ifdef PLATFORM_LINUX
#if 0 // alic
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	usb_free_coherent(dev, size, addr, dma);
#else
	usb_buffer_free(dev, size, addr, dma);
#endif
#endif
kfree(addr);
#endif /* PLATFORM_LINUX */

#ifdef PLATFORM_FREEBSD
	free(addr, M_USBDEV);
#endif /* PLATFORM_FREEBSD */
}
#endif /* CONFIG_USB_HCI */

#if defined(DBG_MEM_ALLOC)

struct rtw_mem_stat {
	ATOMIC_T alloc; // the memory bytes we allocate currently
	ATOMIC_T peak; // the peak memory bytes we allocate
	ATOMIC_T alloc_cnt; // the alloc count for alloc currently
	ATOMIC_T alloc_err_cnt; // the error times we fail to allocate memory
};

struct rtw_mem_stat rtw_mem_type_stat[mstat_tf_idx(MSTAT_TYPE_MAX)];
#ifdef RTW_MEM_FUNC_STAT
struct rtw_mem_stat rtw_mem_func_stat[mstat_ff_idx(MSTAT_FUNC_MAX)];
#endif

char *MSTAT_TYPE_str[] = {
	"VIR",
	"PHY",
	"SKB",
	"USB",
};

#ifdef RTW_MEM_FUNC_STAT
char *MSTAT_FUNC_str[] = {
	"UNSP",
	"IO",
	"TXIO",
	"RXIO",
	"TX",
	"RX",
};
#endif

void rtw_mstat_dump(void *sel)
{
	int i;
	int value_t[4][mstat_tf_idx(MSTAT_TYPE_MAX)];
#ifdef RTW_MEM_FUNC_STAT
	int value_f[4][mstat_ff_idx(MSTAT_FUNC_MAX)];
#endif

	int vir_alloc, vir_peak, vir_alloc_err, phy_alloc, phy_peak, phy_alloc_err;
	int tx_alloc, tx_peak, tx_alloc_err, rx_alloc, rx_peak, rx_alloc_err;

	for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		value_t[0][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc));
		value_t[1][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].peak));
		value_t[2][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc_cnt));
		value_t[3][i] = ATOMIC_READ(&(rtw_mem_type_stat[i].alloc_err_cnt));
	}

	#ifdef RTW_MEM_FUNC_STAT
	for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
		value_f[0][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc));
		value_f[1][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].peak));
		value_f[2][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc_cnt));
		value_f[3][i] = ATOMIC_READ(&(rtw_mem_func_stat[i].alloc_err_cnt));
	}
	#endif

	DBG_871X_SEL_NL(sel, "===================== MSTAT =====================\n");
	DBG_871X_SEL_NL(sel, "%4s %10s %10s %10s %10s\n", "TAG", "alloc", "peak", "aloc_cnt", "err_cnt");
	DBG_871X_SEL_NL(sel, "-------------------------------------------------\n");
	for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
		DBG_871X_SEL_NL(sel, "%4s %10d %10d %10d %10d\n", MSTAT_TYPE_str[i], value_t[0][i], value_t[1][i], value_t[2][i], value_t[3][i]);
	}
	#ifdef RTW_MEM_FUNC_STAT
	DBG_871X_SEL_NL(sel, "-------------------------------------------------\n");
	for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
		DBG_871X_SEL_NL(sel, "%4s %10d %10d %10d %10d\n", MSTAT_FUNC_str[i], value_f[0][i], value_f[1][i], value_f[2][i], value_f[3][i]);
	}
	#endif
}

void rtw_mstat_update(const enum mstat_f flags, const MSTAT_STATUS status, u32 sz)
{
	static u32 update_time = 0;
	int peak, alloc;
	int i;

	/* initialization */
	if(!update_time) {
		for(i=0;i<mstat_tf_idx(MSTAT_TYPE_MAX);i++) {
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].peak), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc_cnt), 0);
			ATOMIC_SET(&(rtw_mem_type_stat[i].alloc_err_cnt), 0);
		}
		#ifdef RTW_MEM_FUNC_STAT
		for(i=0;i<mstat_ff_idx(MSTAT_FUNC_MAX);i++) {
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].peak), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc_cnt), 0);
			ATOMIC_SET(&(rtw_mem_func_stat[i].alloc_err_cnt), 0);
		}
		#endif
	}

	switch(status) {
		case MSTAT_ALLOC_SUCCESS:
			ATOMIC_INC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc), sz);
			peak=ATOMIC_READ(&(rtw_mem_type_stat[mstat_tf_idx(flags)].peak));
			if (peak<alloc)
				ATOMIC_SET(&(rtw_mem_type_stat[mstat_tf_idx(flags)].peak), alloc);

			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_INC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_cnt));
			alloc = ATOMIC_ADD_RETURN(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc), sz);
			peak=ATOMIC_READ(&(rtw_mem_func_stat[mstat_ff_idx(flags)].peak));
			if (peak<alloc)
				ATOMIC_SET(&(rtw_mem_func_stat[mstat_ff_idx(flags)].peak), alloc);
			#endif
			break;

		case MSTAT_ALLOC_FAIL:
			ATOMIC_INC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_err_cnt));
			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_INC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_err_cnt));
			#endif
			break;

		case MSTAT_FREE:
			ATOMIC_DEC(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc_cnt));
			ATOMIC_SUB(&(rtw_mem_type_stat[mstat_tf_idx(flags)].alloc), sz);
			#ifdef RTW_MEM_FUNC_STAT
			ATOMIC_DEC(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc_cnt));
			ATOMIC_SUB(&(rtw_mem_func_stat[mstat_ff_idx(flags)].alloc), sz);
			#endif
			break;
	};

	//if (rtw_get_passing_time_ms(update_time) > 5000) {
	//	rtw_mstat_dump(RTW_DBGDUMP);
		update_time=rtw_get_current_time();
	//}
}

#ifndef SIZE_MAX
	#define SIZE_MAX (~(size_t)0)
#endif

struct mstat_sniff_rule {
	enum mstat_f flags;
	size_t lb;
	size_t hb;
};

struct mstat_sniff_rule mstat_sniff_rules[] = {
	{MSTAT_TYPE_PHY, 4097, SIZE_MAX},
};

int mstat_sniff_rule_num = sizeof(mstat_sniff_rules)/sizeof(struct mstat_sniff_rule);

bool match_mstat_sniff_rules(const enum mstat_f flags, const size_t size)
{
	int i;
	for (i = 0; i<mstat_sniff_rule_num; i++) {
		if (mstat_sniff_rules[i].flags == flags
				&& mstat_sniff_rules[i].lb <= size
				&& mstat_sniff_rules[i].hb >= size)
			return _TRUE;
	}

	return _FALSE;
}

inline u8* dbg_rtw_vmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8  *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_vmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zvmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_zvmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_vmfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	_rtw_vmfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline u8* dbg_rtw_malloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p=_rtw_malloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline u8* dbg_rtw_zmalloc(u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	u8 *p;

	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	p = _rtw_zmalloc((sz));

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, sz
	);

	return p;
}

inline void dbg_rtw_mfree(u8 *pbuf, u32 sz, const enum mstat_f flags, const char *func, const int line)
{
	if (match_mstat_sniff_rules(flags, sz))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d)\n", func, line, __FUNCTION__, (sz));

	_rtw_mfree((pbuf), (sz));

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, sz
	);
}

inline struct sk_buff * dbg_rtw_skb_alloc(unsigned int size, const enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;
	unsigned int truesize = 0;

	skb = _rtw_skb_alloc(size);

	if(skb)
		truesize = skb->truesize;

	if(!skb || truesize < size || match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%d), skb:%p, truesize=%u\n", func, line, __FUNCTION__, size, skb, truesize);

	rtw_mstat_update(
		flags
		, skb ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb;
}

inline void dbg_rtw_skb_free(struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	unsigned int truesize = skb->truesize;

	if(match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize);

	_rtw_skb_free(skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);
}

inline struct sk_buff *dbg_rtw_skb_copy(const struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cp;
	unsigned int truesize = skb->truesize;
	unsigned int cp_truesize = 0;

	skb_cp = _rtw_skb_copy(skb);
	if(skb_cp)
		cp_truesize = skb_cp->truesize;

	if(!skb_cp || cp_truesize < truesize || match_mstat_sniff_rules(flags, cp_truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%u), skb_cp:%p, cp_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cp, cp_truesize);

	rtw_mstat_update(
		flags
		, skb_cp ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cp;
}

inline struct sk_buff *dbg_rtw_skb_clone(struct sk_buff *skb, const enum mstat_f flags, const char *func, const int line)
{
	struct sk_buff *skb_cl;
	unsigned int truesize = skb->truesize;
	unsigned int cl_truesize = 0;

	skb_cl = _rtw_skb_clone(skb);
	if(skb_cl)
		cl_truesize = skb_cl->truesize;

	if(!skb_cl || cl_truesize < truesize || match_mstat_sniff_rules(flags, cl_truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%u), skb_cl:%p, cl_truesize=%u\n", func, line, __FUNCTION__, truesize, skb_cl, cl_truesize);

	rtw_mstat_update(
		flags
		, skb_cl ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, truesize
	);

	return skb_cl;
}

inline int dbg_rtw_netif_rx(_nic_hdl ndev, struct sk_buff *skb, const enum mstat_f flags, const char *func, int line)
{
	int ret;
	unsigned int truesize = skb->truesize;

	if(match_mstat_sniff_rules(flags, truesize))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s, truesize=%u\n", func, line, __FUNCTION__, truesize);

	ret = _rtw_netif_rx(ndev, skb);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, truesize
	);

	return ret;
}

inline void dbg_rtw_skb_queue_purge(struct sk_buff_head *list, enum mstat_f flags, const char *func, int line)
{
	struct sk_buff *skb;

	while ((skb = skb_dequeue(list)) != NULL)
		dbg_rtw_skb_free(skb, flags, func, line);
}

#ifdef CONFIG_USB_HCI
inline void *dbg_rtw_usb_buffer_alloc(struct usb_device *dev, size_t size, dma_addr_t *dma, const enum mstat_f flags, const char *func, int line)
{
	void *p;

	if(match_mstat_sniff_rules(flags, size))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%zu)\n", func, line, __FUNCTION__, size);

	p = _rtw_usb_buffer_alloc(dev, size, dma);

	rtw_mstat_update(
		flags
		, p ? MSTAT_ALLOC_SUCCESS : MSTAT_ALLOC_FAIL
		, size
	);

	return p;
}

inline void dbg_rtw_usb_buffer_free(struct usb_device *dev, size_t size, void *addr, dma_addr_t dma, const enum mstat_f flags, const char *func, int line)
{

	if(match_mstat_sniff_rules(flags, size))
		DBG_871X("DBG_MEM_ALLOC %s:%d %s(%zu)\n", func, line, __FUNCTION__, size);

	_rtw_usb_buffer_free(dev, size, addr, dma);

	rtw_mstat_update(
		flags
		, MSTAT_FREE
		, size
	);
}
#endif /* CONFIG_USB_HCI */

#endif /* defined(DBG_MEM_ALLOC) */

void* rtw_malloc2d(int h, int w, size_t size)
{
	int j;

	void **a = (void **) rtw_zmalloc( h*sizeof(void *) + h*w*size );
	if(a == NULL)
	{
		DBG_871X("%s: alloc memory fail!\n", __FUNCTION__);
		return NULL;
	}

	for( j=0; j<h; j++ )
		a[j] = ((char *)(a+h)) + j*w*size;

	return a;
}

void rtw_mfree2d(void *pbuf, int h, int w, int size)
{
	rtw_mfree((u8 *)pbuf, h*sizeof(void*) + w*h*size);
}

void _rtw_memcpy(void *dst, const void *src, u32 sz)
{
if (sz > 1000) { print("999999999999999999999-%d\n", sz); dump_stack(print);}
#if defined (PLATFORM_LINUX)|| defined (PLATFORM_FREEBSD)

	mem_cpy(dst, src, sz);

#endif

#ifdef PLATFORM_WINDOWS

	NdisMoveMemory(dst, src, sz);

#endif

}

int	_rtw_memcmp(void *dst, void *src, u32 sz)
{

#if defined (PLATFORM_LINUX)|| defined (PLATFORM_FREEBSD)
//under Linux/GNU/GLibc, the return value of memcmp for two same mem. chunk is 0

	if (!(memcmp(dst, src, sz)))
		return _TRUE;
	else
		return _FALSE;
#endif


#ifdef PLATFORM_WINDOWS
//under Windows, the return value of NdisEqualMemory for two same mem. chunk is 1

	if (NdisEqualMemory (dst, src, sz))
		return _TRUE;
	else
		return _FALSE;

#endif



}

void _rtw_memset(void *pbuf, int c, u32 sz)
{

#if defined (PLATFORM_LINUX)|| defined (PLATFORM_FREEBSD)

        mem_set(pbuf, c, sz);

#endif

#ifdef PLATFORM_WINDOWS
#if 0
	NdisZeroMemory(pbuf, sz);
	if (c != 0) mem_set(pbuf, c, sz);
#else
	NdisFillMemory(pbuf, sz, c);
#endif
#endif

}

#ifdef PLATFORM_FREEBSD
static inline void __list_add(_list *pnew, _list *pprev, _list *pnext)
 {
         pnext->prev = pnew;
         pnew->next = pnext;
         pnew->prev = pprev;
         pprev->next = pnew;
}
#endif /* PLATFORM_FREEBSD */


void _rtw_init_listhead(_list *list)
{

#ifdef PLATFORM_LINUX

        INIT_LIST_HEAD(list);

#endif

#ifdef PLATFORM_FREEBSD
         list->next = list;
         list->prev = list;
#endif
#ifdef PLATFORM_WINDOWS

        NdisInitializeListHead(list);

#endif

}


/*
For the following list_xxx operations,
caller must guarantee the atomic context.
Otherwise, there will be racing condition.
*/
u32	rtw_is_list_empty(_list *phead)
{

#ifdef PLATFORM_LINUX

	if (list_empty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif
#ifdef PLATFORM_FREEBSD

	if (phead->next == phead)
		return _TRUE;
	else
		return _FALSE;

#endif


#ifdef PLATFORM_WINDOWS

	if (IsListEmpty(phead))
		return _TRUE;
	else
		return _FALSE;

#endif


}

void rtw_list_insert_head(_list *plist, _list *phead)
{

#ifdef PLATFORM_LINUX
	list_add(plist, phead);
#endif

#ifdef PLATFORM_FREEBSD
	__list_add(plist, phead, phead->next);
#endif

#ifdef PLATFORM_WINDOWS
	InsertHeadList(phead, plist);
#endif
}

void rtw_list_insert_tail(_list *plist, _list *phead)
{

#ifdef PLATFORM_LINUX

	list_add_tail(plist, phead);

#endif
#ifdef PLATFORM_FREEBSD

	__list_add(plist, phead->prev, phead);

#endif
#ifdef PLATFORM_WINDOWS

  InsertTailList(phead, plist);

#endif

}

void rtw_init_timer(_timer *ptimer, void *padapter, TIMER_FUNC_PTR pfunc)
{
	_adapter *adapter = (_adapter *)padapter;

#ifdef PLATFORM_LINUX
	_init_timer(ptimer, 0, pfunc, adapter);
#endif
#ifdef PLATFORM_FREEBSD
	_init_timer(ptimer, adapter->pifp, pfunc, adapter->mlmepriv.nic_hdl);
#endif
#ifdef PLATFORM_WINDOWS
	_init_timer(ptimer, adapter->hndis_adapter, pfunc, adapter->mlmepriv.nic_hdl);
#endif
}

/*

Caller must check if the list is empty before calling rtw_list_delete

*/


void _rtw_init_sema(_sema	*sema, int init_val)
{

#ifdef PLATFORM_LINUX

	sema_init(sema, init_val);

#endif
#ifdef PLATFORM_FREEBSD
	sema_init(sema, init_val, "rtw_drv");
#endif
#ifdef PLATFORM_OS_XP

	KeInitializeSemaphore(sema, init_val,  SEMA_UPBND); // count=0;

#endif

#ifdef PLATFORM_OS_CE
	if(*sema == NULL)
		*sema = CreateSemaphore(NULL, init_val, SEMA_UPBND, NULL);
#endif

}

void _rtw_free_sema(_sema	*sema)
{
#ifdef PLATFORM_FREEBSD
	sema_destroy(sema);
#endif
#ifdef PLATFORM_OS_CE
	CloseHandle(*sema);
#endif

}

void _rtw_up_sema(_sema	*sema)
{

#ifdef PLATFORM_LINUX

	up(sema);

#endif
#ifdef PLATFORM_FREEBSD
	sema_post(sema);
#endif
#ifdef PLATFORM_OS_XP

	KeReleaseSemaphore(sema, IO_NETWORK_INCREMENT, 1,  FALSE );

#endif

#ifdef PLATFORM_OS_CE
	ReleaseSemaphore(*sema,  1,  NULL );
#endif
}

u32 _rtw_down_sema(_sema *sema)
{

#ifdef PLATFORM_LINUX

	if (down_interruptible(sema))
		return _FAIL;
	else
		return _SUCCESS;

#endif
#ifdef PLATFORM_FREEBSD
	sema_wait(sema);
	return  _SUCCESS;
#endif
#ifdef PLATFORM_OS_XP

	if(STATUS_SUCCESS == KeWaitForSingleObject(sema, Executive, KernelMode, TRUE, NULL))
		return  _SUCCESS;
	else
		return _FAIL;
#endif

#ifdef PLATFORM_OS_CE
	if(WAIT_OBJECT_0 == WaitForSingleObject(*sema, INFINITE ))
		return _SUCCESS;
	else
		return _FAIL;
#endif
}



void	_rtw_mutex_init(_mutex *pmutex)
{
#ifdef PLATFORM_LINUX
create_critical_section(pmutex, __LINE__);
#if 0
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_init(pmutex);
#else
	init_MUTEX(pmutex);
#endif
#endif
#endif

#ifdef PLATFORM_FREEBSD
	mtx_init(pmutex, "", NULL, MTX_DEF|MTX_RECURSE);
#endif
#ifdef PLATFORM_OS_XP

	KeInitializeMutex(pmutex, 0);

#endif

#ifdef PLATFORM_OS_CE
	*pmutex =  CreateMutex( NULL, _FALSE, NULL);
#endif
}

void	_rtw_mutex_free(_mutex *pmutex);
void	_rtw_mutex_free(_mutex *pmutex)
{
#ifdef PLATFORM_LINUX
destroy_critical_section(*pmutex);
#if 0
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))
	mutex_destroy(pmutex);
#else
#endif
#endif
#ifdef PLATFORM_FREEBSD
	sema_destroy(pmutex);
#endif

#endif

#ifdef PLATFORM_OS_XP

#endif

#ifdef PLATFORM_OS_CE

#endif
}

void	_rtw_spinlock_init(_lock *plock)
{

#ifdef PLATFORM_LINUX

	init_spinlock(plock);

#endif
#ifdef PLATFORM_FREEBSD
		mtx_init(plock, "", NULL, MTX_DEF|MTX_RECURSE);
#endif
#ifdef PLATFORM_WINDOWS

	NdisAllocateSpinLock(plock);

#endif

}

void	_rtw_spinlock_free(_lock *plock)
{
#ifdef PLATFORM_FREEBSD
	 mtx_destroy(plock);
#endif

#ifdef PLATFORM_WINDOWS

	NdisFreeSpinLock(plock);

#endif

}
#ifdef PLATFORM_FREEBSD
extern PADAPTER prtw_lock;

void rtw_mtx_lock(_lock *plock){
	if(prtw_lock){
		mtx_lock(&prtw_lock->glock);
	}
	else{
		printf("%s prtw_lock==NULL",__FUNCTION__);
	}
}
void rtw_mtx_unlock(_lock *plock){
	if(prtw_lock){
		mtx_unlock(&prtw_lock->glock);
	}
	else{
		printf("%s prtw_lock==NULL",__FUNCTION__);
	}

}
#endif //PLATFORM_FREEBSD


void	_rtw_spinlock(_lock	*plock)
{

#ifdef PLATFORM_LINUX
    flog("_rtw_spinlock\n");
    wtflog();
	spin_lock(plock);

#endif
#ifdef PLATFORM_FREEBSD
	mtx_lock(plock);
#endif
#ifdef PLATFORM_WINDOWS

	NdisAcquireSpinLock(plock);

#endif

}

void	_rtw_spinunlock(_lock *plock)
{

#ifdef PLATFORM_LINUX
    flog("_rtw_spinunlock\n");
    wtflog();
	spin_unlock(plock);

#endif
#ifdef PLATFORM_FREEBSD
	mtx_unlock(plock);
#endif
#ifdef PLATFORM_WINDOWS

	NdisReleaseSpinLock(plock);

#endif
}


void	_rtw_spinlock_ex(_lock	*plock)
{

#ifdef PLATFORM_LINUX
    flog("_rtw_spinlock_ex\n");
    wtflog();
	spin_lock(plock);

#endif
#ifdef PLATFORM_FREEBSD
	mtx_lock(plock);
#endif
#ifdef PLATFORM_WINDOWS

	NdisDprAcquireSpinLock(plock);

#endif

}

void	_rtw_spinunlock_ex(_lock *plock)
{

#ifdef PLATFORM_LINUX
    flog("_rtw_spinunlock_ex\n");
    wtflog();
	spin_unlock(plock);

#endif
#ifdef PLATFORM_FREEBSD
	mtx_unlock(plock);
#endif
#ifdef PLATFORM_WINDOWS

	NdisDprReleaseSpinLock(plock);

#endif
}



void	_rtw_init_queue(_queue	*pqueue)
{

	_rtw_init_listhead(&(pqueue->queue));

	_rtw_spinlock_init(&(pqueue->lock));

}

u32	  _rtw_queue_empty(_queue	*pqueue)
{
	return (rtw_is_list_empty(&(pqueue->queue)));
}


u32 rtw_end_of_queue_search(_list *head, _list *plist)
{
	if (head == plist)
		return _TRUE;
	else
		return _FALSE;
}


u32	rtw_get_current_time(void)
{

#ifdef PLATFORM_LINUX
	return system_tick();
#endif
#ifdef PLATFORM_FREEBSD
	struct timeval tvp;
	getmicrotime(&tvp);
	return tvp.tv_sec;
#endif
#ifdef PLATFORM_WINDOWS
	LARGE_INTEGER	SystemTime;
	NdisGetCurrentSystemTime(&SystemTime);
	return (u32)(SystemTime.LowPart);// count of 100-nanosecond intervals
#endif
}

inline u32 rtw_systime_to_ms(u32 systime)
{
#ifdef PLATFORM_LINUX
	return systime * 1000 / OS_HZ;
#endif
#ifdef PLATFORM_FREEBSD
	return systime * 1000;
#endif
#ifdef PLATFORM_WINDOWS
	return systime / 10000 ;
#endif
}

inline u32 rtw_ms_to_systime(u32 ms)
{
#ifdef PLATFORM_LINUX
	return ms * OS_HZ / 1000;
#endif
#ifdef PLATFORM_FREEBSD
	return ms /1000;
#endif
#ifdef PLATFORM_WINDOWS
	return ms * 10000 ;
#endif
}

// the input parameter start use the same unit as returned by rtw_get_current_time
inline s32 rtw_get_passing_time_ms(u32 start)
{
#ifdef PLATFORM_LINUX
	return rtw_systime_to_ms(system_tick()-start);
#endif
#ifdef PLATFORM_FREEBSD
	return rtw_systime_to_ms(rtw_get_current_time());
#endif
#ifdef PLATFORM_WINDOWS
	LARGE_INTEGER	SystemTime;
	NdisGetCurrentSystemTime(&SystemTime);
	return rtw_systime_to_ms((u32)(SystemTime.LowPart) - start) ;
#endif
}

inline s32 rtw_get_time_interval_ms(u32 start, u32 end)
{
#ifdef PLATFORM_LINUX
	return rtw_systime_to_ms(end-start);
#endif
#ifdef PLATFORM_FREEBSD
	return rtw_systime_to_ms(rtw_get_current_time());
#endif
#ifdef PLATFORM_WINDOWS
	return rtw_systime_to_ms(end-start);
#endif
}


void rtw_sleep_schedulable(int ms)
{

#ifdef PLATFORM_LINUX

    u32 delta;

    delta = (ms * OS_HZ)/1000;//(ms)
    if (delta == 0) {
        delta = 1;// 1 ms
    }
    delay_task(delta, __LINE__);
#if 0
    set_current_state(TASK_INTERRUPTIBLE);
    if (schedule_timeout(delta) != 0) {
        return ;
    }
    return;
#endif
#endif
#ifdef PLATFORM_FREEBSD
	DELAY(ms*1000);
	return ;
#endif

#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif

}


void rtw_msleep_os(int ms)
{

#ifdef PLATFORM_LINUX
#if 0
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	if (ms < 20) {
		unsigned long us = ms * 1000UL;
		usleep_range(us, us + 1000UL);
	} else
	#endif
  	msleep((unsigned int)ms);
#endif
delay_ms(ms);
#endif
#ifdef PLATFORM_FREEBSD
       //Delay for delay microseconds
	DELAY(ms*1000);
	return ;
#endif
#ifdef PLATFORM_WINDOWS

	NdisMSleep(ms*1000); //(us)*1000=(ms)

#endif


}
void rtw_usleep_os(int us)
{
#ifdef PLATFORM_LINUX
#if 0
	// msleep((unsigned int)us);
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36))
	usleep_range(us, us + 1);
	#else
	if ( 1 < (us/1000) )
      		msleep(1);
      else
		msleep( (us/1000) + 1);
	#endif
#endif
    delay_us(us);
#endif

#ifdef PLATFORM_FREEBSD
	//Delay for delay microseconds
	DELAY(us);

	return ;
#endif
#ifdef PLATFORM_WINDOWS

	NdisMSleep(us); //(us)

#endif


}


#ifdef DBG_DELAY_OS
void _rtw_mdelay_os(int ms, const char *func, const int line)
{
	#if 0
	if(ms>10)
		DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);
		rtw_msleep_os(ms);
	return;
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, ms);

#if defined(PLATFORM_LINUX)

   	mdelay((unsigned long)ms);

#elif defined(PLATFORM_WINDOWS)

	NdisStallExecution(ms*1000); //(us)*1000=(ms)

#endif


}
void _rtw_udelay_os(int us, const char *func, const int line)
{

	#if 0
	if(us > 1000) {
	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);
		rtw_usleep_os(us);
		return;
	}
	#endif


	DBG_871X("%s:%d %s(%d)\n", func, line, __FUNCTION__, us);


#if defined(PLATFORM_LINUX)

      rtw_usleep_os((unsigned long)us);

#elif defined(PLATFORM_WINDOWS)

	NdisStallExecution(us); //(us)

#endif

}
#else
void rtw_mdelay_os(int ms)
{

#ifdef PLATFORM_LINUX

   	mdelay((unsigned long)ms);

#endif
#ifdef PLATFORM_FREEBSD
	DELAY(ms*1000);
	return ;
#endif
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(ms*1000); //(us)*1000=(ms)

#endif


}
void rtw_udelay_os(int us)
{

#ifdef PLATFORM_LINUX

    rtw_usleep_os((unsigned long)us);

#endif
#ifdef PLATFORM_FREEBSD
	//Delay for delay microseconds
	DELAY(us);
	return ;
#endif
#ifdef PLATFORM_WINDOWS

	NdisStallExecution(us); //(us)

#endif

}
#endif

void rtw_yield_os(void)
{
#ifdef PLATFORM_LINUX
	yield();
#endif
#ifdef PLATFORM_FREEBSD
	yield();
#endif
#ifdef PLATFORM_WINDOWS
	SwitchToThread();
#endif
}

#define RTW_SUSPEND_LOCK_NAME "rtw_wifi"
#define RTW_SUSPEND_EXT_LOCK_NAME "rtw_wifi_ext"
#define RTW_SUSPEND_RX_LOCK_NAME "rtw_wifi_rx"
#define RTW_SUSPEND_TRAFFIC_LOCK_NAME "rtw_wifi_traffic"
#define RTW_SUSPEND_RESUME_LOCK_NAME "rtw_wifi_resume"
#define RTW_RESUME_SCAN_LOCK_NAME "rtw_wifi_scan"
#ifdef CONFIG_WAKELOCK
static struct wake_lock rtw_suspend_lock;
static struct wake_lock rtw_suspend_ext_lock;
static struct wake_lock rtw_suspend_rx_lock;
static struct wake_lock rtw_suspend_traffic_lock;
static struct wake_lock rtw_suspend_resume_lock;
static struct wake_lock rtw_resume_scan_lock;
#elif defined(CONFIG_ANDROID_POWER)
static android_suspend_lock_t rtw_suspend_lock ={
	.name = RTW_SUSPEND_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_ext_lock ={
	.name = RTW_SUSPEND_EXT_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_rx_lock ={
	.name = RTW_SUSPEND_RX_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_traffic_lock ={
	.name = RTW_SUSPEND_TRAFFIC_LOCK_NAME
};
static android_suspend_lock_t rtw_suspend_resume_lock ={
	.name = RTW_SUSPEND_RESUME_LOCK_NAME
};
static android_suspend_lock_t rtw_resume_scan_lock ={
	.name = RTW_RESUME_SCAN_LOCK_NAME
};
#endif

inline void rtw_suspend_lock_init(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_init(&rtw_suspend_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_LOCK_NAME);
	wake_lock_init(&rtw_suspend_ext_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_EXT_LOCK_NAME);
	wake_lock_init(&rtw_suspend_rx_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_RX_LOCK_NAME);
	wake_lock_init(&rtw_suspend_traffic_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_TRAFFIC_LOCK_NAME);
	wake_lock_init(&rtw_suspend_resume_lock, WAKE_LOCK_SUSPEND, RTW_SUSPEND_RESUME_LOCK_NAME);
	wake_lock_init(&rtw_resume_scan_lock, WAKE_LOCK_SUSPEND, RTW_RESUME_SCAN_LOCK_NAME);
	#elif defined(CONFIG_ANDROID_POWER)
	android_init_suspend_lock(&rtw_suspend_lock);
	android_init_suspend_lock(&rtw_suspend_ext_lock);
	android_init_suspend_lock(&rtw_suspend_rx_lock);
	android_init_suspend_lock(&rtw_suspend_traffic_lock);
	android_init_suspend_lock(&rtw_suspend_resume_lock);
	android_init_suspend_lock(&rtw_resume_scan_lock);
	#endif
}

inline void rtw_suspend_lock_uninit(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_destroy(&rtw_suspend_lock);
	wake_lock_destroy(&rtw_suspend_ext_lock);
	wake_lock_destroy(&rtw_suspend_rx_lock);
	wake_lock_destroy(&rtw_suspend_traffic_lock);
	wake_lock_destroy(&rtw_suspend_resume_lock);
	wake_lock_destroy(&rtw_resume_scan_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_uninit_suspend_lock(&rtw_suspend_lock);
	android_uninit_suspend_lock(&rtw_suspend_ext_lock);
	android_uninit_suspend_lock(&rtw_suspend_rx_lock);
	android_uninit_suspend_lock(&rtw_suspend_traffic_lock);
	android_uninit_suspend_lock(&rtw_suspend_resume_lock);
	android_uninit_suspend_lock(&rtw_resume_scan_lock);
	#endif
}

inline void rtw_lock_suspend(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}

inline void rtw_unlock_suspend(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}

inline void rtw_resume_lock_suspend(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock(&rtw_suspend_resume_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend(&rtw_suspend_resume_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}

inline void rtw_resume_unlock_suspend(void)
{
	#ifdef CONFIG_WAKELOCK
	wake_unlock(&rtw_suspend_resume_lock);
	#elif defined(CONFIG_ANDROID_POWER)
	android_unlock_suspend(&rtw_suspend_resume_lock);
	#endif

	#if  defined(CONFIG_WAKELOCK) || defined(CONFIG_ANDROID_POWER)
	//DBG_871X("####%s: suspend_lock_count:%d####\n", __FUNCTION__, rtw_suspend_lock.stat.count);
	#endif
}

inline void rtw_lock_suspend_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_lock, rtw_ms_to_systime(timeout_ms));
	#endif
}

inline void rtw_lock_ext_suspend_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_ext_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_ext_lock, rtw_ms_to_systime(timeout_ms));
	#endif
	//DBG_871X("EXT lock timeout:%d\n", timeout_ms);
}

inline void rtw_lock_rx_suspend_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_rx_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_rx_lock, rtw_ms_to_systime(timeout_ms));
	#endif
	//DBG_871X("RX lock timeout:%d\n", timeout_ms);
}


inline void rtw_lock_traffic_suspend_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_suspend_traffic_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_suspend_traffic_lock, rtw_ms_to_systime(timeout_ms));
	#endif
	//DBG_871X("traffic lock timeout:%d\n", timeout_ms);
}

inline void rtw_lock_resume_scan_timeout(u32 timeout_ms)
{
	#ifdef CONFIG_WAKELOCK
	wake_lock_timeout(&rtw_resume_scan_lock, rtw_ms_to_systime(timeout_ms));
	#elif defined(CONFIG_ANDROID_POWER)
	android_lock_suspend_auto_expire(&rtw_resume_scan_lock, rtw_ms_to_systime(timeout_ms));
	#endif
	//DBG_871X("resume scan lock:%d\n", timeout_ms);
}

void ATOMIC_SET(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_set(v,i);
	#elif defined(PLATFORM_WINDOWS)
	*v=i;// other choice????
	#elif defined(PLATFORM_FREEBSD)
	atomic_set_int(v,i);
	#endif
}

int ATOMIC_READ(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_read(v);
	#elif defined(PLATFORM_WINDOWS)
	return *v; // other choice????
	#elif defined(PLATFORM_FREEBSD)
	return atomic_load_acq_32(v);
	#endif
}

void ATOMIC_ADD(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_add(v,i);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedAdd(v,i);
	#elif defined(PLATFORM_FREEBSD)
	atomic_add_int(v,i);
	#endif
}
void ATOMIC_SUB(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	atomic_sub(v,i);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedAdd(v,-i);
	#elif defined(PLATFORM_FREEBSD)
	atomic_subtract_int(v,i);
	#endif
}

void ATOMIC_INC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_inc(v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedIncrement(v);
	#elif defined(PLATFORM_FREEBSD)
	atomic_add_int(v,1);
	#endif
}

void ATOMIC_DEC(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	atomic_dec(v);
	#elif defined(PLATFORM_WINDOWS)
	InterlockedDecrement(v);
	#elif defined(PLATFORM_FREEBSD)
	atomic_subtract_int(v,1);
	#endif
}

int ATOMIC_ADD_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_add_return(i,v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedAdd(v,i);
	#elif defined(PLATFORM_FREEBSD)
	atomic_add_int(v,i);
	return atomic_load_acq_32(v);
	#endif
}

int ATOMIC_SUB_RETURN(ATOMIC_T *v, int i)
{
	#ifdef PLATFORM_LINUX
	return atomic_sub_return(i,v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedAdd(v,-i);
	#elif defined(PLATFORM_FREEBSD)
	atomic_subtract_int(v,i);
	return atomic_load_acq_32(v);
	#endif
}

int ATOMIC_INC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_inc_return(v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedIncrement(v);
	#elif defined(PLATFORM_FREEBSD)
	atomic_add_int(v,1);
	return atomic_load_acq_32(v);
	#endif
}

int ATOMIC_DEC_RETURN(ATOMIC_T *v)
{
	#ifdef PLATFORM_LINUX
	return atomic_dec_return(v);
	#elif defined(PLATFORM_WINDOWS)
	return InterlockedDecrement(v);
	#elif defined(PLATFORM_FREEBSD)
	atomic_subtract_int(v,1);
	return atomic_load_acq_32(v);
	#endif
}

#if 0 // alic
#ifdef PLATFORM_LINUX
/*
* Open a file with the specific @param path, @param flag, @param mode
* @param fpp the pointer of struct file pointer to get struct file pointer while file opening is success
* @param path the path of the file to open
* @param flag file operation flags, please refer to linux document
* @param mode please refer to linux document
* @return Linux specific error code
*/
static int openFile(struct file **fpp, char *path, int flag, int mode)
{
	struct file *fp;

	fp=filp_open(path, flag, mode);
	if(IS_ERR(fp)) {
		*fpp=NULL;
		return PTR_ERR(fp);
	}
	else {
		*fpp=fp;
		return 0;
	}
}

/*
* Close the file with the specific @param fp
* @param fp the pointer of struct file to close
* @return always 0
*/
static int closeFile(struct file *fp)
{
	filp_close(fp,NULL);
	return 0;
}

static int readFile(struct file *fp,char *buf,int len)
{
	int rlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->read)
		return -EPERM;

	while(sum<len) {
		rlen=fp->f_op->read(fp,buf+sum,len-sum, &fp->f_pos);
		if(rlen>0)
			sum+=rlen;
		else if(0 != rlen)
			return rlen;
		else
			break;
	}

	return  sum;

}

static int writeFile(struct file *fp,char *buf,int len)
{
	int wlen=0, sum=0;

	if (!fp->f_op || !fp->f_op->write)
		return -EPERM;

	while(sum<len) {
		wlen=fp->f_op->write(fp,buf+sum,len-sum, &fp->f_pos);
		if(wlen>0)
			sum+=wlen;
		else if(0 != wlen)
			return wlen;
		else
			break;
	}

	return sum;

}

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return Linux specific error code
*/
static int isFileReadable(char *path)
{
	struct file *fp;
	int ret = 0;
	mm_segment_t oldfs;
	char buf;

	fp=filp_open(path, O_RDONLY, 0);
	if(IS_ERR(fp)) {
		ret = PTR_ERR(fp);
	}
	else {
		oldfs = get_fs(); set_fs(get_ds());

		if(1!=readFile(fp, &buf, 1))
			ret = PTR_ERR(fp);

		set_fs(oldfs);
		filp_close(fp,NULL);
	}
	return ret;
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read, or Linux specific error code
*/
static int retriveFromFile(char *path, u8* buf, u32 sz)
{
	int ret =-1;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp,path, O_RDONLY, 0)) ){
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=readFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s readFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written, or Linux specific error code
*/
static int storeToFile(char *path, u8* buf, u32 sz)
{
	int ret =0;
	mm_segment_t oldfs;
	struct file *fp;

	if(path && buf) {
		if( 0 == (ret=openFile(&fp, path, O_CREAT|O_WRONLY, 0666)) ) {
			DBG_871X("%s openFile path:%s fp=%p\n",__FUNCTION__, path ,fp);

			oldfs = get_fs(); set_fs(get_ds());
			ret=writeFile(fp, buf, sz);
			set_fs(oldfs);
			closeFile(fp);

			DBG_871X("%s writeFile, ret:%d\n",__FUNCTION__, ret);

		} else {
			DBG_871X("%s openFile path:%s Fail, ret:%d\n",__FUNCTION__, path, ret);
		}
	} else {
		DBG_871X("%s NULL pointer\n",__FUNCTION__);
		ret =  -EINVAL;
	}
	return ret;
}
#endif //PLATFORM_LINUX

/*
* Test if the specifi @param path is a file and readable
* @param path the path of the file to test
* @return _TRUE or _FALSE
*/
int rtw_is_file_readable(char *path)
{
#ifdef PLATFORM_LINUX
	if(isFileReadable(path) == 0)
		return _TRUE;
	else
		return _FALSE;
#else
	//Todo...
	return _FALSE;
#endif
}

/*
* Open the file with @param path and retrive the file content into memory starting from @param buf for @param sz at most
* @param path the path of the file to open and read
* @param buf the starting address of the buffer to store file content
* @param sz how many bytes to read at most
* @return the byte we've read
*/
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{
#ifdef PLATFORM_LINUX
	int ret =retriveFromFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}

/*
* Open the file with @param path and wirte @param sz byte of data starting from @param buf into the file
* @param path the path of the file to open and write
* @param buf the starting address of the data to write into file
* @param sz how many bytes to write at most
* @return the byte we've written
*/
int rtw_store_to_file(char *path, u8* buf, u32 sz)
{
#ifdef PLATFORM_LINUX
	int ret =storeToFile(path, buf, sz);
	return ret>=0?ret:0;
#else
	//Todo...
	return 0;
#endif
}
#endif
#if 0 // alic
#ifdef PLATFORM_LINUX
struct net_device *rtw_alloc_etherdev_with_old_priv(int sizeof_priv, void *old_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);
	pnpi->priv=old_priv;
	pnpi->sizeof_priv=sizeof_priv;

RETURN:
	return pnetdev;
}

struct net_device *rtw_alloc_etherdev(int sizeof_priv)
{
	struct net_device *pnetdev;
	struct rtw_netdev_priv_indicator *pnpi;

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,35))
	pnetdev = alloc_etherdev_mq(sizeof(struct rtw_netdev_priv_indicator), 4);
#else
	pnetdev = alloc_etherdev(sizeof(struct rtw_netdev_priv_indicator));
#endif
	if (!pnetdev)
		goto RETURN;

	pnpi = netdev_priv(pnetdev);

	pnpi->priv = rtw_zvmalloc(sizeof_priv);
	if (!pnpi->priv) {
		free_netdev(pnetdev);
		pnetdev = NULL;
		goto RETURN;
	}

	pnpi->sizeof_priv=sizeof_priv;
RETURN:
	return pnetdev;
}

void rtw_free_netdev(struct net_device * netdev)
{
	struct rtw_netdev_priv_indicator *pnpi;

	if(!netdev)
		goto RETURN;

	pnpi = netdev_priv(netdev);

	if(!pnpi->priv)
		goto RETURN;

	rtw_vmfree(pnpi->priv, pnpi->sizeof_priv);
	free_netdev(netdev);

RETURN:
	return;
}

/*
* Jeff: this function should be called under ioctl (rtnl_lock is accquired) while
* LINUX_VERSION_CODE < KERNEL_VERSION(2,6,26)
*/
int rtw_change_ifname(_adapter *padapter, const char *ifname)
{
	struct net_device *pnetdev;
	struct net_device *cur_pnetdev;
	struct rereg_nd_name_data *rereg_priv;
	int ret;

	if(!padapter)
		goto error;

	cur_pnetdev = padapter->pnetdev;
	rereg_priv = &padapter->rereg_nd_name_priv;

	//free the old_pnetdev
	if(rereg_priv->old_pnetdev) {
		free_netdev(rereg_priv->old_pnetdev);
		rereg_priv->old_pnetdev = NULL;
	}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		unregister_netdev(cur_pnetdev);
	else
#endif
		unregister_netdevice(cur_pnetdev);

	rereg_priv->old_pnetdev=cur_pnetdev;

	pnetdev = rtw_init_netdev(padapter);
	if (!pnetdev)  {
		ret = -1;
		goto error;
	}

	SET_NETDEV_DEV(pnetdev, dvobj_to_dev(adapter_to_dvobj(padapter)));

	rtw_init_netdev_name(pnetdev, ifname);

	_rtw_memcpy(pnetdev->dev_addr, adapter_mac_addr(padapter), ETH_ALEN);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,26))
	if(!rtnl_is_locked())
		ret = register_netdev(pnetdev);
	else
#endif
		ret = register_netdevice(pnetdev);

	if ( ret != 0) {
		RT_TRACE(_module_hci_intfs_c_,_drv_err_,("register_netdev() failed\n"));
		goto error;
	}

	return 0;

error:

	return -1;

}
#endif
#endif
#ifdef PLATFORM_FREEBSD
/*
 * Copy a buffer from userspace and write into kernel address
 * space.
 *
 * This emulation just calls the FreeBSD copyin function (to
 * copy data from user space buffer into a kernel space buffer)
 * and is designed to be used with the above io_write_wrapper.
 *
 * This function should return the number of bytes not copied.
 * I.e. success results in a zero value.
 * Negative error values are not returned.
 */
unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
        if ( copyin(from, to, n) != 0 ) {
                /* Any errors will be treated as a failure
                   to copy any of the requested bytes */
                return n;
        }

        return 0;
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	if ( copyout(from, to, n) != 0 ) {
		/* Any errors will be treated as a failure
		   to copy any of the requested bytes */
		return n;
	}

	return 0;
}


/*
 * The usb_register and usb_deregister functions are used to register
 * usb drivers with the usb subsystem. In this compatibility layer
 * emulation a list of drivers (struct usb_driver) is maintained
 * and is used for probing/attaching etc.
 *
 * usb_register and usb_deregister simply call these functions.
 */
int
usb_register(struct usb_driver *driver)
{
        rtw_usb_linux_register(driver);
        return 0;
}


int
usb_deregister(struct usb_driver *driver)
{
        rtw_usb_linux_deregister(driver);
        return 0;
}

void module_init_exit_wrapper(void *arg)
{
        int (*func)(void) = arg;
        func();
        return;
}

#endif //PLATFORM_FREEBSD

#ifdef CONFIG_PLATFORM_SPRD
#ifdef do_div
#undef do_div
#endif
#include <asm-generic/div64.h>
#endif

u64 rtw_modular64(u64 x, u64 y)
{
#ifdef PLATFORM_LINUX
	return do_div(x, y);
#elif defined(PLATFORM_WINDOWS)
	return (x % y);
#elif defined(PLATFORM_FREEBSD)
	return (x %y);
#endif
}

u64 rtw_division64(u64 x, u64 y)
{
#ifdef PLATFORM_LINUX
	do_div(x, y);
	return x;
#elif defined(PLATFORM_WINDOWS)
	return (x / y);
#elif defined(PLATFORM_FREEBSD)
	return (x / y);
#endif
}

inline u32 rtw_random32(void)
{
    return 0;
#if 0
#ifdef PLATFORM_LINUX
	#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3,8,0))
	return prandom_u32();
	#elif (LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,18))
	u32 random_int;
	get_random_bytes( &random_int , 4 );
	return random_int;
	#else
	return random32();
	#endif
#elif defined(PLATFORM_WINDOWS)
	#error "to be implemented\n"
#elif defined(PLATFORM_FREEBSD)
	#error "to be implemented\n"
#endif
#endif
}

void rtw_buf_free(u8 **buf, u32 *buf_len)
{
	u32 ori_len;

	if (!buf || !buf_len)
		return;

	ori_len = *buf_len;

	if (*buf) {
		u32 tmp_buf_len = *buf_len;
		*buf_len = 0;
		rtw_mfree(*buf, tmp_buf_len);
		*buf = NULL;
	}
}

void rtw_buf_update(u8 **buf, u32 *buf_len, u8 *src, u32 src_len)
{
	u32 ori_len = 0, dup_len = 0;
	u8 *ori = NULL;
	u8 *dup = NULL;

	if (!buf || !buf_len)
		return;

	if (!src || !src_len)
		goto keep_ori;

	/* duplicate src */
	dup = rtw_malloc(src_len);
	if (dup) {
		dup_len = src_len;
		_rtw_memcpy(dup, src, dup_len);
	}

keep_ori:
	ori = *buf;
	ori_len = *buf_len;

	/* replace buf with dup */
	*buf_len = 0;
	*buf = dup;
	*buf_len = dup_len;

	/* free ori */
	if (ori && ori_len > 0)
		rtw_mfree(ori, ori_len);
}


/**
 * rtw_cbuf_full - test if cbuf is full
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is full
 */
inline bool rtw_cbuf_full(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read-1)? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_empty - test if cbuf is empty
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Returns: _TRUE if cbuf is empty
 */
inline bool rtw_cbuf_empty(struct rtw_cbuf *cbuf)
{
	return (cbuf->write == cbuf->read)? _TRUE : _FALSE;
}

/**
 * rtw_cbuf_push - push a pointer into cbuf
 * @cbuf: pointer of struct rtw_cbuf
 * @buf: pointer to push in
 *
 * Lock free operation, be careful of the use scheme
 * Returns: _TRUE push success
 */
bool rtw_cbuf_push(struct rtw_cbuf *cbuf, void *buf)
{
	if (rtw_cbuf_full(cbuf))
		return _FAIL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->write);
	cbuf->bufs[cbuf->write] = buf;
	cbuf->write = (cbuf->write+1)%cbuf->size;

	return _SUCCESS;
}

/**
 * rtw_cbuf_pop - pop a pointer from cbuf
 * @cbuf: pointer of struct rtw_cbuf
 *
 * Lock free operation, be careful of the use scheme
 * Returns: pointer popped out
 */
void *rtw_cbuf_pop(struct rtw_cbuf *cbuf)
{
	void *buf;
	if (rtw_cbuf_empty(cbuf))
		return NULL;

	if (0)
		DBG_871X("%s on %u\n", __func__, cbuf->read);
	buf = cbuf->bufs[cbuf->read];
	cbuf->read = (cbuf->read+1)%cbuf->size;

	return buf;
}

/**
 * rtw_cbuf_alloc - allocte a rtw_cbuf with given size and do initialization
 * @size: size of pointer
 *
 * Returns: pointer of srtuct rtw_cbuf, NULL for allocation failure
 */
struct rtw_cbuf *rtw_cbuf_alloc(u32 size)
{
	struct rtw_cbuf *cbuf;

	cbuf = (struct rtw_cbuf *)rtw_malloc(sizeof(*cbuf) + sizeof(void*)*size);

	if (cbuf) {
		cbuf->write = cbuf->read = 0;
		cbuf->size = size;
	}

	return cbuf;
}

/**
 * rtw_cbuf_free - free the given rtw_cbuf
 * @cbuf: pointer of struct rtw_cbuf to free
 */
void rtw_cbuf_free(struct rtw_cbuf *cbuf)
{
	rtw_mfree((u8*)cbuf, sizeof(*cbuf) + sizeof(void*)*cbuf->size);
}

int skb_copy_bits(const struct sk_buff *skb, int offset, void *to, int len)
{
	if (offset > (int)skb->len - len)
		goto fault;

    mem_cpy(to, skb->data + offset, len);
    return 0;

fault:
	return -EFAULT;
}

int rtw_ndev_notifier_register(void) { UNDEFINED_FUNCTION; }
void rtw_ndev_notifier_unregister(void) { UNDEFINED_FUNCTION; }
int usb_deregister(struct usb_driver *driver) { UNDEFINED_FUNCTION; }
void rtw_unregister_netdevs(struct dvobj_priv *dvobj) { UNDEFINED_FUNCTION; }
void rtw_free_netdev(struct net_device * netdev) { UNDEFINED_FUNCTION; }
unsigned long long simple_strtoul(const char *cp, char **endp, unsigned int base)
{ UNDEFINED_FUNCTION; }
int rtw_change_ifname(_adapter *padapter, const char *ifname)
{ UNDEFINED_FUNCTION; }
int rtw_android_priv_cmd(struct net_device *net, struct ifreq *ifr, int cmd)
{ UNDEFINED_FUNCTION; }
//u32 rtw_start_drv_threads(_adapter *padapter) { UNDEFINED_FUNCTION; return _SUCCESS; }
void rtw_stop_drv_threads (_adapter *padapter) { UNDEFINED_FUNCTION; }
void rtw_netif_wake_queue(struct net_device *pnetdev)
{ UNDEFINED_FUNCTION; }
void rtw_netif_stop_queue(struct net_device *pnetdev)
{ UNDEFINED_FUNCTION; }
void netif_carrier_off(struct net_device *dev)
{ UNDEFINED_FUNCTION; }
int rtw_retrive_from_file(char *path, u8* buf, u32 sz)
{ UNDEFINED_FUNCTION; }
int time_after(u32 now, u32 old)
{ UNDEFINED_FUNCTION; }

unsigned long
copy_from_user(void *to, const void *from, unsigned long n)
{
        if ( mem_cpy(to, from, n) != 0 ) {
                /* Any errors will be treated as a failure
                   to copy any of the requested bytes */
                return n;
        }

        return 0;
}

unsigned long
copy_to_user(void *to, const void *from, unsigned long n)
{
	if ( mem_cpy(to, from, n) != 0 ) {
		/* Any errors will be treated as a failure
		   to copy any of the requested bytes */
		return n;
	}

	return 0;
}
void sema_init(_sema	*sema, int init_val)
{
    *sema = create_mevent((os_u32)init_val, "rtw_sema", __LINE__);
}

static int cfg80211_rtw_connect(_adapter *padapter, u8 *ssid, size_t ssid_len)
{
	int ret=0;
	_irqL irqL;
	_list *phead;
	struct wlan_network *pnetwork = NULL;
	NDIS_802_11_AUTHENTICATION_MODE authmode;
	NDIS_802_11_SSID ndis_ssid;
	u8 *dst_ssid, *src_ssid;
	u8 *dst_bssid, *src_bssid;
	//u8 matched_by_bssid=_FALSE;
	//u8 matched_by_ssid=_FALSE;
	u8 matched=_FALSE;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
	_queue *queue = &pmlmepriv->scanned_queue;

	padapter->mlmepriv.not_indic_disco = _TRUE;

#ifdef CONFIG_PLATFORM_MSTAR_SCAN_BEFORE_CONNECT
	printk("MStar Android!\n");
	if(adapter_wdev_data(padapter)->bandroid_scan == _FALSE)
	{
#ifdef CONFIG_P2P
		struct wifidirect_info *pwdinfo= &(padapter->wdinfo);
		if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_NONE))
#endif //CONFIG_P2P
		{
			ret = -EBUSY;
			printk("Android hasn't attached yet!\n");
			goto exit;
		}
	}
#endif

	rtw_ps_deny(padapter, PS_DENY_JOIN);
	if(_FAIL == rtw_pwr_wakeup(padapter)) {
		ret= -EPERM;
		goto exit;
	}

	if(check_fwstate(pmlmepriv, WIFI_AP_STATE)) {
		ret = -EPERM;
		goto exit;
	}

#ifdef CONFIG_CONCURRENT_MODE
	if (check_buddy_fwstate(padapter, _FW_UNDER_LINKING) == _TRUE) {
		DBG_8192C("%s, but buddy_intf is under linking\n", __FUNCTION__);
		ret = -EINVAL;
		goto exit;
	}
	if (check_buddy_fwstate(padapter, _FW_UNDER_SURVEY) == _TRUE) {
		rtw_scan_abort(padapter->pbuddy_adapter);
	}
#endif

	if (!ssid || !ssid_len)
	{
		ret = -EINVAL;
		goto exit;
	}

	if (ssid_len > IW_ESSID_MAX_SIZE){

		ret= -E2BIG;
		goto exit;
	}

	_rtw_memset(&ndis_ssid, 0, sizeof(NDIS_802_11_SSID));
	ndis_ssid.SsidLength = ssid_len;
	_rtw_memcpy(ndis_ssid.Ssid, (u8 *)ssid, ssid_len);

	DBG_8192C("ssid=%s, len=%d\n", ndis_ssid.Ssid, ssid_len);

	if (check_fwstate(pmlmepriv, _FW_UNDER_LINKING) == _TRUE) {
		ret = -EBUSY;
		DBG_8192C("%s, fw_state=0x%x, goto exit\n", __FUNCTION__, pmlmepriv->fw_state);
		goto exit;
	}
	if (check_fwstate(pmlmepriv, _FW_UNDER_SURVEY) == _TRUE) {
		rtw_scan_abort(padapter);
	}

	psecuritypriv->ndisencryptstatus = Ndis802_11EncryptionDisabled;
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; //open system
	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;

#ifdef CONFIG_WAPI_SUPPORT
	 padapter->wapiInfo.bWapiEnable = false;
#endif

#ifdef CONFIG_WAPI_SUPPORT
	if(sme->crypto.wpa_versions & NL80211_WAPI_VERSION_1)
	{
		padapter->wapiInfo.bWapiEnable = true;
		padapter->wapiInfo.extra_prefix_len = WAPI_EXT_LEN;
		padapter->wapiInfo.extra_postfix_len = SMS4_MIC_LEN;
	}
#endif

#ifdef CONFIG_WAPI_SUPPORT
	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_WAPI)
		padapter->mlmeextpriv.mlmext_info.auth_algo = psecuritypriv->dot11AuthAlgrthm;
#endif

#ifdef CONFIG_WAPI_SUPPORT
      if(sme->crypto.akm_suites[0] ==WLAN_AKM_SUITE_WAPI_PSK){
		padapter->wapiInfo.bWapiPSK = true;
	}
	else if(sme->crypto.akm_suites[0] ==WLAN_AKM_SUITE_WAPI_CERT){
	      padapter->wapiInfo.bWapiPSK = false;
	}
#endif

	authmode = psecuritypriv->ndisauthtype;

	//rtw_set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);
	if (rtw_set_802_11_connect(padapter, NULL, &ndis_ssid) == _FALSE) {
		ret = -1;
		goto exit;
	}
print("conn 4\n");
	DBG_8192C("set ssid:dot11AuthAlgrthm=%d, dot11PrivacyAlgrthm=%d, dot118021XGrpPrivacy=%d\n", psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm, psecuritypriv->dot118021XGrpPrivacy);

exit:

	rtw_ps_deny_cancel(padapter, PS_DENY_JOIN);

	DBG_8192C("<=%s, ret %d\n",__FUNCTION__, ret);

	padapter->mlmepriv.not_indic_disco = _FALSE;

	return ret;
}

static int rtw_cfg80211_set_wpa_ie(_adapter *padapter, u8 *pie, size_t ielen)
{
	u8 *buf=NULL, *pos=NULL;
	u32 left;
	int group_cipher = 0, pairwise_cipher = 0;
	int ret = 0;
	s32 wpa_ielen=0;
	s32 wpa2_ielen=0;
	u8 *pwpa, *pwpa2;
	u8 null_addr[]= {0,0,0,0,0,0};

	if (pie == NULL || !ielen) {
		/* Treat this as normal case, but need to clear WIFI_UNDER_WPS */
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		goto exit;
	}

	if (ielen > MAX_WPA_IE_LEN+MAX_WPS_IE_LEN+MAX_P2P_IE_LEN) {
		ret = -EINVAL;
		goto exit;
	}

	buf = rtw_zmalloc(ielen);
	if (buf == NULL){
		ret =  -ENOMEM;
		goto exit;
	}

	_rtw_memcpy(buf, pie , ielen);

	//dump
	{
		int i;
		DBG_8192C("set wpa_ie(length:%d):\n", ielen);
		for(i=0;i<ielen;i=i+8)
			DBG_8192C("0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x \n",buf[i],buf[i+1],buf[i+2],buf[i+3],buf[i+4],buf[i+5],buf[i+6],buf[i+7]);
	}

	pos = buf;
	if(ielen < RSN_HEADER_LEN){
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("Ie len too short %d\n", ielen));
		ret  = -1;
		goto exit;
	}

	pwpa = rtw_get_wpa_ie(buf, &wpa_ielen, ielen);
	if(pwpa && wpa_ielen>0)
	{
		if(rtw_parse_wpa_ie(pwpa, wpa_ielen+2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPAPSK;
			_rtw_memcpy(padapter->securitypriv.supplicant_ie, &pwpa[0], wpa_ielen+2);

			DBG_8192C("got wpa_ie, wpa_ielen:%d\n", wpa_ielen);
		}
	}

	pwpa2 = rtw_get_wpa2_ie(buf, &wpa2_ielen, ielen);
	if(pwpa2 && wpa2_ielen>0)
	{
		if(rtw_parse_wpa2_ie(pwpa2, wpa2_ielen+2, &group_cipher, &pairwise_cipher, NULL) == _SUCCESS)
		{
			padapter->securitypriv.dot11AuthAlgrthm= dot11AuthAlgrthm_8021X;
			padapter->securitypriv.ndisauthtype=Ndis802_11AuthModeWPA2PSK;
			_rtw_memcpy(padapter->securitypriv.supplicant_ie, &pwpa2[0], wpa2_ielen+2);

			DBG_8192C("got wpa2_ie, wpa2_ielen:%d\n", wpa2_ielen);
		}
	}

	if (group_cipher == 0)
	{
		group_cipher = WPA_CIPHER_NONE;
	}
	if (pairwise_cipher == 0)
	{
		pairwise_cipher = WPA_CIPHER_NONE;
	}

	switch(group_cipher)
	{
		case WPA_CIPHER_NONE:
			padapter->securitypriv.dot118021XGrpPrivacy=_NO_PRIVACY_;
			padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
			break;
		case WPA_CIPHER_WEP40:
			padapter->securitypriv.dot118021XGrpPrivacy=_WEP40_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		case WPA_CIPHER_TKIP:
			padapter->securitypriv.dot118021XGrpPrivacy=_TKIP_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case WPA_CIPHER_CCMP:
			padapter->securitypriv.dot118021XGrpPrivacy=_AES_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		case WPA_CIPHER_WEP104:
			padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
	}

	switch(pairwise_cipher)
	{
		case WPA_CIPHER_NONE:
			padapter->securitypriv.dot11PrivacyAlgrthm=_NO_PRIVACY_;
			padapter->securitypriv.ndisencryptstatus=Ndis802_11EncryptionDisabled;
			break;
		case WPA_CIPHER_WEP40:
			padapter->securitypriv.dot11PrivacyAlgrthm=_WEP40_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
		case WPA_CIPHER_TKIP:
			padapter->securitypriv.dot11PrivacyAlgrthm=_TKIP_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption2Enabled;
			break;
		case WPA_CIPHER_CCMP:
			padapter->securitypriv.dot11PrivacyAlgrthm=_AES_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption3Enabled;
			break;
		case WPA_CIPHER_WEP104:
			padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
			padapter->securitypriv.ndisencryptstatus = Ndis802_11Encryption1Enabled;
			break;
	}

	{/* handle wps_ie */
		s32 wps_ielen;
		u8 *wps_ie;

		wps_ie = rtw_get_wps_ie(buf, ielen, NULL, &wps_ielen);
		if (wps_ie && wps_ielen > 0) {
			DBG_8192C("got wps_ie, wps_ielen:%d\n", wps_ielen);
			padapter->securitypriv.wps_ie_len = wps_ielen<MAX_WPS_IE_LEN?wps_ielen:MAX_WPS_IE_LEN;
			_rtw_memcpy(padapter->securitypriv.wps_ie, wps_ie, padapter->securitypriv.wps_ie_len);
			set_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS);
		} else {
			_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
		}
	}

	#ifdef CONFIG_P2P
	{//check p2p_ie for assoc req;
		s32 p2p_ielen=0;
		u8 *p2p_ie;
		struct mlme_priv *pmlmepriv = &(padapter->mlmepriv);

		if((p2p_ie=rtw_get_p2p_ie(buf, ielen, NULL, &p2p_ielen)))
		{
			#ifdef CONFIG_DEBUG_CFG80211
			DBG_8192C("%s p2p_assoc_req_ielen=%d\n", __FUNCTION__, p2p_ielen);
			#endif

			if(pmlmepriv->p2p_assoc_req_ie)
			{
				u32 free_len = pmlmepriv->p2p_assoc_req_ie_len;
				pmlmepriv->p2p_assoc_req_ie_len = 0;
				rtw_mfree(pmlmepriv->p2p_assoc_req_ie, free_len);
				pmlmepriv->p2p_assoc_req_ie = NULL;
			}

			pmlmepriv->p2p_assoc_req_ie = rtw_malloc(p2p_ielen);
			if ( pmlmepriv->p2p_assoc_req_ie == NULL) {
				DBG_8192C("%s()-%d: rtw_malloc() ERROR!\n", __FUNCTION__, __LINE__);
				goto exit;
			}
			_rtw_memcpy(pmlmepriv->p2p_assoc_req_ie, p2p_ie, p2p_ielen);
			pmlmepriv->p2p_assoc_req_ie_len = p2p_ielen;
		}
	}
	#endif //CONFIG_P2P

	//TKIP and AES disallow multicast packets until installing group key
	if(padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_
		|| padapter->securitypriv.dot11PrivacyAlgrthm == _TKIP_WTMIC_
		|| padapter->securitypriv.dot11PrivacyAlgrthm == _AES_)
		//WPS open need to enable multicast
		//|| check_fwstate(&padapter->mlmepriv, WIFI_UNDER_WPS) == _TRUE)
		rtw_hal_set_hwreg(padapter, HW_VAR_OFF_RCR_AM, null_addr);

	RT_TRACE(_module_rtl871x_ioctl_os_c, _drv_info_,
		("rtw_set_wpa_ie: pairwise_cipher=0x%x padapter->securitypriv.ndisencryptstatus=%d padapter->securitypriv.ndisauthtype=%d\n",
		pairwise_cipher, padapter->securitypriv.ndisencryptstatus, padapter->securitypriv.ndisauthtype));

exit:
	if (buf)
		rtw_mfree(buf, ielen);
	if (ret)
		_clr_fwstate_(&padapter->mlmepriv, WIFI_UNDER_WPS);
	return ret;
}

void update_sec_info(_adapter *padapter, os_u8 *ie, os_uint ie_len)
{
	int ret=0;
	NDIS_802_11_AUTHENTICATION_MODE authmode;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;

	psecuritypriv->ndisencryptstatus = Ndis802_11EncryptionDisabled;
	psecuritypriv->dot11PrivacyAlgrthm = _NO_PRIVACY_;
	psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
	psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Open; //open system
	psecuritypriv->ndisauthtype = Ndis802_11AuthModeOpen;

	DBG_8192C("%s, ie_len=%d\n", __func__, ie_len);

	ret = rtw_cfg80211_set_wpa_ie(padapter, (u8 *)ie, ie_len);
	if (ret < 0)
		goto exit;
#if 0
	//For WEP Shared auth
	if((psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_Shared
		|| psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_Auto) && sme->key
	)
	{
		u32 wep_key_idx, wep_key_len,wep_total_len;
		NDIS_802_11_WEP	 *pwep = NULL;
		DBG_871X("%s(): Shared/Auto WEP\n",__FUNCTION__);

		wep_key_idx = sme->key_idx;
		wep_key_len = sme->key_len;

		if (sme->key_idx > WEP_KEYS) {
			ret = -EINVAL;
			goto exit;
		}

		if (wep_key_len > 0)
		{
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;
			wep_total_len = wep_key_len + FIELD_OFFSET(NDIS_802_11_WEP, KeyMaterial);
		 	pwep =(NDIS_802_11_WEP	 *) rtw_malloc(wep_total_len);
			if(pwep == NULL){
				DBG_871X(" wpa_set_encryption: pwep allocate fail !!!\n");
				ret = -ENOMEM;
				goto exit;
			}

		 	_rtw_memset(pwep, 0, wep_total_len);

		 	pwep->KeyLength = wep_key_len;
			pwep->Length = wep_total_len;

			if(wep_key_len==13)
			{
				padapter->securitypriv.dot11PrivacyAlgrthm=_WEP104_;
				padapter->securitypriv.dot118021XGrpPrivacy=_WEP104_;
			}
		}
		else {
			ret = -EINVAL;
			goto exit;
		}

		pwep->KeyIndex = wep_key_idx;
		pwep->KeyIndex |= 0x80000000;

		_rtw_memcpy(pwep->KeyMaterial,  (void *)sme->key, pwep->KeyLength);

		if(rtw_set_802_11_add_wep(padapter, pwep) == (u8)_FAIL)
		{
			ret = -EOPNOTSUPP ;
		}

		if (pwep) {
			rtw_mfree((u8 *)pwep,wep_total_len);
		}

		if(ret < 0)
			goto exit;
	}
#endif

	authmode = psecuritypriv->ndisauthtype;
	rtw_set_802_11_authentication_mode(padapter, authmode);

	//rtw_set_802_11_encryption_mode(padapter, padapter->securitypriv.ndisencryptstatus);

    DBG_8192C("set ssid:dot11AuthAlgrthm=%d, dot11PrivacyAlgrthm=%d, dot118021XGrpPrivacy=%d\n", psecuritypriv->dot11AuthAlgrthm, psecuritypriv->dot11PrivacyAlgrthm, psecuritypriv->dot118021XGrpPrivacy);

exit:

	return;
}

static int cfg80211_rtw_disconnect(_adapter *padapter, u16 reason_code)
{
	padapter->mlmepriv.not_indic_disco = _TRUE;

	rtw_set_to_roam(padapter, 0);

	//if(check_fwstate(&padapter->mlmepriv, _FW_LINKED))
	{
		rtw_scan_abort(padapter);
		LeaveAllPowerSaveMode(padapter);
		rtw_disassoc_cmd(padapter, 500, _FALSE);

		DBG_871X("%s...call rtw_indicate_disconnect\n", __FUNCTION__);

		rtw_indicate_disconnect(padapter);

		rtw_free_assoc_resources(padapter, 1);
		rtw_pwr_wakeup(padapter);
	}

	padapter->mlmepriv.not_indic_disco = _FALSE;

	return 0;
}

extern _adapter  *rtw_sw_export;
#if 0
LOCALC os_ret rtw_disconn(os_void *user)
{
    if (rtw_sw_export) {
        cfg80211_rtw_disconnect(rtw_sw_export, 0);
    }
    return OS_SUCC;
}

os_u8 *passwd;

LOCALC os_ret rtw_conn(os_void *user, os_u8 *ssid, os_u8 *pass)
{
    static os_u8 device_opened = OS_FALSE;
    int ret;

    passwd = pass;
    if (rtw_sw_export) {
        if (OS_FALSE == device_opened) {
            print("opening dev...%x\n", rtw_sw_export);
            netdev_open(rtw_sw_export->pnetdev);
            print("opened dev\n");
            device_opened = OS_TRUE;
        }

        cfg80211_rtw_disconnect(rtw_sw_export, 0);
        start_recv();
        print("connect...%d\n", rtw_to_roam(rtw_sw_export));
        //cfg80211_rtw_set_power_mgmt(rtw_sw_export, OS_FALSE, 0);
        ret = cfg80211_rtw_connect(rtw_sw_export, ssid, str_len(ssid));
        flog("cfg80211_rtw_connect %d, %d\n", ret, rtw_to_roam(rtw_sw_export));
    }
    return OS_SUCC;
}

void init_net_dev(_adapter *padapter, os_u8 *ssid, os_uint len, os_u8 *bssid)
{
    WLAN_BSSID_EX *curr_network;

    curr_network = &rtw_sw_export->mlmepriv.cur_network.network;
    create_dbg_wpa_dev(padapter->pnetdev, adapter_mac_addr(rtw_sw_export),
        bssid, ssid, len, passwd);
}
#else
LOCALC os_ret rtw_disconn(os_void *user)
{
    cfg80211_rtw_disconnect(user, 0);
    return OS_SUCC;
}

LOCALC os_ret rtw_conn(os_void *user, os_u8 *ssid, os_u8 *pass)
{
    static os_u8 device_opened = OS_FALSE;
    _adapter *padapter;
    int ret;

    padapter = user;
    mem_cpy(padapter->password, pass, strnlen(pass, sizeof(padapter->password) - 1) + 1);
    if (OS_FALSE == device_opened) {
        print("opening dev...\n");
        netdev_open(padapter->pnetdev);
        print("opened dev\n");
        device_opened = OS_TRUE;
    }

    cfg80211_rtw_disconnect(padapter, 0);
    resume_task(padapter->recvpriv.recv_taskhandle, __LINE__);
    print("connect...%d\n", rtw_to_roam(padapter));
    ret = cfg80211_rtw_connect(padapter, ssid, str_len(ssid));
    flog("cfg80211_rtw_connect %d, %d\n", ret, rtw_to_roam(padapter));

    return OS_SUCC;
}

void init_net_dev(_adapter *padapter, os_u8 *ssid, os_uint len, os_u8 *bssid)
{
    WLAN_BSSID_EX *curr_network;

    curr_network = &padapter->mlmepriv.cur_network.network;
    create_dbg_wpa_dev(padapter->pnetdev, adapter_mac_addr(padapter),
        bssid, ssid, len, padapter->password);
}
#endif

s32 update_eapol_attrib(_adapter *padapter, struct pkt_attrib *pattrib)
{

	struct sta_info *psta = NULL;
	struct sta_priv		*pstapriv = &padapter->stapriv;
	struct security_priv	*psecuritypriv = &padapter->securitypriv;
	struct mlme_priv	*pmlmepriv = &padapter->mlmepriv;
	struct qos_priv		*pqospriv= &pmlmepriv->qospriv;

	s32 res=_SUCCESS;

	psta = rtw_get_stainfo(pstapriv, pattrib->ra);
	if (psta == NULL)	{
		res =_FAIL;
		goto exit;
	}

	pattrib->mac_id = psta->mac_id;
	pattrib->psta = psta;
	pattrib->ack_policy = 0;
	// get ether_hdr_len
	pattrib->pkt_hdrlen = ETH_HLEN;

	// [TDLS] TODO: setup req/rsp should be AC_BK
	if (pqospriv->qos_option &&  psta->qos_option) {
		pattrib->priority = 4;	//tdls management frame should be AC_VI
		pattrib->hdrlen = WLAN_HDR_A3_QOS_LEN;
		pattrib->subtype = WIFI_QOS_DATA_TYPE;
	} else {
		pattrib->priority = 0;
		pattrib->hdrlen = WLAN_HDR_A3_LEN;
		pattrib->subtype = WIFI_DATA_TYPE;
	}

	//TODO:_lock
	if(update_attrib_sec_info(padapter, pattrib, psta) == _FAIL)
	{
		res = _FAIL;
		goto exit;
	}

	update_attrib_phy_info(padapter, pattrib, psta);


exit:

	return res;
}

int issue_eapol_data(struct net_device *dev, os_u8 *data, os_uint len)
{
    _adapter *padapter;
    struct xmit_frame *pmgntframe;
    struct pkt_attrib *pattrib;
    struct mlme_priv *pmlmepriv;
    struct xmit_priv *pxmitpriv;
    u8 baddr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int ret = _FAIL;

    DBG_871X("[%s]\n", __FUNCTION__);

    padapter = dev->priv;

    pmlmepriv = &padapter->mlmepriv;
    pxmitpriv = &(padapter->xmitpriv);
    if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
        goto exit;
flog("step0\n");
    pattrib = &pmgntframe->attrib;

    pmgntframe->frame_tag = DATA_FRAMETAG;
    pattrib->ether_type = 0x888e; // eapol

    //_rtw_memcpy(pattrib->dst, baddr, ETH_ALEN);
    _rtw_memcpy(pattrib->dst, get_bssid(pmlmepriv), ETH_ALEN);
    _rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
    _rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
    _rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
flog("step1\n");
    update_eapol_attrib(padapter, pattrib);
    pattrib->qsel = 0;//pattrib->priority;
#if 0
     mem_cpy(pmgntframe->buf_addr + TXDESC_OFFSET, data, len);
    pattrib->nr_frags = 1;
	pattrib->last_txcmdsz = pattrib->pktlen;
#else
    print("\n(%d))))", pmgntframe->frame_tag);
//    if (rtw_xmitframe_coalesce(padapter, pmgntframe->pkt, pmgntframe) != _SUCCESS) {
//	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
    if (rtw_xmit_eapol_coalesce(padapter, pmgntframe, data, len) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}flog("step2\n");
#endif
    dump_mgntframe(padapter, pmgntframe);
    ret = _SUCCESS;
exit:
    return ret;
}

int issue_wifi_data(_adapter *padapter, u16 ether_type, os_u8 *data, os_uint len)
{
    struct xmit_frame *pmgntframe;
    struct pkt_attrib *pattrib;
    struct mlme_priv *pmlmepriv;
    struct xmit_priv *pxmitpriv;
    u8 baddr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int ret = _FAIL;

    DBG_871X("[%s]\n", __FUNCTION__);

    cassert(OS_NULL != padapter); // padapter = rtw_sw_export;

    pmlmepriv = &padapter->mlmepriv;
    pxmitpriv = &(padapter->xmitpriv);
    if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
        goto exit;
flog("step0\n");
    pattrib = &pmgntframe->attrib;

    pmgntframe->frame_tag = DATA_FRAMETAG;
    pattrib->ether_type = ether_type;
    do {
        unsigned short				*fctrl, *qc;
        unsigned char					*pframe;
        pattrib->hdrlen +=2;
        pattrib->qos_en = _TRUE;
        pattrib->eosp = 1;
        pattrib->ack_policy = 0;
        pattrib->mdata = 0;
        pattrib->retry_ctrl = _FALSE;
        pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
        qc = (unsigned short *)(pframe + pattrib->hdrlen - 2);
        SetPriority(qc, 7);	/* Set priority to VO */
        SetEOSP(qc, pattrib->eosp);
        SetAckpolicy(qc, pattrib->ack_policy);
    } while (0);

    //_rtw_memcpy(pattrib->dst, baddr, ETH_ALEN);
    _rtw_memcpy(pattrib->dst, get_bssid(pmlmepriv), ETH_ALEN);
    _rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
    _rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
    _rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
flog("step1\n");
    update_eapol_attrib(padapter, pattrib);
    pattrib->qsel = 0;//pattrib->priority;
#if 0
     mem_cpy(pmgntframe->buf_addr + TXDESC_OFFSET, data, len);
    pattrib->nr_frags = 1;
	pattrib->last_txcmdsz = pattrib->pktlen;
#else
    print("\n(%d))))", pmgntframe->frame_tag);
//    if (rtw_xmitframe_coalesce(padapter, pmgntframe->pkt, pmgntframe) != _SUCCESS) {
//	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
    if (rtw_xmit_eapol_coalesce(padapter, pmgntframe, data, len) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}flog("step2\n");
#endif
    dump_mgntframe(padapter, pmgntframe);
    ret = _SUCCESS;
exit:
    return ret;
}

int issue_broad_data(_adapter *padapter, u16 ether_type, os_u8 *data, os_uint len)
{
    struct xmit_frame *pmgntframe;
    struct pkt_attrib *pattrib;
    struct mlme_priv *pmlmepriv;
    struct xmit_priv *pxmitpriv;
    u8 baddr[ETH_ALEN] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    int ret = _FAIL;

    DBG_871X("[%s]\n", __FUNCTION__);

    cassert(OS_NULL != padapter);

    pmlmepriv = &padapter->mlmepriv;
    pxmitpriv = &(padapter->xmitpriv);
    if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
        goto exit;
flog("step0\n");
    pattrib = &pmgntframe->attrib;

    pmgntframe->frame_tag = DATA_FRAMETAG;
    pattrib->ether_type = ether_type;

    //_rtw_memcpy(pattrib->dst, baddr, ETH_ALEN);
    _rtw_memcpy(pattrib->dst, baddr, ETH_ALEN);
    _rtw_memcpy(pattrib->src, adapter_mac_addr(padapter), ETH_ALEN);
    _rtw_memcpy(pattrib->ra, get_bssid(pmlmepriv), ETH_ALEN);
    _rtw_memcpy(pattrib->ta, pattrib->src, ETH_ALEN);
flog("step1\n");
    update_eapol_attrib(padapter, pattrib);
    pattrib->qsel = 0;//pattrib->priority;
#if 0
     mem_cpy(pmgntframe->buf_addr + TXDESC_OFFSET, data, len);
    pattrib->nr_frags = 1;
	pattrib->last_txcmdsz = pattrib->pktlen;
#else
    print("\n(%d))))", pmgntframe->frame_tag);
//    if (rtw_xmitframe_coalesce(padapter, pmgntframe->pkt, pmgntframe) != _SUCCESS) {
//	if (rtw_xmit_tdls_coalesce(padapter, pmgntframe, &txmgmt) != _SUCCESS) {
    if (rtw_xmit_eapol_coalesce(padapter, pmgntframe, data, len) != _SUCCESS) {
		rtw_free_xmitbuf(pxmitpriv, pmgntframe->pxmitbuf);
		rtw_free_xmitframe(pxmitpriv, pmgntframe);
		goto exit;
	}flog("step2\n");
#endif
    dump_mgntframe(padapter, pmgntframe);
    ret = _SUCCESS;
exit:
    return ret;
}

os_ret rtw_issue(os_void)
{
    os_u8 arp[] = {
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x30, 0xb4, 0x9e, 0x3f, 0xfc, 0xea,
        0x00, 0x01,
        0x08, 0x00,
        0x06,
        0x04,
        0x00, 0x01,
        0x30, 0xb4, 0x9e, 0x3f, 0xfc, 0xea,
        0xc0, 0xa8, 0x1d, 0x09,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0xc0, 0xa8, 0x1d, 0x01
    };
#if 0 // v
    struct sk_buff *skb;

//    rtw_lps_ctrl_wk_cmd(rtw_sw_export, LPS_CTRL_LEAVE, 1);

    skb = _rtw_skb_alloc(sizeof(arp));
    mem_cpy(skb->data, arp, sizeof(arp));
    rtw_xmit(rtw_sw_export, &skb);

    flog("issue start\n");// WIFI_DATA_TYPE
    _rtw_skb_free(skb);
#elif 0 // x
    issue_wifi_data(rtw_sw_export, ETH_P_ARP, arp, sizeof(arp));
#elif 0 // x
    issue_broad_data(rtw_sw_export, ETH_P_ARP, arp, sizeof(arp));
#elif 1 // vvv
    os_u8 icmp_req[] = {
        0x2c, 0xab, 0x25, 0x4d, 0xcd, 0x51,
        0x30, 0xb4, 0x9e, 0x3f, 0xfc, 0xea,
        0x08, 0x00,
        0x45, 0x0, 0x0, 0x54, 0x56, 0xbd, 0x40, 0x0, 0x40, 0x1,
        0x28, 0x93, // crc
        0xc0, 0xa8, 0x1d, 0x7, // src ip
        0xc0, 0xa8, 0x1d, 0x1, // dst ip
        0x8, 0x0, 0xf5, 0xfe, 0x1b, 0x24, 0x0, 0x3, 0x17, 0x7e, 0x60, 0x59, 0x76, 0xff, 0xd, 0x0, 0x8, 0x9, 0xa, 0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20,
        0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37
    };

    struct sk_buff *skb;
    print("issue td ->\n");// WIFI_DATA_TYPE
    skb = _rtw_skb_alloc(sizeof(icmp_req));
    mem_cpy(skb->data, icmp_req, sizeof(icmp_req));
    skb_put(skb, sizeof(icmp_req));
    flog("%x\n", skb);
    rtw_xmit(rtw_sw_export, &skb);print("xmit%x.", skb);
    flog("issue td\n");// WIFI_DATA_TYPE
    delay_task(0x80, __LINE__);
    //_rtw_skb_free(skb);

#elif 1 // v
    os_u8 td[] = { 0xd0, 0xc7, 0xc0, 0xe8, 0x0f, 0x08, 0x30, 0xb4, 0x9e, 0x3f, 0xfc, 0xea, 0x08, 0x00, 0x45, 0x00, 0x00, 0x54, 0x41, 0xd2, 0x40, 0x00, 0x40, 0x01, 0x3d, 0x16, 0xc0, 0xa8, 0x1d, 0x6f, 0xc0, 0xa8, 0x1d, 0x01, 0x08, 0x00, 0xba, 0x6d, 0x1b, 0xac, 0x00, 0x04, 0xf8, 0xd5, 0x45, 0x59, 0xf7, 0xaf, 0x01, 0x00, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37 };
    struct sk_buff *skb;
    flog("issue td ->\n");// WIFI_DATA_TYPE
    skb = _rtw_skb_alloc(sizeof(td));
    mem_cpy(skb->data, td, sizeof(td));
    rtw_xmit(rtw_sw_export, &skb);
    flog("issue td\n");// WIFI_DATA_TYPE
    _rtw_skb_free(skb);
#elif 1 // v
    eapol_sm_txStart();
#endif
    return OS_SUCC;
}
#include <net.h>
os_ret wifi_sendpacket(os_void* user, os_void* msg802_3, os_u32 len)
{
    struct ethhdr *hdr;
    os_uint i;
    os_u8 *dd = msg802_3;

//    print("send packet %x %x\n", rtw_sw_export, user);
    hdr = (struct ethhdr *) msg802_3;

flog("wifi send %d\n", system_tick());

//    delay_task(0x40, __LINE__);debug_return;

#if 0
    issue_wifi_data(OS_NULL, hdr->h_proto, (os_u8 *)(hdr + 1), len - sizeof(struct ethhdr));
#else
    if (1) {
        struct sk_buff *skb;

    //rtw_lps_ctrl_wk_cmd(rtw_sw_export, LPS_CTRL_LEAVE, 1);

    //delay_task(0x10, __LINE__);

    skb = _rtw_skb_alloc(len);
    mem_cpy(skb->data, msg802_3, len);
    skb_put(skb, len);

    i = rtw_xmit(user, &skb);

    flog("issue end, free now\n");// WIFI_DATA_TYPE
//delay_task(0x40, __LINE__);
    //_rtw_skb_free(skb);
    //check_sem(__LINE__);
    }
#endif
    //print("send end\n");
}

os_ret ping_wifi(os_void)
{
    os_u8  mac_addr[6];
    os_u8  ip_addr[4];
    ETH_DEV_INFO_STRU dev;
    static int creat = 1;

    flog("start ping\n");
    if (creat) {
        creat = 0;

        dev.dev_no = 0;
        dev.mac_addr[0] = 0x30;
        dev.mac_addr[1] = 0xb4;
        dev.mac_addr[2] = 0x9e;
        dev.mac_addr[3] = 0x3f;
        dev.mac_addr[4] = 0xfc;
        dev.mac_addr[5] = 0xea;
        dev.SendPacket = wifi_sendpacket;
        dev.dedicated = rtw_sw_export;

        Eth_RegDevice(&dev);
        IP_AddLocalIPAddr(0, 0xc0a81d09, 0xffffff00);
#if 0
        mac_addr[0] = 0x2c;
        mac_addr[1] = 0xab;
        mac_addr[2] = 0x25;
        mac_addr[3] = 0x4d;
        mac_addr[4] = 0xcd;
        mac_addr[5] = 0x51;
        ip_addr[0] = 0xc0;
        ip_addr[1] = 0xa8;
        ip_addr[2] = 0x1d;
        ip_addr[3] = 0x01;
        add_static_arp_item(0, mac_addr, ip_addr);
#endif
//        RefreshArpTableInfo(0, mac_addr, ip_addr, NORMAL, DEFAULT_ARP_AGE, DYNAMIC_TABLE);
    }
    ping("192.168.29.1", 10);
}

#ifdef CONFIG_AP_MODE
static int rtw_cfg80211_ap_set_encryption(_adapter *padapter, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	struct sta_info *psta = NULL, *pbcmc_sta = NULL;
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv* psecuritypriv=&(padapter->securitypriv);
	struct sta_priv *pstapriv = &padapter->stapriv;

	DBG_8192C("%s\n", __FUNCTION__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	//sizeof(struct ieee_param) = 64 bytes;
	//if (param_len !=  (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	if (param_len !=  sizeof(struct ieee_param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		if (param->u.crypt.idx >= WEP_KEYS)
		{
			ret = -EINVAL;
			goto exit;
		}
	}
	else
	{
		psta = rtw_get_stainfo(pstapriv, param->sta_addr);
		if(!psta)
		{
			//ret = -EINVAL;
			DBG_8192C("rtw_set_encryption(), sta has already been removed or never been added\n");
			goto exit;
		}
	}

	if (str_cmp(param->u.crypt.alg, "none") == 0 && (psta==NULL))
	{
		//todo:clear default encryption keys

		DBG_8192C("clear default encryption keys, keyid=%d\n", param->u.crypt.idx);

		goto exit;
	}


	if (str_cmp(param->u.crypt.alg, "WEP") == 0 && (psta==NULL))
	{
		DBG_8192C("r871x_set_encryption, crypt.alg = WEP\n");

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		DBG_8192C("r871x_set_encryption, wep_key_idx=%d, len=%d\n", wep_key_idx, wep_key_len);

		if((wep_key_idx >= WEP_KEYS) || (wep_key_len<=0))
		{
			ret = -EINVAL;
			goto exit;
		}

		if (wep_key_len > 0)
		{
		 	wep_key_len = wep_key_len <= 5 ? 5 : 13;
		}

		if (psecuritypriv->bWepDefaultKeyIdxSet == 0)
		{
			//wep default key has not been set, so use this key index as default key.

			psecuritypriv->dot11AuthAlgrthm = dot11AuthAlgrthm_Auto;
			psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm=_WEP40_;
			psecuritypriv->dot118021XGrpPrivacy=_WEP40_;

			if(wep_key_len == 13)
			{
				psecuritypriv->dot11PrivacyAlgrthm=_WEP104_;
				psecuritypriv->dot118021XGrpPrivacy=_WEP104_;
			}

			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;
		}

		_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), param->u.crypt.key, wep_key_len);

		psecuritypriv->dot11DefKeylen[wep_key_idx] = wep_key_len;

		rtw_ap_set_wep_key(padapter, param->u.crypt.key, wep_key_len, wep_key_idx, 1);

		goto exit;

	}


	if(!psta && check_fwstate(pmlmepriv, WIFI_AP_STATE)) // //group key
	{
		if(param->u.crypt.set_tx == 0) //group key
		{
			if(str_cmp(param->u.crypt.alg, "WEP") == 0)
			{
				DBG_8192C("%s, set group_key, WEP\n", __FUNCTION__);

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
				if(param->u.crypt.key_len==13)
				{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
				}

			}
			else if(str_cmp(param->u.crypt.alg, "TKIP") == 0)
			{
				DBG_8192C("%s, set group_key, TKIP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
				//set mic key
				_rtw_memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
				_rtw_memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

				psecuritypriv->busetkipkey = _TRUE;

			}
			else if(str_cmp(param->u.crypt.alg, "CCMP") == 0)
			{
				DBG_8192C("%s, set group_key, CCMP\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _AES_;

				_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
			}
			else
			{
				DBG_8192C("%s, set group_key, none\n", __FUNCTION__);

				psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
			}

			psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

			psecuritypriv->binstallGrpkey = _TRUE;

			psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

			rtw_ap_set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta)
			{
				pbcmc_sta->ieee8021x_blocked = _FALSE;
				pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
			}

		}

		goto exit;

	}

	if(psecuritypriv->dot11AuthAlgrthm == dot11AuthAlgrthm_8021X && psta) // psk/802_1x
	{
		if(check_fwstate(pmlmepriv, WIFI_AP_STATE))
		{
			if(param->u.crypt.set_tx ==1) //pairwise key
			{
				_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

				if(str_cmp(param->u.crypt.alg, "WEP") == 0)
				{
					DBG_8192C("%s, set pairwise key, WEP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{
						psta->dot118021XPrivacy = _WEP104_;
					}
				}
				else if(str_cmp(param->u.crypt.alg, "TKIP") == 0)
				{
					DBG_8192C("%s, set pairwise key, TKIP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _TKIP_;

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
					_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;

				}
				else if(str_cmp(param->u.crypt.alg, "CCMP") == 0)
				{

					DBG_8192C("%s, set pairwise key, CCMP\n", __FUNCTION__);

					psta->dot118021XPrivacy = _AES_;
				}
				else
				{
					DBG_8192C("%s, set pairwise key, none\n", __FUNCTION__);

					psta->dot118021XPrivacy = _NO_PRIVACY_;
				}

				rtw_ap_set_pairwise_key(padapter, psta);

				psta->ieee8021x_blocked = _FALSE;

				psta->bpairwise_key_installed = _TRUE;

			}
			else//group key???
			{
				if(str_cmp(param->u.crypt.alg, "WEP") == 0)
				{
					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					psecuritypriv->dot118021XGrpPrivacy = _WEP40_;
					if(param->u.crypt.key_len==13)
					{
						psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
					}
				}
				else if(str_cmp(param->u.crypt.alg, "TKIP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _TKIP_;

					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					//DEBUG_ERR("set key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len);
					//set mic key
					_rtw_memcpy(psecuritypriv->dot118021XGrptxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[16]), 8);
					_rtw_memcpy(psecuritypriv->dot118021XGrprxmickey[param->u.crypt.idx].skey, &(param->u.crypt.key[24]), 8);

					psecuritypriv->busetkipkey = _TRUE;

				}
				else if(str_cmp(param->u.crypt.alg, "CCMP") == 0)
				{
					psecuritypriv->dot118021XGrpPrivacy = _AES_;

					_rtw_memcpy(psecuritypriv->dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
				}
				else
				{
					psecuritypriv->dot118021XGrpPrivacy = _NO_PRIVACY_;
				}

				psecuritypriv->dot118021XGrpKeyid = param->u.crypt.idx;

				psecuritypriv->binstallGrpkey = _TRUE;

				psecuritypriv->dot11PrivacyAlgrthm = psecuritypriv->dot118021XGrpPrivacy;//!!!

				rtw_ap_set_group_key(padapter, param->u.crypt.key, psecuritypriv->dot118021XGrpPrivacy, param->u.crypt.idx);

				pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
				if(pbcmc_sta)
				{
					pbcmc_sta->ieee8021x_blocked = _FALSE;
					pbcmc_sta->dot118021XPrivacy= psecuritypriv->dot118021XGrpPrivacy;//rx will use bmc_sta's dot118021XPrivacy
				}

			}

		}

	}

exit:

	return ret;

}
#endif

static int rtw_cfg80211_set_encryption(_adapter *padapter, struct ieee_param *param, u32 param_len)
{
	int ret = 0;
	u32 wep_key_idx, wep_key_len,wep_total_len;
	struct mlme_priv 	*pmlmepriv = &padapter->mlmepriv;
	struct security_priv *psecuritypriv = &padapter->securitypriv;
#ifdef CONFIG_P2P
	struct wifidirect_info* pwdinfo = &padapter->wdinfo;
#endif //CONFIG_P2P

_func_enter_;

	DBG_8192C("%s\n", __func__);

	param->u.crypt.err = 0;
	param->u.crypt.alg[IEEE_CRYPT_ALG_NAME_LEN - 1] = '\0';

	if (param_len < (u32) ((u8 *) param->u.crypt.key - (u8 *) param) + param->u.crypt.key_len)
	{
		ret =  -EINVAL;
		goto exit;
	}

	if (param->sta_addr[0] == 0xff && param->sta_addr[1] == 0xff &&
	    param->sta_addr[2] == 0xff && param->sta_addr[3] == 0xff &&
	    param->sta_addr[4] == 0xff && param->sta_addr[5] == 0xff)
	{
		if (param->u.crypt.idx >= WEP_KEYS
#ifdef CONFIG_IEEE80211W
			&& param->u.crypt.idx > BIP_MAX_KEYID
#endif //CONFIG_IEEE80211W
		)
		{
			ret = -EINVAL;
			goto exit;
		}
	} else {
#ifdef CONFIG_WAPI_SUPPORT
		if (str_cmp(param->u.crypt.alg, "SMS4"))
#endif
		{
		ret = -EINVAL;
		goto exit;
	}
	}

	if (str_cmp(param->u.crypt.alg, "WEP") == 0)
	{
		RT_TRACE(_module_rtl871x_ioctl_os_c,_drv_err_,("wpa_set_encryption, crypt.alg = WEP\n"));
		DBG_8192C("wpa_set_encryption, crypt.alg = WEP\n");

		wep_key_idx = param->u.crypt.idx;
		wep_key_len = param->u.crypt.key_len;

		if ((wep_key_idx > WEP_KEYS) || (wep_key_len <= 0))
		{
			ret = -EINVAL;
			goto exit;
		}

		if (psecuritypriv->bWepDefaultKeyIdxSet == 0)
		{
			//wep default key has not been set, so use this key index as default key.

			wep_key_len = wep_key_len <= 5 ? 5 : 13;

              	psecuritypriv->ndisencryptstatus = Ndis802_11Encryption1Enabled;
			psecuritypriv->dot11PrivacyAlgrthm = _WEP40_;
			psecuritypriv->dot118021XGrpPrivacy = _WEP40_;

			if(wep_key_len==13)
			{
				psecuritypriv->dot11PrivacyAlgrthm = _WEP104_;
				psecuritypriv->dot118021XGrpPrivacy = _WEP104_;
			}

			psecuritypriv->dot11PrivacyKeyIndex = wep_key_idx;
		}

		_rtw_memcpy(&(psecuritypriv->dot11DefKey[wep_key_idx].skey[0]), param->u.crypt.key, wep_key_len);

		psecuritypriv->dot11DefKeylen[wep_key_idx] = wep_key_len;

		rtw_set_key(padapter, psecuritypriv, wep_key_idx, 0, _TRUE);

		goto exit;
	}

	if(padapter->securitypriv.dot11AuthAlgrthm == dot11AuthAlgrthm_8021X) // 802_1x
	{
		struct sta_info * psta,*pbcmc_sta;
		struct sta_priv * pstapriv = &padapter->stapriv;

		//DBG_8192C("%s, : dot11AuthAlgrthm == dot11AuthAlgrthm_8021X \n", __func__);

		if (check_fwstate(pmlmepriv, WIFI_STATION_STATE | WIFI_MP_STATE) == _TRUE) //sta mode
		{
			psta = rtw_get_stainfo(pstapriv, get_bssid(pmlmepriv));
			if (psta == NULL) {
				//DEBUG_ERR( ("Set wpa_set_encryption: Obtain Sta_info fail \n"));
				DBG_8192C("%s, : Obtain Sta_info fail \n", __func__);
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (str_cmp(param->u.crypt.alg, "none") != 0)
					psta->ieee8021x_blocked = _FALSE;


				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					psta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}

				if(param->u.crypt.set_tx ==1)//pairwise key
				{

					DBG_8192C("%s, : param->u.crypt.set_tx ==1 \n", __func__);

					_rtw_memcpy(psta->dot118021x_UncstKey.skey,  param->u.crypt.key, (param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));

					if(str_cmp(param->u.crypt.alg, "TKIP") == 0)//set mic key
					{
						//DEBUG_ERR(("\nset key length :param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						_rtw_memcpy(psta->dot11tkiptxmickey.skey, &(param->u.crypt.key[16]), 8);
						_rtw_memcpy(psta->dot11tkiprxmickey.skey, &(param->u.crypt.key[24]), 8);

						padapter->securitypriv.busetkipkey=_FALSE;
						//_set_timer(&padapter->securitypriv.tkip_timer, 50);
					}

					//DEBUG_ERR((" param->u.crypt.key_len=%d\n",param->u.crypt.key_len));
					DBG_871X(" ~~~~set sta key:unicastkey\n");

					rtw_setstakey_cmd(padapter, psta, UNICAST_KEY, _TRUE);
				}
				else//group key
				{
					if(str_cmp(param->u.crypt.alg, "TKIP") == 0 || str_cmp(param->u.crypt.alg, "CCMP") == 0)
					{
						_rtw_memcpy(padapter->securitypriv.dot118021XGrpKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						_rtw_memcpy(padapter->securitypriv.dot118021XGrptxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[16]),8);
						_rtw_memcpy(padapter->securitypriv.dot118021XGrprxmickey[param->u.crypt.idx].skey,&(param->u.crypt.key[24]),8);
	                                        padapter->securitypriv.binstallGrpkey = _TRUE;
						//DEBUG_ERR((" param->u.crypt.key_len=%d\n", param->u.crypt.key_len));
						DBG_871X(" ~~~~set sta key:groupkey\n");

						padapter->securitypriv.dot118021XGrpKeyid = param->u.crypt.idx;
						rtw_set_key(padapter,&padapter->securitypriv,param->u.crypt.idx, 1, _TRUE);
					}
#ifdef CONFIG_IEEE80211W
					else if(str_cmp(param->u.crypt.alg, "BIP") == 0)
					{
						int no;
						//DBG_871X("BIP key_len=%d , index=%d @@@@@@@@@@@@@@@@@@\n", param->u.crypt.key_len, param->u.crypt.idx);
						//save the IGTK key, length 16 bytes
						_rtw_memcpy(padapter->securitypriv.dot11wBIPKey[param->u.crypt.idx].skey,  param->u.crypt.key,(param->u.crypt.key_len>16 ?16:param->u.crypt.key_len));
						/*DBG_871X("IGTK key below:\n");
						for(no=0;no<16;no++)
							printk(" %02x ", padapter->securitypriv.dot11wBIPKey[param->u.crypt.idx].skey[no]);
						DBG_871X("\n");*/
						padapter->securitypriv.dot11wBIPKeyid = param->u.crypt.idx;
						padapter->securitypriv.binstallBIPkey = _TRUE;
						DBG_871X(" ~~~~set sta key:IGKT\n");
					}
#endif //CONFIG_IEEE80211W

#ifdef CONFIG_P2P
					if(pwdinfo->driver_interface == DRIVER_CFG80211 )
					{
						if(rtw_p2p_chk_state(pwdinfo, P2P_STATE_PROVISIONING_ING))
						{
							rtw_p2p_set_state(pwdinfo, P2P_STATE_PROVISIONING_DONE);
						}
					}
#endif //CONFIG_P2P

				}
			}

			pbcmc_sta=rtw_get_bcmc_stainfo(padapter);
			if(pbcmc_sta==NULL)
			{
				//DEBUG_ERR( ("Set OID_802_11_ADD_KEY: bcmc stainfo is null \n"));
			}
			else
			{
				//Jeff: don't disable ieee8021x_blocked while clearing key
				if (str_cmp(param->u.crypt.alg, "none") != 0)
					pbcmc_sta->ieee8021x_blocked = _FALSE;

				if((padapter->securitypriv.ndisencryptstatus == Ndis802_11Encryption2Enabled)||
						(padapter->securitypriv.ndisencryptstatus ==  Ndis802_11Encryption3Enabled))
				{
					pbcmc_sta->dot118021XPrivacy = padapter->securitypriv.dot11PrivacyAlgrthm;
				}
			}
		}
		else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE)) //adhoc mode
		{
		}
	}

#ifdef CONFIG_WAPI_SUPPORT
	if (str_cmp(param->u.crypt.alg, "SMS4") == 0)
	{
		PRT_WAPI_T			pWapiInfo = &padapter->wapiInfo;
		PRT_WAPI_STA_INFO	pWapiSta;
		u8					WapiASUEPNInitialValueSrc[16] = {0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;
		u8					WapiAEPNInitialValueSrc[16] = {0x37,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;
		u8 					WapiAEMultiCastPNInitialValueSrc[16] = {0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C,0x36,0x5C} ;

		if(param->u.crypt.set_tx == 1)
		{
			list_for_each_entry(pWapiSta, &pWapiInfo->wapiSTAUsedList, list) {
				if(_rtw_memcmp(pWapiSta->PeerMacAddr,param->sta_addr,6))
				{
					_rtw_memcpy(pWapiSta->lastTxUnicastPN,WapiASUEPNInitialValueSrc,16);

					pWapiSta->wapiUsk.bSet = true;
					_rtw_memcpy(pWapiSta->wapiUsk.dataKey,param->u.crypt.key,16);
					_rtw_memcpy(pWapiSta->wapiUsk.micKey,param->u.crypt.key+16,16);
					pWapiSta->wapiUsk.keyId = param->u.crypt.idx ;
					pWapiSta->wapiUsk.bTxEnable = true;

					_rtw_memcpy(pWapiSta->lastRxUnicastPNBEQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNBKQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNVIQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPNVOQueue,WapiAEPNInitialValueSrc,16);
					_rtw_memcpy(pWapiSta->lastRxUnicastPN,WapiAEPNInitialValueSrc,16);
					pWapiSta->wapiUskUpdate.bTxEnable = false;
					pWapiSta->wapiUskUpdate.bSet = false;

					if (psecuritypriv->sw_encrypt== false || psecuritypriv->sw_decrypt == false)
					{
						//set unicast key for ASUE
						rtw_wapi_set_key(padapter, &pWapiSta->wapiUsk, pWapiSta, false, false);
					}
				}
			}
		}
		else
		{
			list_for_each_entry(pWapiSta, &pWapiInfo->wapiSTAUsedList, list) {
				if(_rtw_memcmp(pWapiSta->PeerMacAddr,get_bssid(pmlmepriv),6))
				{
					pWapiSta->wapiMsk.bSet = true;
					_rtw_memcpy(pWapiSta->wapiMsk.dataKey,param->u.crypt.key,16);
					_rtw_memcpy(pWapiSta->wapiMsk.micKey,param->u.crypt.key+16,16);
					pWapiSta->wapiMsk.keyId = param->u.crypt.idx ;
					pWapiSta->wapiMsk.bTxEnable = false;
					if(!pWapiSta->bSetkeyOk)
						pWapiSta->bSetkeyOk = true;
					pWapiSta->bAuthenticateInProgress = false;

					_rtw_memcpy(pWapiSta->lastRxMulticastPN, WapiAEMultiCastPNInitialValueSrc, 16);

					if (psecuritypriv->sw_decrypt == false)
					{
						//set rx broadcast key for ASUE
						rtw_wapi_set_key(padapter, &pWapiSta->wapiMsk, pWapiSta, true, false);
					}
				}

			}
		}
	}
#endif


exit:

	DBG_8192C("%s, ret=%d\n", __func__, ret);

	_func_exit_;

	return ret;
}

enum wpa_alg {
	WPA_ALG_NONE,
	WPA_ALG_WEP,
	WPA_ALG_TKIP,
	WPA_ALG_CCMP,
	WPA_ALG_IGTK,
	WPA_ALG_PMK
};
static inline int is_broadcast_ether_addr(const u8 *a)
{
	return (a[0] & a[1] & a[2] & a[3] & a[4] & a[5]) == 0xff;
}

int cfg80211_rtw_add_key(struct net_device *dev,
            u32 cipher, u8 *mac_addr, u8 key_index,
            u8 *seq, int seq_len, u8 *key, int key_len)
{
    // TODO: there is no dev info actually
    _adapter *padapter = dev->priv; //rtw_sw_export;
	char *alg_name;
	u32 param_len;
	struct ieee_param *param = NULL;
	int ret=0;
	struct mlme_priv *pmlmepriv = &padapter->mlmepriv;
#ifdef CONFIG_TDLS
	struct sta_info *ptdls_sta;
#endif /* CONFIG_TDLS */

    cassert(OS_NULL == dev->priv);

	DBG_871X(FUNC_NDEV_FMT" adding key for %pM\n", FUNC_NDEV_ARG(ndev), mac_addr);
	DBG_871X("cipher=0x%x\n", cipher);
	DBG_871X("key_len=0x%x\n", key_len);
	DBG_871X("seq_len=0x%x\n", seq_len);
	DBG_871X("key_index=%d\n", key_index);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37)) || defined(COMPAT_KERNEL_RELEASE)
	DBG_871X("pairwise=%d\n", pairwise);
#endif	// (LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,37))

	param_len = sizeof(struct ieee_param) + key_len;
	param = (struct ieee_param *)rtw_malloc(param_len);
	if (param == NULL)
		return -1;

	_rtw_memset(param, 0, param_len);

	param->cmd = IEEE_CMD_SET_ENCRYPTION;
	_rtw_memset(param->sta_addr, 0xff, ETH_ALEN);

	switch (cipher) {
	case WPA_ALG_NONE:
		//todo: remove key
		//remove = 1;
		alg_name = "none";
		break;
	case WPA_ALG_WEP:
		alg_name = "WEP";
		break;
	case WPA_ALG_TKIP:
		alg_name = "TKIP";
		break;
	case WPA_ALG_CCMP:
		alg_name = "CCMP";
		break;
#ifdef CONFIG_IEEE80211W
	case WLAN_CIPHER_SUITE_AES_CMAC:
		alg_name = "BIP";
		break;
#endif //CONFIG_IEEE80211W
#ifdef CONFIG_WAPI_SUPPORT
	case WLAN_CIPHER_SUITE_SMS4:
		alg_name= "SMS4";
		if(pairwise == NL80211_KEYTYPE_PAIRWISE) {
			if (key_index != 0 && key_index != 1) {
				ret = -ENOTSUPP;
				goto addkey_end;
			}
			_rtw_memcpy((void*)param->sta_addr, (void*)mac_addr, ETH_ALEN);
		} else {
			DBG_871X("mac_addr is null \n");
		}
		DBG_871X("rtw_wx_set_enc_ext: SMS4 case \n");
		break;
#endif

	default:
		ret = -1;
		goto addkey_end;
	}

	str_ncpy((char *)param->u.crypt.alg, alg_name, IEEE_CRYPT_ALG_NAME_LEN);

	if (!mac_addr || is_broadcast_ether_addr(mac_addr))
	{
		param->u.crypt.set_tx = 0; //for wpa/wpa2 group key
	} else {
		param->u.crypt.set_tx = 1; //for wpa/wpa2 pairwise key
	}


	//param->u.crypt.idx = key_index - 1;
	param->u.crypt.idx = key_index;

	if (seq_len && seq)
	{
		_rtw_memcpy(param->u.crypt.seq, (u8 *)seq, seq_len);
	}

	if(key_len && key)
	{
		param->u.crypt.key_len = key_len;
		_rtw_memcpy(param->u.crypt.key, (u8 *)key, key_len);
	}

	if(check_fwstate(pmlmepriv, WIFI_STATION_STATE) == _TRUE)
	{
#ifdef CONFIG_TDLS
		if (rtw_tdls_is_driver_setup(padapter) == _FALSE && mac_addr) {
			ptdls_sta = rtw_get_stainfo(&padapter->stapriv, (void *)mac_addr);
			if (ptdls_sta != NULL && ptdls_sta->tdls_sta_state) {
				_rtw_memcpy(ptdls_sta->tpk.tk, params->key, params->key_len);
				rtw_tdls_set_key(padapter, ptdls_sta);
				goto addkey_end;
			}
		}
#endif /* CONFIG_TDLS */

		ret =  rtw_cfg80211_set_encryption(padapter, param, param_len);
	}
	else if(check_fwstate(pmlmepriv, WIFI_AP_STATE) == _TRUE)
	{
#ifdef CONFIG_AP_MODE
		if(mac_addr)
			_rtw_memcpy(param->sta_addr, (void*)mac_addr, ETH_ALEN);

		ret = rtw_cfg80211_ap_set_encryption(padapter, param, param_len);
#endif
	}
        else if(check_fwstate(pmlmepriv, WIFI_ADHOC_STATE) == _TRUE
                || check_fwstate(pmlmepriv, WIFI_ADHOC_MASTER_STATE) == _TRUE)
        {
                //DBG_8192C("@@@@@@@@@@ fw_state=0x%x, iftype=%d\n", pmlmepriv->fw_state, rtw_wdev->iftype);
                ret =  rtw_cfg80211_set_encryption(padapter, param, param_len);
        }
	else
	{
		DBG_8192C("error! fw_state=0x%x\n", pmlmepriv->fw_state);

	}

addkey_end:
	if(param)
	{
		rtw_mfree((u8*)param, param_len);
	}

	return ret;

}

static int _new_issue_nulldata(_adapter *padapter, unsigned char *da, unsigned int power_mode, int wait_ack)
{
	int ret = _FAIL;
	struct xmit_frame			*pmgntframe;
	struct pkt_attrib			*pattrib;
	unsigned char					*pframe;
	struct rtw_ieee80211_hdr	*pwlanhdr;
	unsigned short				*fctrl;
	struct xmit_priv	*pxmitpriv;
	struct mlme_ext_priv	*pmlmeext;
	struct mlme_ext_info	*pmlmeinfo;

	DBG_871X("%s:%d\n", __FUNCTION__, power_mode);

	if(!padapter)
		goto exit;

	pxmitpriv = &(padapter->xmitpriv);
	pmlmeext = &(padapter->mlmeextpriv);
	pmlmeinfo = &(pmlmeext->mlmext_info);

	if ((pmgntframe = alloc_mgtxmitframe(pxmitpriv)) == NULL)
	{
		goto exit;
	}

	//update attribute
	pattrib = &pmgntframe->attrib;
	update_mgntframe_attrib(padapter, pattrib);
	pattrib->retry_ctrl = _FALSE;

	_rtw_memset(pmgntframe->buf_addr, 0, WLANHDR_OFFSET + TXDESC_OFFSET);

	pframe = (u8 *)(pmgntframe->buf_addr) + TXDESC_OFFSET;
	pwlanhdr = (struct rtw_ieee80211_hdr *)pframe;

	fctrl = &(pwlanhdr->frame_ctl);
	*(fctrl) = 0;

	if((pmlmeinfo->state&0x03) == WIFI_FW_AP_STATE)
	{
		SetFrDs(fctrl);
	}
	else if((pmlmeinfo->state&0x03) == WIFI_FW_STATION_STATE)
	{
		SetToDs(fctrl);
	}

	if (power_mode)
	{
		SetPwrMgt(fctrl);
	}

	_rtw_memcpy(pwlanhdr->addr1, da, ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr2, adapter_mac_addr(padapter), ETH_ALEN);
	_rtw_memcpy(pwlanhdr->addr3, get_my_bssid(&(pmlmeinfo->network)), ETH_ALEN);

	SetSeqNum(pwlanhdr, pmlmeext->mgnt_seq);
	pmlmeext->mgnt_seq++;
	SetFrameSubType(pframe, WIFI_DATA);

	pframe += sizeof(struct rtw_ieee80211_hdr_3addr);
	pattrib->pktlen = sizeof(struct rtw_ieee80211_hdr_3addr);

	pattrib->last_txcmdsz = pattrib->pktlen;

	if(wait_ack)
	{
		ret = dump_mgntframe_and_wait_ack(padapter, pmgntframe);
	}
	else
	{
		dump_mgntframe(padapter, pmgntframe);
		ret = _SUCCESS;
	}

exit:
	return ret;
}


int new_issue_nulldata(_adapter *padapter)
{
	int ret;
	int i = 0;
	struct mlme_ext_priv	*pmlmeext = &(padapter->mlmeextpriv);
	struct mlme_ext_info	*pmlmeinfo = &(pmlmeext->mlmext_info);
	struct sta_info *psta;
	unsigned char *da;
	unsigned int power_mode;
    int try_cnt;

    try_cnt = 3;
    power_mode = 0;
    da = get_my_bssid(&(pmlmeinfo->network));

	psta = rtw_get_stainfo(&padapter->stapriv, da);
	if (psta) {
		if (power_mode)
			rtw_hal_macid_sleep(padapter, psta->mac_id);
		else
			rtw_hal_macid_wakeup(padapter, psta->mac_id);
	} else {
		DBG_871X(FUNC_ADPT_FMT ": Can't find sta info for " MAC_FMT ", skip macid %s!!\n",
			FUNC_ADPT_ARG(padapter), MAC_ARG(da), power_mode?"sleep":"wakeup");
		rtw_warn_on(1);
	}

	do {
		ret = _new_issue_nulldata(padapter, da, power_mode, _FALSE);

		i++;

		if (padapter->bDriverStopped || padapter->bSurpriseRemoved)
			break;

		rtw_msleep_os(100);

	}while((i<try_cnt) && (ret==_FAIL));

	return ret;
}

int debug_timer_cnt = 0;

/* 192.168.29.220 */
#define STATIC_IP_ADDR 0xc0a81ddc

void create_wpa_dev(_adapter *padapter)
{
    os_u8 mac_addr[6];
    os_u8 ip_addr[4];
    ETH_DEV_INFO_STRU dev;

    dev.dev_no = alloc_device_id();
    padapter->dev_id = dev.dev_no;
    mem_cpy(dev.mac_addr, adapter_mac_addr(padapter), ETH_ALEN);
    dev.SendPacket = wifi_sendpacket;
    dev.connect = rtw_conn;
    dev.disconnect = rtw_disconn;
    dev.dedicated = padapter;

    Eth_RegDevice(&dev);
    //IP_AddLocalIPAddr(dev.dev_no, STATIC_IP_ADDR, 0xffffff00);
    //IP_AddLocalIPAddr(dev.dev_no, 0xc0a80709, 0xffffff00);
}
