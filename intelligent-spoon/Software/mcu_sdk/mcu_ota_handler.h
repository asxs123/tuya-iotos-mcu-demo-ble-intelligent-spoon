/****************************************Copyright (c)*************************
**                               Copyright (C) 2014-2020, Tuya Inc., All Rights Reserved
**
**                                 http://www.tuya.com
**
**--------------Revision record---------------------------------------------------
** version: v1.0
** date : may 3, 2017 
description: Initial version
**

**version::v2.0
** date: March 23, 2020
** description: 
1. Added module unbinding interface support, command code 0x09.
2.Add rf RF test interface support, command code 0x0e.
3.Add record-based data reporting interface support,command code 0xe0.
4. Added access to real-time time API support,command code 0xe1.
5. Added support for modifying sleep mode state bluetooth broadcast interval,command code 0xe2.
6. Added support for turning off system clock,command code 0xe4.
7. Increase low power consumption to enable support,commadn code 0xe5.
8. Add dynamic password authentication interface support,command code 0xe6.
9. Added support for disconnecting Bluetooth connection,command code 0xe7.
10. Added support for querying MCU version number,command code 0xe8.
11. Added support for MCU to actively send version Numbers,command code 0xe9.
12. Add OTA upgrade request support,command code 0xea.
13. Add OTA update file information support,command 0xeb.
14. Add OTA upgrade file migration request support,command code 0xec.
15. Add OTA upgrade data support,command code 0xed.
16. Add OTA upgrade end support,command code 0xee.
17. Added support for MCU to acquire module version information,commadn code 0xa0.
18. Added support for resuming factory Settings notifications,command code 0xa1.
19. Add MCU OTA demo code.
20. Optimized bt_uart_service.
**
**-----------------------------------------------------------------------------
******************************************************************************/

/******************************************************************************
						  Pay special attention!
The way of MCU OTA is strongly related to the chip. The MCU OTA program demo is not necessarily suitable for all chip platforms, but it is more or less the same. 
Users can modify the demo according to the upgrade mode of their chip platform or refer to the demo to complete the MCU OTA function.

******************************************************************************/



#ifndef  TUYA_OTA_HANDLER_H_
#define  TUYA_OTA_HANDLER_H_

#include <stdint.h>

#include "bluetooth.h"
/******************************************************************************
						  1: configuration upgrade
MCU_OTA_VERSION :The version of the mcu software
MCU_OTA_TYPE    0//0x00-Allow upgrade， 0x01-Refuse to upgrade

******************************************************************************/

#define MCU_OTA_VERSION MCU_APP_VER_NUM

#define MCU_OTA_TYPE    0//0x00-Allow upgrade， 0x01-Refuse to upgrade


/******************************************************************************
                          2:LOG         
If you want to print some log, you can improve the following macros

******************************************************************************/

#define TUYA_OTA_LOG(...)
#define  TUYA_OTA_HEXDUMP(...)



/******************************************************************************
						  3: configure the upgrade address
APP_NEW_FW_START_ADR :New firmware start address

APP_NEW_FW_END_ADR    0  :New firmware end address

APP_NEW_FW_SETTING_ADR  	:Firmware upgrade configuration information storage address

******************************************************************************/

#define	    APP_NEW_FW_START_ADR	            (0x44000)

#define	    APP_NEW_FW_END_ADR	                (0x61000-1)

#define 	APP_NEW_FW_SETTING_ADR				(0x63000)

#define	    APP_NEW_FW_MAX_SIZE                (APP_NEW_FW_END_ADR - APP_NEW_FW_START_ADR)//116K


