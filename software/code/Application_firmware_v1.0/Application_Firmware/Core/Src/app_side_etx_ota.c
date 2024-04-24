/** @addtogroup app_side_etx_ota
 * @{
 */

#include "app_side_etx_ota.h"
#include <stdio.h>	// Library from which "printf()" is located at.
#include <string.h>	// Library from which "memset()" is located at.

#define ETX_OTA_SOF  				(0xAA)    		/**< @brief Designated Start Of Frame (SOF) byte to indicate the start of an ETX OTA Packet. */
#define ETX_OTA_EOF  				(0xBB)    		/**< @brief Designated End Of Frame (EOF) byte to indicate the end of an ETX OTA Packet. */
#define ETX_OTA_SOF_SIZE			(1U)			/**< @brief	Designated SOF field size in bytes in a ETX OTA Packet. */
#define ETX_OTA_PACKET_TYPE_SIZE	(1U)			/**< @brief	Designated Packet Type field size in bytes in a ETX OTA Packet. */
#define ETX_OTA_DATA_LENGTH_SIZE	(2U)			/**< @brief	Designated Data Length field size in bytes in a ETX OTA Packet. */
#define ETX_OTA_DATA_MAX_SIZE 		(1024U)  		/**< @brief Designated maximum expected "Data" field's size in the General Data Format of an ETX OTA Packet. @note This definition's value does not stand for the size of the entire ETX OTA Packet. Instead, it represents the size of the "Data" field that is inside the General Data Format of an ETX OTA Packet. */
#define ETX_OTA_CRC32_SIZE			(4U)			/**< @brief	Designated 32-bit CRC field size in bytes in a ETX OTA Packet. */
#define ETX_OTA_EOF_SIZE			(1U)			/**< @brief	Designated EOF field size in bytes in a ETX OTA Packet. */
#define ETX_OTA_DATA_OVERHEAD 		(ETX_OTA_SOF_SIZE + ETX_OTA_PACKET_TYPE_SIZE + ETX_OTA_DATA_LENGTH_SIZE + ETX_OTA_CRC32_SIZE + ETX_OTA_EOF_SIZE)  	/**< @brief Data overhead in bytes of an ETX OTA Packet, which represents the bytes of an ETX OTA Packet except for the ones that it has at the Data field. */
#define ETX_OTA_PACKET_MAX_SIZE 	(ETX_OTA_DATA_MAX_SIZE + ETX_OTA_DATA_OVERHEAD)																		/**< @brief Total bytes in an ETX OTA Packet. */
#define ETX_OTA_DATA_FIELD_INDEX	(ETX_OTA_SOF_SIZE + ETX_OTA_PACKET_TYPE_SIZE + ETX_OTA_DATA_LENGTH_SIZE) 											/**< @brief Index position of where the Data field bytes of a ETX OTA Packet starts at. */
#define ETX_OTA_BL_FW_SIZE          (FLASH_PAGE_SIZE_IN_BYTES * ETX_BL_FLASH_PAGES_SIZE)   	/**< @brief Maximum size allowable for a Bootloader Firmware Image to have. */
#define ETX_OTA_APP_FW_SIZE         (FLASH_PAGE_SIZE_IN_BYTES * ETX_APP_FLASH_PAGES_SIZE)   /**< @brief Maximum size allowable for an Application Firmware Image to have. */

/**@brief	ETX OTA process states.
 *
 * @details	The ETX OTA process states are used in the functions of the @ref app_side_etx_ota module to either indicate
 * 			or identify in what part of the whole ETX OTA process is our MCU/MPU currently at.
 * @details	The ETX OTA process consists of several sub-processes or states that must be given place in the following
 *          orderly fashion:
 *			<ol>
 *				<li>Idle State</li>
 *				<li>Start State</li>
 *				<li>Header State</li>
 *				<li>Data State</li>
 *				<li>End State</li>
 *			</ol>
 *
 * @note	If the ETX OTA process gives place in a different order as shown above (e.g., Idle State --> Start State -->
 *          Data State ..., where instead, the order of the states expected were Idle State --> Start State --> Header
 *          State ...), then the ETX OTA process will be terminated with the corresponding @ref ETX_OTA_Status Exception
 *          code.
 */
typedef enum
{
	ETX_OTA_STATE_IDLE    = 0U,   	//!< ETX OTA Idle State. @details This state stands for when our MCU/MPU is not in a ETX OTA Firmware Update.
	ETX_OTA_STATE_START   = 1U,   	//!< ETX OTA Start State. @details This state gives place when our MCU/MPU receives a Command Type Packet from the host right after leaving the ETX OTA Idle State. Our MCU/MPU will expect that Packet to contain the Start Command so that our MCU/MPU starts an ETX OTA Firmware Update.
	ETX_OTA_STATE_HEADER  = 2U,   	//!< ETX OTA Header State. @details This state starts right after the Command Type Packet of the ETX OTA Start State is processed. In the ETX OTA Header State, our MCU/MPU will expect to receive a Header Type Packet in which our MCU/MPU will obtain the size in bytes of the Firmware Image to be received, its recorded 32-bits CRC and the sub-type of the ETX OTA Data Type Packets to be received (i.e., @ref ETX_OTA_Payload_t ).
	ETX_OTA_STATE_DATA    = 3U,   	//!< ETX OTA Data State. @details This state starts right after the Header Type Packet of the ETX OTA Header State is processed. In the ETX OTA Data State, our MCU/MPU will expect to receive one or more Data Type Packets from which our MCU/MPU will receive a Firmware Image from the host. It is also in this State where our MCU/MPU will apply that Firmware Image to itself.
	ETX_OTA_STATE_END     = 4U		//!< ETX OTA End State. @details This state starts right after the Data Type Packet(s) of the ETX OTA Data State is/are processed. In the ETX OTA End State, our MCU/MPU will expect to receive a Command Type Packet containing the End Command so that our MCU/MPU confirms the conclusion of the ETX OTA Firmware Update.
} ETX_OTA_State;

/**@brief	Packet Type definitions available in the ETX OTA Protocol.
 */
typedef enum
{
	ETX_OTA_PACKET_TYPE_CMD       = 0U,   	//!< ETX OTA Command Type Packet. @details This Packet Type is expected to be send by the host to our MCU/MPU to request a certain ETX OTA Command to our MCU/MPU (see @ref ETX_OTA_Command ).
	ETX_OTA_PACKET_TYPE_DATA      = 1U,   	//!< ETX OTA Data Type Packet. @details This Packet Type will contain either the full or a part/chunk of a Firmware Image being send from the host to our MCU/MPU.
	ETX_OTA_PACKET_TYPE_HEADER    = 2U,   	//!< ETX OTA Header Type Packet. @details This Packet Type will provide the size in bytes of the Firmware Image that our MCU/MPU will receive, its recorded 32-bits CRC and the sub-type of the ETX OTA Data Type Packets to be received (i.e., @ref ETX_OTA_Payload_t ).
	ETX_OTA_PACKET_TYPE_RESPONSE  = 3U		//!< ETX OTA Response Type Packet. @details This Packet Type contains a response from our MCU/MPU that is given to the host to indicate to it whether or not our MCU/MPU was able to successfully process the latest request or Packet from the host.
} ETX_OTA_Packet_t;

/**@brief	ETX OTA Commands definitions.
 *
 * @details	These are the different Commands that the host can request to our MCU/MPU whenever the host sends an ETX OTA
 *          Command Type Packet to that external device.
 */
typedef enum
{
	ETX_OTA_CMD_START = 0U,		    //!< ETX OTA Firmware Update Start Command. @details This command indicates to the MCU/MPU that the host is connected to (via @ref COMPORT_NUMBER ) to start an ETX OTA Process.
	ETX_OTA_CMD_END   = 1U,    		//!< ETX OTA Firmware Update End command. @details This command indicates to the MCU/MPU that the host is connected to (via @ref COMPORT_NUMBER ) to end the current ETX OTA Process.
	ETX_OTA_CMD_ABORT = 2U    		//!< ETX OTA Abort Command. @details This command is used by the host to request to our MCU/MPU to abort whatever ETX OTA Process that our MCU/MPU is working on. @note Unlike the other Commands, this one can be legally requested to our MCU/MPU at any time and as many times as the host wants to.
} ETX_OTA_Command;

