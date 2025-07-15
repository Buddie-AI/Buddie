#ifndef  __SCSI_H__
#define  __SCSI_H__

#define USB_MSD_MAX_LUN	0xfe
#define USB_MSD_RESET   0xff


/*
 *      SCSI opcodes
 */
#define TEST_UNIT_READY       0x00
#define REZERO_UNIT           0x01
#define REQUEST_SENSE         0x03
#define FORMAT_UNIT           0x04
#define READ_BLOCK_LIMITS     0x05
#define REASSIGN_BLOCKS       0x07
#define INITIALIZE_ELEMENT_STATUS 0x07
#define READ_6                0x08
#define WRITE_6               0x0a
#define SEEK_6                0x0b
#define READ_REVERSE          0x0f
#define WRITE_FILEMARKS       0x10
#define SPACE                 0x11
#define INQUIRY               0x12
#define RECOVER_BUFFERED_DATA 0x14
#define MODE_SELECT           0x15
#define RESERVE               0x16
#define RELEASE               0x17
#define COPY                  0x18
#define ERASE                 0x19
#define MODE_SENSE            0x1a
#define START_STOP            0x1b
#define RECEIVE_DIAGNOSTIC    0x1c
#define SEND_DIAGNOSTIC       0x1d
#define ALLOW_MEDIUM_REMOVAL  0x1e

#define READ_FORMAT_CAPACITIES 0x23
#define SET_WINDOW            0x24
#define READ_CAPACITY         0x25
#define READ_10               0x28
#define WRITE_10              0x2a
#define SEEK_10               0x2b
#define POSITION_TO_ELEMENT   0x2b
#define WRITE_VERIFY          0x2e
#define VERIFY                0x2f
#define SEARCH_HIGH           0x30
#define SEARCH_EQUAL          0x31
#define SEARCH_LOW            0x32
#define SET_LIMITS            0x33
#define PRE_FETCH             0x34
#define READ_POSITION         0x34
#define SYNCHRONIZE_CACHE     0x35
#define LOCK_UNLOCK_CACHE     0x36
#define READ_DEFECT_DATA      0x37
#define MEDIUM_SCAN           0x38
#define COMPARE               0x39
#define COPY_VERIFY           0x3a
#define WRITE_BUFFER          0x3b
#define READ_BUFFER           0x3c
#define UPDATE_BLOCK          0x3d
#define READ_LONG             0x3e
#define WRITE_LONG            0x3f
#define CHANGE_DEFINITION     0x40
#define WRITE_SAME            0x41
#define UNMAP		      0x42
#define READ_TOC              0x43
#define READ_HEADER           0x44
#define GET_EVENT_STATUS_NOTIFICATION 0x4a
#define LOG_SELECT            0x4c
#define LOG_SENSE             0x4d
#define XDWRITEREAD_10        0x53
#define MODE_SELECT_10        0x55
#define RESERVE_10            0x56
#define RELEASE_10            0x57
#define MODE_SENSE_10         0x5a
#define PERSISTENT_RESERVE_IN 0x5e
#define PERSISTENT_RESERVE_OUT 0x5f
#define VARIABLE_LENGTH_CMD   0x7f
#define REPORT_LUNS           0xa0
#define SECURITY_PROTOCOL_IN  0xa2
#define MAINTENANCE_IN        0xa3
#define MAINTENANCE_OUT       0xa4
#define MOVE_MEDIUM           0xa5
#define EXCHANGE_MEDIUM       0xa6
#define READ_12               0xa8
#define WRITE_12              0xaa
#define READ_MEDIA_SERIAL_NUMBER 0xab
#define WRITE_VERIFY_12       0xae
#define VERIFY_12	      0xaf
#define SEARCH_HIGH_12        0xb0
#define SEARCH_EQUAL_12       0xb1
#define SEARCH_LOW_12         0xb2
#define SECURITY_PROTOCOL_OUT 0xb5
#define READ_ELEMENT_STATUS   0xb8
#define SEND_VOLUME_TAG       0xb6
#define WRITE_LONG_2          0xea
#define EXTENDED_COPY         0x83
#define RECEIVE_COPY_RESULTS  0x84
#define ACCESS_CONTROL_IN     0x86
#define ACCESS_CONTROL_OUT    0x87
#define READ_16               0x88
#define WRITE_16              0x8a
#define READ_ATTRIBUTE        0x8c
#define WRITE_ATTRIBUTE	      0x8d
#define VERIFY_16	      0x8f
#define SYNCHRONIZE_CACHE_16  0x91
#define WRITE_SAME_16	      0x93
#define SERVICE_ACTION_IN     0x9e

