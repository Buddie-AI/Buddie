#ifndef __GX_FIFO_H__
#define __GX_FIFO_H__

typedef int fifo_handle_t;
fifo_handle_t gx_fifo_create(int block_size, int block_num);
int gx_fifo_destroy(fifo_handle_t fifo);
int gx_fifo_write(fifo_handle_t fifo, const char *write_data,  int write_size);
int gx_fifo_read(fifo_handle_t fifo, char *read_data, int read_size, int timeout);

#endif
