/* Copyright(C) JieLi Technology
 * usb audio 1.0
 * All right reserved
 *
 */

#include "system/includes.h"
#include "usb_config.h"
#include "usb/device/usb_stack.h"
#include "usb/device/uac_audio.h"
#include "app_config.h"

#if TCFG_USB_SLAVE_AUDIO_ENABLE

#define LOG_TAG_CONST       USB
#define LOG_TAG             "[USB]"
#define LOG_ERROR_ENABLE
#define LOG_DEBUG_ENABLE
#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
#define LOG_CLI_ENABLE
#include "debug.h"
static u32 mic_samplingfrequency;
/*********************************************************/
/*
			   Audio Class
*/
/*********************************************************/
///Standard Interface Association Descriptor
static const u8 uac_audio_interface_association[] = {
    USB_DT_INTERFACE_ASSOCIATION_SIZE, //Size of this descriptor in bytes: 8
    USB_DT_INTERFACE_ASSOCIATION,//INTERFACE ASSOCIATION Descriptor
    0,//Interface number of the first interface that is associated with this function
    0,//Number of contiguous interfaces that are associated with this function.
    USB_CLASS_AUDIO,//AUDIO_FUNCTION Function Class code
    USB_SUBCLASS_AUDIOSTREAMING,//FUNCTION_SUBCLASS_UNDEFINED
    PC_PROTOCOL_UNDEFINED,//AF_VERSION_02_00 Function Protocol code.
    SPEAKER_STR_INDEX,//Index of a string descriptor that describes this interface.
};
static const u8 uac_ac_standard_interface_desc[] = {
///standard interface AC descriptor(Interface 0, Alternate Setting 0)
    USB_DT_INTERFACE_SIZE,    //Length
    USB_DT_INTERFACE,     //DescriptorType:Inerface
    0x00,     //InterfaceNum
    0x00,       //AlternateSetting
    0x00,       //NumEndpoint
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOCONTROL,       //InterfaceSubClass:audio ctl
    PC_PROTOCOL_UNDEFINED,       //InterfaceProtocol
    SPEAKER_STR_INDEX,      //Interface String
};

static const u8 uac_spk_ac_interface[] = {
///class-specific AC interface descriptor
    0x09,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_HEADER,     //DescriptorSubType:audio control header
    0x00, 0x01, //bcdADC:audio Device Class v1.00
    0x28, 0x00, //TotalLength of class-specific AC interface descriptors
    0x01,      //InCollection:1 AudioStreaming interface
    0x02,      //InterfaceNr(1) - AS #2 id AudioStreaming interface 1 belongs to this AudioControl interface
    /* 0x03,      //InterfaceNr(1) - AS #2 id AudioStreaming interface 1 belongs to this AudioControl interface */
};
static const u8 uac_audio_ac_interface[] = {
///class-specific AC interface descriptor
    0x0a,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_HEADER,     //DescriptorSubType:audio control header
    0x00, 0x01, //bcdADC:audio Device Class v1.00
    0x47, 0x00, //TotalLength of class-specific AC interface descriptors
    0x02,      //InCollection:1 AudioStreaming interface
    0x01,      //InterfaceNr(1) - AS #2 id AudioStreaming interface 1 belongs to this AudioControl interface
    0x02,      //InterfaceNr(1) - AS #2 id AudioStreaming interface 1 belongs to this AudioControl interface
};


static const u8 uac_spk_input_terminal_desc[] = {

///USB USB Streaming IT(Speaker)
    0x0c,       //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_INPUT_TERMINAL,       //DescriptorSubType:Input Terminal
    SPK_INPUT_TERMINAL_ID,       //TerminalID
    LOBYTE(UAC_TERMINAL_STREAMING), HIBYTE(UAC_TERMINAL_STREAMING), //TerminalType:USB Streaming
    0x00,       //AssocTerminal
    SPK_CHANNEL,       //NrChannels:2 channel
#if SPK_CHANNEL == 1
    0x04, 0x00, //Mono center front channel
#else
    0x03, 0x00, //ChannelConfig:Left Front,Right Front,
#endif
    0x00,       //ChannelName String
    0x00,       //Terminal String
};
///Audio Feature Unit Descriptor(Speaker)
static const u8 uac_spk_feature_desc[] = {
    0x07 + (SPK_CHANNEL + 1) * 1, //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_FEATURE_UNIT,       //DescriptorSubType:Audio Feature Unit
    SPK_FEATURE_UNIT_ID,       //UnitID
    SPK_INPUT_TERMINAL_ID,      //SourceID:1 #USB Streaming IT
    0x01,      //ControlSize:1 byte
    0x01,      //Controls:BIT[0……7]:Mute,Volume Bass Mid Treble......
    0x02,       //left ch
#if SPK_CHANNEL == 2
    0x02,       //right ch
#endif
    0x00,       //Feature String
};
//SELECTOR_UNIT
static const u8 uac_spk_selector_uint_desc[] = {
    0x7,    //  bLength	7
    USB_DT_CS_INTERFACE,//  bDescriptorType	CS_INTERFACE (0x24)
    UAC_SELECTOR_UNIT,//    bDescriptorSubtype	SELECTOR_UNIT (0x05)
    SPK_SELECTOR_UNIT_ID,//bUnitID	33
    1,//bNrInPins	1
    SPK_FEATURE_UNIT_ID,   //baSourceID(1)	50
    0,//iSelector	None (0)
};
static const u8 uac_spk_output_terminal_desc[] = {
///USB Speaker OT
    0x09,      //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_OUTPUT_TERMINAL,      //DescriptorSubTYpe:Output Terminal
    SPK_OUTPUT_TERMINAL_ID,      //TerminalID
    LOBYTE(UAC_OUTPUT_TERMINAL_SPEAKER), HIBYTE(UAC_OUTPUT_TERMINAL_SPEAKER), //TerminalType:Speaker
    0,       //AssocTerminal:
    //SPK_FEATURE_UNIT_ID,      //SourceID: Feature UNIT
    SPK_SELECTOR_UNIT_ID,      //SourceID: Selector UNIT
    0x00,      //Terminal String
};

