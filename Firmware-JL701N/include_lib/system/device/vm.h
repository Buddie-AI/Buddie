#ifndef _VM_H_
#define _VM_H_

#include "ioctl.h"
#include "device/device.h"

#define IOCTL_SET_VM_INFO               _IOW('V', 1, 1)
#define IOCTL_GET_VM_INFO               _IOW('V', 2, 1)
// enum {
//     #<{(|
//      *  用户自定义配置项 (0-64)
//      |)}>#
//     #<{(| ... |)}>#
// }

// VM define and api
typedef u16 vm_hdl;

struct vm_table {
    u16  index;
    u16 value_byte;
    int value;      //cache value which value_byte <= 4
};

typedef enum _vm_err {
    VM_ERR_NONE = 0,
    VM_INDEX_ERR = -0x100,
    VM_INDEX_EXIST,     //0xFF
    VM_DATA_LEN_ERR,    //0xFE
    VM_READ_NO_INDEX,   //0xFD
    VM_READ_DATA_ERR,   //0xFC
    VM_WRITE_OVERFLOW,  //0xFB
    VM_NOT_INIT,
    VM_INIT_ALREADY,
    VM_DEFRAG_ERR,
    VM_ERR_INIT,
    VM_ERR_PROTECT
} VM_ERR;


// vm api
VM_ERR vm_eraser(void);
VM_ERR vm_init(void *dev_hdl, u32 vm_addr, u32 vm_len, u8 vm_mode);
//VM_ERR vm_db_create_table(const struct vm_table *table, int num);
void vm_check_all(u8 level);    //level : default 0
u8   get_vm_statu(void);
// io api
//s32 vm_read(vm_hdl hdl, void *data_buf, u16 len);
//s32 vm_write(vm_hdl hdl, const void *data_buf, u16 len);

void spi_port_hd(u8 level);

bool sfc_erase_zone(u32 addr, u32 len);

void vm_api_write_mult(u16 start_id, u16 end_id, void *buf, u16 len, u32 delay);
int vm_api_read_mult(u16 start_id, u16 end_id, void *buf, u16 len);


#endif  //_VM_H_

