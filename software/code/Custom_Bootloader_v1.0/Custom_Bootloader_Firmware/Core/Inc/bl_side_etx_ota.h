/** @file
 *	@brief	ETX OTA Protocol source file for Bootloader Firmwares.
 *
 * @defgroup bl_side_etx_ota Firmware Update module
 * @{
 *
 * @brief	This module provides the functions required to enable the application to be able to handle Firmware Update
 *          requests via ETX OTA Protocol, where it will temporarily install whatever applicable Firmware Image (which
 *          may either be a Bootlaoder or Application Firmware Image) into our MCU/MPU's designated Flash Memory space
 *          for the Application Firmware through a desired Hardware Protocol.
 *
 * @note    For more details on the available Hardware Protocol, see @ref ETX_OTA_hw_Protocol .
 *
 * @details The way in which these temporary Firmware installation processes will be made is via the ETX OTA Protocol,
 *          which is a protocol that serves for the purpose of transferring data from a certain host device to a slave
 *          device (i.e., our MCU/MPU) and, in particular, for data whose integrity is considered to be critically
 *          important since this protocol prioritizes data transfer reliability over data transfer speed.
 * @details	Now, speaking more specifically regarding how the ETX OTA process sends/receives data through Packets, the
 *          following information will give more details about that.
 * @details There 4 types of Packets:
 *          <ol>
 *              <li>Command Type Packets</li>
 *              <li>Header Type Packets</li>
 *              <li>Data Type Packets</li>
 *              <li>Response Type Packets</li>
 *          </ol>
 *          where it is suggested to see @ref ETX_OTA_Packet_t for more details.
 * @details The General Data Format for all types of Packets is the following set of bytes:
 *          <ol>
 *              <li>Start of Frame (SOF): 1 byte</li>
 *              <li>Packet Type: 1 byte</li>
 *              <li>Data Length: 2 bytes</li>
 *              <li>Data: 1 up to 1024 bytes (size must be perfectly divisible by 4 bytes)</li>
 *              <li>CRC32: 4 bytes</li>
 *              <li>End of Frame (EOF): 1 byte</li>
 *          </ol>
 *          but for finer details on the Data Format for each type of ETX OTA Packet, see @ref ETX_OTA_Command_Packet_t ,
 *          @ref ETX_OTA_Header_Packet_t , @ref ETX_OTA_Data_Packet_t and @ref ETX_OTA_Response_Packet_t .
 * @details On the other hand, a whole ETX OTA Transaction has 5 different states, where each of them can indicate in
 *          what part of the whole transaction we current are at, which are given by the following in that order:
 *          <ol>
 *              <li>ETX OTA Idle State</li>
 *              <li>ETX OTA Start State</li>
 *              <li>ETX OTA Header State</li>
 *              <li>ETX OTA Data State</li>
 *              <li>ETX OTA End State</li>
 *          </ol>
 *          where it is suggested to see @ref ETX_OTA_State for more details.
 * @details Finally, the way our MCU/MPU will interact with a host is that the host will send a single packet for each
 * 			ETX OTA State, except maybe in the Data State since multiple packets are allowed there. Nonetheless, our
 * 			MCU/MPU will expect the ETX OTA States to transition in the order previously shown and our MCU/MPU will
 * 			also both validate the received data and, if it is successful on that, it will always respond back to the
 * 			host with an ETX OTA Response Type Packet to let it know whether or not our MCU/MPU was able to successfully
 * 			get and process the data of the each Packet by sending an ACK, or otherwise with a NACK.
 *
 * @note    Since this module makes use of the System Tick Time for several of its processes, it is highly suggested not
 *          to make time critical time applications that also use the System Tick Time. This is because whenever the
 *          processes of this module are triggered, then those processes will take over whatever application you
 *          developed. Instead, whenever requiring critical time applications, use one of the Timer peripherals of your
 *          MCU/MPU.
 * @note    <b style="color:orange">IMPORTANT:</b> The @ref hm10_ble , which is included by this @ref bl_side_etx_ota
 *          (via @ref firmware_update_config --> @ref etx_ota_config ) has included the "stm32f1xx_hal.h" header file
 *          to be able to use HAL functions in this module. However, this header file is specifically meant for the
 *          STM32F1 series devices. If yours is from a different type, then you will have to substitute the right one
 *          here for your particular STMicroelectronics device. However, if you cant figure out what the name of that
 *          header file is, then simply substitute that line of code from this @ref hm10_ble by: #include "main.h"
 * @note    For an application code example of this module, see the application code example in the main.c file.<br><br>
 * @note    <b style="color:red">WARNING:</b> This Library should be able to perfectly compile with no GCC Optimization
 *          Level (i.e., GCC "O0" Optimization Level). However, If you were to compile a certain project that uses this
 *          Library with a certain GCC Optimization Level, then it is very likely that this Library will not work. The
 *          reason for this does not lie in this actual Library, but in the HAL Library of the STMicroelectronics Team.
 *          In particular, this gives place in the @ref HAL_FLASH_Unlock function, which requires to be executed
 *          exactly as how the STMicroelectronics Team coded it, which does not happen whenever you Optimize that code.
 *          Therefore, one solution for this problem is to tell the GCC Compiler not to optimize the code of the
 *          @ref HAL_FLASH_Unlock function.  One way to do this is by using the Pragma Tags as in the following
 *          example:<br>
 * @code
 #pragma GCC push_options
 #pragma GCC optimize ("O0")
 HAL_StatusTypeDef HAL_FLASH_Unlock(void)
 {
     // Code of the HAL_FLASH_Unlock() Function here (I did not pasted its code here for simplicity reasons).
 }
 #pragma GCC pop_options
 * @endcode
 *          where these Pragma tags will tell the GCC compiler not to Optimize all the code inside
 *          "#pragma GCC push_options" and "#pragma GCC pop_options" Tags.<br>
 *          However, this will only work if your GCC is v4.4 or higher. Therefore, if your GCC has a lower version, then
 *          use the following method to tell the GCC Compiler not to optimize that code:
 * @code
 HAL_StatusTypeDef __attribute__((optimize("O0"))) HAL_FLASH_Unlock(void)
 {
     // Code of the HAL_FLASH_Unlock() Function here (I did not pasted its code here for simplicity reasons).
 }
 * @endcode
 *          <b style="color:orange">Whenever any of these solutions is implemented, it is very important to</b> make
 *          sure to rewrite the HAL Library of the STMicroelectronics Team with the chosen solution after each time you
 *          re-configure your project via the STM32CubeMx App since it is likely for this App to rewrite the HAL Library
 *          as it originally was released by the STMicroelectronics Team, which as previously stated, it does not
 *          include any of the proposed solutions.
 *
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	December 13, 2023.
 * @date    LAST UPDATE: February 10, 2024.
 */
