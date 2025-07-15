/**
 * <linux/usb/audio.h> -- USB Audio definitions.
 *
 * Copyright (C) 2006 Thumtronics Pty Ltd.
 * Developed for Thumtronics by Grey Innovation
 * Ben Williamson <ben.williamson@greyinnovation.com>
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 2, as published by the Free Software Foundation.
 *
 * This file holds USB constants and structures defined
 * by the USB Device Class Definition for Audio Devices.
 * Comments below reference relevant sections of that document:
 *
 * http://www.usb.org/developers/devclass_docs/audio10.pdf
 *
 * Types and defines in this file are either specific to version 1.0 of
 * this standard or common for newer versions.
 */

#ifndef __LINUX_USB_AUDIO_H
#define __LINUX_USB_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif
#define __le16  u16

#define u8      unsigned char   // u8  to u32 special for struct
#define u16     unsigned short  // u16 to u32 special for struct
#ifndef __s16
#define s16     short   		 // only s16
#endif
#define u32     unsigned int

/* A.8. Audio Class-Specific Request Codes */
#define UAC_RC_UNDEFINED		    0x00
#define UAC_SET_CUR					0x01
#define UAC_GET_CUR					0x81
#define UAC_GET_MIN					0x82
#define UAC_GET_MAX					0x83
#define UAC_GET_RES					0x84
#define UAC_GET_LEN					0x85
#define UAC_GET_INFO			    0x86
#define UAC_GET_DEF					0x87


/** bInterfaceProtocol values to denote the version of the standard used */
#define UAC_VERSION_1			0x00
#define UAC_VERSION_2			0x20

/** A.2 Audio Interface Subclass Codes */
#define USB_SUBCLASS_AUDIOCONTROL	0x01
#define USB_SUBCLASS_AUDIOSTREAMING	0x02
#define USB_SUBCLASS_MIDISTREAMING	0x03

/** A.5 Audio Class-Specific AC Interface Descriptor Subtypes */
#define UAC_HEADER			0x01
#define UAC_INPUT_TERMINAL		0x02
#define UAC_OUTPUT_TERMINAL		0x03
#define UAC_MIXER_UNIT			0x04
#define UAC_SELECTOR_UNIT		0x05
#define UAC_FEATURE_UNIT		0x06
#define UAC1_PROCESSING_UNIT		0x07
#define UAC1_EXTENSION_UNIT		0x08

/** A.6 Audio Class-Specific AS Interface Descriptor Subtypes */
#define UAC_AS_GENERAL			0x01
#define UAC_FORMAT_TYPE			0x02
#define UAC_FORMAT_SPECIFIC		0x03

/** A.7 Processing Unit Process Types */
#define UAC_PROCESS_UNDEFINED		0x00
#define UAC_PROCESS_UP_DOWNMIX		0x01
#define UAC_PROCESS_DOLBY_PROLOGIC	0x02
#define UAC_PROCESS_STEREO_EXTENDER	0x03
#define UAC_PROCESS_REVERB		0x04
#define UAC_PROCESS_CHORUS		0x05
#define UAC_PROCESS_DYN_RANGE_COMP	0x06

/** A.8 Audio Class-Specific Endpoint Descriptor Subtypes */
#define UAC_EP_GENERAL			0x01

/** A.9 Audio Class-Specific Request Codes */

#define UAC_GET_STAT			0xff

/** A.10 Control Selector Codes */

/** A.10.1 Terminal Control Selectors */
#define UAC_TERM_COPY_PROTECT		0x01

/** A.10.2 Feature Unit Control Selectors */
#define UAC_FU_MUTE			0x01
#define UAC_FU_VOLUME			0x02
#define UAC_FU_BASS			0x03
#define UAC_FU_MID			0x04
#define UAC_FU_TREBLE			0x05
#define UAC_FU_GRAPHIC_EQUALIZER	0x06
#define UAC_FU_AUTOMATIC_GAIN		0x07
#define UAC_FU_DELAY			0x08
#define UAC_FU_BASS_BOOST		0x09
#define UAC_FU_LOUDNESS			0x0a

