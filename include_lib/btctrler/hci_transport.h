/*********************************************************************************************
    *   Filename        : hci_transport.h

    *   Description     :

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2017-01-17 15:14

    *   Copyright:(c)JIELI  2011-2016  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef __BTCONTROLLER_HCI_TRANSPORT_H
#define __BTCONTROLLER_HCI_TRANSPORT_H

//#include <stdint.h>
#include "generic/typedef.h"

#if defined __cplusplus
extern "C" {
#endif

    /* API_START */
    typedef struct {
        uint32_t   baudrate;
        int        flowcontrol;
        const char *device_name;
    } btstack_uart_config_t;

    typedef enum {
        // UART active, sleep off
        BTSTACK_UART_SLEEP_OFF = 0,
        // used for eHCILL
        BTSTACK_UART_SLEEP_RTS_HIGH_WAKE_ON_CTS_PULSE,
        // used for H5 and for eHCILL without support for wake on CTS pulse
        BTSTACK_UART_SLEEP_RTS_LOW_WAKE_ON_RX_EDGE,

    } btstack_uart_sleep_mode_t;

    typedef enum {
        BTSTACK_UART_SLEEP_MASK_RTS_HIGH_WAKE_ON_CTS_PULSE  = 1 << BTSTACK_UART_SLEEP_RTS_HIGH_WAKE_ON_CTS_PULSE,
        BTSTACK_UART_SLEEP_MASK_RTS_LOW_WAKE_ON_RX_EDGE     = 1 << BTSTACK_UART_SLEEP_RTS_LOW_WAKE_ON_RX_EDGE
    } btstack_uart_sleep_mode_mask_t;

    typedef struct {
        /**
         * init transport
         * @param uart_config
         */
        int (*init)(const btstack_uart_config_t *uart_config);

        /**
         * open transport connection
         */
        int (*open)(void);

        /**
         * close transport connection
         */
        int (*close)(void);

        /**
         * set callback for block received. NULL disables callback
         */
        void (*set_block_received)(void (*block_handler)(void));

        /**
         * set callback for sent. NULL disables callback
         */
        void (*set_block_sent)(void (*block_handler)(void));

        /**
         * set baudrate
         */
        int (*set_baudrate)(uint32_t baudrate);

        /**
         * set parity
         */
        int (*set_parity)(int parity);

        /**
         * set flowcontrol
         */
        int (*set_flowcontrol)(int flowcontrol);

        /**
         * receive block
         */
        void (*receive_block)(uint8_t *buffer, uint16_t len);

        /**
         * send block
         */
        void (*send_block)(const uint8_t *buffer, uint16_t length);

        // support for sleep modes in TI's H4 eHCILL and H5

        /**
         * query supported wakeup mechanisms
         * @return supported_sleep_modes mask
         */
        int (*get_supported_sleep_modes)(void);

        /**
         * set UART sleep mode - allows to turn off UART and it's clocks to save energy
         * Supported sleep modes:
         * - off: UART active, RTS low if receive_block was called and block not read yet
         * - RTS high, wake on CTS: RTS should be high. On CTS pulse, UART gets enabled again and RTS goes to low
         * - RTS low, wake on RX: data on RX will trigger UART enable, bytes might get lost
         */
        void (*set_sleep)(btstack_uart_sleep_mode_t sleep_mode);

        /**
         * set wakeup handler - needed to notify hci transport of wakeup requests by Bluetooth controller
         * Called upon CTS pulse or RX data. See sleep modes.
         */
        void (*set_wakeup_handler)(void (*wakeup_handler)(void));

    } btstack_uart_block_t;

// common implementations
    const btstack_uart_block_t *btstack_uart_block_posix_instance(void);
    const btstack_uart_block_t *btstack_uart_block_windows_instance(void);
    const btstack_uart_block_t *btstack_uart_block_embedded_instance(void);
    const btstack_uart_block_t *btstack_uart_block_freertos_instance(void);

    /* HCI packet types */
    typedef struct {
        /**
         * transport name
         */
        const char *name;

        /**
         * init transport
         * @param transport_config
         */
        void (*init)(const void *transport_config);

        /**
         * open transport connection
         */
        int (*open)(void);

        /**
         * close transport connection
         */
        int (*close)(void);

        /**
         * register packet handler for HCI packets: ACL, SCO, and Events
         */
        void (*register_packet_handler)(void (*handler)(int packet_type, const u8 *packet, int size));

        /**
         * support async transport layers, e.g. IRQ driven without buffers
         */
        int (*can_send_packet_now)(uint8_t packet_type);

        /**
         * send packet
         */
        int (*send_packet)(int packet_type, const u8 *packet, int size);

        /**
         * extension for UART transport implementations
         */
        int (*set_baudrate)(uint32_t baudrate);

        /**
         * extension for UART H5 on CSR: reset BCSP/H5 Link
         */
        void (*reset_link)(void);

        /**
         * extension for USB transport implementations: config SCO connections
         */
        void (*set_sco_config)(uint16_t voice_setting, int num_connections);

    } hci_transport_t;

    typedef enum {
        HCI_TRANSPORT_CONFIG_UART,
        HCI_TRANSPORT_CONFIG_USB
    } hci_transport_config_type_t;

    typedef struct {
        hci_transport_config_type_t type;
    } hci_transport_config_t;

    typedef struct {
        hci_transport_config_type_t type; // == HCI_TRANSPORT_CONFIG_UART
        uint32_t   baudrate_init; // initial baud rate
        uint32_t   baudrate_main; // = 0: same as initial baudrate
        int        flowcontrol;   //
        const char *device_name;
    } hci_transport_config_uart_t;


// inline various hci_transport_X.h files

    /*
     * @brief Setup H4 instance with uart_driver
     * @param uart_driver to use
     */
    const hci_transport_t *hci_transport_h4_instance(const btstack_uart_block_t *uart_driver);

    /*
     * @brief Setup H5 instance with uart_driver
     * @param uart_driver to use
     */
    const hci_transport_t *hci_transport_h5_instance(const btstack_uart_block_t *uart_driver);

    /*
     * @brief Enable H5 Low Power Mode: enter sleep mode after x ms of inactivity
     * @param inactivity_timeout_ms or 0 for off
     */
    void hci_transport_h5_set_auto_sleep(uint16_t inactivity_timeout_ms);

    /*
     * @brief Enable BSCP mode H5, by enabling event parity
     */
    void hci_transport_h5_enable_bcsp_mode(void);

    /*
     * @brief
     */
    const hci_transport_t *hci_transport_usb_instance(void);

    const hci_transport_t *hci_transport_uart_instance(void);

    const hci_transport_t *hci_transport_h4_controller_instance(void);

    const hci_transport_t *hci_transport_h4_host_instance(void);
    /**
     * @brief Specify USB Bluetooth device via port numbers from root to device
     */
    void hci_transport_usb_set_path(int len, uint8_t *port_numbers);

    /* API_END */
    extern const hci_transport_t *hci_transport;

#if defined __cplusplus
}
#endif

#endif // __HCI_TRANSPORT_H