static const u8 uac_spk_as_interface_desc[] = {
//-------------------Speaker  interface---------------------//
///standard interface AS descriptor(Interface 1, Alternate Setting 0)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum:2
    0x00,       //AlternateSetting:0
    0x00,      //NumEndpoint:0
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,      //InterfaceSubClass:audio streaming
    PC_PROTOCOL_UNDEFINED,      //InterfaceProtocol
    0x00,       //Interface String

///standard interface AS descriptor(Interface 2, Alternate Setting 1)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x00,       //InterfaceNum
    0x01,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    PC_PROTOCOL_UNDEFINED,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL(0x01),
    0x07,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    SPK_INPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming IT
    0x01,      //Delay:1
    (SPK_AUDIO_TYPE & 0xFF), (SPK_AUDIO_TYPE >> 8),  //FormatTag:PCM

///Type 1 Format type descriptor
    0x0b,     //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I,       //FormatType:Format type 1
    SPK_CHANNEL,       //NumberOfChannel
    0x02,       //SubframeSize:2byte
    SPK_AUDIO_RES,      //BitsResolution:16bit
    0x01,      //SampleFreqType:One sampling frequency.
    DW1BYTE(SPK_AUDIO_RATE), DW2BYTE(SPK_AUDIO_RATE), DW3BYTE(SPK_AUDIO_RATE),

//Isochronous,Synchronization Type(Asynchronous)
    0x09,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_OUT | SPK_ISO_EP_OUT,     //EndpointAddress:Output endpoint 1
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ADAPTIVE, //0x09,
    LOBYTE(SPK_FRAME_LEN), HIBYTE(SPK_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame 0x00,  //Interval    //Asynchronous
    0x00,//unused
    0x00,//unused

//Audio Endpoint descriptor,General
    0x07,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
//  0x01,        //   bmAttributes (Sampling Freq Control)
    0x00,      //Attributes
    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay
};

const u8 speakerStringDescriptor[20] = {
    20,         //该描述符的长度为20字节
    0x03,       //字符串描述符的类型编码为0x03
    'U', 0x00, //U
    'S', 0x00, //S
    'B', 0x00, //B
    ' ', 0x00, //
    'A', 0x00, //A
    'u', 0x00, //u
    'd', 0x00, //d
    'i', 0x00, //i
    'o', 0x00, //o
};
/*********************************************************/
/*
			   Microphone Class
*/
/*********************************************************/

///Standard Interface Association Descriptor
static const u8 uac_mic_interface_association[] = {
    USB_DT_INTERFACE_ASSOCIATION_SIZE,//Size of this descriptor in bytes
    USB_DT_INTERFACE_ASSOCIATION,//INTERFACE ASSOCIATION Descriptor
    2,//Interface number of the first interface that is associated with this function ****
    1,//Number of contiguous interfaces that are associated with this function.
    USB_CLASS_AUDIO,//AUDIO_FUNCTION Function Class code
    USB_SUBCLASS_AUDIOSTREAMING,//FUNCTION_SUBCLASS_UNDEFINED
    PC_PROTOCOL_UNDEFINED,//AF_VERSION_02_00 Function Protocol code.
    MIC_STR_INDEX,//Index of a string descriptor that describes this interface
};
static const u8 uac_mic_ac_interface[] = {
///class-specific AC interface descriptor
    0x09,    //Length
    USB_DT_CS_INTERFACE,      //DescriptorType
    UAC_HEADER,     //DescriptorSubType
    0x00, 0x01, //bcdADC
    0x27, 0x00, /*TotalLength of class-specific AC interface descriptors */
    /* Includes the combined length               */
    /* of this descriptor header and all Unit and */
    /* Terminal descriptors.                      */
    0x01,      //InCollection:1 AudioStreaming interface
    0x01,      //InterfaceNr(1) ****
};
///USB USB Streaming IT(Microphone)
static const u8  uac_mic_input_terminal_desc[] = {
    0x0c,       //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_INPUT_TERMINAL,       //DescriptorSubType:Input Terminal
    MIC_INPUT_TERMINAL_ID,       //TerminalID
    LOBYTE(UAC_INPUT_TERMINAL_MICROPHONE), HIBYTE(UAC_INPUT_TERMINAL_MICROPHONE), //TerminalType:Microphone
    0,       //AssocTerminal
    MIC_CHANNEL,       //NrChannels
#if MIC_CHANNEL == 2
    0x03, 0x00, //wChannelConfig:Left Front right Front
#else
    0x00, 0x00, //wChannelConfig:
#endif
    0x00,       //ChannelName String
    0x00,       //Terminal String
};
//SELECTOR_UNIT
static const u8 uac_mic_selector_uint_desc[] = {
    0x7,    //  bLength	7
    USB_DT_CS_INTERFACE,//  bDescriptorType	CS_INTERFACE (0x24)
    UAC_SELECTOR_UNIT,//    bDescriptorSubtype	SELECTOR_UNIT (0x05)
    MIC_SELECTOR_UNIT_ID,//bUnitID	33
    1,//bNrInPins	1
    MIC_FEATURE_UNIT_ID,   //baSourceID(1)	50
    0,//iSelector	None (0)
};

///Audio Feature Unit Descriptor(Microphone)
static const u8 uac_mic_feature_desc[] = {
    7 + (MIC_CHANNEL + 1) * 1,    	//Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_FEATURE_UNIT,       //DescriptorSubType:Audio Feature Unit
    MIC_FEATURE_UNIT_ID,       //UnitID
    MIC_INPUT_TERMINAL_ID,      //SourceID: #Microphone
    0x01,      //ControlSize:1 byte
    0x43, 0x00,  //   bmaControls[0] (Mute,Volume,Automatic)
#if MIC_CHANNEL == 2
    0x00,
#endif
    0x00,       //Feature String
};
///USB Microphone OT
static const u8  uac_mic_output_terminal_desc[] = {
    0x09,      //Length
    USB_DT_CS_INTERFACE,      //DescriptorType:audio interface descriptor
    UAC_OUTPUT_TERMINAL,      //DescriptorSubTYpe:Output Terminal
    MIC_OUTPUT_TERMINAL_ID,      //TerminalID
    LOBYTE(UAC_TERMINAL_STREAMING), HIBYTE(UAC_TERMINAL_STREAMING), //TerminalType:USB Sreaming
    2,       //AssocTerminal:
    MIC_SELECTOR_UNIT_ID,      //SourceID:A #Feature UNIT
    0x00,      //Terminal String
};

///standard interface AS descriptor(Interface 1, Alternate Setting 0)
static const u8 uac_mic_as_interface_desc[] = {
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x03,       //InterfaceNum ****
    0x00,       //AlternateSetting
    0x00,      //NumEndpoint
    USB_CLASS_AUDIO,       //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,      //InterfaceSubClass:audio streaming
    0x00,      //InterfaceProtocol
    0x00,       //Interface String

///standard interface AS descriptor(Interface 2, Alternate Setting 1)
    USB_DT_INTERFACE_SIZE,      //Length
    USB_DT_INTERFACE,      //DescriptorType:Interface
    0x02,       //InterfaceNum ****
    0x01,       //AlternateSetting
    0x01,       //NumEndpoint
    USB_CLASS_AUDIO,      //InterfaceClass:audio
    USB_SUBCLASS_AUDIOSTREAMING,       //InterfaceSubClass:audio streaming
    0x00,       //InterfaceProtocol
    0x00,      //Interface String

///Audio Streaming Interface Descriptor:AS_GENERAL
    0x07,      //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:AS_GENERAL
    MIC_OUTPUT_TERMINAL_ID,      //TerminalLink:#1 USB Streaming OT
    0x01,      //Delay:1
    (MIC_AUDIO_TYPE & 0xFF), (MIC_AUDIO_TYPE >> 8),  //FormatTag:PCM

///Type 1 Format type descriptor
    0x08 + 3 * (MIC_SamplingFrequency), //Length
    USB_DT_CS_INTERFACE,       //DescriptorType:audio interface descriptor
    UAC_FORMAT_TYPE,      //DescriptorSubType:Format_type
    UAC_FORMAT_TYPE_I_PCM,       //FormatType:Format type 1
    MIC_CHANNEL,       //NumberOfChannel:1
    MIC_AUDIO_RES / 8, //SubframeSize:2byte
    MIC_AUDIO_RES,     //BitsResolution:16bit
#if MIC_SamplingFrequency > 1
    MIC_SamplingFrequency,
    DW1BYTE(MIC_AUDIO_RATE_1), DW2BYTE(MIC_AUDIO_RATE_1), DW3BYTE(MIC_AUDIO_RATE_1),
    DW1BYTE(MIC_AUDIO_RATE_2), DW2BYTE(MIC_AUDIO_RATE_2), DW3BYTE(MIC_AUDIO_RATE_2),
    DW1BYTE(MIC_AUDIO_RATE), DW2BYTE(MIC_AUDIO_RATE), DW3BYTE(MIC_AUDIO_RATE),
    DW1BYTE(MIC_AUDIO_RATE_4), DW2BYTE(MIC_AUDIO_RATE_4), DW3BYTE(MIC_AUDIO_RATE_4),
#else
    0x01,      //SampleFreqType:One sampling frequency.
    DW1BYTE(MIC_AUDIO_RATE), DW2BYTE(MIC_AUDIO_RATE), DW3BYTE(MIC_AUDIO_RATE),
#endif

///Endpoint 1 - Standard Descriptor:Output Endpoint1
//Isochronous,Synchronization Type(Asynchronous)
    USB_DT_ENDPOINT_AUDIO_SIZE,      //Length
    USB_DT_ENDPOINT,       //DescriptorType:endpoint descriptor
    USB_DIR_IN | MIC_ISO_EP_IN,       //EndpointAddress:Output endpoint
    USB_ENDPOINT_XFER_ISOC | USB_ENDPOINT_SYNC_ASYNC,
    LOBYTE(MIC_FRAME_LEN), HIBYTE(MIC_FRAME_LEN), //MaxPacketSize
    UAC_ISO_INTERVAL,//one packet per frame   //Interval
    0x00,//unused
    0x00,//unused

///Endpoint - Audio Streaming Descriptor
//Audio Endpoint descriptor,General
    0x07,       //Length
    USB_DT_CS_ENDPOINT,       //DescriptorType:audio endpoint descriptor
    UAC_AS_GENERAL,       //DescriptorSubType:audio endpiont general
#if MIC_SamplingFrequency > 1
    0x01,        //   bmAttributes (Sampling Freq Control)
#else
    0x00,      //Attributes
#endif

    0x00,       //LockDelayUnits
    0x00, 0x00, //LockDelay
};
static const u8 micStringDescriptor[30] = {
    30,         //该描述符的长度为30字节
    0x03,       //字符串描述符的类型编码为0x03
    'U', 0x00, //U
    'S', 0x00, //S
    'B', 0x00, //B
    ' ', 0x00, //
    'M', 0x00, //M
    'i', 0x00, //i
    'c', 0x00, //c
    'r', 0x00, //r
    'o', 0x00, //o
    'p', 0x00, //p
    'h', 0x00, //h
    'o', 0x00, //o
    'n', 0x00, //n
    'e', 0x00, //e
};

const u8 *uac_get_string(u32 id)
{
    if (id == SPEAKER_STR_INDEX) {
        return speakerStringDescriptor;
    } else if (id == MIC_STR_INDEX) {
        return micStringDescriptor;
    }
    return NULL;
}
struct uac_info_t {
    u16 spk_left_vol;
    u16 spk_right_vol;
    u16 spk_max_vol;
    u16 spk_min_vol;
    u16 spk_def_vol;
    u16 spk_vol_res;
    u8 *spk_dma_buffer;

    u16 mic_max_vol;
    u16 mic_min_vol;
    u16 mic_def_vol;
    u16 mic_vol_res;
    u16 mic_vol;
    u8 *mic_dma_buffer;

    u8 spk_mute: 1;
    u8 mic_mute: 1;
    u8 bAGC: 1;
    u8 ps4_mode: 1;
    u8 res: 4;
};


struct uac_info_t *uac_info;
#if USB_MALLOC_ENABLE
#else
struct uac_info_t _uac_info SEC(.uac_var);
#endif

static u32 vol_convert(u16 v)
{
    const u16 vol_table[] = {
        0xe3a0, 0xe461, 0xe519, 0xe5c8, 0xe670, 0xe711, 0xe7ac, 0xe840, 0xe8cf, 0xe959,
        0xe9df, 0xea60, 0xeadc, 0xeb55, 0xebca, 0xec3c, 0xecab, 0xed16, 0xed7f, 0xede5,
        0xee48, 0xeea9, 0xef08, 0xef64, 0xefbe, 0xf016, 0xf06c, 0xf0c0, 0xf113, 0xf164,
        0xf1b3, 0xf200, 0xf24c, 0xf297, 0xf2e0, 0xf328, 0xf36e, 0xf3b4, 0xf3f8, 0xf43a,
        0xf47c, 0xf4bd, 0xf4fc, 0xf53b, 0xf579, 0xf5b5, 0xf5f1, 0xf62c, 0xf666, 0xf69f,
        0xf6d7, 0xf70e, 0xf745, 0xf77b, 0xf7b0, 0xf7e5, 0xf818, 0xf84b, 0xf87e, 0xf8b0,
        0xf8e1, 0xf911, 0xf941, 0xf970, 0xf99f, 0xf9cd, 0xf9fb, 0xfa28, 0xfa55, 0xfa81,
        0xfaad, 0xfad8, 0xfb03, 0xfb2d, 0xfb56, 0xfb80, 0xfba9, 0xfbd1, 0xfbf9, 0xfc21,
        0xfc48, 0xfc6f, 0xfc95, 0xfcbb, 0xfce1, 0xfd06, 0xfd2b, 0xfd50, 0xfd74, 0xfd98,
        0xfdbc, 0xfddf, 0xfe02, 0xfe25, 0xfe47, 0xfe69, 0xfe8b, 0xfead, 0xfece, 0xfeef,
        0xff0f,
    };

    if (v <= 100) {
        return vol_table[v];
    }

    for (int i = 0; i < sizeof(vol_table) / 2; i++) {
        if (v <= vol_table[i]) {
            return i;
        }
    }

    return 0;
}

void uac_get_cur_vol(const usb_dev usb_id, u16 *l_vol, u16 *r_vol)
{
    if (l_vol) {
        *l_vol = 0;
    }
    if (r_vol) {
        *r_vol = 0;
    }
    if (uac_info) {
        if (!uac_info->spk_mute) {
            if (l_vol) {
                *l_vol = vol_convert(uac_info->spk_left_vol);
            }
            if (r_vol) {
                *r_vol = vol_convert(uac_info->spk_right_vol);
            }
        }
    }
}
u16 uac_get_mic_vol(const usb_dev usb_id)
{
    if (uac_info) {
        if (!uac_info->mic_mute) {
            return vol_convert(uac_info->mic_vol);
        }
    }
    return 0;
}

u16 uac_get_mic_sameplerate(void *priv)
{
    u16 sample_rate = (u16)mic_samplingfrequency;
    return sample_rate;
}

static u32 uac_vol_handler(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ret = 0;
    u8 read_ep[8];
    u8 mute;
    u16 volume = 0;
    usb_read_ep0(usb_id, read_ep, setup->wLength);
    ret = USB_EP0_STAGE_SETUP;
    switch (HIBYTE(setup->wValue)) {
    case UAC_FU_MUTE:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            memcpy((u8 *)&mute, read_ep, 1);
            if (mute) {
                uac_mute_volume(SPK_FEATURE_UNIT_ID, 0, 0);
                uac_info->spk_mute = 1;
            } else {
                uac_mute_volume(SPK_FEATURE_UNIT_ID,
                                vol_convert(uac_info->spk_left_vol),
                                vol_convert(uac_info->spk_right_vol));
                uac_info->spk_mute = 0;
            }
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            memcpy((u8 *)&mute, read_ep, 1);
            if (mute) {
                uac_mute_volume(MIC_FEATURE_UNIT_ID, 0, 0);
                uac_info->mic_mute = 1;
            } else {
                uac_mute_volume(MIC_FEATURE_UNIT_ID, vol_convert(uac_info->mic_vol), 0);
                uac_info->mic_mute = 0;
            }
        }
        break;
    case UAC_FU_VOLUME:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            memcpy((u8 *)&volume, read_ep, setup->wLength);
            if (LOBYTE(setup->wValue) == 1) {
                uac_info->spk_left_vol = volume;
            } else if (LOBYTE(setup->wValue) == 2) {
                uac_info->spk_right_vol = volume;
            }
            if (uac_info->ps4_mode) {
                uac_info->spk_left_vol = vol_convert(100);
                uac_info->spk_right_vol = vol_convert(100);
            }
            if (!uac_info->spk_mute && !uac_info->ps4_mode) {
                uac_mute_volume(SPK_FEATURE_UNIT_ID,
                                vol_convert(uac_info->spk_left_vol),
                                vol_convert(uac_info->spk_right_vol));
            }
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            memcpy((u8 *)&uac_info->mic_vol, read_ep, sizeof(uac_info->mic_vol));
            /* if (uac_info->mic_vol == 0x8000) { */
            /* uac_info->mic_vol = 0; */
            /* } else if (uac_info->mic_vol >= 100) { */
            /* uac_info->mic_vol = 99; */
            /* } */
            if (!uac_info->mic_mute) {
                uac_mute_volume(MIC_FEATURE_UNIT_ID, vol_convert(uac_info->mic_vol), 0);
            }
        }
        break;
    case UAC_FU_AUTOMATIC_GAIN:
        if (read_ep[0]) {
            uac_info->bAGC = 1;
        } else {
            uac_info->bAGC = 0;
        }
        break;

    case 0: //bSelectorUnitID
        if (HIBYTE(setup->wIndex) == SPK_SELECTOR_UNIT_ID) {
            uac_info->ps4_mode = read_ep[0];
            if (read_ep[0]) {
                printf("SPK_FEATURE_UNIT_ID");
                uac_info->spk_left_vol = vol_convert(100);
                uac_info->spk_right_vol = vol_convert(100);
                uac_mute_volume(SPK_FEATURE_UNIT_ID, 100, 100);
            }
        }
        break;

    default :
        ret = USB_EP0_SET_STALL;
        break;
    }

    return ret;
}
#if USB_ROOT2
static u8 SetInterface_0_Lock;
#endif
static u32 uac_endpoint_recv(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 ret = 0;
    u8 read_ep[8];

    usb_read_ep0(usb_id, read_ep, setup->wLength);

    ret = USB_EP0_STAGE_SETUP;
    u32 ep = LOBYTE(setup->wIndex);
    switch (ep) {
    case  MIC_ISO_EP_IN|USB_DIR_IN:
        memcpy(&mic_samplingfrequency, read_ep, 3);
        break;
    case SPK_ISO_EP_OUT|USB_DIR_OUT:
        /* memcpy(&spk_samplingfrequency,read_ep,3); */
        break;
    }
    return ret;
}
u32 uac_setup_endpoint(struct usb_device_t *usb_device, struct usb_ctrlrequest *req)
{
    u32 ret = 0;
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 type = req->bRequestType & USB_TYPE_MASK;
    if (type != USB_TYPE_CLASS) {
        return ret;
    }
    if (req->bRequest != 0x01) {
        return ret;
    }
    if (req->wValue != 0x0100) {
        return ret;
    }
    usb_set_setup_recv(usb_device, uac_endpoint_recv);
    return 1;
}
static u32 audio_ac_itf_handler(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    u8 mute;

    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;
    switch (setup->bRequest) {
    case USB_REQ_SET_INTERFACE:
#if USB_ROOT2
        SetInterface_0_Lock = 1;
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);//no alt setting
        }
