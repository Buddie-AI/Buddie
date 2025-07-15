/*********************************************************************************************
    *   Filename        : config_transport.h

    *   Description     : Config Interface

    *   Author          : Bingquan

    *   Email           : bingquan_cai@zh-jieli.com

    *   Last modifiled  : 2019-01-07 14:33

    *   Copyright:(c)JIELI  2011-2017  @ , All Rights Reserved.
*********************************************************************************************/
#ifndef _CI_TRANSPORT_H
#define _CI_TRANSPORT_H

#include "typedef.h"

/* CI packet types */
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
     * register packet handler for CI packets
     */
    void (*register_packet_handler)(void (*handler)(const u8 *packet, int size));

    /**
     * support async transport layers, e.g. IRQ driven without buffers
     */
    int (*can_send_packet_now)(uint8_t packet_type);

    /**
     * send packet
     */
    int (*send_packet)(const u8 *packet, int size);

    /**
     * extension for UART transport implementations
     */
    int (*set_baudrate)(uint32_t baudrate);
} ci_transport_t;

typedef enum {
    CI_TRANSPORT_CONFIG_UART,
    CI_TRANSPORT_CONFIG_USB,
    CI_TRANSPORT_CONFIG_BLE,
} ci_transport_config_type_t;

typedef struct {
    ci_transport_config_type_t type;
} ci_transport_config_t;

typedef struct {
    ci_transport_config_type_t type; // == CI_TRANSPORT_CONFIG_UART
    uint32_t   baudrate_init; // initial baud rate
    uint32_t   baudrate_main; // = 0: same as initial baudrate
    int        flowcontrol;   //
    const char *device_name;
} ci_transport_config_uart_t;

typedef struct {
    //head
    u16 id;
    u16 length;

    u8  payload[0];
} _GNU_PACKED_ ci_packet_t;

#define CI_FORMAT_HEAD  sizeof(ci_packet_t)

const ci_transport_t *ci_transport_uart_instance(void);

#endif