/**@brief	Payload Type definitions available in the ETX OTA Firmware Update process.
 *
 * @details	Whenever the host sends to our MCU/MPU a @ref ETX_OTA_PACKET_TYPE_DATA Packet Type (i.e., a Data Type
 * 			Packet), there are several possible sub-types for this and any other Data Type Packet that indicate the type
 * 			of data that our MCU/MPU should expect to receive from the whole Data Type Packets that it will receive.
 * 			These sub-types are given by @ref ETX_OTA_Payload_t .
 */
typedef enum
{
    ETX_OTA_Application_Firmware_Image  = 0U,   	//!< ETX OTA Application Firmware Image Data Packet Type.
    ETX_OTA_Bootloader_Firmware_Image   = 1U,  		//!< ETX OTA Bootloader Firmware Image Data Packet Type.
    ETX_OTA_Custom_Data                 = 2U   		//!< ETX OTA Custom Data Packet Type.
} ETX_OTA_Payload_t;

/**@brief	Response Status definitions available in the ETX OTA Protocol.
 *
 * @details	Whenever the host sends to our MCU/MPU a Packet, depending on whether our MCU/MPU was able to successfully
 * 			process the data of that Packet of not, our MCU/MPU will respond back to the host with a Response Type
 * 			Packet containing the value of a Response Status correspondingly.
 */
typedef enum
{
	ETX_OTA_ACK  = 0U,   		//!< Acknowledge (ACK) data byte used in an ETX OTA Response Type Packet to indicate to the host that the latest ETX OTA Packet has been processed successfully by our MCU/MPU.
	ETX_OTA_NACK   = 1U   		//!< Not Acknowledge (NACK) data byte used in an ETX OTA Response Type Packet to indicate to the host that the latest ETX OTA Packet has not been processed successfully by our MCU/MPU.
} ETX_OTA_Response_Status;

/**@brief	Is ETX OTA Enabled Flag Status definitions available in the ETX OTA Protocol.
 *
 * @details	These definitions indicate the values with with it is possible to enabled or disable ETX OTA Transactions.
 */
typedef enum
{
    ETX_OTA_DISABLED  = 0U,   		//!< ETX OTA Transactions are disabled.
    ETX_OTA_ENABLED   = 1U   		//!< ETX OTA Transactions are enabled.
} is_ETX_OTA_enabled_flag_status;

static uint8_t Rx_Buffer[ETX_OTA_PACKET_MAX_SIZE];			                    /**< @brief Global buffer that will be used by our MCU/MPU to hold the whole data of a received ETX OTA Packet from the host. */
static ETX_OTA_State etx_ota_state = ETX_OTA_STATE_IDLE;	                    /**< @brief Global variable used to hold the ETX OTA Process State at which our MCU/MPU is currently at. */
static uint32_t etx_ota_fw_received_size = 0;				                    /**< @brief Global variable used to indicate the Total Size in bytes of the whole ETX OTA Payload that our MCU/MPU has received and written into the Flash Memory designated to the ETX OTA Protocol. */
static is_ETX_OTA_enabled_flag_status is_etx_ota_enabled = ETX_OTA_DISABLED;    /**< @brief Global Flag used enable or disable ETX OTA Transactions. */
static firmware_update_config_data_t *p_fw_config;			                    /**< @brief Global pointer to the latest data of the @ref firmware_update_config sub-module. */
static etx_ota_custom_data_t *p_custom_data;                                    /**< @brief Global pointer to the handling struct of a received ETX OTA Custom Data. */
static UART_HandleTypeDef *p_huart;							                    /**< @brief Our MCU/MPU's Hardware Protocol UART Handle from which the ETX OTA Protocol will be used on. */
static ETX_OTA_hw_Protocol ETX_OTA_hardware_protocol;                           /**< @brief Hardware Protocol into which the ETX OTA Protocol will be used for sending/receiving data to/from the host. */
static HM10_GPIO_def_t *p_GPIO_is_hm10_default_settings = NULL;                 /**< @brief Pointer to the GPIO Definition Type of the GPIO Pin from which it can be requested to reset the Configuration Settings of the HM-10 BT Device to its default settings. @details This Input Mode GPIO will be used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. @note The following are the possible values of the GPIO Pin designated here:<br><br>* 0 (i.e., Low State) = Do not reset/change the configuration settings of the HM-10 BT Device.<br>* 1 (i.e., High State) = User requests to reset the configuration settings of the HM-10 BT Device to its default settings. */

/**@brief	ETX OTA Command Type Packet's parameters structure.
 *
 * @details	This structure contains all the fields of an ETX OTA Packet of @ref ETX_OTA_PACKET_TYPE_CMD Type.
 * @details	The following illustrates the ETX OTA format of this Type of Packets:
 *
 * <table style="border-collapse:collapse;border-spacing:0;background-color:#ffffff;text-align:center;vertical-align:top;">
 * <thead>
 *   <tr>
 *     <th style="border-color:black;border-style:solid;border-width:1px">SOF</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Packet<br>Type</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Len</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Command</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">CRC</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">EOF</th>
 *   </tr>
 * </thead>
 * <tbody>
 *   <tr>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">2B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">4B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *   </tr>
 * </tbody>
 * </table>
 *
 * where B = byte(s).
 */
typedef struct __attribute__ ((__packed__)) {
	uint8_t   sof;				//!< Start of Frame (SOF). @details All ETX OTA Packets must start with a SOF byte, whose value is @ref ETX_OTA_SOF .
	uint8_t   packet_type;		//!< Packet Type. @details The value of this parameter has to be @ref ETX_OTA_PACKET_TYPE_CMD because this is meant to be used for Command Type Packets.
	uint16_t  data_len;			//!< Length of the Command value in bytes. @note The length of any @ref ETX_OTA_Command value is 1 byte.
	uint8_t   cmd;				//!< Command value. @note See @ref ETX_OTA_Command for more details.
	uint32_t  crc;				//!< 32-bit CRC of the \c cmd parameter of this structure.
	uint8_t   eof;				//!< Start of Frame (EOF). @details All ETX OTA Packets must end with an EOF byte, whose value is given by @ref ETX_OTA_EOF .
} ETX_OTA_Command_Packet_t;

/**@brief	Header Data parameters structure.
 *
 * @details	This structure contains all the fields of the Header data that is expected to be received by our MCU/MPU
 * 			in a @ref ETX_OTA_PACKET_TYPE_HEADER Type Packet (i.e., in a Header Type Packet).
 */
typedef struct __attribute__ ((__packed__)) {
	uint32_t 	package_size;		//!< Total length/size in bytes of the data expected to be received by our MCU/MPU from the host via all the @ref ETX_OTA_PACKET_TYPE_DATA Type Packet(s) (i.e., in a Data Type Packet or Packets) to be received.
	uint32_t 	package_crc;		//!< 32-bit CRC of the whole data to be obtained from all the @ref ETX_OTA_PACKET_TYPE_DATA Type Packets (i.e., in a Data Type Packets).
	uint32_t 	reserved1;			//!< 32-bits reserved for future changes on this firmware.
	uint16_t 	reserved2;			//!< 16-bits reserved for future changes on this firmware.
	uint8_t 	reserved3;			//!< 8-bits reserved for future changes on this firmware.
	uint8_t		payload_type;	    //!< Expected payload type to be received whenever receiving the @ref ETX_OTA_PACKET_TYPE_DATA Type Packets (i.e., in a Data Type Packets). @note see @ref ETX_OTA_Payload_t to learn about the available Payload Types.
} header_data_t;