#include <stdint.h>
#include "firmware_update_config.h" // We call the library that holds the Firmware Update Configurations sub-module.

#ifndef BL_SIDE_ETX_OTA_H_
#define BL_SIDE_ETX_OTA_H_

/**@brief	ETX OTA Exception codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref bl_side_etx_ota module to indicate the
 *          resulting status of having executed the process contained in each of those functions. For example, to
 *          indicate that the process executed by a certain function was successful or that it has failed.
 */
typedef enum
{
    ETX_OTA_EC_OK       = 0U,    //!< ETX OTA Protocol was successful. @note The code from the @ref HAL_ret_handler function contemplates that this value will match the one given for \c HAL_OK from @ref HAL_StatusTypeDef .
    ETX_OTA_EC_STOP     = 1U,    //!< ETX OTA Protocol process or transaction has been stopped.
    ETX_OTA_EC_NR		= 2U,	 //!< ETX OTA Protocol has concluded with no response from Host.
    ETX_OTA_EC_NA       = 3U,    //!< ETX OTA Payload received or to be received Not Applicable.
    ETX_OTA_EC_ERR      = 4U     //!< ETX OTA Protocol has failed.
} ETX_OTA_Status;

/**@brief	ETX OTA Hardware Protocol definitions.
 *
 * @details	These definitions define the available ETX OTA <u>Hardware Protocols</u> that our MCU can use to establish a
 *          "physical" connection with a host machine, in which both ends will communicate with each other by using the
 *          ETX OTA <u>Software Protocol</u> for interpreting the data send/received on both sides.
 */
typedef enum
{
    ETX_OTA_hw_Protocol_UART    = 0U,    //!< UART Hardware Protocol to be used for the ETX OTA Protocol. @details The default expected UART configuration settings are 115'200 baud rate, 8 data-bits, no parity, 1 stop-bit and no Flow Control. However, these can be changed as long as these configurations are also changed on the host app, where it is indispensable that they match.
    ETX_OTA_hw_Protocol_BT      = 1U     //!< Bluetooth Hardware Protocol to be used for the ETX OTA Protocol. @details The BT module of our MCU is expected to use is the HM-10 BT Device via the @ref hm10_ble , which our MCU communicates with via UART under the default expected configuration settings of 9600 baud rate, 8 data-bits, no parity, 1 stop-bit and no Flow Control. However, these can be changed as long as these configurations are changed both on the HM-10 BT Device and in our MCU's UART towards which that HM-10 BT Device is connected to, where it is indispensable that they both match.
} ETX_OTA_hw_Protocol;