#else
        if (setup->wValue == 0) { //alt 0
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
#endif
        break;
    case USB_REQ_GET_INTERFACE:
#if USB_ROOT2
        if (usb_root2_testing()) {
            SetInterface_0_Lock = 0;
        }
#endif
        if (setup->wValue || (setup->wLength != 1)) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
            tx_len = 1;
            tx_payload[0] = 0x00;
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
        }
        break;
    case UAC_SET_CUR:
        /* USB_DEBUG_PRINTF("set vol && mute"); */
        usb_set_setup_recv(usb_device, uac_vol_handler);
        break;
    case UAC_GET_CUR:
        switch (HIBYTE(setup->wValue)) {
        case UAC_FU_MUTE:
            if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
                mute = uac_info->spk_mute ;
            } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
                mute = uac_info->mic_mute ;
            }
            tx_payload = &mute;
            tx_len = setup->wLength;
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            break;

        case UAC_FU_VOLUME:
            if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
                if (LOBYTE(setup->wValue) == 1) {
                    tx_payload = (u8 *)&uac_info->spk_left_vol;
                } else if (LOBYTE(setup->wValue) == 2) {
                    tx_payload = (u8 *)&uac_info->spk_right_vol;
                }
            } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
                tx_payload = (u8 *)&uac_info->mic_vol;
            }
            tx_len = setup->wLength;
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            break;
        case UAC_FU_AUTOMATIC_GAIN:
            if (uac_info->bAGC) {
                tx_payload[0] = 0x01;
            } else {
                tx_payload[0] = 0x00;
            }
            tx_len = setup->wLength;
            usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            break;
        }
        break;
    case UAC_GET_DEF:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->spk_def_vol;
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->mic_def_vol;
        }
        tx_len = setup->wLength;
        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
        break;
    case UAC_GET_MAX:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->spk_max_vol;
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->mic_max_vol;
        }
        tx_len = setup->wLength;
        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
        break;
    case UAC_GET_MIN:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->spk_min_vol;
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->mic_min_vol;
        }
        tx_len = setup->wLength;
        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
        break;
    case UAC_GET_RES:
        if (HIBYTE(setup->wIndex) == SPK_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->spk_vol_res;
        } else if (HIBYTE(setup->wIndex) == MIC_FEATURE_UNIT_ID) {
            tx_payload = (u8 *)&uac_info->mic_vol_res;
        }
        tx_len = setup->wLength;
        usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
        break;
    case UAC_GET_LEN:
        break;
    case UAC_GET_INFO:
        break;
    case USB_REQ_GET_STATUS:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
        break;
    default:
        ret = 1 ;
        break;
    }
    return ret;
}
#define     UAC_PLC_EN  0
#if UAC_PLC_EN
typedef struct _USBAUDIO_PLC_API {
    unsigned int (*need_buf)();
    void (*open)(unsigned char *ptr, int sr, int nch);
    int (*run)(unsigned char *ptr, short *inbuf, short *oubuf, short len, short err_flag);
} USBAUDIO_PLC_API;