/**@brief	ETX OTA Header Type Packet's parameters structure.
 *
 * @details	This structure contains all the fields of an ETX OTA Packet of @ref ETX_OTA_STATE_HEADER Type.
 * @details	The following illustrates the ETX OTA format of this Type of Packets:
 *
 * <table style="border-collapse:collapse;border-spacing:0;background-color:#ffffff;text-align:center;vertical-align:top;">
 * <thead>
 *   <tr>
 *     <th style="border-color:black;border-style:solid;border-width:1px">SOF</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Packet<br>Type</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Len</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Header<br>Data</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">CRC</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">EOF</th>
 *   </tr>
 * </thead>
 * <tbody>
 *   <tr>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">2B</td>
 *     <td style="border-color:#ffffff">16B</td>
 *     <td style="border-color:#ffffff">4B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *   </tr>
 * </tbody>
 * </table>
 *
 * where B = byte(s).
 */
typedef struct __attribute__ ((__packed__)) {
	uint8_t     	sof;			//!< Start of Frame (SOF). @details All ETX OTA Packets must start with a SOF byte, whose value is @ref ETX_OTA_SOF .
	uint8_t     	packet_type;	//!< Packet Type. @details The value of this parameter has to be @ref ETX_OTA_PACKET_TYPE_HEADER because this is meant to be used for Header Type Packets.
	uint16_t    	data_len;		//!< Length of the \c meta_data parameter in bytes. @note The length of the \c meta_data parameter value must be 16 bytes.
	header_data_t	meta_data;		//!< Header data. @note See @ref header_data_t for more details.
	uint32_t    	crc;			//!< 32-bit CRC of the \c meta_data parameter of this structure.
	uint8_t     	eof;			//!< Start of Frame (EOF). @details All ETX OTA Packets must end with an EOF byte, whose value is given by @ref ETX_OTA_EOF .
} ETX_OTA_Header_Packet_t;

/**@brief	ETX OTA Data Type Packet's parameters structure.
 *
 * @details	This structure contains all the fields of an ETX OTA Packet of @ref ETX_OTA_STATE_DATA Type.
 * @details	The following illustrates the ETX OTA format of this Type of Packets:
 *
 * <table style="border-collapse:collapse;border-spacing:0;background-color:#ffffff;text-align:center;vertical-align:top;">
 * <thead>
 *   <tr>
 *     <th style="border-color:black;border-style:solid;border-width:1px">SOF</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Packet<br>Type</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Len</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Payload Data</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">CRC</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">EOF</th>
 *   </tr>
 * </thead>
 * <tbody>
 *   <tr>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">2B</td>
 *     <td style="border-color:#ffffff">@ref data_len B</td>
 *     <td style="border-color:#ffffff">4B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *   </tr>
 * </tbody>
 * </table>
 *
 * where B = byte(s) and where the value of the "Payload Data" field must be perfectly divisible by 4 bytes.
 *
 * @note	The CRC is not within the fields of this structure because this structure was formulated in such a way that
 *          it could represent any possible ETX OTA Data Type Packet in consideration that they may vary in size and
 *          because it is desired to make it compatible with the General Data Format explained in @ref app_side_etx_ota
 *          . Therefore, the strategy that has been applied in the @ref app_side_etx_ota module is to append all the
 *          data contained in the @ref ETX_OTA_Data_Packet_t::data field from all the received ETX OTA Data Type Packets
 *          within a single ETX OTA Process to then calculate the CRC of that whole data and validate it with the one
 *          received for that whole data via the @ref header_data_t::package_crc field.
 * @note	Another consequence of what was just explained is that the EOF is also not within the fields of this
 *          structure but it is always validated at the end of each ETX OTA Data Type Packet received.
 */
typedef struct __attribute__ ((__packed__)) {
	uint8_t     sof;			//!< Start of Frame (SOF). @details All ETX OTA Packets must start with a SOF byte, whose value is @ref ETX_OTA_SOF .
	uint8_t     packet_type;	//!< Packet Type. @details The value of this parameter must be @ref ETX_OTA_PACKET_TYPE_DATA because this is meant to be used for Data Type Packets.
	uint16_t    data_len;		//!< Length of the actual "Data" field contained inside the \c data parameter in bytes (i.e., This does not take into account the "CRC32" and "EOF" fields length). @note This length is defined by the host, but its maximum value can be @ref ETX_OTA_DATA_MAX_SIZE (i.e., 1024 bytes).
	uint8_t     *data;			//!< Payload data, 32-bit CRC and EOF. @details the Data bytes may have a length of 1 byte up to a maximum of @ref ETX_OTA_DATA_MAX_SIZE (i.e., 1024 bytes). Next to them, the 32-bit CRC will be appended and then the EOF byte will be appended subsequently. @details The appended 32-bit CRC is calculated with respect to the Data bytes received in for this single/particular ETX OTA Data Type Packet (i.e., Not for the whole data previously explained).
} ETX_OTA_Data_Packet_t;

/**@brief	ETX OTA Response Type Packet's parameters structure.
 *
 * @details	This structure contains all the fields of an ETX OTA Packet of @ref ETX_OTA_PACKET_TYPE_RESPONSE Type.
 * @details	The following illustrates the ETX OTA format of this Type of Packets:
 *
 * <table style="border-collapse:collapse;border-spacing:0;background-color:#ffffff;text-align:center;vertical-align:top;">
 * <thead>
 *   <tr>
 *     <th style="border-color:black;border-style:solid;border-width:1px">SOF</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Packet<br>Type</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Len</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">Status</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">CRC</th>
 *     <th style="border-color:black;border-style:solid;border-width:1px">EOF</th>
 *   </tr>
 * </thead>
 * <tbody>
 *   <tr>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">2B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *     <td style="border-color:#ffffff">4B</td>
 *     <td style="border-color:#ffffff">1B</td>
 *   </tr>
 * </tbody>
 * </table>
 *
 * where B = byte(s).
 */
typedef struct __attribute__ ((__packed__)) {
	uint8_t   sof;				//!< Start of Frame (SOF). @details All ETX OTA Packets must start with a SOF byte, whose value is @ref ETX_OTA_SOF .
	uint8_t   packet_type;		//!< Packet Type. @details The value of this parameter must be @ref ETX_OTA_PACKET_TYPE_RESPONSE because this is meant to be used for Response Type Packets.
	uint16_t  data_len;			//!< Length of the \c status parameter in bytes. @note The length of any valid value for the \c status parameter is 1 byte.
	uint8_t   status;			//!< Response Status value. @note See @ref ETX_OTA_Response_Status for more details.
	uint32_t  crc;				//!< 32-bit CRC of the \c status parameter of this structure.
	uint8_t   eof;				//!< Start of Frame (EOF). @details All ETX OTA Packets must end with an EOF byte, whose value is given by @ref ETX_OTA_EOF .
} ETX_OTA_Response_Packet_t;

/**@brief   Starts an ETX OTA Transaction with the host machine.
 *
 * @note    This function should only be called right after an @ref ETX_OTA_SOF byte is received.
 *
 * @details Waits to receive as many ETX OTA Packets are required to conclude the started ETX OTA Transaction and also
 *          processes those Packets correspondingly.
 *
 * @note    For more details on how the ETX OTA Protocol works, see the @ref app_side_etx_ota module.
 *
 * @retval  ETX_OTA_EC_OK
 * @retval	ETX_OTA_EC_STOP
 * @retval	ETX_OTA_EC_NR
 * @retval	ETX_OTA_EC_NA
 * @retval	ETX_OTA_EC_ERR
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date    November 25, 2023.
 */
static ETX_OTA_Status start_etx_ota_transaction();

/**
 * @brief   Gets one Packet from the ETX OTA process, if any is given.
 *
 * @details The General Data Format that is expected to be received is as explained in the Doxygen documentation of the
 *          @ref ETX_OTA_Data_Packet_t parameters structure, which starts with a SOF byte that is followed up with the
 *          Packet Type field, the Data Length field, the Data field, the CRC32 field and ends up with an EOF byte.
 *
 * @note	The HAL Timeout for each reception of data is given by @ref ETX_CUSTOM_HAL_TIMEOUT .
 *
 * @param[in, out] buf 		Buffer pointer into which the bytes of the received packet from the host, will be stored.
 *
 * @retval					ETX_OTA_EC_OK
 * @retval					ETX_OTA_EC_NR
 * @retval					ETX_OTA_EC_ERR
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date November 24, 2023.
 */
static ETX_OTA_Status etx_ota_receive_packet(uint8_t *buf);

