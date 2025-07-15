#include "includes.h"
#include "gx_fifo.h"

#define os_mutex_t           OS_MUTEX *
#define gx_os_mutex_lock     os_mutex_pend
#define gx_os_mutex_unlock 	 os_mutex_post

#define os_sem_t             OS_SEM *
#define gx_os_sem_post       os_sem_post
#define gx_os_sem_wait 	     os_sem_pend

typedef struct {
    char *data;
    unsigned int length;
} fifoBlock;

typedef struct {
    int fifo_head;
    int fifo_tail;
    int block_num;
    int block_total;
    int block_size;
    fifoBlock *fifo_block;
    os_mutex_t fifo_mutex;
    os_sem_t  fifo_sem;
} GxFIFO;


//======================== OS porting ======================================//
static void *gx_os_calloc(u32 nmemb, u32 size)
{
    return zalloc(nmemb * size);
}

static void gx_os_free(void *p)
{
    if (p) {
        free(p);
    }
}

static os_mutex_t gx_os_mutex_create()
{
    os_mutex_t mutex;
    mutex = (os_mutex_t)gx_os_calloc((sizeof(*mutex)), 1);

    ASSERT(mutex, "mutex alloc err");

    os_mutex_create(mutex);

    return mutex;
}

static void gx_os_mutex_destroy(os_mutex_t mutex)
{
    if (mutex) {
        os_mutex_del(mutex, 0);
        gx_os_free(mutex);
    }
}

static os_sem_t gx_os_sem_create(u32 cnt)
{
    os_sem_t sem;
    sem = (os_mutex_t)gx_os_calloc((sizeof(*sem)), 1);

    ASSERT(sem, "sem alloc err");

    os_sem_create(sem, cnt);

    return sem;
}

static void gx_os_sem_destroy(os_sem_t sem)
{
    if (sem) {
        os_sem_del(sem, 0);
        gx_os_free(sem);
    }
}



//==============================================================//

int gx_fifo_destroy(fifo_handle_t fifo)
{
    GxFIFO *temp_fifo = (GxFIFO *)fifo;
    int i = 0;

    if (temp_fifo == NULL) {
        return -1;
    }

    gx_os_mutex_lock(temp_fifo->fifo_mutex, 0);
    if (temp_fifo->fifo_block != NULL) {
        for (i = 0; i < temp_fifo->block_total; i++) {
            if (temp_fifo->fifo_block[i].data != NULL) {
                gx_os_free(temp_fifo->fifo_block[i].data);
                temp_fifo->fifo_block[i].data = NULL;
            }
        }
        gx_os_free(temp_fifo->fifo_block);
        temp_fifo->fifo_block = NULL;
    }

    gx_os_sem_destroy(temp_fifo->fifo_sem);
    gx_os_mutex_destroy(temp_fifo->fifo_mutex);

    gx_os_free(temp_fifo);

    return 0;
}

fifo_handle_t gx_fifo_create(int block_size, int block_num)
{
    GxFIFO *temp_fifo = NULL;
    int i = 0;

    if ((block_size == 0) || (block_num == 0)) {
        goto fifo_failed;
    }

    if ((temp_fifo = (GxFIFO *)gx_os_calloc(sizeof(GxFIFO), 1)) == NULL) {
        goto fifo_failed;
    }

    if ((temp_fifo->fifo_block = (fifoBlock *)gx_os_calloc(block_num * sizeof(fifoBlock), 1)) == NULL) {
        goto fifo_failed;
    }

    temp_fifo->fifo_mutex = gx_os_mutex_create();
    temp_fifo->fifo_sem = gx_os_sem_create(0);

    for (i = 0; i < block_num; i++) {
        if ((temp_fifo->fifo_block[i].data = (char *)gx_os_calloc(block_size, 1)) == NULL) {
            goto fifo_failed;
        }
    }

    temp_fifo->block_total = block_num;
    temp_fifo->block_size = block_size;
    temp_fifo->fifo_head = 0;
    temp_fifo->fifo_tail = -1;

    return (fifo_handle_t)temp_fifo;

fifo_failed:
    gx_fifo_destroy((fifo_handle_t)temp_fifo);
    return 0;
}