#define UAC_CONTROL_BIT(CS)	(1 << ((CS) - 1))

/** A.10.3.1 Up/Down-mix Processing Unit Controls Selectors */
#define UAC_UD_ENABLE			0x01
#define UAC_UD_MODE_SELECT		0x02

/** A.10.3.2 Dolby Prologic (tm) Processing Unit Controls Selectors */
#define UAC_DP_ENABLE			0x01
#define UAC_DP_MODE_SELECT		0x02

/** A.10.3.3 3D Stereo Extender Processing Unit Control Selectors */
#define UAC_3D_ENABLE			0x01
#define UAC_3D_SPACE			0x02

/** A.10.3.4 Reverberation Processing Unit Control Selectors */
#define UAC_REVERB_ENABLE		0x01
#define UAC_REVERB_LEVEL		0x02
#define UAC_REVERB_TIME			0x03
#define UAC_REVERB_FEEDBACK		0x04

/** A.10.3.5 Chorus Processing Unit Control Selectors */
#define UAC_CHORUS_ENABLE		0x01
#define UAC_CHORUS_LEVEL		0x02
#define UAC_CHORUS_RATE			0x03
#define UAC_CHORUS_DEPTH		0x04

/** A.10.3.6 Dynamic Range Compressor Unit Control Selectors */
#define UAC_DCR_ENABLE			0x01
#define UAC_DCR_RATE			0x02
#define UAC_DCR_MAXAMPL			0x03
#define UAC_DCR_THRESHOLD		0x04
#define UAC_DCR_ATTACK_TIME		0x05
#define UAC_DCR_RELEASE_TIME		0x06

/** A.10.4 Extension Unit Control Selectors */
#define UAC_XU_ENABLE			0x01

/** MIDI - A.1 MS Class-Specific Interface Descriptor Subtypes */
#define UAC_MS_HEADER			0x01
#define UAC_MIDI_IN_JACK		0x02
#define UAC_MIDI_OUT_JACK		0x03

/** MIDI - A.1 MS Class-Specific Endpoint Descriptor Subtypes */
#define UAC_MS_GENERAL			0x01

/** Terminals - 2.1 USB Terminal Types */
#define UAC_TERMINAL_UNDEFINED		0x0100
#define UAC_TERMINAL_STREAMING		0x0101
#define UAC_TERMINAL_VENDOR_SPEC	0x01FF

/** Terminal Control Selectors */
/** 4.3.2  Class-Specific AC Interface Descriptor */
struct uac1_ac_header_descriptor {
    u8  bLength;			/** 8 + n */
    u8  bDescriptorType;		/** USB_DT_CS_INTERFACE */
    u8  bDescriptorSubtype;	/** UAC_MS_HEADER */
    __le16 bcdADC;			/** 0x0100 */
    __le16 wTotalLength;		/** includes Unit and Terminal desc. */
    u8  bInCollection;		/** n */
    u8  baInterfaceNr[];		/** [n] */
} __attribute__((packed));

#define UAC_DT_AC_HEADER_SIZE(n)	(8 + (n))

/** As above, but more useful for defining your own descriptors: */
#define DECLARE_UAC_AC_HEADER_DESCRIPTOR(n)			\
    struct uac1_ac_header_descriptor_##n {			\
        u8  bLength;						\
        u8  bDescriptorType;					\
        u8  bDescriptorSubtype;				\
        __le16 bcdADC;						\
        __le16 wTotalLength;					\
        u8  bInCollection;					\
        u8  baInterfaceNr[n];					\
    } __attribute__ ((packed))

/** 4.3.2.1 Input Terminal Descriptor */
struct uac_input_terminal_descriptor {
    u8  bLength;			/** in bytes: 12 */
    u8  bDescriptorType;		/** CS_INTERFACE descriptor type */
    u8  bDescriptorSubtype;	/** INPUT_TERMINAL descriptor subtype */
    u8  bTerminalID;		/** Constant uniquely terminal ID */
    __le16 wTerminalType;		/** USB Audio Terminal Types */
    u8  bAssocTerminal;		/** ID of the Output Terminal associated */
    u8  bNrChannels;		/** Number of logical output channels */
    __le16 wChannelConfig;
    u8  iChannelNames;
    u8  iTerminal;
} __attribute__((packed));