/*
 *  SENSE KEYS
 */

#define NO_SENSE            0x00
#define RECOVERED_ERROR     0x01
#define NOT_READY           0x02
#define MEDIUM_ERROR        0x03
#define HARDWARE_ERROR      0x04
#define ILLEGAL_REQUEST     0x05
#define UNIT_ATTENTION      0x06
#define DATA_PROTECT        0x07
#define BLANK_CHECK         0x08
#define COPY_ABORTED        0x0a
#define ABORTED_COMMAND     0x0b
#define VOLUME_OVERFLOW     0x0d
#define MISCOMPARE          0x0e
/* Additional Sense code definition*/
#define ASC_NO_ADDITIONAL_SENSE_INFORMATION		0x00
#define ASC_RECOVERED_DATA_WITH_RETRIES			0x17
#define ASC_RECOVERED_DATA_WITH_ECC				0x18
#define ASC_MEDIUM_PRESENT						0x3A
#define ASC_LOGICAL_DRIVE_NOT_READY_BEING_READY	0x04
#define ASC_LOGICAL_DRIVE_NOT_READY_FMT_IN_PRGS	0x04
#define ASC_NO_REFERENCE_POSITION_FOUND			0x06
#define ASC_NO_SEEK_COMPLETE					0x02
#define ASC_WRITE_FAULT							0x03
#define ASC_ID_CRC_ERROR						0x10
#define ASC_UNRECOVERED_READ_ERROR				0x11
#define ASC_ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD	0x12
#define ASC_RECORDED_ENTITY_NOT_FOUND			0x14
#define ASC_INCOMPATIBLE_MEDIUM_INSTALLED		0x30
#define ASC_CANNOT_READ_MEDIUM_INCOMPATIBLE_FMT	0x30
#define ASC_CANNOT_READ_MEDIUM_UNKNOWN_FORMAT	0x30
#define ASC_FORMAT_COMMAND_FAILED				0x31
#define ASC_INVALID_COMMAND_OPERATION_CODE		0x20
#define ASC_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE	0x21
#define ASC_INVALID_FIELD_IN_COMMAND_PACKET		0x24
#define ASC_LOGICAL_UNIT_NOT_SUPPORTED			0x25
#define ASC_INVALID_FIELD_IN_PARAMETER_LIST		0x26
#define ASC_MEDIUM_REMOVAL_PREVENTED			0x53
#define ASC_NOT_READY_TO_READY_TRANSIT_MDI_CHNG	0x28
#define ASC_POWER_ON_OR_BUS_DEVICE_RESET		0x29
#define ASC_WRITE_PROTECTED_MEDIA				0x27
#define ASC_OVERLAPPED_COMMAND_ATTEMPTED		0x4E