/**@brief	Processes and validates the latest received ETX OTA Packet.
 *
 * @details	This function will read the current value of the @ref etx_ota_state global variable to determine at which
 *          ETX OTA State the process is currently at in order to process the latest ETX OTA Packet correspondingly. In
 *          addition, this function will modify the value of that variable to the next ETX OTA State whenever it
 *          corresponds.
 *
 * @details In addition, this function will do the following depending on the current ETX OTA State:
 *          <ul>
 *              <li><b>ETX OTA Idle State:</b> Do nothing.</li>
 *              <li><b>ETX OTA Start State:</b> Validate the latest ETX OTA Packet to be that of an ETX OTA Command Type
 *                                              Packet containing the Start Command in it and, if successful, then
 *                                              change current ETX OTA State to ETX OTA Header State.</li>
 *              <li><b>ETX OTA Header State:</b> Validate the latest ETX OTA Packet to be that of an ETX OTA Header
 *                                               Type Packet and, if successful, then in the case of receiving a Firmware
 *                                               Image, then terminate function with an @ref
 *                                               ETX_OTA_Status::ETX_OTA_EC_NA status code. Otherwise, validate that the
 *                                               incoming data corresponds to that of an ETX OTA Custom Data. If
 *                                               everything goes well up to this point, then change the current ETX OTA
 *                                               State to ETX OTA Data State.</li>
 *              <li><b>ETX OTA Data State:</b> Validate the latest ETX OTA Packet to be that of an ETX OTA Data Type
 *                                             Packet and, if successful, then write the contents of the "Data" field
 *                                             of that Packet into the designated RAM Memory location address of
 *                                             our MCU/MPU's, which is towards the data that the @ref p_custom_data
 *                                             pointer points to. Repeat this explained process for all the ETX OTA Data
 *                                             Type Packets to be received by our MCU/MPU. After the last ETX OTA Data
 *                                             Packet has been successfully, change the current ETX OTA State to ETX OTA
 *                                             End State.</li>
 *              <li><b>ETX OTA End State:</b> Validate the latest ETX OTA Packet to be that of an ETX OTA Command Type
 *                                            Packet containing the End Command in it and, if successful, then validate
 *                                            the CRC of the whole ETX OTA Custom Data that has just been received. If
 *                                            this is successful, then change the current ETX OTA State to ETX OTA Idle
 *                                            State.</li>
 *          </ul>
 *
 * @note	Each time the implementer/programmer calls this function, the @ref etx_ota_receive_packet function must to
 * 			be called once before this @ref etx_ota_process_data function.
 *
 * @param[in] buf	Buffer pointer to the data of the latest ETX OTA Packet.
 *
 * @retval						ETX_OTA_EC_OK
 * @retval						ETX_OTA_EC_STOP
 * @retval						ETX_OTA_EC_NR
 * @retval						ETX_OTA_EC_NA
 * @retval						ETX_OTA_EC_ERR
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date November 24, 2023.
 */
static ETX_OTA_Status etx_ota_process_data(uint8_t *buf);

/**@brief	Sends an ETX OTA Response Type Packet with a desired Response Status (i.e., ACK or NACK) to the host either
 *          via the UART or the BT Hardware Protocol correspondingly.
 *
 * @note    This function decides on sending the data on a certain Hardware Protocol according to the current value of
 *          @ref ETX_OTA_hardware_protocol , which should be set only via the @ref init_firmware_update_module function.
 *
 * @param response_status	\c ETX_OTA_ACK or \c ETX_OTA_NACK .
 *
 * @retval  ETX_OTA_EC_OK
 * @retval 	ETX_OTA_EC_NR
 * @retval 	ETX_OTA_EC_ERR
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @author 	EmbeTronicX (<a href=https://github.com/Embetronicx/STM32-Bootloader/tree/ETX_Bootloader_3.0>STM32-Bootloader GitHub Repository under ETX_Bootloader_3.0 branch</a>)
 * @date October 30, 2023.
 */
static ETX_OTA_Status etx_ota_send_resp(ETX_OTA_Response_Status response_status);

/**@brief	Write the contents of the "Data" field contained in a given ETX OTA Data Type Packet into the RAM Memory
 *          of our MCU/MPU's Application Firmware.
 *
 * @details The place where this function will store the received data from the ETX OTA Data Type Packet into the RAM
 *          Memory will be the \c data field of the @ref p_custom_data Global Pointer.
 *
 * @param[in] data			Pointer to the "Data" field of a given ETX OTA Data Type Packet, whose data wants to be
 *                          written into the RAM Memory of our MCU/MPU's Application Firmware.
 * @param data_len			Length in bytes of the "Data" field of the ETX Data Type Packet that is being pointed
 *                          towards to, via the \p data param.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date November 24, 2023.
 */
static void write_data_to_ram(uint8_t *data, uint16_t data_len);

/**@brief	Gets the corresponding @ref ETX_OTA_Status value depending on the given @ref HAL_StatusTypeDef value.
 *
 * @param HAL_status	HAL Status value (see @ref HAL_StatusTypeDef ) that wants to be converted into its equivalent
 * 						of a @ref ETX_OTA_Status value.
 *
 * @retval				ETX_OTA_EC_NR if \p HAL_status param equals \c HAL_BUSY or \c ETX_CUSTOM_HAL_TIMEOUT .
 * @retval				ETX_OTA_EC_ERR if \p HAL_status param equals \c HAL_ERROR .
 * @retval				HAL_status param otherwise.
 *
 * @note	For more details on the returned values listed, see @ref ETX_OTA_Status and @ref HAL_StatusTypeDef .
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date September 26, 2023.
 */
static ETX_OTA_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status);