extern USBAUDIO_PLC_API *get_USBplc_api();

static USBAUDIO_PLC_API *uac_plc_api = 0;
static u8 *uac_plc_buffer;

void usb_audio_plc_open()
{
    uac_plc_api = get_USBplc_api();
    u32 bufsize = uac_plc_api->need_buf();
    uac_plc_buffer = malloc(bufsize);
    uac_plc_api->open(uac_plc_buffer, SPK_AUDIO_RATE, SPK_CHANNEL);
}
void usb_audio_plc_close()
{
    if (uac_plc_api) {
        uac_plc_api = NULL;
    }
    if (uac_plc_buffer) {
        free(uac_plc_buffer);
        uac_plc_buffer = NULL;
    }
}
#endif

static void spk_transfer(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer = uac_info->spk_dma_buffer;
    u32 spk_frame_len = SPK_AUDIO_RATE * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    spk_frame_len += (SPK_AUDIO_RATE % 1000 ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
    u32 rx_len = usb_g_iso_read(usb_id, SPK_ISO_EP_OUT, NULL, spk_frame_len, 0);

#if UAC_PLC_EN
    if (rx_len) {
        uac_plc_api->run(uac_plc_buffer, ep_buffer, ep_buffer, rx_len, 0);
    } else {
        uac_plc_api->run(uac_plc_buffer, ep_buffer, ep_buffer, rx_len, 1);
    }
#endif

    uac_speaker_stream_write(ep_buffer, rx_len);
}
static void open_spk(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 spk_frame_len = SPK_AUDIO_RATE * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    spk_frame_len += (SPK_AUDIO_RATE % 1000 ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
    u8 *ep_buffer = uac_info->spk_dma_buffer;
    uac_speaker_stream_open(SPK_AUDIO_RATE, SPK_CHANNEL);

#if UAC_PLC_EN
    usb_audio_plc_open();
#endif

    usb_g_set_intr_hander(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, spk_transfer);

    usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, spk_frame_len);
}
static void close_spk(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    const usb_dev usb_id = usb_device2id(usb_device);
    usb_clr_intr_rxe(usb_id, SPK_ISO_EP_OUT);
    usb_g_set_intr_hander(usb_id, SPK_ISO_EP_OUT, NULL);

#if UAC_PLC_EN
    usb_audio_plc_close();
#endif

    uac_speaker_stream_close();

}
static u8 spk_itf_status;
static u32 spk_as_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u32 tx_len;
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;

    switch (setup->bRequest) {
    case USB_REQ_SET_INTERFACE:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
#if USB_ROOT2
            if (!usb_root2_testing()) {
                SetInterface_0_Lock = 1;
            }
            if (SetInterface_0_Lock == 0) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
                if (setup->wValue == 1) { //alt 1
                    open_spk(usb_device);
                } else if (setup->wValue == 0) { //alt 0
                    close_spk(usb_device);
                } else {
                    usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
                }
            }
#else
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            if (setup->wValue == 1) { //alt 1
                open_spk(usb_device);
                spk_itf_status = 1;
            } else if (setup->wValue == 0) { //alt 0
                close_spk(usb_device);
                spk_itf_status = 0;
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
#endif
        }

        break;
    case USB_REQ_GET_INTERFACE:
        if (usb_root2_testing()) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            if (setup->wValue || (setup->wLength != 1)) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                tx_payload[0] = spk_itf_status;
                usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            }
        }
        break;
    case USB_REQ_GET_STATUS:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
        break;
    default:
        break;
    }
    return ret;
}
static u8 mic_no_data;
static void mic_transfer(struct usb_device_t *usb_device, u32 ep)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *ep_buffer = uac_info->mic_dma_buffer;

    if (mic_samplingfrequency == 0) {
        mic_samplingfrequency = MIC_AUDIO_RATE;
    }

    u32 mic_frame_len = ((mic_samplingfrequency * MIC_AUDIO_RES / 8 * MIC_CHANNEL) / 1000);
    mic_frame_len += (mic_samplingfrequency % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);

    int len = uac_mic_stream_read(ep_buffer,  mic_frame_len);
    if (len) {
        mic_no_data = 0;
    } else if (mic_no_data) {
        len = mic_frame_len;
        memset(ep_buffer, 0, len);
    }

    usb_g_iso_write(usb_id, MIC_ISO_EP_IN, NULL, len);

}
static void open_mic(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    mic_no_data = 1;
    const usb_dev usb_id = usb_device2id(usb_device);

    usb_enable_ep(usb_id, MIC_ISO_EP_IN);

    u8 *ep_buffer = uac_info->mic_dma_buffer;

    usb_g_set_intr_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, mic_transfer);

    if (mic_samplingfrequency == 0) {
        mic_samplingfrequency = MIC_AUDIO_RATE;
    }
    u32 mic_frame_len = ((mic_samplingfrequency * MIC_AUDIO_RES / 8 * MIC_CHANNEL) / 1000);
    mic_frame_len += (mic_samplingfrequency % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);

    uac_mic_stream_open(mic_samplingfrequency, mic_frame_len, MIC_CHANNEL);

    usb_g_ep_config(usb_id,
                    MIC_ISO_EP_IN | USB_DIR_IN,
                    USB_ENDPOINT_XFER_ISOC,
                    1, ep_buffer, mic_frame_len);

    mic_transfer(usb_device, MIC_ISO_EP_IN);
}
static void close_mic(struct usb_device_t *usb_device)
{
    log_info("%s", __func__);
    const usb_dev usb_id = usb_device2id(usb_device);
    usb_clr_intr_txe(usb_id, MIC_ISO_EP_IN);
    usb_g_set_intr_hander(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, NULL);
    uac_mic_stream_close();
}
static u8 mic_itf_status;
static u32 mic_as_itf_hander(struct usb_device_t *usb_device, struct usb_ctrlrequest *setup)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    u8 *tx_payload = usb_get_setup_buffer(usb_device);
    u32 tx_len;
    u32 bRequestType = setup->bRequestType & USB_TYPE_MASK;
    u32 ret = 0;

    switch (setup->bRequest) {
    case USB_REQ_SET_INTERFACE:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
#if USB_ROOT2
            if (!usb_root2_testing()) {
                SetInterface_0_Lock = 1;
            }
            if (SetInterface_0_Lock == 0) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
                if (setup->wValue == 1) {//alt 1
                    open_mic(usb_device);
                } else if (setup->wValue == 0) { //alt 0
                    close_mic(usb_device);
                } else {
                    usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
                }
            }