/* Definition of additional sense code qualifier*/
/* Additional Sense code definition */
#define ASCQ_NO_ADDITIONAL_SENSE_INFORMATION		0x00
#define ASCQ_RECOVERED_DATA_WITH_RETRIES			0x01
#define ASCQ_RECOVERED_DATA_WITH_ECC				0x00
#define ASCQ_MEDIUM_PRESENT							0x00
#define ASCQ_LOGICAL_DRIVE_NOT_READY_BEING_READY	0x01
#define ASCQ_LOGICAL_DRIVE_NOT_READY_FMT_IN_PRGS	0x04
#define ASCQ_NO_REFERENCE_POSITION_FOUND			0x00
#define ASCQ_NO_SEEK_COMPLETE						0x00
#define ASCQ_WRITE_FAULT							0x00
#define ASCQ_ID_CRC_ERROR							0x00
#define ASCQ_UNRECOVERED_READ_ERROR					0x00
#define ASCQ_ADDRESS_MARK_NOT_FOUND_FOR_ID_FIELD	0x00
#define ASCQ_RECORDED_ENTITY_NOT_FOUND				0x00
#define ASCQ_INCOMPATIBLE_MEDIUM_INSTALLED			0x00
#define ASCQ_CANNOT_READ_MEDIUM_INCOMPATIBLE_FMT	0x02
#define ASCQ_CANNOT_READ_MEDIUM_UNKNOWN_FORMAT		0x01
#define ASCQ_FORMAT_COMMAND_FAILED					0x01
#define ASCQ_INVALID_COMMAND_OPERATION_CODE			0x00
#define ASCQ_LOGICAL_BLOCK_ADDRESS_OUT_OF_RANGE		0x00
#define ASCQ_INVALID_FIELD_IN_COMMAND_PACKET		0x00
#define ASCQ_LOGICAL_UNIT_NOT_SUPPORTED				0x00
#define ASCQ_INVALID_FIELD_IN_PARAMETER_LIST		0x00
#define ASCQ_MEDIUM_REMOVAL_PREVENTED				0x02
#define ASCQ_NOT_READY_TO_READY_TRANSIT_MDI_CHNG	0x00
#define ASCQ_POWER_ON_OR_BUS_DEVICE_RESET			0x00
#define ASCQ_WRITE_PROTECTED_MEDIA					0x00
#define ASCQ_OVERLAPPED_COMMAND_ATTEMPTED			0x00
#define     USB_LITTLE_ENDIAN
#ifdef USB_LITTLE_ENDIAN
#define CBW_SIGNATURE   0x43425355L
#define CSW_SIGNATURE   0x53425355L
#define CBW_TAG         0x34675326L
#elif	defined(USB_BIG_ENDIAN)
#define CBW_SIGNATURE   0x55534243L
#define CSW_SIGNATURE   0x55534253L
#define CBW_TAG         0x26536734L
#else
#error not define endian
#endif

struct usb_scsi_cbw {
    u32 dCBWSignature;          //[3:0]
    u32 dCBWTag;                //[7:4]
    u32 dCBWDataTransferLength; //[11:8]
    u8  bmCBWFlags;             //[12]
    u8  bCBWLUN;                //[13]  lun=[3:0] res=[7:4]
    u8  bCBWLength;             //[14]  len=[4:0] res=[7:5]
    u8  operationCode;
    u8  lun;            //<Logical Unit Number
    u8  lba[4];           //<Logical Block Address[7:31]
    u8  Reserved;
    u8  LengthH;         //<Transfer or Parameter List or Allocation Length
    u8  LengthL;
    u8  XLength;
    u8  Null[6];
} __attribute__((packed)) ;

struct usb_scsi_csw {
    u32 dCSWSignature;          //[3:0]
    u32 dCSWTag;                //[7:4]
    u32 uCSWDataResidue;        //[11:8]
    volatile u8  bCSWStatus;             //[12]
} __attribute__((packed)) ;

struct inquiry_data {
    u8  PeripheralDeviceType;
    u8  RMB;
    u8  ISO;
    u8  ResponseDataFormat;
    u8  AdditionalLength;
    u8  Reserved[3];
    u8  VendorInfo[8];
    u8  ProductInfo[16];
    u8  ProductRevisionLevel[4];
} __attribute__((packed)) ;

struct read_capacity_data {
    u32  block_num;
    u32  block_size;
} __attribute__((packed)) ;

struct request_sense_data {
    u8  ErrorCode;
    u8  Reserved;
    volatile u8  SenseKey;
    u8  Info[4];
    u8  ASL;
    u8  Reserved1[4];
    u8  ASC;
    u8  ASCQ;
    u8  Reserved2[4];
} __attribute__((packed)) ;

#endif  /*SCSI_H*/