int gx_fifo_write(fifo_handle_t fifo, const char *write_data,  int write_size)
{
    GxFIFO *temp_fifo = (GxFIFO *)fifo;
    int cp_size = 0;

    if ((temp_fifo == NULL) || (write_data == NULL) || (write_size == 0)) {
        return 0;
    }

    gx_os_mutex_lock(temp_fifo->fifo_mutex, 0);
    if (temp_fifo->block_num >= temp_fifo->block_total) {
        printf("[%s]%d: fifo full!!!\n", __func__, __LINE__);
        gx_os_mutex_unlock(temp_fifo->fifo_mutex);
        return 0;
    }

    temp_fifo->fifo_tail++;
    if (temp_fifo->fifo_tail >= temp_fifo->block_total) {
        temp_fifo->fifo_tail = 0;
    }

    (write_size >= temp_fifo->block_size)
    ? (cp_size = temp_fifo->block_size) : (cp_size = write_size);
    memcpy(temp_fifo->fifo_block[temp_fifo->fifo_tail].data, write_data, cp_size);
    temp_fifo->fifo_block[temp_fifo->fifo_tail].length = cp_size;
#if 1
    if (temp_fifo->block_num == 0) {
        //printf("[%s]%d new data: %d!!!\n", __func__, __LINE__, os_sem_query(temp_fifo->fifo_sem));
        gx_os_sem_post(temp_fifo->fifo_sem);
    }
#endif
    temp_fifo->block_num++;

    // gx_os_sem_post(temp_fifo->fifo_sem);
    gx_os_mutex_unlock(temp_fifo->fifo_mutex);

    return cp_size;
}

int gx_fifo_read(fifo_handle_t fifo, char *read_data,  int read_size, int timeout)
{
    GxFIFO *temp_fifo = (GxFIFO *)fifo;
    int cp_size = 0;

    if ((temp_fifo == NULL) || (read_data == NULL) || (read_size == 0)) {
        return 0;
    }
#if 0
    int ret = gx_os_sem_wait(temp_fifo->fifo_sem, timeout);
    if (ret != 0) {
        printf("[%s]%d: wait data timeout: %d !!!!!!!\n", __func__, __LINE__, ret);
        return 0;
    }
#endif
    gx_os_mutex_lock(temp_fifo->fifo_mutex, 0);

    while (temp_fifo->block_num == 0) {
        //printf("[%s]%d: fifo empty: %d!!!\n", __func__, __LINE__, gx_os_sem_query(temp_fifo->fifo_sem));
        gx_os_mutex_unlock(temp_fifo->fifo_mutex);

        if (gx_os_sem_wait(temp_fifo->fifo_sem, timeout) != 0) {
            printf("[%s]%d: wait data timeout !!!!!!!\n", __func__, __LINE__);
            return 0;
        }
        //os_sem_pend(&temp_fifo->fifo_sem, 0);
        //os_mutex_pend(&temp_fifo->fifo_mutex, 0);
        gx_os_mutex_lock(temp_fifo->fifo_mutex, 0);
        //printf("[%s]%d, fifo has data: %d!!!\n", __func__, __LINE__, os_sem_query(&temp_fifo->fifo_sem));
        //return 0;
    }

    (read_size >= temp_fifo->fifo_block[temp_fifo->fifo_head].length)
    ? (cp_size = temp_fifo->fifo_block[temp_fifo->fifo_head].length) : (cp_size = read_size);
    memcpy(read_data, temp_fifo->fifo_block[temp_fifo->fifo_head].data, cp_size);
    read_size = temp_fifo->fifo_block[temp_fifo->fifo_head].length;

    temp_fifo->fifo_head++;
    if (temp_fifo->fifo_head >= temp_fifo->block_total) {
        temp_fifo->fifo_head = 0;
    }

    temp_fifo->block_num--;

    gx_os_mutex_unlock(temp_fifo->fifo_mutex);

    return cp_size;
}
