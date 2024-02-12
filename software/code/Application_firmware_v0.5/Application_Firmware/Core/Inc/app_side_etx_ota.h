/** @file
 *	@brief	ETX OTA Protocol source file for Application Firmwares.
 *
 * @defgroup app_side_etx_ota ETX OTA Protocol module
 * @{
 *
 * @brief	This module provides the functions required to enable the application to be able to handle transaction
 *          requests via ETX OTA Protocol, via a desired Hardware Protocol, which can involve either handling a
 *          Bootloader or an Application Firmware Image request or an ETX OTA Custom Data request from a certain
 *          host machine.
 *
 * @note    For more details on the available Hardware Protocol, see @ref ETX_OTA_hw_Protocol .
 *
 * @details The way in which these requests will be handled is via the ETX OTA Protocol, which is a protocol that serves
 *          for the purpose of transferring data from a certain host device to a slave device (i.e., our MCU/MPU) and,
 *          in particular, for data whose integrity is considered to be critically important since this protocol
 *          prioritizes data transfer reliability over data transfer speed.
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
 *              <li>Data: 1 up to 1024 bytes</li>
 *              <li>CRC32: 4 bytes</li>
 *              <li>End of Frame (EOF): 1 byte</li>
 *          </ol>
 *          but for finer details on the Data Format for each type of ETX OTA Packets, see @ref ETX_OTA_Command_Packet_t ,
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
 * @details Finally, the way our MCU/MPU will interact with a host is that the host will first send an ETX OTA Command
 *          Type Packet with the Abort Command over and over until the host gets an ACK response back. Only after that,
 *          the host will start with the actually desired ETX OTA Transaction, which is to send a desired payload to our
 *          MCU/MPU, where a single packet for each ETX OTA ETX OTA State will be send, except maybe in the Data State
 *          since multiple packets are allowed there. Nonetheless, our MCU/MPU will expect the ETX OTA States to
 *          transition in the order previously shown and our MCU/MPU will also both validate the received data and, if
 *          it is successful on that, it will always respond back to the host with an ETX OTA Response Type Packet to
 *          let it know whether or not our MCU/MPU was able to successfully get and process the data of the each Packet
 *          by sending an ACK, or otherwise with a NACK.
 *
 * @note    Since this module makes use of the System Tick Time for several of its processes, it is highly suggested not
 *          to make time critical time applications that also use the System Tick Time. This is because whenever the
 *          processes of this module are triggered (e.g., @ref etx_ota_pre_transaction_handler or @ref
 *          etx_ota_status_resp_handler ), then those processes will take over whatever application you developed.
 *          Instead, whenever requiring critical time applications, use one of the Timer peripherals of your MCU/MPU.
 * @note    As for why the process requires that the host first sends an ETX OTA Command Type Packet with the Abort
 *          Command over and over until it receives an ACK response, this is because although this module starts
 *          receiving Data in non-blocking mode, it actually proceeds with receiving the second and next bytes by
 *          polling the data until end of ETX OTA Transaction. However, because these polls are made during the
 *          non-blocking mode, at least on the STMicroelectronics devices, that means that our MCU/MPU could get into
 *          the case where it gets stuck indefinitely waiting for another byte whenever an ETX OTA Transaction is
 *          interrupted because the polling's timeouts are disabled during the non-blocking mode. Therefore, whenever
 *          the host desires to start a new ETX OTA Transaction, the most reliable way to guarantee a successful
 *          transaction is by first sending as many Abort Commands as necessary until our MCU/MPU responds back with an
 *          ACK response to then send the actually desired ETX OTA Transaction.
 * @note    For those who may not know, non-blocking mode data transaction allows a certain code to be in the background
 *          while the MCU/MPU works with whatever code it was programmed in its main application code (i.e., main.c ),
 *          and where that background code gets triggered and takes control only after it starts receiving or
 *          sending some data, where in this particular case, the ETX OTA Protocol's code only takes control over the
 *          application right after our MCU/MPU starts an ETX OTA Transaction with a certain host device and will
 *          eventually release the control, but only after that transaction concludes.
 * @note    <b style="color:orange">IMPORTANT:</b> The @ref hm10_ble , which is included by this @ref app_side_etx_ota
 *          (via @ref firmware_update_config --> @ref etx_ota_config ) has included the "stm32f1xx_hal.h" header file
 *          to be able to use HAL functions in this module. However, this header file is specifically meant for the
 *          STM32F1 series devices. If yours is from a different type, then you will have to substitute the right one
 *          here for your particular STMicroelectronics device. However, if you cant figure out what the name of that
 *          header file is, then simply substitute that line of code from this @ref hm10_ble by: #include "main.h"
 * @note    For an application code example of this module, see the application code example inside the main.c file.
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date	December 13, 2023.
 */