ETX_OTA_Status init_firmware_update_module(ETX_OTA_hw_Protocol hardware_protocol,
                                           UART_HandleTypeDef *huart,
                                           firmware_update_config_data_t *fw_config,
										   HM10_GPIO_def_t *GPIO_is_hm10_default_settings,
                                           etx_ota_custom_data_t *etx_ota_custom_data)
{
    /** <b>Local variable ret:</b> Used to hold the exception code value returned by either a @ref FirmUpdConf_Status , a @ref ETX_OTA_Status, a @ref HM10_BT_Connection_Status or a @ref HM10_Status function type. */
    uint16_t ret;
    #if ETX_OTA_VERBOSE
        printf("Initializing the Firmware Update Module...\r\n");
    #endif

    /* Persist the requested hardware protocol into which the ETX OTA Protocol will be used on. */
    ETX_OTA_hardware_protocol = hardware_protocol;

    /* Persist the pointer to the UART into which the ETX OTA Protocol will be used on. */
    p_huart = huart;

    /* Persist the pointer to the Firmware Update Configurations sub-module to the one that was given. */
    p_fw_config = fw_config;

    /* Persist the pointer to the GPIO Definition Type of the GPIO Pin from which it can be requested to reset the Configuration Settings of the HM-10 BT Device to its default settings. */
    p_GPIO_is_hm10_default_settings = GPIO_is_hm10_default_settings;

    /* Persist the pointer to the handling struct of an ETX OTA Custom Data. */
    p_custom_data = etx_ota_custom_data;

    /* Clear SOF bit from ETX OTA Buffer. */
    Rx_Buffer[0] = 0;

    /* Validate the requested hardware protocol to be used and, if required, initialized it. */
    switch (hardware_protocol)
    {
        case ETX_OTA_hw_Protocol_UART:
            #if ETX_OTA_VERBOSE
                printf("The UART Hardware Protocol has been selected by the Firmware Update Module.\r\n");
            #endif
            break;
        case ETX_OTA_hw_Protocol_BT:
            #if ETX_OTA_VERBOSE
                printf("The BT Hardware Protocol has been selected by the Firmware Update Module.\r\n");
            #endif

            /* Initializing the HM-10 Bluetooth module. */
            #if ETX_OTA_VERBOSE
                printf("Initializing the HM-10 Bluetooth module...\r\n");
            #endif
            init_hm10_module(p_huart);

            /* Sending test command to HM-10 BT Device to make sure that it is not in a Bluetooth Connection for the next steps to be made and to make sure that it is currently in working condition. */
            ret = disconnect_hm10_from_bt_address();
            if (ret == HM10_BT_Connection_Status_Unknown)
            {
				#if ETX_OTA_VERBOSE
					printf("ERROR: Something went wrong whenever sending the initial Test Command sent to the HM-10 BT Device(Bluetooth Connection Status Code returned = %d).\r\n", ret);
				#endif
                return ETX_OTA_EC_ERR;
            }

            /* Resetting the Configuration Settings of the HM-10 BT Device to its Default Settings, but only if user is requesting it. */
            if (HAL_GPIO_ReadPin(p_GPIO_is_hm10_default_settings->GPIO_Port, p_GPIO_is_hm10_default_settings->GPIO_Pin) == GPIO_PIN_SET)
            {
				#if ETX_OTA_VERBOSE
					printf("MCU has been requested to reset the configuration settings of the HM-10 BT Device to its default settings.\r\n");
					printf("Resetting configuration settings of the HM-10 BT Device...\r\n");
				#endif

				/* Restore all the HM-10 Setup values to factory setup. */
				ret = send_hm10_renew_cmd();
				if (ret != HM10_EC_OK)
				{
					#if ETX_OTA_VERBOSE
						printf("ERROR: The HM-10 BT device could not be restored to its factory setup via the AT+RENEW Command (Exception Code = %d).\r\n", ret);
					#endif
					return ETX_OTA_EC_ERR;
				}

                /* Setting BT Name in HM-10 BT Device. */
                /** <b>Local variable default_ble_name:</b> Used to hold the Default BT Name of the HM-10 BT Device as given by @ref HM10_DEFAULT_BLE_NAME . */
                uint8_t default_ble_name[] = {HM10_DEFAULT_BLE_NAME};
                ret = set_hm10_name(default_ble_name, sizeof(default_ble_name));
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: The BT Name of the HM-10 BT device could not be set to its default value (Exception Code = %d).\r\n", ret);
					#endif
                    return ETX_OTA_EC_ERR;
                }

                /* Setting Role in BT Device. */
                ret = set_hm10_role(HM10_DEFAULT_ROLE);
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: The Role of the HM-10 BT device could not be set to its default value (Exception Code = %d).\r\n", ret);
					#endif
                    return ETX_OTA_EC_ERR;
                }

                /* Setting a Pin in the BT Device. */
                /** <b>Local variable default_pin_code:</b> Used to hold the Default Pin Code of the HM-10 BT Device as given by @ref HM10_DEFAULT_PIN . */
                uint8_t default_pin_code[HM10_PIN_VALUE_SIZE] = {HM10_DEFAULT_PIN};
                ret = set_hm10_pin(default_pin_code);
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: The Pin of the HM-10 BT device could not be set to its default value (Exception Code = %d).\r\n", ret);
					#endif
                    return ETX_OTA_EC_ERR;
                }

                /* Setting a Pin Code Mode in the BT Device. */
                ret = set_hm10_pin_code_mode(HM10_DEFAULT_PIN_CODE_MODE);
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: The Pin Code Mode of the HM-10 BT device could not be set to its default value (Exception Code = %d).\r\n", ret);
					#endif
                    return ETX_OTA_EC_ERR;
                }

                /* Setting the Module Work Mode in the BT Device. */
                ret = set_hm10_module_work_mode(HM10_DEFAULT_MODULE_WORK_MODE);
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: The Module Work Mode of the HM-10 BT device could not be set to its default value (Exception Code = %d).\r\n", ret);
					#endif
					return ETX_OTA_EC_ERR;
				}

                /* Resetting the BT Device. */
                ret = send_hm10_reset_cmd();
                if (ret != HM10_EC_OK)
                {
					#if ETX_OTA_VERBOSE
						printf("ERROR: Could not reset the HM-10 BT device (Exception Code = %d).\r\n", ret);
					#endif
                    return ETX_OTA_EC_ERR;
                }
				#if ETX_OTA_VERBOSE
					printf("The reset of the configuration settings of the HM-10 BT Device has been successfully completed.\r\n");
				#endif
            }
			#if ETX_OTA_VERBOSE
				printf("Initialization of the HM-10 Bluetooth module has been completed successfully.\r\n");
			#endif
            break;
        default:
            #if ETX_OTA_VERBOSE
                printf("ERROR: The requested Hardware Protocol %d is not recognized by the ETX OTA Protocol.\r\n", hardware_protocol);
            #endif
            return ETX_OTA_EC_ERR;
    }

    /* Start ETX OTA data reception in UART's non blocking mode. */
    start_etx_ota();

    return ETX_OTA_EC_OK;
}

void start_etx_ota()
{
	is_etx_ota_enabled = ETX_OTA_ENABLED;
	HAL_UART_Receive_IT(p_huart, Rx_Buffer, ETX_OTA_SOF_SIZE);
}

void stop_etx_ota()
{
	is_etx_ota_enabled = ETX_OTA_DISABLED;
}

/**@brief   Actions that are desired to be made with the ETX OTA Protocol whenever the non blocking mode, of the chosen
 *          Hardware Protocol, receives some data.
 *
 * @note    See the @ref init_firmware_update_module function to learn more details about the non blocking mode used in
 *          the @ref app_side_etx_ota .
 *
 * @details Regardless of the Hardware Protocol chosen, this function will indefinitely request to receive a byte of
 *          data from the chosen Hardware Protocol until it receives the @ref ETX_OTA_SOF byte. After that, it will call
 *          the @ref start_etx_ota_transaction function in order to receive the ETX OTA Transaction that should have
 *          started to give place by then.
 *
 * @param[in] huart	Pointer to the UART struct from which the current Receive Callback has been called from.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    November 25, 2023.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (is_etx_ota_enabled)
	{
		switch (ETX_OTA_hardware_protocol)
		{
			case ETX_OTA_hw_Protocol_UART:
			case ETX_OTA_hw_Protocol_BT:
				/* If the current byte received is an ETX OTA SOF byte, then enter into an ETX OTA Transaction Mode. Otherwise, wait for an ETX OTA SOF byte. */
				if (Rx_Buffer[0] == ETX_OTA_SOF)
				{
					etx_ota_pre_transaction_handler();
					etx_ota_status_resp_handler(start_etx_ota_transaction());
				}
				else
				{
					#if ETX_OTA_VERBOSE
						printf("Warning: Expected to receive the SOF field value from the first byte of an ETX OTA Transaction.\r\n");
					#endif
					HAL_UART_Receive_IT(p_huart, Rx_Buffer, ETX_OTA_SOF_SIZE); // Request to receive UART data in non blocking mode.
				}
				break;
			default:
				/* This should not happen since it should have been previously validated. */
				#if ETX_OTA_VERBOSE
					printf("ERROR: Expected a Hardware Protocol value, but received something else: %d.\r\n", ETX_OTA_hardware_protocol);
				#endif
				etx_ota_status_resp_handler(ETX_OTA_EC_ERR);
		}
	}
}