#define UAC_DT_INPUT_TERMINAL_SIZE			12

/** Terminals - 2.2 Input Terminal Types */
#define UAC_INPUT_TERMINAL_UNDEFINED			0x200
#define UAC_INPUT_TERMINAL_MICROPHONE			0x201
#define UAC_INPUT_TERMINAL_DESKTOP_MICROPHONE		0x202
#define UAC_INPUT_TERMINAL_PERSONAL_MICROPHONE		0x203
#define UAC_INPUT_TERMINAL_OMNI_DIR_MICROPHONE		0x204
#define UAC_INPUT_TERMINAL_MICROPHONE_ARRAY		0x205
#define UAC_INPUT_TERMINAL_PROC_MICROPHONE_ARRAY	0x206

/** Terminals - control selectors */

#define UAC_TERMINAL_CS_COPY_PROTECT_CONTROL		0x01

/** 4.3.2.2 Output Terminal Descriptor */
struct uac1_output_terminal_descriptor {
    u8  bLength;			/** in bytes: 9 */
    u8  bDescriptorType;		/** CS_INTERFACE descriptor type */
    u8  bDescriptorSubtype;	/** OUTPUT_TERMINAL descriptor subtype */
    u8  bTerminalID;		/** Constant uniquely terminal ID */
    __le16 wTerminalType;		/** USB Audio Terminal Types */
    u8  bAssocTerminal;		/** ID of the Input Terminal associated */
    u8  bSourceID;		/** ID of the connected Unit or Terminal*/
    u8  iTerminal;
} __attribute__((packed));

#define UAC_DT_OUTPUT_TERMINAL_SIZE			9

/** Terminals - 2.3 Output Terminal Types */
#define UAC_OUTPUT_TERMINAL_UNDEFINED			0x300
#define UAC_OUTPUT_TERMINAL_SPEAKER			0x301
#define UAC_OUTPUT_TERMINAL_HEADPHONES			0x302
#define UAC_OUTPUT_TERMINAL_HEAD_MOUNTED_DISPLAY_AUDIO	0x303
#define UAC_OUTPUT_TERMINAL_DESKTOP_SPEAKER		0x304
#define UAC_OUTPUT_TERMINAL_ROOM_SPEAKER		0x305
#define UAC_OUTPUT_TERMINAL_COMMUNICATION_SPEAKER	0x306
#define UAC_OUTPUT_TERMINAL_LOW_FREQ_EFFECTS_SPEAKER	0x307
#define UAC_OUTPUT_TERMINAL_HEADSET     0x0402

/** Set bControlSize = 2 as default setting */
#define UAC_DT_FEATURE_UNIT_SIZE(ch)		(7 + ((ch) + 1) * 2)

/** As above, but more useful for defining your own descriptors: */
#define DECLARE_UAC_FEATURE_UNIT_DESCRIPTOR(ch)			\
    struct uac_feature_unit_descriptor_##ch {			\
        u8  bLength;						\
        u8  bDescriptorType;					\
        u8  bDescriptorSubtype;				\
        u8  bUnitID;						\
        u8  bSourceID;					\
        u8  bControlSize;					\
        __le16 bmaControls[ch + 1];				\
        u8  iFeature;						\
    } __attribute__ ((packed))

/** 4.3.2.3 Mixer Unit Descriptor */
struct uac_mixer_unit_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bUnitID;
    u8 bNrInPins;
    u8 baSourceID[];
} __attribute__((packed));



/** 4.3.2.4 Selector Unit Descriptor */
struct uac_selector_unit_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bUintID;
    u8 bNrInPins;
    u8 baSourceID[];
} __attribute__((packed));