#include <stdint.h>
#include "firmware_update_config.h" // We call the library that holds the Firmware Update Configurations sub-module.

#ifndef APP_SIDE_ETX_OTA_H_
#define APP_SIDE_ETX_OTA_H_

/**@brief	ETX OTA Exception codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref app_side_etx_ota module to indicate the
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
    ETX_OTA_hw_Protocol_BT	    = 1U     //!< Bluetooth Hardware Protocol to be used for the ETX OTA Protocol. @details The BT module of our MCU is expected to use is the HM-10 BT Device via the @ref hm10_ble , which our MCU communicates with via UART under the default expected configuration settings of 9600 baud rate, 8 data-bits, no parity, 1 stop-bit and no Flow Control. However, these can be changed as long as these configurations are changed both on the HM-10 BT Device and in our MCU's UART towards which that HM-10 BT Device is connected to, where it is indispensable that they both match.
} ETX_OTA_hw_Protocol;

/**@brief	ETX OTA Custom Data parameters structure. This contains all the fields required for handling ETX OTA Custom
 *          Data.
 */
typedef struct
{
    uint32_t size;                             //!< Size in bytes of the ETX OTA Custom Data received by our MCU/MPU.
    uint32_t rec_crc;                          //!< Recorded CRC of the ETX OTA Custom Data received by our MCU/MPU.
    uint8_t data[CUSTOM_DATA_MAX_SIZE];        //!< Size in bytes of the ETX OTA Custom Data received by our MCU/MPU.
} etx_ota_custom_data_t;

/**@brief   Initializes the @ref app_side_etx_ota in order to be able to use its provided functions.
 *
 * @details This function will also update several of its Global Static Pointers and Variables that are used in the
 *          @ref app_side_etx_ota and will then validate the requested Hardware Protocol into which it is desired to
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
 *          - The @ref p_custom_data Global Static Pointer to point to the address towards which the
 *            \p etx_ota_custom_data param points.
 *          - Resets the first byte of @ref Rx_Buffer to \c 0 (i.e., the position for the @ref ETX_OTA_SOF byte).
 *
 * @note    The logic of this function and of the @ref firmware_image_download_and_install function require to have the
 *          first byte of @ref Rx_Buffer set to \c 0 each time before a new ETX OTA Packet is received. This will allow
 *          the functions contained in the @ref app_side_etx_ota to be able to differentiate an @ref ETX_OTA_SOF byte
 *          received from a non blocking mode reception (via @ref HAL_UART_RxCpltCallback) and a poll reception (via
 *          either @ref HAL_UART_Receive or @ref get_hm10_ota_data function ) with respect to how the logic of the code
 *          has been written.
 *
 * @details After the validation process mentioned, this function will not do anything else in the case that the UART
 *          Hardware Protocol is chosen. However, if the BT Hardware Protocol is selected instead, then this function
 *          will initialize the HM-10 BT Device whose UART peripheral connection is indicated via the \p huart param.
 * @details Finally, this function will start ETX OTA Data Reception via the @ref start_etx_ota function.
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
 *          reset/change the configuration settings of the HM-10 BT Device.<br>* 1 (i.e., High State) = User requests
 *          to reset the configuration settings of the HM-10 BT Device to its default settings.
 *
 * @param[in] etx_ota_custom_data               Pointer to the handling struct of an ETX OTA Custom Data.
 *
 * @note    For more details on the etx_ota_custom_data param, see @ref ETX_OTA_Payload_t .
 *
 * @retval  ETX_OTA_EC_OK   if the ETX OTA Protocol Module has been successfully initialized.
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
										   HM10_GPIO_def_t *GPIO_is_hm10_default_settings,
                                           etx_ota_custom_data_t *etx_ota_custom_data);

/**@brief	Either starts or enables back again the ETX OTA data reception.
 *
 * @details	This function sets the @ref is_etx_ota_enabled Global Flag to its enabled value so that the
 *          @ref app_side_etx_ota enables the ETX OTA data reception. In addition, this function sets the next ETX OTA
 *          byte to be received in non blocking mode.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 21, 2023.
 */