static ETX_OTA_Status start_etx_ota_transaction()
{
	/** <b>Local variable ret:</b> Used to hold the exception code value returned by a @ref FirmUpdConf_Status or a @ref ETX_OTA_Status function type. */
	ETX_OTA_Status ret;

	/* Reset the global variables related to: 1) The Header data of a received Firmware Image and 2) The ETX OTA Process State. */
	etx_ota_fw_received_size = 0U;
	etx_ota_state            = ETX_OTA_STATE_START;

	/* Attempt to receive an ETX OTA Request from the host and, if applicable, install it. */
	do
	{
		#if ETX_OTA_VERBOSE
			if (Rx_Buffer[0] == 0)
			{
				printf("Waiting for an ETX OTA Packet from the host...\r\n");
			}
		#endif
		ret = etx_ota_receive_packet(Rx_Buffer);
		switch (ret)
		{
		  case ETX_OTA_EC_OK:
			/* Since the ETX OTA Packet was received successfully, proceed into processing that data correspondingly. */
			ret = etx_ota_process_data(Rx_Buffer);
			Rx_Buffer[0] = 0; // Clearing SOF bit.
			switch (ret)
			{
			  case ETX_OTA_EC_OK:
				  #if ETX_OTA_VERBOSE
					  printf("DONE: The current ETX OTA Packet was processed successfully. Therefore, sending ACK...\r\n");
				  #endif
				  etx_ota_send_resp(ETX_OTA_ACK);
				  break;
			  case ETX_OTA_EC_STOP:
				  #if ETX_OTA_VERBOSE
					  printf("DONE: The ETX OTA process has been requested to be stopped by the host. Therefore, sending ACK...\r\n");
				  #endif
				  etx_ota_send_resp(ETX_OTA_ACK);
				  return ETX_OTA_EC_STOP;
			  case ETX_OTA_EC_NR:
				  #if ETX_OTA_VERBOSE
					  printf("ERROR: Our MCU/MPU's HAL responded with a No Response ETX OTA Exception code during a part of the process where this was not expected.\r\n");
				  #endif
				  return ETX_OTA_EC_ERR;
			  case ETX_OTA_EC_NA:
				  #if ETX_OTA_VERBOSE
					  printf("WARNING: The host has requested to start a Firmware Image Update. Therefore, sending NACK...\r\n");
				  #endif
				  etx_ota_send_resp(ETX_OTA_NACK);
				  return ETX_OTA_EC_NA;
			  case ETX_OTA_EC_ERR:
				  #if ETX_OTA_VERBOSE
					  printf("ERROR: An Error Exception Code has been generated during the ETX OTA process. Therefore, sending NACK...\r\n");
				  #endif
				  etx_ota_send_resp(ETX_OTA_NACK);
				  return ETX_OTA_EC_ERR;
			  default:
				  /* This should not happen. */
				  #if ETX_OTA_VERBOSE
					  printf("ERROR: The ETX OTA Exception code %d that has been generated is unrecognized by our MCU/MPU. Therefore, sending NACK...\r\n", ret);
				  #endif
				  return ret;
			}
			break;

		  case ETX_OTA_EC_NR:
			  #if ETX_OTA_VERBOSE
				  printf("DONE: No response from host.\r\n");
			  #endif
			  HAL_UART_Receive_IT(p_huart, Rx_Buffer, ETX_OTA_SOF_SIZE); // Request to receive UART data in non blocking mode.
			  return ETX_OTA_EC_NR;

		  case ETX_OTA_EC_ERR:
			  #if ETX_OTA_VERBOSE
				  printf("ERROR: An Error Exception Code has been generated during the ETX OTA process. Therefore, sending NACK...\r\n");
			  #endif
			  etx_ota_send_resp(ETX_OTA_NACK);
			  return ETX_OTA_EC_ERR;

		  default:
			  /* The "default" case should not be called. */
			  #if ETX_OTA_VERBOSE
				  printf("ERROR: The ETX OTA Exception code %d that has been generated either should not have been generated or is unrecognized by our MCU/MPU. Therefore, sending NACK...\r\n", ret);
			  #endif
			  etx_ota_send_resp(ETX_OTA_NACK);
			  return ret;
		}
	}
	while (etx_ota_state != ETX_OTA_STATE_IDLE);

	#if ETX_OTA_VERBOSE
		printf("DONE: The current whole ETX OTA Transaction has concluded successfully.\r\n");
	#endif
	HAL_UART_Receive_IT(p_huart, Rx_Buffer, ETX_OTA_SOF_SIZE); // Request to receive UART data in non blocking mode.
	return ETX_OTA_EC_OK;
}