/** 4.3.2.5 Feature Unit Descriptor */
struct uac_feature_unit_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bUnitID;
    u8 bSourceID;
    u8 bControlSize;
    u8 bmaControls[0]; /** variable length */
} __attribute__((packed));



/** 4.3.2.6 Processing Unit Descriptors */
struct uac_processing_unit_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bUnitID;
    u16 wProcessType;
    u8 bNrInPins;
    u8 baSourceID[];
} __attribute__((packed));




/** 4.5.2 Class-Specific AS Interface Descriptor */
struct uac1_as_header_descriptor {
    u8  bLength;			/** in bytes: 7 */
    u8  bDescriptorType;		/** USB_DT_CS_INTERFACE */
    u8  bDescriptorSubtype;	/** AS_GENERAL */
    u8  bTerminalLink;		/** Terminal ID of connected Terminal */
    u8  bDelay;			/** Delay introduced by the data path */
    __le16 wFormatTag;		/** The Audio Data Format */
} __attribute__((packed));

#define UAC_DT_AS_HEADER_SIZE		7

/** Formats - A.1.1 Audio Data Format Type I Codes */
#define UAC_FORMAT_TYPE_I_UNDEFINED	0x0
#define UAC_FORMAT_TYPE_I_PCM		0x1
#define UAC_FORMAT_TYPE_I_PCM8		0x2
#define UAC_FORMAT_TYPE_I_IEEE_FLOAT	0x3
#define UAC_FORMAT_TYPE_I_ALAW		0x4
#define UAC_FORMAT_TYPE_I_MULAW		0x5

struct uac_format_type_i_continuous_descriptor {
    u8  bLength;			/** in bytes: 8 + (ns * 3) */
    u8  bDescriptorType;		/** USB_DT_CS_INTERFACE */
    u8  bDescriptorSubtype;	/** FORMAT_TYPE */
    u8  bFormatType;		/** FORMAT_TYPE_1 */
    u8  bNrChannels;		/** physical channels in the stream */
    u8  bSubframeSize;		/** */
    u8  bBitResolution;
    u8  bSamFreqType;
    u8  tLowerSamFreq[3];
    u8  tUpperSamFreq[3];
} __attribute__((packed));

#define UAC_FORMAT_TYPE_I_CONTINUOUS_DESC_SIZE	14

struct uac_format_type_i_discrete_descriptor {
    u8  bLength;			/** in bytes: 8 + (ns * 3) */
    u8  bDescriptorType;		/** USB_DT_CS_INTERFACE */
    u8  bDescriptorSubtype;	/** FORMAT_TYPE */
    u8  bFormatType;		/** FORMAT_TYPE_1 */
    u8  bNrChannels;		/** physical channels in the stream */
    u8  bSubframeSize;		/** */
    u8  bBitResolution;
    u8  bSamFreqType;
    u8  tSamFreq[][3];
} __attribute__((packed));

#define DECLARE_UAC_FORMAT_TYPE_I_DISCRETE_DESC(n)		\
    struct uac_format_type_i_discrete_descriptor_##n {		\
        u8  bLength;						\
        u8  bDescriptorType;					\
        u8  bDescriptorSubtype;				\
        u8  bFormatType;					\
        u8  bNrChannels;					\
        u8  bSubframeSize;					\
        u8  bBitResolution;					\
        u8  bSamFreqType;					\
        u8  tSamFreq[n][3];					\
    } __attribute__ ((packed))

#define UAC_FORMAT_TYPE_I_DISCRETE_DESC_SIZE(n)	(8 + (n * 3))

struct uac_format_type_i_ext_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bFormatType;
    u8 bSubslotSize;
    u8 bBitResolution;
    u8 bHeaderLength;
    u8 bControlSize;
    u8 bSideBandProtocol;
} __attribute__((packed));

/** Formats - Audio Data Format Type I Codes */

#define UAC_FORMAT_TYPE_II_MPEG	0x1001
#define UAC_FORMAT_TYPE_II_AC3	0x1002

struct uac_format_type_ii_discrete_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bFormatType;
    __le16 wMaxBitRate;
    __le16 wSamplesPerFrame;
    u8 bSamFreqType;
    u8 tSamFreq[][3];
} __attribute__((packed));