#pragma anon_unions
typedef struct
{
    uint32_t command_size;              /**< The size of the current init command stored in the DFU settings. */
    uint32_t command_offset;            /**< The offset of the currently received init command data. The offset will increase as the init command is received. */
    uint32_t command_crc;               /**< The calculated CRC of the init command (calculated after the transfer is completed). */
    uint32_t data_object_size;          /**< The size of the last object created. Note that this size is not the size of the whole firmware image.*/
    struct
    {
        uint32_t firmware_file_version;
        uint32_t firmware_file_length;
        uint32_t firmware_file_crc;
        uint8_t  firmware_file_md5[16];        
    };
    union
    {
        struct
        {
            uint32_t firmware_image_crc;        /**< CRC value of the current firmware (continuously calculated as data is received). */
            uint32_t firmware_image_crc_last;   /**< The CRC of the last executed object. */
            uint32_t firmware_image_offset;     /**< The offset of the current firmware image being transferred. Note that this offset is the offset in the entire firmware image and not only the current object. */
            uint32_t firmware_image_offset_last;/**< The offset of the last executed object from the start of the firmware image. */
        };
        struct
        {
            uint32_t update_start_address;      /**< Value indicating the start address of the new firmware (before copy). It's always used, but it's most important for an SD/SD+BL update where the SD changes size or if the DFU process had a power loss when updating a SD with changed size. */
        };
    };
} dfu_progress_t;

/** @brief Description of a single bank. */
#pragma pack(4)
typedef struct
{
    uint32_t                image_size;         /**< Size of the image in the bank. */
    uint32_t                image_crc;          /**< CRC of the image. If set to 0, the CRC is ignored. */
    uint32_t                bank_code;          /**< Identifier code for the bank. */
} dfu_bank_t;

typedef struct
{
    uint32_t            crc;                /**< CRC for the stored DFU settings, not including the CRC itself. If 0xFFFFFFF, the CRC has never been calculated. */
    uint32_t            settings_version;   /**< Version of the current DFU settings struct layout. */
    uint32_t            app_version;        /**< Version of the last stored application. */
    uint32_t            bootloader_version; /**< Version of the last stored bootloader. */

    uint32_t            bank_layout;        /**< Bank layout: single bank or dual bank. This value can change. */
    uint32_t            bank_current;       /**< The bank that is currently used. */

    dfu_bank_t      bank_0;             /**< Bank 0. */
    dfu_bank_t      bank_1;             /**< Bank 1. */

    uint32_t            write_offset;       /**< Write offset for the current operation. */
    uint32_t            sd_size;            /**< Size of the SoftDevice. */

    dfu_progress_t      progress;           /**< Current DFU progress. */

    uint32_t            enter_buttonless_dfu;
    uint8_t             init_command[256];  /**< Buffer for storing the init command. */
} dfu_settings_t;


#define NRF_DFU_BANK_INVALID     0x00 /**< Invalid image. */
#define NRF_DFU_BANK_VALID_APP   0x01 /**< Valid application. */
#define NRF_DFU_BANK_VALID_SD    0xA5 /**< Valid SoftDevice. */
#define NRF_DFU_BANK_VALID_BL    0xAA /**< Valid bootloader. */
#define NRF_DFU_BANK_VALID_SD_BL 0xAC /**< Valid SoftDevice and bootloader. */

#define DFU_SETTING_SAVE_ADDR	APP_NEW_FW_SETTING_ADR


typedef enum 
{
	MCU_OTA_STATUS_NONE,
	MCU_OTA_STATUS_START,
	MCU_OTA_STATUS_FILE_INFO,
	MCU_OTA_STATUS_FILE_OFFSET,
    MCU_OTA_STATUS_FILE_DATA,
    MCU_OTA_STATUS_FILE_END,
	MCU_OTA_STATUS_MAX,
}mcu_ota_status_t;

/*****************************************************************************
Function name: mcu_ota_proc
Function description: mcu ota processing function
Input parameters: 
Return parameter: none
Instructions for use:
*****************************************************************************/

void mcu_ota_proc(uint16_t cmd,uint8_t *recv_data,uint32_t recv_len);
/*****************************************************************************
Function name: mcu_ota_init
Function description: mcu ota initialization function
Input parameters: 
Return parameter: 
Instructions for use: users need to improve the flash initialization function here. If there is a flash initialization operation elsewhere, the function can not be called.
*****************************************************************************/

uint32_t mcu_ota_init(void);
/*****************************************************************************
Function name: mcu_ota_init_disconnect
Function description: ota state initialization function
Input parameters: 
Return parameter: 
Instructions for use: restart\ module offline\ ota needs to be called actively when it fails.
*****************************************************************************/

uint8_t mcu_ota_init_disconnect(void);



#endif /* TUYA_OTA_H_ */





