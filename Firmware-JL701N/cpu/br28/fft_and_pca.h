#include "typedef.h"


#define PCA_TASK_NAME "pca"

#define N_FFT 512
#define LOG2N 9

#define MIC_QUEUE_SIZE 4000
#define SPK_QUEUE_SIZE 2000

#define MIC_QUEUE_LEAST 2000
#define SPK_QUEUE_LEAST 0

#define OVERLAP_SIZE 1
#define HOP_SIZE 256
#define DCT_SIZE 257        // OVERLAP_SIZE + HOP_SIZE

#define PCA_SIZE 47
#define PCA_BYTE 47

#define SIGN_BYTE 33        // ceil(N_DCT / (sizeof(u16) * 8))

#define BLOCK_BYTE 80        // PCA_BYTE + SIGN_BYTE

#define BLOCK_NUM 3
#define BLOCKS_BYTE 240     // BLOCK_SIZE * BLOCK_NUM

#define HEADER_BYTE 1
#define DEBUG_BYTE 3
#define PACKAGE_BYTE 244    // BLOCKS_BYTE + DEBUG_BYTE

#define OPUS_PART_BYTE 40
#define OPUS_PACKAGE_BYTE 84

#define MAX_BUFFER_COUNT 5
#define MAX_CONFLICT_COUNT 5


void mic_insert_data(s16* data, int len);
void spk_insert_data(s16* data, int len);

int pca_open();
