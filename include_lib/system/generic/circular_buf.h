#ifndef CIRCULAR_BUF_INTERFACE_H
#define CIRCULAR_BUF_INTERFACE_H

#include "typedef.h"
#include "system/spinlock.h"




#ifndef CONFIG_CBUF_IN_MASKROM

/* --------------------------------------------------------------------------*/
/**
 * @brief cbuffer结构体
 */
/* ----------------------------------------------------------------------------*/
typedef struct _cbuffer {
    u8  *begin;
    u8  *end;
    u8  *read_ptr;
    u8  *write_ptr;
    u8  *tmp_ptr ;
    u32 tmp_len;
    u32 data_len;
    u32 total_len;
    spinlock_t lock;
} cbuffer_t;

#else /* #ifndef CONFIG_CBUF_IN_MASKROM */

/* ---------------------------------------------------------------------------- */
/**
 * @brief cbuffer结构体
 * @note: 以下结构体成员不可修改
 */
/* ---------------------------------------------------------------------------- */
typedef struct _cbuffer {
    u8  *begin;
    u8  *end;
    u8  *read_ptr;
    u8  *write_ptr;
    u8  *tmp_ptr ;
    u32 tmp_len;
    u32 data_len;
    u32 total_len;
    spinlock_t lock;
} cbuffer_t;

#endif /* #ifndef CONFIG_CBUF_IN_MASKROM */


/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief cbuffer初始化
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] buf 缓存空间
 * @param [in] size 缓存总大小
 */
/* ----------------------------------------------------------------------------*/
void cbuf_init(cbuffer_t *cbuffer, void *buf, u32 size);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理
 * @brief 把cbuffer_t结构体管理的内存空间的数据拷贝到buf数组

 * @param [in] cbuffer cbuffer 句柄
 * @param [out] buf 指向用于存储读取内容的目标数组
 * @param [in] len 要读取的字节长度
 *
 * @return 成功读取的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_read(cbuffer_t *cbuffer, void *buf, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理
 * @brief 把buf数组数据拷贝cbuffer_t结构体管理的内存空间
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] buf 指向用于存储写入内容的目标数组
 * @param [in] len 要写入的字节长度
 *
 * @return 成功写入的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_write(cbuffer_t *cbuffer, void *buf, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 判断是否可写入len字节长度的数据
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] len len字节长度的数据
 *
 * @return  返回可以写入的len字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_is_write_able(cbuffer_t *cbuffer, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:外部内存管理
 * @brief 预分配待写入数据的空间，要和cbuf_write_updata()配套使用,更新cbuf管理handle数据。
 *
 * @param [in] cbuffer cbuffer句柄
 * @param [in] len 回传可以最多写入len字节长度的数据
 *
 * @return 当前写指针的地址
 */
/* ----------------------------------------------------------------------------*/
void *cbuf_write_alloc(cbuffer_t *cbuffer, u32 *len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:外部内存管理
 * @brief 更新cbuf管理handle的写指针位置和数据长度
 *
 * @param [in] cbuffer cbuffer句柄
 * @param [in] len 在非cbuffer_t结构体包含的内存空间中写入的数据的实际字节长度
 *
 * @return 当前写指针的地址
 */
/* ----------------------------------------------------------------------------*/
void cbuf_write_updata(cbuffer_t *cbuffer, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:外部内存管理
 * @brief 预分配待读取数据的空间,需要和cbuf_read_updata()配套使用,更新cbuf管理handle数据
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] len 回传可以最多读取len字节长度的数据
 *
 * @return 当前读指针的地址
 */
/* ----------------------------------------------------------------------------*/
void *cbuf_read_alloc(cbuffer_t *cbuffer, u32 *len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:外部内存管理
 * @brief 更新cbuf管理handle的读指针位置和数据长度
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] len 在非cbuffer_t结构体包含的内存空间中读取的数据的实际字节长度
 */
/* ----------------------------------------------------------------------------*/
void cbuf_read_updata(cbuffer_t *cbuffer, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:外部内存管理
 * @brief 清空cbuffer空间
 *
 * @param [in] cbuffer cbuffer 句柄

 */
/* ----------------------------------------------------------------------------*/
void cbuf_clear(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理。
 * @brief 指定位置进行数据重写
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] begin 指向需要重写内容的开始地址
 * @param [out] buf 指向用于存储重写内容的目标数组
 * @param [in] len 待重写内容的长度
 *
 * @return 成功重写的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_rewrite(cbuffer_t *cbuffer, void *begin, void *buf, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理
 * @brief 更新指向上一次操作的指针为当前指针,并刷新数据长度
 *
 * @param [in] cbuffer cbuffer 句柄
 */
/* ----------------------------------------------------------------------------*/
void  cbuf_discard_prewrite(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理
 * @brief 操作指针回退到上一次的操作位置,并刷新数据长度
 *
 * @param [in] cbuffer cbuffer 句柄
 */
/* ----------------------------------------------------------------------------*/
void cbuf_updata_prewrite(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:cb_memcpy管理
 * @brief 回退写入内容，从上一次的操作的指针处进行覆盖填充
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [out] buf 指向用于覆盖写入内容的目标数组
 * @param [in] len 填充数据的字节长度
 *
 * @return 成功填充数据的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_prewrite(cbuffer_t *cbuffer, void *buf, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 获取指向写指针的地址
 *
 * @param [in] cbuffer cbuffer 句柄
 *
 * @return 写指针的地址
 */
/* ----------------------------------------------------------------------------*/
void *cbuf_get_writeptr(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 获取cbuffer存储的数据的字节长度
 *
 * @param [in] cbuffer cbuffer句柄
 *
 * @return 获取cbuffer存储的数据的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_get_data_size(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 获取指向读指针的地址
 *
 * @param [in] cbuffer cbuffer句柄
 *
 * @return 读指针的地址
 */
/* ----------------------------------------------------------------------------*/
void *cbuf_get_readptr(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 读指针向后回退
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] len 要回退的字节长度
 *
 * @return 成功回退的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_read_goback(cbuffer_t *cbuffer, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:全局
 * @brief 获取存储数据的字节长度
 *
 * @param [in] cbuffer cbuffer 句柄
 *
 * @return 存储数据的字节长度
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_get_data_len(cbuffer_t *cbuffer);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:memcpy管理+cbuf_read_alloc系列管理函数
 * @brief 预分配待读取数据的空间,并把读取到的数据存入buf数组
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [out] buf 存储读取的数据的目标buf数组
 * @param [in] len 要读取的数据的字节长度
 *
 * @return
 */
/* ----------------------------------------------------------------------------*/
u32 cbuf_read_alloc_len(cbuffer_t *cbuffer, void *buf, u32 len);

/* --------------------------------------------------------------------------*/
/**
 * @brief 适用范围:memcpy管理+cbuf_read_alloc系列管理函数
 * @brief 更新cbuf管理handle的读指针位置和数据长度
 *
 * @param [in] cbuffer cbuffer 句柄
 * @param [in] len 要更新的数据的字节长度
 */
/* ----------------------------------------------------------------------------*/
void cbuf_read_alloc_len_updata(cbuffer_t *cbuffer, u32 len);

#endif