#else
            usb_set_setup_phase(usb_device, USB_EP0_STAGE_SETUP);
            if (setup->wValue == 1) {//alt 1
                open_mic(usb_device);
                mic_itf_status = 1;
            } else if (setup->wValue == 0) { //alt 0
                close_mic(usb_device);
                mic_itf_status = 0;
            } else {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            }
#endif
        }
        break;
    case USB_REQ_GET_INTERFACE:
        if (usb_root2_testing()) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            if (setup->wValue || (setup->wLength != 1)) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_DEFAULT) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_ADDRESS) {
                usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
            } else if (usb_device->bDeviceStates == USB_CONFIGURED) {
                tx_len = 1;
                tx_payload[0] = mic_itf_status;
                usb_set_data_payload(usb_device, setup, tx_payload, tx_len);
            }
        }
        break;
    case USB_REQ_GET_STATUS:
        if (usb_device->bDeviceStates == USB_DEFAULT) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else if (usb_device->bDeviceStates == USB_ADDRESS) {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        } else {
            usb_set_setup_phase(usb_device, USB_EP0_SET_STALL);
        }
        break;
    default:
        break;
    }
    return ret;
}
void spk_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);
    u8 *ep_buffer = uac_info->spk_dma_buffer;
    usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, SPK_FRAME_LEN);
}
u32 uac_spk_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;

    log_debug("spkdc:%d\n", *cur_itf_num);
    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_ac_interface, sizeof(uac_spk_ac_interface));
    tptr[5] += 7;  //spk selector unit
