#ifndef __GX8002_NPU_H__
#define __GX8002_NPU_H__

enum GX8002_MSG {
    GX8002_ENC_MSG_RUN = ('G' << 24) | ('E' << 16) | ('N' << 8) | ('C' << 0),
    GX8002_ENC_MSG_CLOSE,
};


#endif /* #ifndef __GX8002_NPU_H__ */