static ETX_OTA_Status etx_ota_receive_packet(uint8_t *buf)
{
	/** <b>Local variable ret:</b> Used to hold the exception code value returned by either a @ref FirmUpdConf_Status , a @ref ETX_OTA_Status or a @ref HM10_Status function type. */
	int16_t  ret;
	/** <b>Local variable len:</b> Current index against which the bytes of the current ETX OTA Packet have been fetched. */
	uint16_t len = 0;
	/** <b>Local variable data_len:</b> "Data Length" field value of the ETX OTA Packet that is currently being received via the chosen Hardware Protocol. @details	The value of this field should stand for the length in bytes of the current ETX OTA Packet's "Data" field. */
	uint16_t data_len;
	/** <b>Local variable cal_data_crc:</b> Our MCU/MPU's calculated CRC of the ETX OTA Packet received via the chosen Hardware Protocol. */
	uint32_t cal_data_crc;
	/** <b>Local variable rec_data_crc:</b> Value holder of the "Recorded CRC" contained in the ETX OTA Packet received via the chosen Hardware Protocol. */
	uint32_t rec_data_crc;

	#if ETX_OTA_VERBOSE
		if (Rx_Buffer[0] == 0)
		{
			printf("Waiting to receive an ETX OTA Packet from the host...\r\n");
		}
	#endif
	switch (ETX_OTA_hardware_protocol)
	{
		case ETX_OTA_hw_Protocol_UART:
			/* Wait to receive the first byte of data from the host and validate it to be the SOF byte of an ETX OTA Packet. */
			if (Rx_Buffer[0] == 0)
            {
                ret = HAL_UART_Receive(p_huart, &buf[len], ETX_OTA_SOF_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
                ret = HAL_ret_handler(ret);
                if (ret != HAL_OK)
                {
                    return ret;
                }
                if (buf[len] != ETX_OTA_SOF)
                {
                    #if ETX_OTA_VERBOSE
                        printf("ERROR: Expected to receive the SOF field value from the current ETX OTA Packet.\r\n");
                    #endif
                    return ETX_OTA_EC_ERR;
                }
            }
			len++;

			/* Wait to receive the next 1-byte of data from the host and validate it to be a "Packet Type" field value of an ETX OTA Packet. */
			ret = HAL_UART_Receive(p_huart, &buf[len], ETX_OTA_PACKET_TYPE_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			ret = HAL_ret_handler(ret);
			if (ret != HAL_OK)
			{
				return ret;
			}
			switch (buf[len++])
			{
				case ETX_OTA_PACKET_TYPE_CMD:
				case ETX_OTA_PACKET_TYPE_DATA:
				case ETX_OTA_PACKET_TYPE_HEADER:
				case ETX_OTA_PACKET_TYPE_RESPONSE:
					break;
				default:
					#if ETX_OTA_VERBOSE
						printf("ERROR: The data received from the Packet Type field of the currently received ETX OTA Packet contains a value not recognized by our MCU/MPU.\r\n");
					#endif
					return ETX_OTA_EC_ERR;
			}

			/* Wait to receive the next 2-bytes of data from the host, which our MCU/MPU will interpret as the "Data Length" field value of an ETX OTA Packet. */
			ret = HAL_UART_Receive(p_huart, &buf[len], ETX_OTA_DATA_LENGTH_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			ret = HAL_ret_handler(ret);
			if (ret != HAL_OK)
			{
				return ret;
			}
			data_len = *(uint16_t *)&buf[len];
			len += ETX_OTA_DATA_LENGTH_SIZE;

			/* Wait to receive the next \c data_len bytes of data from the host, which our MCU/MPU will interpret as the "Data" field value of an ETX OTA Packet. */
			for (uint16_t i=0; i<data_len; i++)
			{
				ret = HAL_UART_Receive(p_huart, &buf[len++], 1, ETX_CUSTOM_HAL_TIMEOUT);
				ret = HAL_ret_handler(ret);
				if (ret != HAL_OK)
				{
					return ret;
				}
			}

			/* Wait to receive the next 4-bytes of data from the host, which our MCU/MPU will interpret as the "CRC32" field value of an ETX OTA Packet. */
			ret = HAL_UART_Receive(p_huart, &buf[len], ETX_OTA_CRC32_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			ret = HAL_ret_handler(ret);
			if (ret != HAL_OK)
			{
				return ret;
			}
			rec_data_crc = *(uint32_t *) &buf[len];
			len += ETX_OTA_CRC32_SIZE;

			/* Wait to receive the next 1-byte of data from the host and validate it to be a "EOF" field value of an ETX OTA Packet. */
			ret = HAL_UART_Receive(p_huart, &buf[len], ETX_OTA_EOF_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			ret = HAL_ret_handler(ret);
			if (ret != HAL_OK)
			{
				return ret;
			}
			break;
		case ETX_OTA_hw_Protocol_BT:
			/* Wait to receive the first byte of data from the host and validate it to be the SOF byte of an ETX OTA Packet. */
            if (Rx_Buffer[0] == 0)
            {
                ret = get_hm10_ota_data(&buf[len], ETX_OTA_SOF_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
                if (ret != HAL_OK)
                {
                    return ret;
                }
                if (buf[len] != ETX_OTA_SOF)
                {
                    #if ETX_OTA_VERBOSE
                        printf("ERROR: Expected to receive the SOF field value from the current ETX OTA Packet.\r\n");
                    #endif
                    return ETX_OTA_EC_ERR;
                }
            }
            len++;

			/* Wait to receive the next 1-byte of data from the host and validate it to be a "Packet Type" field value of an ETX OTA Packet. */
			ret = get_hm10_ota_data(&buf[len], ETX_OTA_PACKET_TYPE_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			if (ret != HAL_OK)
			{
				return ret;
			}
			switch (buf[len++])
			{
				case ETX_OTA_PACKET_TYPE_CMD:
				case ETX_OTA_PACKET_TYPE_DATA:
				case ETX_OTA_PACKET_TYPE_HEADER:
				case ETX_OTA_PACKET_TYPE_RESPONSE:
					break;
				default:
					#if ETX_OTA_VERBOSE
						printf("ERROR: The data received from the Packet Type field of the currently received ETX OTA Packet contains a value not recognized by our MCU/MPU.\r\n");
					#endif
					return ETX_OTA_EC_ERR;
			}

			/* Wait to receive the next 2-bytes of data from the host, which our MCU/MPU will interpret as the "Data Length" field value of an ETX OTA Packet. */
			ret = get_hm10_ota_data(&buf[len], ETX_OTA_DATA_LENGTH_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			if (ret != HAL_OK)
			{
				return ret;
			}
			data_len = *(uint16_t *)&buf[len];
			len += ETX_OTA_DATA_LENGTH_SIZE;

			/* Wait to receive the next \c data_len bytes of data from the host, which our MCU/MPU will interpret as the "Data" field value of an ETX OTA Packet. */
			for (uint16_t i=0; i<data_len; i++)
			{
				ret = get_hm10_ota_data(&buf[len++], 1, ETX_CUSTOM_HAL_TIMEOUT);
				if (ret != HAL_OK)
				{
					return ret;
				}
			}

			/* Wait to receive the next 4-bytes of data from the host, which our MCU/MPU will interpret as the "CRC32" field value of an ETX OTA Packet. */
			ret = get_hm10_ota_data(&buf[len], ETX_OTA_CRC32_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			if (ret != HAL_OK)
			{
				return ret;
			}
			rec_data_crc = *(uint32_t *) &buf[len];
			len += ETX_OTA_CRC32_SIZE;

			/* Wait to receive the next 1-byte of data from the host and validate it to be a "EOF" field value of an ETX OTA Packet. */
			ret = get_hm10_ota_data(&buf[len], ETX_OTA_EOF_SIZE, ETX_CUSTOM_HAL_TIMEOUT);
			if (ret != HAL_OK)
			{
				return ret;
			}
			break;
		default:
			/* This should not happen since it should have been previously validated. */
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected a Hardware Protocol value, but received something else: %d.\r\n", ETX_OTA_hardware_protocol);
			#endif
			return ETX_OTA_EC_ERR;
	}

	/* Validate that the latest byte received corresponds to an ETX OTA End of Frame (EOF) byte. */
	if (buf[len++] != ETX_OTA_EOF)
	{
		#if ETX_OTA_VERBOSE
			printf("ERROR: Expected to receive the EOF field value from the current ETX OTA Packet.\r\n");
		#endif
		return ETX_OTA_EC_ERR;
	}

	/* Calculate the 32-bit CRC only with respect to the contents of the "Data" field from the current ETX OTA Packet that has just been received. */
	cal_data_crc = crc32_mpeg2(&buf[ETX_OTA_DATA_FIELD_INDEX], data_len);

	/* Validate that the Calculated CRC matches the Recorded CRC. */
	if (cal_data_crc != rec_data_crc)
	{
		#if ETX_OTA_VERBOSE
			printf("ERROR: CRC mismatch with current ETX OTA Packet [Calculated CRC = 0x%08X] [Recorded CRC = 0x%08X]\r\n",
													   (unsigned int) cal_data_crc, (unsigned int) rec_data_crc);
		#endif
		return ETX_OTA_EC_ERR;
	}

	if (ETX_OTA_PACKET_MAX_SIZE < len)
	{
		#if ETX_OTA_VERBOSE
			printf("ERROR: Received more data than expected (Expected = %d, Received = %d)\r\n", ETX_OTA_PACKET_MAX_SIZE, len);
		#endif
		return ETX_OTA_EC_ERR;
	}

	#if ETX_OTA_VERBOSE
		printf("ETX OTA Packet has been successfully received.\r\n");
	#endif
	return ETX_OTA_EC_OK;
}

static ETX_OTA_Status etx_ota_process_data(uint8_t *buf)
{
	/** <b>Local pointer cmd:</b> Points to the data of the latest ETX OTA Packet but in @ref ETX_OTA_Command_Packet_t type. */
	ETX_OTA_Command_Packet_t *cmd = (ETX_OTA_Command_Packet_t *) buf;

	/* Check if we received the ETX OTA Abort Command and, if true, stop the ETX OTA process. */
	#if ETX_OTA_VERBOSE
		printf("Processing data of the latest ETX OTA Packet...\r\n");
	#endif
	if (cmd->packet_type == ETX_OTA_PACKET_TYPE_CMD)
	{
		if (cmd->cmd == ETX_OTA_CMD_ABORT)
		{
			#if ETX_OTA_VERBOSE
				printf("DONE: ETX OTA Abort command received. Stopping the process...\r\n");
			#endif
			return ETX_OTA_EC_STOP;
		}
	}

	switch (etx_ota_state)
	{
		case ETX_OTA_STATE_IDLE:
			#if ETX_OTA_VERBOSE
				printf("DONE: ETX OTA Process is in Idle State.\r\n");
			#endif
			return ETX_OTA_EC_OK;

		case ETX_OTA_STATE_START:
			if ((cmd->packet_type==ETX_OTA_PACKET_TYPE_CMD) && (cmd->cmd == ETX_OTA_CMD_START))
			{
				#if ETX_OTA_VERBOSE
					printf("DONE: Received ETX OTA Start Command.\r\n");
				#endif
				etx_ota_state = ETX_OTA_STATE_HEADER;
				return ETX_OTA_EC_OK;
			}
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected ETX OTA Command Type Packet containing an ETX OTA Start Command, but something else was received instead.\r\n");
			#endif
			return ETX_OTA_EC_ERR;

		case ETX_OTA_STATE_HEADER:
			/** <b>Local pointer header:</b> Points to the data of the latest ETX OTA Packet but in @ref ETX_OTA_Header_Packet_t type. */
			ETX_OTA_Header_Packet_t *header = (ETX_OTA_Header_Packet_t *) buf;

			if (header->packet_type == ETX_OTA_PACKET_TYPE_HEADER)
			{
				/* We validate that the Payload Type to be received and take an action correspondingly. */
				switch (header->meta_data.payload_type)
				{
					case ETX_OTA_Application_Firmware_Image:
					case ETX_OTA_Bootloader_Firmware_Image:
						#if ETX_OTA_VERBOSE
							printf("WARNING: An ETX OTA Firmware Update request has been received.\r\n");
						#endif
						return ETX_OTA_EC_NA;
					case ETX_OTA_Custom_Data:
						#if ETX_OTA_VERBOSE
							printf("An ETX OTA Custom Data request has been received.\r\n");
						#endif
						p_custom_data->size = header->meta_data.package_size;
						p_custom_data->rec_crc = header->meta_data.package_crc;
						#if ETX_OTA_VERBOSE
							printf("Received ETX OTA Header with a Payload Size of %ld bytes.\r\n", p_custom_data->size);
						#endif
						break;
					default:
						#if ETX_OTA_VERBOSE
							printf("ERROR: An unknown Payload Type was identified from the Header of the current ETX OTA Packet.\r\n");
						#endif
						return ETX_OTA_EC_ERR;
				}

				etx_ota_state = ETX_OTA_STATE_DATA;
				return ETX_OTA_EC_OK;
			}
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected ETX OTA Header Type Packet, but something else was received instead.\r\n");
			#endif
			return ETX_OTA_EC_ERR;

		case ETX_OTA_STATE_DATA:
			/** <b>Local pointer data:</b> Points to the data of the latest ETX OTA Packet but in @ref ETX_OTA_Data_Packet_t type. */
			ETX_OTA_Data_Packet_t *data = (ETX_OTA_Data_Packet_t *) buf;

			if (data->packet_type == ETX_OTA_PACKET_TYPE_DATA)
			{
				/* Write the ETX OTA Data Type Packet into our MCU/MPU's RAM. */
				write_data_to_ram(buf+ETX_OTA_DATA_FIELD_INDEX, data->data_len);
				#if ETX_OTA_VERBOSE
					if (p_custom_data->size%ETX_OTA_DATA_MAX_SIZE == 0)
					{
						printf("[%ld/%ld] parts of the current ETX OTA transaction are now stored into our MCU/MPUs RAM...\r\n", etx_ota_fw_received_size/ETX_OTA_DATA_MAX_SIZE, p_custom_data->size/ETX_OTA_DATA_MAX_SIZE);
					}
					else
					{
						if (etx_ota_fw_received_size%ETX_OTA_DATA_MAX_SIZE == 0)
						{
							printf("[%ld/%ld] parts of the current ETX OTA transaction are now stored into our MCU/MPUs RAM...\r\n", etx_ota_fw_received_size/ETX_OTA_DATA_MAX_SIZE, p_custom_data->size/ETX_OTA_DATA_MAX_SIZE+1);
						}
						else
						{
							printf("[%ld/%ld] parts of the current ETX OTA transaction are now stored into our MCU/MPUs RAM...\r\n", (etx_ota_fw_received_size/ETX_OTA_DATA_MAX_SIZE+1), p_custom_data->size/ETX_OTA_DATA_MAX_SIZE+1);
						}
					}
				#endif
				if (etx_ota_fw_received_size >= p_custom_data->size)
				{
					/* received the full data. Therefore, move to the End State of the ETX OTA Process. */
					etx_ota_state = ETX_OTA_STATE_END;
				}
				return ETX_OTA_EC_OK;
			}
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected ETX OTA Data Type Packet, but something else was received instead.\r\n");
			#endif
			return ETX_OTA_EC_ERR;

		case ETX_OTA_STATE_END:
			if ((cmd->packet_type==ETX_OTA_PACKET_TYPE_CMD) && (cmd->cmd==ETX_OTA_CMD_END))
			{
				/** <b>Local variable cal_crc:</b> Value holder for the calculated 32-bit CRC of the ETX OTA data that has just been received by our MCU/MPU. */
				uint32_t cal_crc = crc32_mpeg2(p_custom_data->data, p_custom_data->size);

				/* Validate the 32-bit CRC of the whole data received from the current whole ETX OTA Transaction. */
				#if ETX_OTA_VERBOSE
					printf("Validating the received data from the current whole ETX OTA Transaction...\r\n");
				#endif
				if (cal_crc != p_custom_data->rec_crc)
				{
					#if ETX_OTA_VERBOSE
						printf("The 32-bit CRC of the Data of the received from the whole current ETX OTA Transaction mismatches with the calculated one: [Calculated CRC = 0x%08X] [Recorded CRC = 0x%08X]\r\n",
								(unsigned int) cal_crc, (unsigned int) p_custom_data->rec_crc);
					#endif
					return ETX_OTA_EC_ERR;
				}
				#if ETX_OTA_VERBOSE
					printf("DONE: 32-bit CRC of the Data received from the whole current ETX OTA Transaction has been successfully validated.\r\n");
				#endif
				etx_ota_state = ETX_OTA_STATE_IDLE;
				return ETX_OTA_EC_OK;
			}
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected ETX OTA Command Type Packet containing an ETX OTA End Command, but something else was received instead.\r\n");
			#endif
			return ETX_OTA_EC_ERR;

		default:
			/* This case should never occur. */
			#if ETX_OTA_VERBOSE
				printf("ERROR: The current ETX OTA State %d is unrecognized by our MCU/MPU.\r\n", etx_ota_state);
			#endif
			return ETX_OTA_EC_ERR;
			break;
	}
}

//#pragma GCC diagnostic ignored "-Wstringop-overflow=" // This pragma definition will tell the compiler to ignore an expected Compilation Warning (due to a code functionality that it is strictly needed to work that way) that gives using the HAL_CRC_Calculate() function inside the etx_ota_send_resp() function,. which states the following: 'HAL_CRC_Calculate' accessing 4 bytes in a region of size 1.
static ETX_OTA_Status etx_ota_send_resp(ETX_OTA_Response_Status response_status)
{
	/** <b>Local variable ret:</b> Return value of a @ref ETX_OTA_Status function function type. */
	ETX_OTA_Status  ret;

	ETX_OTA_Response_Packet_t response =
	{
		.sof         	= ETX_OTA_SOF,
		.packet_type 	= ETX_OTA_PACKET_TYPE_RESPONSE,
		.data_len    	= 1U,
		.status      	= response_status,
		.crc			= 0U,
		.eof         	= ETX_OTA_EOF
	};
	response.crc = crc32_mpeg2((uint8_t *) &response.status, 1);

	switch (ETX_OTA_hardware_protocol)
	{
		case ETX_OTA_hw_Protocol_UART:
			ret = HAL_UART_Transmit(p_huart, (uint8_t *) &response, sizeof(ETX_OTA_Response_Packet_t), ETX_CUSTOM_HAL_TIMEOUT);
			ret = HAL_ret_handler(ret);
			break;
		case ETX_OTA_hw_Protocol_BT:
			ret = send_hm10_ota_data((uint8_t *) &response, sizeof(ETX_OTA_Response_Packet_t), ETX_CUSTOM_HAL_TIMEOUT);
			break;
		default:
			/* This should not happen since it should have been previously validated. */
			#if ETX_OTA_VERBOSE
				printf("ERROR: Expected a Hardware Protocol value, but received something else: %d.\r\n", ETX_OTA_hardware_protocol);
			#endif
			return ETX_OTA_EC_ERR;
	}

	return ret;
}

static void write_data_to_ram(uint8_t *data, uint16_t data_len)
{
	/* Write the current chunk/packet that is part of the whole ETX OTA Custom Data into our MCU/MPU's RAM Memory. */
	for (uint16_t bytes_written=0; bytes_written<data_len; bytes_written++)
	{
		p_custom_data->data[etx_ota_fw_received_size] = data[bytes_written];
        etx_ota_fw_received_size++;
	}
}

static ETX_OTA_Status HAL_ret_handler(HAL_StatusTypeDef HAL_status)
{
  switch (HAL_status)
    {
  	  case HAL_BUSY:
	  case HAL_TIMEOUT:
		return ETX_OTA_EC_NR;
	  case HAL_ERROR:
		return ETX_OTA_EC_ERR;
	  default:
		return HAL_status;
    }
}

__attribute__((weak)) void etx_ota_pre_transaction_handler()
{
	/*
	   NOTE: This function should not be modified here. Instead, the implementer should override this function on
	         wherever the @ref app_side_etx_ota was implemented at.
	 */
}

__attribute__((weak)) void etx_ota_status_resp_handler(ETX_OTA_Status resp)
{
	/* Reset the MCU/MPU so that it enters into its Bootloader Mode again in order to try again in receiving the currently requested ETX OTA Firwmare Update Request. */
	if (resp == ETX_OTA_EC_NA)
	{
		HAL_NVIC_SystemReset(); // We reset our MCU/MPU to try installing a Firmware Image there.
	}
	/*
	   NOTE: This function should not be modified here. Instead, the implementer should override this function on
			 wherever the @ref app_side_etx_ota was implemented at.
	 */
}

/** @} */