/**@brief   Attempts to receive and install an Application Firmware Image into our MCU/MPU.
 *
 * @details The Application Firmware Image is expected to be received, during the Timeout specified in
 *          @ref ETX_CUSTOM_HAL_TIMEOUT, from a certain host via the initialized Hardware Protocol (see
 *          @ref init_firmware_update_module function) and by using the ETX OTA Communication Protocol.
 *
 * @note    This function may not be able to successfully complete an entire ETX OTA Transaction if there are any
 *          Non-Blocking Callback functions working while this function is still running. Therefore, if your program
 *          required those type of Callback functions, make sure to stop them before calling this function and then
 *          start them again after this function terminates.
 * @note    For more details on how the ETX OTA Protocol works, see the @ref bl_side_etx_ota module.
 * @note	This function expects that the @ref firmware_update_config has already been initialized via the
 *          @ref firmware_update_configurations_init function.
 *
 * @retval  ETX_OTA_EC_OK
 * @retval	ETX_OTA_EC_STOP
 * @retval	ETX_OTA_EC_NR
 * @retval	ETX_OTA_EC_NA
 * @retval	ETX_OTA_EC_ERR
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date    November 14, 2023.
 */
ETX_OTA_Status firmware_image_download_and_install();

/**@brief   Initializes the @ref bl_side_etx_ota in order to be able to use its provided functions.
 *
 * @details This function will update several of its Global Static Pointers and Variables that are used in the
 *          @ref bl_side_etx_ota and will then validate the requested Hardware Protocol into which it is desired to
 *          apply the ETX OTA Protocol.
 *
 * @note    This function updates the following Global Static Pointers and variables:<br>
 *          - The @ref ETX_OTA_hardware_protocol Global Static Variable with the value of the \p hardware_protocol
 *            param.
 *          - The @ref p_huart Global Static Pointer to point to the address towards which the \p huart param points.
 *          - The @ref p_fw_config Global Static Pointer to point to the address towards which the \p fw_config param
 *            points.
 *          - The @ref p_GPIO_is_hm10_default_settings_Pin Global Static Pointer to point to the address towards which
 *            the \p GPIO_is_hm10_default_settings_Pin param points.
 *
 * @details After the validation process mentioned, this function will not do anything else in the case that the UART
 *          Hardware Protocol is chosen. However, if the BT Hardware Protocol is selected instead, then this function
 *          will initialize the HM-10 BT Device whose UART peripheral connection is indicated via the \p huart param.
 *
 * @param hardware_protocol                     The Hardware Protocol into which it is desired to use the ETX OTA
 *                                              Protocol.
 * @param[in] huart	                            Pointer to the UART Handle Structure of the UART that it is desired to
 *                                              use for the requested \p hardware_protocol .
 * @param[in,out] fw_config                     Pointer to the struct that should already contain the latest data of the
 *                                              @ref firmware_update_config .
 *
 * @note	This function expects that the \p p_fw_config pointer points to an already initialized
 *          firmware_update_config_data_t type structure via the @ref firmware_update_configurations_init function of
 *          the @ref firmware_update_config .
 *
 * @param[in] GPIO_is_hm10_default_settings     Pointer to the GPIO Definition Type of the GPIO Pin from which it can be
 *                                              requested to reset the Configuration Settings of the HM-10 BT Device to
 *                                              its default settings. However, if the UART Hardware Protocol is selected
 *                                              (via the \p hardware_protocol param), then this param will not be used
 *                                              and, in that case, it is recommended to set its value to \c NULL.
 *
 * @note    The Input Mode GPIO of the GPIO Definition Type towards which the \p GPIO_is_hm10_default_settings param
 *          points to, will be used so that our MCU can know whether the user wants it to set the default configuration
 *          settings in the HM-10 BT Device.
 * @note    The following are the possible values of the GPIO Pin designated here:<br><br>* 0 (i.e., Low State) = Do not
 *          reset/change the configuration settings of the HM-10 BT Device.<br>* 1 (i.e., High State) = User requests to
 *          reset the configuration settings of the HM-10 BT Device to its default settings.
 *
 * @retval  ETX_OTA_EC_OK   if the Firmware Update Module has been successfully initialized.
 * @retval  ETX_OTA_EC_ERR  <ul>
 *                              <li>
 *                                  If the \p hardware_protocol param requests to use an invalid Hardware Protocol.
 *                              </li>
 *                              <li>
 *                                  If, after the Hardware Protocol Validation process and whenever choosing the BT
 *                                  Hardware Protocol, there is an error/problem that impedes initializing or resetting
 *                                  all the configuration settings of the HM-10 BT Device to its Default Settings.
 *                              </li>
 *                          </ul>
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    December 13, 2023.
 */
ETX_OTA_Status init_firmware_update_module(ETX_OTA_hw_Protocol hardware_protocol,
                                           UART_HandleTypeDef *huart,
                                           firmware_update_config_data_t *fw_config,
                                           HM10_GPIO_def_t *GPIO_is_hm10_default_settings);

#endif /* BL_SIDE_ETX_OTA_H_ */

/** @} */