#if SPK_CHANNEL == 1
    tptr[5]--;
#endif
    tptr[8] = *cur_itf_num + 1;
    tptr += sizeof(uac_spk_ac_interface);//0x09

    memcpy(tptr, (u8 *)uac_spk_input_terminal_desc, sizeof(uac_spk_input_terminal_desc));
    tptr += sizeof(uac_spk_input_terminal_desc);//0x0c

    memcpy(tptr, (u8 *)uac_spk_feature_desc, sizeof(uac_spk_feature_desc));
    tptr += sizeof(uac_spk_feature_desc);//0x0a

    memcpy(tptr, (u8 *)uac_spk_selector_uint_desc, sizeof(uac_spk_selector_uint_desc));
    tptr += sizeof(uac_spk_selector_uint_desc);

    memcpy(tptr, (u8 *)uac_spk_output_terminal_desc, sizeof(uac_spk_output_terminal_desc));
    tptr += sizeof(uac_spk_output_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_as_interface_desc, sizeof(uac_spk_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
    frame_len = SPK_AUDIO_RATE * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    frame_len += (SPK_AUDIO_RATE % 1000 ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
    offset = 9 + 9 + 7 + 11 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
    tptr += sizeof(uac_spk_as_interface_desc);//0x09+0x09+0x07+0x0b+0x09+0x07

    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, spk_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, spk_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }

    (*cur_itf_num) ++;

    i = tptr - ptr;
    return i;
}
void mic_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_error("%s", __func__);
#if USB_ROOT2
    usb_disable_ep(usb_id, SPK_ISO_EP_OUT);
    /* uac_release(usb_id); */
#else
    u8 *ep_buffer = uac_info->mic_dma_buffer;
    usb_g_ep_config(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, MIC_FRAME_LEN);
#endif
}
u32 uac_mic_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;

    log_debug("micdc:%d\n", *cur_itf_num);

    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr[8] = MIC_STR_INDEX;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, uac_mic_ac_interface, sizeof(uac_mic_ac_interface));
