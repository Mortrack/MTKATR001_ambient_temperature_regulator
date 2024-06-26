/** @addtogroup bl_side_etx_ota
 * @{
 */

/** @addtogroup main_etx_ota_config
 * @{
 */

/**@file
 * @brief	ETX OTA Protocol personalized configuration file for Bootloader Firmwares (edit ETX OTA Configurations here).
 *
 * @defgroup app_etx_ota_config Application ETX OTA Protocol Configuration File
 * @{
 *
 * @brief   This file should contain all the Application ETX OTA Protocol Configurations made by the user/implementor.
 *
 * @note    It is highly suggested not to directly edit the Configurations Settings defined in the @ref etx_ota_config
 *          file. Instead of doing that whenever requiring different Configuration Settings, it is suggested to do that
 *          instead in this additional header file named as "app_etx_ota_config.h". However, to enable the use of this
 *          additional header file, you will also need to change the @ref ENABLE_APP_ETX_OTA_CONFIG define, that is
 *          located at @ref etx_ota_config, to a value of 1.
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	February 10, 2024.
 */

#ifndef APP_ETX_OTA_CONFIG
#define APP_ETX_OTA_CONFIG

#define ETX_BL_FLASH_ADDR				(0x08001800U)							      /**< @brief Designated Flash Memory start address for the Bootloader Firmware of our MCU/MPU. @note The Start Memory Location address should correspond to the Flash Memory Page number 8. */
#define ETX_BL_FLASH_PAGES_SIZE 		(26U)   			                          /**< @brief Designated Flash Memory pages for the Bootloader Firmware of our MCU/MPU. */
#define HM10_DEFAULT_BLE_NAME           'M', 'T', 'K', 'A', 'T', 'R','0', '0', '1'    /**< @brief Designated ASCII Code data representing the desired default BT Name that wants to be given to the HM-10 BT Device, whose length has to be @ref HM10_MAX_BLE_NAME_SIZE at the most. */

#endif /* APP_ETX_OTA_CONFIG_H_ */

/** @} */ // app_etx_ota_config

/** @} */ // main_etx_ota_config

/** @} */ // bl_side_etx_ota