void start_etx_ota();

/**@brief	Stops/disables the ETX OTA data reception.
 *
 * @details	This function sets the @ref is_etx_ota_enabled Global Flag to its disabled value so that the
 *          @ref app_side_etx_ota stops the ETX OTA data reception after the current ETX OTA Transaction that is being
 *          processed, if any, concludes.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 21, 2023.
 */
void stop_etx_ota();

/**@brief	Callback function before an ETX OTA Transaction with the host machine is about to give place.
 *
 * @details	This main purpose for providing this function is so that the implementer can use it to override it from
 *          wherever the @ref app_side_etx_ota is implemented from and so that it is possible to stop or do any
 *          necessary preparations from the actual application purpose of our MCU/MPU before an ETX OTA Transaction
 *          gives place (e.g., an emergency stop of a motor or to finish executing a certain task first).
 *
 * @note    If ETX OTA Transactions have been stopped via the @ref stop_etx_ota function and have not yet been started
 *          again via the @ref start_etx_ota function, then this callback function will not be invoked.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 24, 2023.
 */
void etx_ota_pre_transaction_handler();

/**@brief	ETX OTA Status Response Callback.
 *
 * @details	This function is called by the @ref app_side_etx_ota each time an ETX OTA Transaction has been completed, so
 *          that the implementer of @ref app_side_etx_ota can customize handling the resulting ETX OTA Status Exception
 *          Code.
 *
 * @param  resp  Resulting ETX OTA Status Exception Code of the ETX OTA Transaction that has just been completed, where
 *               the only possible values that can be given are the following:<br>
 *               - @ref ETX_OTA_Status::ETX_OTA_EC_OK    (ETX OTA Transactions continues in this case right before this callback function) In this case, some ETX OTA Custom Data has been received from the host.
 *               - @ref ETX_OTA_Status::ETX_OTA_EC_STOP  (ETX OTA Transactions are stopped in this case right before this callback function) In this case, the host has requested to stop a currently on-going ETX OTA Transaction.
 *               - @ref ETX_OTA_Status::ETX_OTA_EC_NR    (ETX OTA Transactions continues in this case right before this callback function) In this case, the host stopped responding to our MCU/MPU during an ETX OTA Transaction.
 *               - @ref ETX_OTA_Status::ETX_OTA_EC_NA    (ETX OTA Transactions are stopped in this case right before this callback function) In this case, an ETX OTA Firmware Update has been requested by the host (remember that this @ref app_side_etx_ota cannot install Firmware Images and, therefore, a reboot is suggested to make the Bootloader Firmware of our MCU/MPU to install that Image).
 *               - @ref ETX_OTA_Status::ETX_OTA_EC_ERR   (ETX OTA Transactions are stopped in this case right before this callback function) In this case, an Error has happened during an on-going ETX OTA Transaction.
 *
 * @note    Whenever the cases where ETX OTA Transactions are stopped give place, if it is desired to continue ETX OTA
 *          transactions after that, then the implementer must use the @ref start_etx_ota function on those cases inside
 *          the custom coded added into this @ref etx_ota_status_resp_handler function. The same should be made for the
 *          cases where the ETX OTA Transactions are continued instead, where the @ref stop_etx_ota function can be used
 *          to stop further transactions if required.
 * @note    Remember to call @ref HAL_NVIC_SystemReset whenever the \p  etx_ota_status_code param equals
 *          @ref ETX_OTA_Status::ETX_OTA_EC_NA to reset our MCU/MPU so that in can enter again into its Bootloader
 *          Firmware again in order to try again in receiving the given ETX OTA Firmware Update Request there, since
 *          only the Bootloader Firmware can install Firmware Images.
 * @note    Also, whenever receiving an ETX OTA Custom Data Request from a host, then that ETX OTA Request can be
 *          handled by overriding this @ref etx_ota_status_resp_handler function at the @ref main program.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 24, 2023.
 */
void etx_ota_status_resp_handler(ETX_OTA_Status resp);

#endif /* APP_SIDE_ETX_OTA_H_ */

/** @} */