struct uac_format_type_ii_ext_descriptor {
    u8 bLength;
    u8 bDescriptorType;
    u8 bDescriptorSubtype;
    u8 bFormatType;
    u16 wMaxBitRate;
    u16 wSamplesPerFrame;
    u8 bHeaderLength;
    u8 bSideBandProtocol;
} __attribute__((packed));

/** type III */
#define UAC_FORMAT_TYPE_III_IEC1937_AC3	0x2001
#define UAC_FORMAT_TYPE_III_IEC1937_MPEG1_LAYER1	0x2002
#define UAC_FORMAT_TYPE_III_IEC1937_MPEG2_NOEXT	0x2003
#define UAC_FORMAT_TYPE_III_IEC1937_MPEG2_EXT	0x2004
#define UAC_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER1_LS	0x2005
#define UAC_FORMAT_TYPE_III_IEC1937_MPEG2_LAYER23_LS	0x2006

/** Formats - A.2 Format Type Codes */
#define UAC_FORMAT_TYPE_UNDEFINED	0x0
#define UAC_FORMAT_TYPE_I		0x1
#define UAC_FORMAT_TYPE_II		0x2
#define UAC_FORMAT_TYPE_III		0x3
#define UAC_EXT_FORMAT_TYPE_I		0x81
#define UAC_EXT_FORMAT_TYPE_II		0x82
#define UAC_EXT_FORMAT_TYPE_III		0x83

struct uac_iso_endpoint_descriptor {
    u8  bLength;			/** in bytes: 7 */
    u8  bDescriptorType;		/** USB_DT_CS_ENDPOINT */
    u8  bDescriptorSubtype;	/** EP_GENERAL */
    u8  bmAttributes;
    u8  bLockDelayUnits;
    __le16 wLockDelay;
} __attribute__((packed));
#define UAC_ISO_ENDPOINT_DESC_SIZE	7

#define UAC_EP_CS_ATTR_SAMPLE_RATE	0x01
#define UAC_EP_CS_ATTR_PITCH_CONTROL	0x02
#define UAC_EP_CS_ATTR_FILL_MAX		0x80

/** status word format (3.7.1.1) */

#define UAC1_STATUS_TYPE_ORIG_MASK		0x0f
#define UAC1_STATUS_TYPE_ORIG_AUDIO_CONTROL_IF	0x0
#define UAC1_STATUS_TYPE_ORIG_AUDIO_STREAM_IF	0x1
#define UAC1_STATUS_TYPE_ORIG_AUDIO_STREAM_EP	0x2

#define UAC1_STATUS_TYPE_IRQ_PENDING		(1 << 7)
#define UAC1_STATUS_TYPE_MEM_CHANGED		(1 << 6)

#include "asm/usb.h"
struct uac1_status_word {
    u8 bStatusType;
    u8 bOriginator;
} __attribute__((packed));

u32 uac_setup_endpoint(struct usb_device_t *usb_device, struct usb_ctrlrequest *req);
u32 uac_spk_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);
void uac_speaker_stream_write(const u8 *obuf, u32 len);
void uac_speaker_stream_open(u32 samplerate, u32 ch);
void uac_speaker_stream_close();

u32 uac_mic_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);
u32 uac_mic_stream_open(u32 samplerate, u32 frame_len, u32 ch);
int uac_mic_stream_read(u8 *buf, u32 len);
void uac_mic_stream_close();
void uac_mute_volume(u32 type, u32 l_vol, u32 r_vol);
int uac_get_spk_vol();
u32 uac_audio_desc_config(const usb_dev usb_id, u8 *ptr, u32 *cur_itf_num);
void uac_audio_disable(const usb_dev usb_id);
const u8 *uac_get_string(u32 id);
u32 uac_register(const usb_dev usb_id, const u32 class);
void uac_release(const usb_dev usb_id);
#ifdef __cplusplus
}
#endif
#endif /** __LINUX_USB_AUDIO_H */