#if MIC_CHANNEL == 2
    tptr[5]++;
#endif
    tptr[8] = *cur_itf_num + 1;
    tptr += sizeof(uac_mic_ac_interface);

    memcpy(tptr, (u8 *)uac_mic_input_terminal_desc, sizeof(uac_mic_input_terminal_desc));
    tptr += sizeof(uac_mic_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_output_terminal_desc, sizeof(uac_mic_output_terminal_desc));
    tptr += sizeof(uac_mic_output_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_selector_uint_desc, sizeof(uac_mic_selector_uint_desc));
    tptr += sizeof(uac_mic_selector_uint_desc);

    memcpy(tptr, (u8 *)uac_mic_feature_desc, sizeof(uac_mic_feature_desc));
    tptr += sizeof(uac_mic_feature_desc);//0x09



    memcpy(tptr, (u8 *)uac_mic_as_interface_desc, sizeof(uac_mic_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
#if MIC_SamplingFrequency > 1
    u32 sr = 0;
    //find the max sample rate
    if (sr < MIC_AUDIO_RATE_1) {
        sr = MIC_AUDIO_RATE_1;
    }
    if (sr < MIC_AUDIO_RATE_2) {
        sr = MIC_AUDIO_RATE_2;
    }
    if (sr < MIC_AUDIO_RATE) {
        sr = MIC_AUDIO_RATE;
    }
    if (sr < MIC_AUDIO_RATE_4) {
        sr = MIC_AUDIO_RATE_4;
    }
    frame_len = sr * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#else
    frame_len = MIC_AUDIO_RATE * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += (MIC_AUDIO_RATE % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#endif
    offset = 9 + 9 + 7 + (8 + 3 * MIC_SamplingFrequency) + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
    tptr += sizeof(uac_mic_as_interface_desc);//0x09

    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }
    if (usb_set_reset_hander(usb_id, *cur_itf_num, mic_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    if (usb_set_interface_hander(usb_id, *cur_itf_num, mic_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }

    (*cur_itf_num) ++;
    i = tptr - ptr;
    return i;
}
void audio_reset(struct usb_device_t *usb_device, u32 ift_num)
{
    const usb_dev usb_id = usb_device2id(usb_device);
    log_debug("%s", __func__);
#if USB_ROOT2
    usb_disable_ep(usb_id, SPK_ISO_EP_OUT);
    /* uac_release(usb_id); */
#else
    u8 *ep_buffer;
    ep_buffer = uac_info->spk_dma_buffer;
    usb_g_ep_config(usb_id, SPK_ISO_EP_OUT | USB_DIR_OUT, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, SPK_FRAME_LEN);

    ep_buffer = uac_info->mic_dma_buffer;
    usb_g_ep_config(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, USB_ENDPOINT_XFER_ISOC, 1, ep_buffer, MIC_FRAME_LEN);
#endif
}
u32 uac_audio_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num)
{
    int i = 0;
    u8 *tptr = ptr;
    u32 offset;
    u32 frame_len;

    memcpy(tptr, (u8 *)uac_audio_interface_association, sizeof(uac_audio_interface_association));
    tptr[2] = *cur_itf_num;
    tptr[3] = 3;
    tptr += sizeof(uac_audio_interface_association);//0x09

    memcpy(tptr, (u8 *)uac_ac_standard_interface_desc, sizeof(uac_ac_standard_interface_desc));
    tptr[2] = *cur_itf_num;
    tptr += sizeof(uac_ac_standard_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_audio_ac_interface, sizeof(uac_audio_ac_interface));
    tptr[5] += 7; //spk selector unit
#if SPK_CHANNEL == 1
    tptr[5]--;
#endif
#if MIC_CHANNEL == 2
    tptr[5]++;
#endif
    tptr[8] = *cur_itf_num + 1;
    tptr[9] = *cur_itf_num + 2;
    tptr += sizeof(uac_audio_ac_interface);//0x09

    memcpy(tptr, (u8 *)uac_spk_input_terminal_desc, sizeof(uac_spk_input_terminal_desc));
    tptr += sizeof(uac_spk_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_input_terminal_desc, sizeof(uac_mic_input_terminal_desc));
    tptr += sizeof(uac_mic_input_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_output_terminal_desc, sizeof(uac_spk_output_terminal_desc));
    tptr += sizeof(uac_spk_output_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_output_terminal_desc, sizeof(uac_mic_output_terminal_desc));
    tptr += sizeof(uac_mic_output_terminal_desc);//0x09

    memcpy(tptr, (u8 *)uac_spk_selector_uint_desc, sizeof(uac_spk_selector_uint_desc));
    tptr += sizeof(uac_spk_selector_uint_desc);

    memcpy(tptr, (u8 *)uac_mic_selector_uint_desc, sizeof(uac_mic_selector_uint_desc));
    tptr += sizeof(uac_mic_selector_uint_desc);

    memcpy(tptr, (u8 *)uac_spk_feature_desc, sizeof(uac_spk_feature_desc));
    tptr += sizeof(uac_spk_feature_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_feature_desc, sizeof(uac_mic_feature_desc));
    tptr += sizeof(uac_mic_feature_desc);//0x09


    memcpy(tptr, (u8 *)uac_spk_as_interface_desc, sizeof(uac_spk_as_interface_desc));
    tptr[2] = *cur_itf_num + 1;
    tptr[9 + 2] = *cur_itf_num + 1;
    frame_len = SPK_AUDIO_RATE * (SPK_AUDIO_RES / 8) * SPK_CHANNEL / 1000;
    frame_len += (SPK_AUDIO_RATE % 1000 ? (SPK_AUDIO_RES / 8) * SPK_CHANNEL : 0);
    offset = 9 + 9 + 7 + 11 + 4;
    //MaxPacketSize of spk iso out
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
    tptr += sizeof(uac_spk_as_interface_desc);//0x09

    memcpy(tptr, (u8 *)uac_mic_as_interface_desc, sizeof(uac_mic_as_interface_desc));
    tptr[2] = *cur_itf_num + 2;
    tptr[9 + 2] = *cur_itf_num + 2;
#if MIC_SamplingFrequency > 1
    u32 sr = 0;
    //find the max sample rate
    if (sr < MIC_AUDIO_RATE_1) {
        sr = MIC_AUDIO_RATE_1;
    }
    if (sr < MIC_AUDIO_RATE_2) {
        sr = MIC_AUDIO_RATE_2;
    }
    if (sr < MIC_AUDIO_RATE) {
        sr = MIC_AUDIO_RATE;
    }
    if (sr < MIC_AUDIO_RATE_4) {
        sr = MIC_AUDIO_RATE_4;
    }
    frame_len = sr * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += (sr % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#else
    frame_len = MIC_AUDIO_RATE * (MIC_AUDIO_RES / 8) * MIC_CHANNEL / 1000;
    frame_len += (MIC_AUDIO_RATE % 1000 ? (MIC_AUDIO_RES / 8) * MIC_CHANNEL : 0);
#endif
    offset = 9 + 9 + 7 + (8 + 3 * MIC_SamplingFrequency) + 4;
    //MaxPacketSize of mic iso in
    tptr[offset] = LOBYTE(frame_len);
    tptr[offset + 1] = HIBYTE(frame_len);
    tptr += sizeof(uac_mic_as_interface_desc);//0x09

    log_debug("audio control interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, audio_ac_itf_handler) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }

    if (usb_set_reset_hander(usb_id, *cur_itf_num, audio_reset) != *cur_itf_num) {

    }
    (*cur_itf_num) ++;
    log_debug("speaker stream interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, spk_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac spk set interface_hander fail");
    }

    (*cur_itf_num) ++;
    log_debug("mic stream interface num:%d\n", *cur_itf_num);
    if (usb_set_interface_hander(usb_id, *cur_itf_num, mic_as_itf_hander) != *cur_itf_num) {
        ASSERT(0, "uac mic set interface_hander fail");
    }

    (*cur_itf_num) ++;

    i = tptr - ptr;
    return i;
}
u32 uac_register(const usb_dev usb_id, const u32 class)
{
    if (uac_info == NULL) {
#if USB_MALLOC_ENABLE
        uac_info = (struct uac_info_t *)malloc(sizeof(struct uac_info_t));
        if (uac_info == NULL) {
            printf("uac_register err\n");
            return -1;
        }
#else
        memset(&_uac_info, 0, sizeof(struct uac_info_t));
        uac_info = &_uac_info;
#endif
        if (class == AUDIO_CLASS) {
            uac_info->spk_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, SPK_ISO_EP_OUT, SPK_FRAME_LEN + MIC_FRAME_LEN);
            uac_info->mic_dma_buffer = uac_info->spk_dma_buffer + SPK_FRAME_LEN;
        } else if (class == SPEAKER_CLASS) {
            uac_info->spk_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, SPK_ISO_EP_OUT, SPK_FRAME_LEN);
        } else if (class == MIC_CLASS) {
            uac_info->mic_dma_buffer = usb_alloc_ep_dmabuffer(usb_id, MIC_ISO_EP_IN | USB_DIR_IN, MIC_FRAME_LEN);
        }
    }
    uac_info->spk_def_vol = vol_convert(uac_get_spk_vol());
    uac_info->spk_left_vol = uac_info->spk_def_vol;
    uac_info->spk_right_vol = uac_info->spk_def_vol;
    uac_info->spk_max_vol = vol_convert(100);
    uac_info->spk_min_vol = vol_convert(0);
    uac_info->spk_vol_res = 0x30;
    uac_info->spk_mute = 0;

    uac_info->mic_max_vol = vol_convert(100);
    uac_info->mic_min_vol = vol_convert(0);
    uac_info->mic_vol_res = 0x30;
    uac_info->mic_def_vol = vol_convert(13 / 14.0 * 100);
    uac_info->mic_vol = uac_info->mic_def_vol;
    uac_info->mic_mute = 0;
    uac_info->ps4_mode = 0;
    return 0;
}
void uac_release(const usb_dev usb_id)
{
    struct usb_device_t *usb_device = usb_id2device(usb_id);

    if (uac_info) {
        close_spk(usb_device);
        close_mic(usb_device);
#if USB_MALLOC_ENABLE
        free(uac_info);
#endif
        uac_info = NULL;
    }
}
#endif
