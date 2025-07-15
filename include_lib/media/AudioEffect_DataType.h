#ifndef _AUDIOEFFECT_DATATYPE_
#define _AUDIOEFFECT_DATATYPE_

enum PCMDataType {
    DATA_INT_16BIT = 0,
    DATA_INT_32BIT,
    DATA_FLOAT_32BIT
};

enum {
    af_DATABIT_NOTSUPPORT = 0x404,
};


typedef struct _af_DataType_ {
    unsigned char IndataBit;
    unsigned char OutdataBit;
    char IndataInc;
    char OutdataInc;
    char Qval;
} af_DataType;

#endif // !_AUDIOEFFECT_DATATYPE_

