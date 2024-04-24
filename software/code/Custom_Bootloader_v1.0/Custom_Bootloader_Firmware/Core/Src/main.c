/* USER CODE BEGIN Header */
/** @file
 	@brief	Main file at which the main/actual application of our MCU/MPU lies at.
 *
 * @defgroup main Main module
 * @{
 *
 * @brief	This is the Main Module, which is where the main part of the application of the whole code is executed.
 *
 * @details	The purpose of this module is to function as the Bootloader Firmware of the MTKATR001 System's MCU/MPU,
 *          which is able to manage and handle Firmware Updates via the Mortrack's ETX OTA Protocol which uses the
 *          @ref firmware_update_config .
 *
 * @note    This @ref main template is actually functioning as a Bootloader Firmware and has an Application Firmware
 *          counterpart that should be located in an independent project to this, but that should be located just
 *          next to the root folder where this @ref main project's File Path is located at.
 *
 * @details Regarding the ETX OTA Protocol as employed in this this @ref main program, the way that either Bootloader or
 *          Application Firmware Updates will be requested to this @ref main program is that if this @ref main program
 *          receives a Bootloader Firmware Image via the ETX OTA Protocol made by Mortrack, then it will temporarily
 *          store it into the Flash Memory designated to the Application Firmware (i.e., at
 *          @ref APLICATION_FIRMWARE_ADDRESS ) and then it will reboot. After reboot, this @ref main program expects
 *          that there is another independent program (i.e., The Pre-Bootloader program) installed into our MCU/MPU that
 *          runs first and handles the installation of the temporarily stored Bootloader Firmware Image into the Flash
 *          Memory designated to the Bootloader Firmware (i.e., at @ref BOOTLOADER_FIRMWARE_ADDRESS ) that was just
 *          mentioned. On the other hand, if an Application Firmware Image is received instead by this @ref main
 *          program, then this @ref main program will install it into the Flash Memory designated to the Application
 *          Firmware (i.e., at @ref APLICATION_FIRMWARE_ADDRESS ) and will then also reboot our MCU/MPU. However, if no
 *          Firmware Image Update is requested to our @ref main program for whatever the @ref ETX_CUSTOM_HAL_TIMEOUT
 *          lasts, then our MCU/MPU will jump into whatever Application Firmware Image it has installed in it but only
 *          if its 32-bit CRC validation passes successfully. Otherwise, if there is no Application Firmware Image
 *          installed into our MCU/MPU, then the @ref main program will loop into waiting for a Firmware Image Update
 *          request from a host machine.
 *
 * @note    This @ref main program expects our MCU/MPU to have an Application Firmware counterpart that uses the ETX OTA
 *          Protocol made by Mortrack, which is where the implementer's actual application purpose for our MCU/MPU
 *          should lie on.
 * @note    In the case that the implementer desires to update either only the Bootloader or both the Bootloader and
 *          Application Firmwares of our MCU/MPU, then the order of the updates should strictly be first for the
 *          Bootloader and then the Application should be installed secondly. The reason for this lies in that if only
 *          the Bootloader is updated, then our MCU/MPU will lose its Application Firmware with which it was last
 *          working with, reason for which, whenever updating the Bootloader, then the Application Firmware should also
 *          be updated after that. On the other hand, if only the Application is desired to be updated, then this can be
 *          directly done and the MCU/MPU should maintain its current Bootloader Firmware intact but now with its new
 *          Application Firmware.
 * @note    Only for the first time you install this @ref main Bootloader program into your MCU/MPU, you must do this
 *          Firmware installation via the STM32 ST-LINK Utility application provided by the STMicroelectronics Team. In
 *          addition, you should also install the Pre-Bootloader (located at an independent project to @ref main , next
 *          to the root folder of the @ref main project File Path) also via the STM32 ST-LINK Utility app. After that,
 *          all the subsequent times that you want to install either an updated code of this @ref main Bootloader or an
 *          Application Firmware (Application Firmware located at independent project to @ref main , next to the root
 *          folder of the @ref main project File Path), then they may now be made and should be made only via the ETX
 *          OTA Protocol made by Mortrack. This is strictly necessary for the Application Firmware because the
 *          Configuration Settings of the @ref firmware_update_config are changed whenever that happens and this is
 *          strictly required and expected by the Application Firmware Installations that uses the ETX OTA Protocol.
 *          Otherwise, your Application Firmware will not run.
 *
 * @details Regarding the other functionalities made by this @ref main program and that relate to the actual MTKATR001
 *          System, this program will initialize all the Outputs of this System with their expected reset state so that
 *          no mechanism of this System starts working during the execution of this @ref main program since this is just
 *          the Bootloader Firmware of this System. However, only the 7-segment Display Device will be used during this
 *          program in order to give visual information to the user to inform him of the stage or processes being run at
 *          this Bootloader Firmware. In addition, if any of the Errors described at @ref MTKATR001_Status give place,
 *          then these will be shown via that Display Device so that the user can take action or is at least informed of
 *          the problem that gave place.
 *
 * @note    Input Peripherals of our MCU/MPU can be leaved at their reset state (i.e., in floating mode) since that
 *          should have no effect on the electronic circuits with which our MCU/MPU will interact with. However, all
 *          Output Peripherals of our MCU/MPU should be configured as such and leaved in their respective initial state
 *          at all times during the execution time of this @ref main (e.g., The PWM Output for the Hot Fan should be
 *          configured at the STM32CubeMx App and should not be started or, if started, should be executed with 0 Duty
 *          Cycle. Another example would be to initialize the GPIO Output of the Hot Water Pump at the STM32CybeMx App
 *          with its Output Level in Low and this should be leaved as such during the whole execution time of this
 *          @ref main in order to guarantee that no mechanisms of the MTKATR001 System will be turned On during the
 *          Bootloader Firmware execution).
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	February 10, 2024.
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#if ETX_OTA_VERBOSE
    #include <stdio.h>	// Library from which "printf" is located at.
#endif
#include <stdint.h> // This library contains the aliases: uint8_t, uint16_t, uint32_t, etc.
#include "bl_side_etx_ota.h" // This custom Mortrack's library contains the functions, definitions and variables required so that the Main module can receive and apply Firmware Update Images to our MCU/MPU.
#include "5641as_display_driver.h" // This custom Mortrack's library contains the functions, definitions and variables that together operate as the driver for the 5641AS 7-segment Display Device.
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define BOOTLOADER_FIRMWARE_ADDRESS					(ETX_BL_FLASH_ADDR)						/**< @brief Designated Memory Location address for the Bootloader Firmware. */
#define APLICATION_FIRMWARE_ADDRESS					(ETX_APP_FLASH_ADDR)					/**< @brief Designated Memory Location address for the Application Firmware. */
#define APPLICATION_FIRMWARE_RESET_HANDLER_ADDRESS 	(APLICATION_FIRMWARE_ADDRESS + 4U)		/**< @brief Designated Memory Location address for the Reset Handler of the Application Firmware. */
#define GPIO_is_hm10_default_settings_Port 			(GPIOC)				                    /**< @brief @ref GPIO_TypeDef Type of the GPIO Port towards which the Input Mode GPIO Pin PC14 will be used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. */
#define GPIO_is_hm10_default_settings_Pin			(GPIO_PIN_14)							/**< @brief Label for the GPIO Pin 14 towards which the GPIO Pin PC14 in Input Mode is at, which is used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. @details The following are the possible values of this Pin:<br><br>* 0 (i.e., Low State) = Do not reset/change the configuration settings of the HM-10 BT Device.<br>* 1 (i.e., High State) = User requests to reset the configuration settings of the HM-10 BT Device to its default settings. */
#define MAJOR 										(1)										/**< @brief Major version number of our MCU/MPU's Bootloader Firmware. */
#define MINOR 										(0)										/**< @brief Minor version number of our MCU/MPU's Bootloader Firmware. */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */
// NOTE: "hadc1" is used for the ADCs used to read the Temperature Sensors Outputs
// NOTE: "htim2" is used by the 5641AS Display Driver Library.
// NOTE: "htim3" is used to generate two PWMs in its Channel 1 and Channel 2, for the Cold and Hot Fans respectively.
// NOTE: "huart3" is used for communicating with the host that will be sending firmware images to our MCU via the ETX OTA Protocol with the BT Hardware Protocol.
const uint8_t BL_version[2] = {MAJOR, MINOR};       /**< @brief Global array variable used to hold the Major and Minor version number of our MCU/MPU's Bootloader Firmware in the 1st and 2nd byte respectively. */
firmware_update_config_data_t fw_config;				       /**< @brief Global struct used to either pass to it the data that we want to write into the designated Flash Memory pages of the @ref firmware_update_config sub-module or, in the case of a read request, where that sub-module will write the latest data contained in the sub-module. */
uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE];    /**< @brief Global array variable used to hold the ASCII characters that are to be sent to the @ref display_5641as . */
uint16_t ascii_error_code[DISPLAY_5641AS_CHARACTERS_SIZE];  /**< @brief Global array variable used to hold the corresponding @ref MTKATR001_Status error code in its equivalent ASCII Characters in case that the main program has a Hard-Crash or fails for whatever reason. */
Display_5641AS_peripherals_def_t display_peripherals;       /**< @brief Global variable used to indicate the GPIO Output peripheral pins of our MCU/MPU with which we want the @ref display_5641as to control the 5641AS 7-segment Display Device. */
HM10_GPIO_def_t GPIO_is_hm10_default_settings;              /**< @brief Global variable that will to hold the GPIO pin parameters of the Input Mode GPIO Pin to be used so that our MCU can know whether the user wants it to set the default configuration settings in the HM-10 BT Device or not. */
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/**@brief	Initializes the @ref display_5641as .
 *
 * @details	Before initializing that module, this function will populate the required parameters for that purpose by
 *          populating the fields contained in the @ref display_peripherals Global Variable with the following fixed
 *          GPIO Output Peripherals:<br>
 *          <ul>
 *              <li>GPIOB Pin 0 --> 5641AS Display Terminal A</li>
 *              <li>GPIOB Pin 1 --> 5641AS Display Terminal B</li>
 *              <li>GPIOA Pin 2 --> 5641AS Display Terminal C</li>
 *              <li>GPIOA Pin 3 --> 5641AS Display Terminal D</li>
 *              <li>GPIOB Pin 5 --> 5641AS Display Terminal E</li>
 *              <li>GPIOB Pin 6 --> 5641AS Display Terminal F</li>
 *              <li>GPIOB Pin 7 --> 5641AS Display Terminal G</li>
 *              <li>GPIOB Pin 8 --> 5641AS Display Terminal Dp</li>
 *              <li>GPIOB Pin 9 --> 5641AS Display Terminal K1</li>
 *              <li>GPIOB Pin 12 --> 5641AS Display Terminal K2</li>
 *              <li>GPIOB Pin 13 --> 5641AS Display Terminal K3</li>
 *              <li>GPIOB Pin 14 --> 5641AS Display Terminal K4</li>
 *          </ul>
 *          In addition, a fixed On-Time and Off-Time steps will be defined, with the values of 6 and 4 respectively.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date	February 06, 2024
 */
static void custom_initialize_5641as_display_driver(void);

/**@brief	Gets the equivalent of a certain number to its equivalent ASCII Numeric Characters.
 *
 * @details	This function is only able to get the equivalent ASCII Numeric Characters for any number greater than -10
 *          and lower than 100.
 *
 * @retval  0   If the given number to convert is greater than -10 and lower than 100 and if its conversion was
 *              successfully made and retrieved.
 * @retval  -1  If this function did not attempted to get or convert the requested number due to either being equal or
 *              lower than -10, or equal or greater than 100.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date	February 03, 2024
 */
static int convert_number_to_ASCII(float src, uint16_t *dst);

/**@brief	Initializes the @ref firmware_update_config sub-module and then loads the latest data that has been written
 *          into it, if there is any. However, in the case that any of these processes fail, then this function will
 *          endlessly loop via a \c while() function and set the corresponding @ref MTKATR001_Status Exception Code on
 *          the Display driven by @ref display_5641as .
 *
 * @details	In case that all the processes conclude successfully, the latest data of the @ref firmware_update_config
 *          sub-module will be copied into the global struct \c fw_config .
 *
 * @details	A maximum of three attempts to initialize this module will be made, with a delay of 0.5 seconds each.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date	November 19, 2023
 */
static void custom_firmware_update_config_init(void);

/**@brief	Initializes the @ref bl_side_firmware_update with a desired Hardware Protocol and, only in the case that the
 *          initialization is unsuccessful, then this function will endlessly loop via a \c while() function and set the
 *          corresponding @ref MTKATR001_Status Exception Code on the Display driven by @ref display_5641as .
 *
 * @details	This function populate the fields required for initializing the @ref bl_side_firmware_update via the
 *          @ref init_firmware_update_module function. In the case where that function initializes the
 *          @ref bl_side_firmware_update successfully, then this @ref custom_firmware_update_config_init function will
 *          terminate. Otherwise, it will endlessly loop via a \c while() function and set the corresponding
 *          @ref MTKATR001_Status Exception Code on the Display driven by @ref display_5641as .
 *
 * @note    The @ref fw_config Global struct must have already been populated with the latest data written into the
 *          @ref firmware_update_config before calling this function.
 *
 * @param hw_protocol   ETX OTA Hardware Protocol that is desired to enable whenever using the ETX OTA Protocol.
 * @param[in] p_huart   Pointer to the UART that wants to be used from our MCU/MPU as the means for using the chosen ETX
 *                      OTA Hardware Protocol.
 *
 * @author	César Miranda Meza
 * @date	November 19, 2023
 */
static void custom_init_etx_ota_protocol_module(ETX_OTA_hw_Protocol hw_protocol, UART_HandleTypeDef *p_huart);

/**@brief	Validates the CRC of our MCU/MPU's current Bootloader Firmware and, only in the case that the Bootloader
 *          Firmware is not valid or is corrupted, then this function will endlessly loop via a \c while() function and
 *          set the corresponding @ref MTKATR001_Status Exception Code on the Display driven by @ref display_5641as .
 *
 * @details	This function reads the data stored in the @ref firmware_update_config via the @ref fw_config Global struct
 *          to get the recorded CRC of the Bootloader Firmware and it calculates the CRC of the actual Bootloader
 *          Firmware currently installed in our MCU/MPU. After this, both CRCs are used for validating our MCU/MPU's
 *          current Bootloader Firmware. If both CRCs match or if there is no recorded CRC stored at the @ref fw_config
 *          Global struct, then this function will terminate. Otherwise, it will endlessly loop via a \c while()
 *          function and set the corresponding @ref MTKATR001_Status Exception Code on the Display driven by
 *          @ref display_5641as .
 *
 * @note    The @ref fw_config Global struct must have already been populated with the latest data written into the
 *          @ref firmware_update_config before calling this function.
 *
 * @author	César Miranda Meza
 * @date	November 19, 2023
 */
static void validate_bootloader_firmware(void);

/**@brief	Validates the CRC of our MCU/MPU's current Application Firmware.
 *
 * @details	This function reads the data stored in the @ref firmware_update_config sub-module to get the recorded CRC of
 *          the Application Firmware and it calculates the CRC of the actual Application Firmware currently installed in
 *          our MCU/MPU. After this, both CRCs are used for validating our MCU/MPU's current Application Firmware.
 *
 * @param[out] is_app_fw_validation_ok  Pointer to the Memory Location Address where the Flag value for used to indicate
 *                                      whether the Application Firmware has been successfully validated or not is
 *                                      located at. The possible valid values of this Flag are the following:<br><br> -
 *                                      0 = There is either no Firmware Image installed at the designated Flash Memory
 *                                      of the Application Firmware or, if there is one, it either did not pass the
 *                                      32-bit CRC validation or, a Bootloader Firmware was identified to be there.<br>
 *                                      - 1 = There is a valid Application Firmware Image installed at the designated
 *                                      Flash Memory of the Application Firmware.
 *
 * @author	César Miranda Meza
 * @date	November 19, 2023
 */
static void validate_application_firmware(uint8_t *is_app_fw_validation_ok);

/**@brief	Flushes the Rx of a desired UART.
 *
 * @param[in] p_huart	Pointer towards the UART from which it is desired to flush its Rx.
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	February 08, 2024.
 */
static void HAL_uart_rx_flush(UART_HandleTypeDef *p_huart);

/**@brief	Makes our MCU/MPU to jump into its Application Firmware.
 *
 * @author	César Miranda Meza
 * @date	July 16, 2023
 */
static void goto_application_firmware(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/**@brief	MTKATR001 Exception codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref main to indicate the resulting status of
 *          having executed the process contained in each of the processes of the main application. For example, to
 *          indicate that the process executed by a certain function was successful or that it has failed.
 */
typedef enum
{
    MTKATR001_EC_OK                                 = 0U,   //!< MTKATR001 System Process was successful.
    MTKATR001_EC_STOP                               = 1U,   //!< MTKATR001 ETX OTA Protocol Process or transaction has been stopped.
    MTKATR001_EC_NR		                            = 2U,	//!< MTKATR001 ETX OTA Protocol has concluded with no response from Host.
    MTKATR001_EC_NA                                 = 3U,   //!< MTKATR001 ETX OTA Payload received or to be received Not Applicable.
    MTKATR001_EC_ERR                                = 4U,   //!< MTKATR001 ETX OTA Protocol has failed.
    MTKATR001_EC_INIT_FW_UPDT_CONF_MODULE_ERR       = 5U,   //!< MTKATR001 Firmware Update Configurations Sub-module could not be initialized.
    MTKATR001_EC_INIT_ETX_OTA_MODULE_ERR            = 6U,   //!< MTKATR001 ETX OTA Module could not be initialized.
    MTKATR001_BOOTLOADER_FIRMWARE_VALIDATION_ERR    = 7U,   //!< MTKATR001 Bootloader Firmware Validation was unsuccessful.
    MTKATR001_APPLICATION_FIRMWARE_VALIDATION_ERR   = 8U    //!< MTKATR001 Application Firmware Validation was unsuccessful.
    //MTKATR001_HOT_WATER_TEMP_IS_UNDER_SHORTCIRCUIT  = 9U,   //!< MTKATR001 Hot Water Temperature Sensor is currently under a short-circuit.
    //MTKATR001_COLD_WATER_TEMP_IS_UNDER_SHORTCIRCUIT = 10U,  //!< MTKATR001 Cold Water Temperature Sensor is currently under a short-circuit.
    //MTKATR001_COLD_WATER_TEMP_ADC_ERR               = 11U,  //!< MTKATR001 ADC with which the Cold Water Temperature Sensor is being read with has responded with a HAL error/problem.
    //MTKATR001_HOT_WATER_TEMP_ADC_ERR                = 12U,  //!< MTKATR001 ADC with which the Hot Water Temperature Sensor is being read with has responded with a HAL error/problem.
    //MTKATR001_INTERNAL_AMBIENT_TEMP_ADC_ERR         = 13U   //!< MTKATR001 ADC with which the Internal Ambient Temperature Sensor is being read with has responded with a HAL error/problem.
} MTKATR001_Status;

/**@brief	ASCII code character definitions that are available in the @ref display_5641as and that are used by the
 *          @ref convert_number_to_ASCII function for converting numbers into their equivalent ASCII Numeric Characters.
 *
 * @note    These definitions are defined with respect to the decimal value that corresponds for each of the available
 *          ASCII code characters.
 */
typedef enum
{
    Command_NULL_in_ASCII                   = 0,     //!< \f$[NULL]_{ASCII} = 0_d\f$.
    Letter_minus_sign_in_ASCII              = 45,    //!< \f$-_{ASCII} = 45_d\f$.
    Number_0_in_ASCII	                    = 48,    //!< \f$0_{ASCII} = 48_d\f$.
    Number_1_in_ASCII	                    = 49,    //!< \f$1_{ASCII} = 49_d\f$.
    Number_2_in_ASCII	                    = 50,    //!< \f$2_{ASCII} = 50_d\f$.
    Number_3_in_ASCII	                    = 51,    //!< \f$3_{ASCII} = 51_d\f$.
    Number_4_in_ASCII	                    = 52,    //!< \f$4_{ASCII} = 52_d\f$.
    Number_5_in_ASCII	                    = 53,    //!< \f$5_{ASCII} = 53_d\f$.
    Number_6_in_ASCII	                    = 54,    //!< \f$6_{ASCII} = 54_d\f$.
    Number_7_in_ASCII	                    = 55,    //!< \f$7_{ASCII} = 55_d\f$.
    Number_8_in_ASCII	                    = 56,    //!< \f$8_{ASCII} = 56_d\f$.
    Number_9_in_ASCII	                    = 57,    //!< \f$9_{ASCII} = 57_d\f$.
    Number_0Dp_in_ASCII	                    = 256,    //!< \f$0._{ASCII} = 256_d custom value\f$.
    Number_1Dp_in_ASCII	                    = 257,    //!< \f$1._{ASCII} = 257_d custom value\f$.
    Number_2Dp_in_ASCII	                    = 258,    //!< \f$2._{ASCII} = 258_d custom value\f$.
    Number_3Dp_in_ASCII	                    = 259,    //!< \f$3._{ASCII} = 259_d custom value\f$.
    Number_4Dp_in_ASCII	                    = 260,    //!< \f$4._{ASCII} = 260_d custom value\f$.
    Number_5Dp_in_ASCII	                    = 261,    //!< \f$5._{ASCII} = 261_d custom value\f$.
    Number_6Dp_in_ASCII	                    = 262,    //!< \f$6._{ASCII} = 262_d custom value\f$.
    Number_7Dp_in_ASCII	                    = 263,    //!< \f$7._{ASCII} = 263_d custom value\f$.
    Number_8Dp_in_ASCII	                    = 264,    //!< \f$8._{ASCII} = 264_d custom value\f$.
    Number_9Dp_in_ASCII	                    = 265     //!< \f$9._{ASCII} = 265_d custom value\f$.
} Display_ASCII_Characters;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  /** <b>Local variable is_app_fw_validation_ok:</b> Flag used to indicate whether the Application Firmware has been successfully validated via its CRC with a \c 1 or, otherwise with a \c 0 . */
  uint8_t is_app_fw_validation_ok = 1;
  /** <b>Local variable end_tick:</b> Will hold the HAL Ticks that our MCU/MPU needs to reach so that 3 seconds have passed with respect to the moment that our MCU/MPU enters into DFU mode. */
  uint32_t end_tick;
  /** <b>Local variable current_tick:</b> Current HAL Tick in our MCU/MPU. */
  uint32_t current_tick = 0;
  /** <b>Local variable ret:</b> Used to hold the exception code value returned by a @ref ETX_OTA_Status function type. */
  ETX_OTA_Status ret;

  /* Send a message from the Bootloader showing the current Bootloader version there. */
  #if ETX_OTA_VERBOSE
    printf("Starting Bootloader v%d.%d\r\n", BL_version[0], BL_version[1]);
  #endif

  /* Initialize the 5641AS 7-segment Display Driver module. */
  custom_initialize_5641as_display_driver();
  display_output[0] = 'B';
  display_output[1] = 'o';
  display_output[2] = 'o';
  display_output[3] = 't';
  set_5641as_display_output(display_output);

  /* We initialize the Firmware Update Configurations sub-module and the ETX OTA Protocol module. */
  custom_firmware_update_config_init();
  custom_init_etx_ota_protocol_module(ETX_OTA_hw_Protocol_BT, &huart3);

  /* Validate both the Bootloader and Application Firmwares in our MCU/MPU. */
  validate_bootloader_firmware();
  validate_application_firmware(&is_app_fw_validation_ok);

  /* Execute the Delay for the Pre ETX OTA Requests Hearing stage and then flush the Rx of the UART from which the ETX OTA Protocol will be used in this MCU/MPU. */
  HAL_Delay(PRE_ETX_OTA_REQUESTS_HEARING_DELAY);
  HAL_uart_rx_flush(&huart3);

  /*
   Check if a Firmware Image is received during the next @ref ETX_CUSTOM_HAL_TIMEOUT seconds and, if true, install it
   if it is an Application Firmware Image. Otherwise, jump the MCU/MPU into its Application Firmware. However, if there
   is no existing or valid Application Firmware installed in our MCU, then wait indefinitely to receive an Application
   Firmware Update via the ETX OTA Protocol.
   */
  #if ETX_OTA_VERBOSE
    printf("Our MCU/MPU has entered into DFU mode...\r\n");
    printf("Waiting to receive Application Firmware Image to trigger firmware update...\r\n");
  #endif
  display_output[0] = '.';
  display_output[1] = '.';
  display_output[2] = '.';
  display_output[3] = '.';
  set_5641as_display_output(display_output);
  end_tick = HAL_GetTick() + ETX_CUSTOM_HAL_TIMEOUT;
  do
  {
	  do
	  {
          #if ETX_OTA_VERBOSE
		    printf("Waiting for an ETX OTA Transaction...\r\n");
          #endif
		  ret = firmware_image_download_and_install();
		  start_5641as_display_module(); // We start back again the 5641AS Driver Timer's Base generation in Interrupt Mode.
		  switch (ret)
		  {
			case ETX_OTA_EC_OK:
				#if ETX_OTA_VERBOSE
					if (fw_config.is_bl_fw_install_pending == IS_PENDING)
					{
						printf("DONE: Bootloader Firmware Image has been successfully stored in the Flash Memory! Rebooting MCU/MPU to install it...\r\n");
					}
					else
					{
						printf("DONE: Application Firmware Update has successfully concluded! Rebooting MCU/MPU...\r\n");
					}
				#endif
				HAL_NVIC_SystemReset();
				break;
			case ETX_OTA_EC_STOP:
				#if ETX_OTA_VERBOSE
					printf("DONE: Firmware Update process has been aborted. Try again...\r\n");
                #endif
				current_tick = HAL_GetTick();
                display_output[0] = 'E';
                display_output[1] = 'O';
                display_output[2] = 0; // This is a \c NULL value.
                display_output[3] = 'Q';
                set_5641as_display_output(display_output);
				break;
			case ETX_OTA_EC_NR:
				// No response was received from host. Therefore, try hearing for a response from the host again in case our MCU/MPU is still in DFU mode.
				current_tick = HAL_GetTick();
				display_output[0] = '.';
				display_output[1] = '.';
				display_output[2] = '.';
				display_output[3] = '.';
				set_5641as_display_output(display_output);
				break;
			case ETX_OTA_EC_NA:
				#if ETX_OTA_VERBOSE
					printf("WARNING: Application or Bootloader Firmware Image was expected, but request for something else was received instead.\r\n");
				#endif
				current_tick = HAL_GetTick();
				display_output[0] = 'E';
				display_output[1] = 'O';
				display_output[2] = 0; // This is a \c NULL value.
				display_output[3] = 'C';
				set_5641as_display_output(display_output);
				break;
			case ETX_OTA_EC_ERR:
				#if ETX_OTA_VERBOSE
				    printf("ERROR: ETX OTA process has failed. Try again...\r\n");
                #endif
				current_tick = HAL_GetTick();
                display_output[0] = 'E';
                display_output[1] = 'O';
                display_output[2] = 0; // This is a \c NULL value.
                display_output[3] = 'E';
                set_5641as_display_output(display_output);
				break;
			default:
				#if ETX_OTA_VERBOSE
					printf("ERROR: Exception Code received %d is not recognized. Our MCU/MPU will halt!.\r\n", ret);
                #endif
                while (1)
                {
                	convert_number_to_ASCII(MTKATR001_EC_ERR, ascii_error_code);
					ascii_error_code[3] = 0;
					display_output[0] = 'E';
					display_output[1] = 'r';
					display_output[2] = 'r';
					display_output[3] = '=';
                    set_5641as_display_output(display_output);
                    HAL_Delay(2000);
                    set_5641as_display_output(ascii_error_code);
                    HAL_Delay(2000);
                }
		  }
	  }
	  while ((current_tick<end_tick) || (is_app_fw_validation_ok==0));

	  /* We validate the currently installed Application Firmware once more in our MCU/MPU to verify that it is still OK. */
	  /*
	     NOTE:  The following validation helps the @ref main program to correctly tell if the current Application
	            Firmware is currently valid whenever we have the case that it was originally valid during the
	            initialization of the @ref program, but where "along the way", there was an ETX OTA Firmware Update
	            transaction that could not be completed and, therefore, having as a result that the current Application
	            Firmware is no longer valid. Therefore, by validating the Application Firmware again here, the @ref main
	            program can guarantee to successfully loop in the Bootloader Firmware and continue asking for an ETX OTA
	            Request until an ETX OTA Firmware Update transaction is successfully completed.
      */
	  validate_application_firmware(&is_app_fw_validation_ok);
  }
  while (is_app_fw_validation_ok == 0);

  /* Turning Off all the LEDs of the 5641AS Device and stopping the non-blocking Interrupts of the MCU. */
  // NOTE: This must be done  before jumping into Application Firmware since they can break the program during the Bootloader-Application Firmware Transition
  display_output[0] = 0;
  display_output[1] = 0;
  display_output[2] = 0;
  display_output[3] = 0;
  set_5641as_display_output(display_output);
  HAL_Delay(1); // Give the right Delay to guarantee that the 5641AS 7-segment Display Device LEDs will all be turned Off.
  stop_5641as_display_module();
  HAL_Delay(1); // Give the right Delay to guarantee that the non-blocking Interrupts will stop working (this is based in the time at which they are triggered, which was configured at the STM32CubeMx App).

  /* Make the MCU/MPU jump into its Application Firmware. */
  #if ETX_OTA_VERBOSE
    printf("Our MCU/MPU has leaved DFU mode.\r\n");
  #endif
  goto_application_firmware();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSE;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV4;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 416-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 0;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 1818-1;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(IATR_LED_GPIO_Output_GPIO_Port, IATR_LED_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Display_C_terminal_GPIO_Output_Pin|Display_D_terminal_GPIO_Output_Pin|Water_Heating_Resistor_GPIO_Output_Pin|Cold_Water_Pump_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Display_A_terminal_GPIO_Output_Pin|Display_B_terminal_GPIO_Output_Pin|Hot_Water_Pump_GPIO_Output_Pin|Display_E_terminal_GPIO_Output_Pin
                          |Display_F_terminal_GPIO_Output_Pin|Display_G_terminal_GPIO_Output_Pin|Display_Dp_terminal_GPIO_Output_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Display_K2_terminal_GPIO_Output_Pin|Display_K3_terminal_GPIO_Output_Pin|Display_K4_terminal_GPIO_Output_Pin|Display_K1_terminal_GPIO_Output_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : IATR_LED_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = IATR_LED_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(IATR_LED_GPIO_Output_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : HM10_is_default_settings_GPIO_Input_Pin */
  GPIO_InitStruct.Pin = HM10_is_default_settings_GPIO_Input_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(HM10_is_default_settings_GPIO_Input_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : Display_C_terminal_GPIO_Output_Pin Display_D_terminal_GPIO_Output_Pin Water_Heating_Resistor_GPIO_Output_Pin Cold_Water_Pump_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = Display_C_terminal_GPIO_Output_Pin|Display_D_terminal_GPIO_Output_Pin|Water_Heating_Resistor_GPIO_Output_Pin|Cold_Water_Pump_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : Display_A_terminal_GPIO_Output_Pin Display_B_terminal_GPIO_Output_Pin Display_K2_terminal_GPIO_Output_Pin Display_K3_terminal_GPIO_Output_Pin
                           Display_K4_terminal_GPIO_Output_Pin Hot_Water_Pump_GPIO_Output_Pin Display_E_terminal_GPIO_Output_Pin Display_F_terminal_GPIO_Output_Pin
                           Display_G_terminal_GPIO_Output_Pin Display_Dp_terminal_GPIO_Output_Pin Display_K1_terminal_GPIO_Output_Pin */
  GPIO_InitStruct.Pin = Display_A_terminal_GPIO_Output_Pin|Display_B_terminal_GPIO_Output_Pin|Display_K2_terminal_GPIO_Output_Pin|Display_K3_terminal_GPIO_Output_Pin
                          |Display_K4_terminal_GPIO_Output_Pin|Hot_Water_Pump_GPIO_Output_Pin|Display_E_terminal_GPIO_Output_Pin|Display_F_terminal_GPIO_Output_Pin
                          |Display_G_terminal_GPIO_Output_Pin|Display_Dp_terminal_GPIO_Output_Pin|Display_K1_terminal_GPIO_Output_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

#if ETX_OTA_VERBOSE
    /**@brief	Compiler definition to be able to use the @ref printf function from stdio.h library in order to print
     *          characters via the UART1 Peripheral but by using that @ref printf function.
     *
     * @return	The \p ch param.
     *
     * author	César Miranda Meza
     * @date	July 07, 2023
     */
    #ifdef __GNUC__
        /* With GCC, small printf (option LD Linker->Libraries->Small printf set to 'Yes') calls __io_putchar(). */
        int __io_putchar(int ch)
    #else
        int fputc(int ch, FILE *f)
    #endif /* __GNUC__ */
    {
        /* Place your implementation of fputc here. */
        /* NOTE: The characters written into the UART1 Protocol will be looped until the end of transmission. */
        HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
        return ch;
    }
#endif

static void custom_initialize_5641as_display_driver(void)
{
    /* Populate the 5641AS 7-segment Display Device's Peripherals Definition parameters structure. */
    display_peripherals.A.GPIO_Port = GPIOB;
    display_peripherals.A.GPIO_Pin = GPIO_PIN_0;
    display_peripherals.B.GPIO_Port = GPIOB;
    display_peripherals.B.GPIO_Pin = GPIO_PIN_1;
    display_peripherals.C.GPIO_Port = GPIOA;
    display_peripherals.C.GPIO_Pin = GPIO_PIN_2;
    display_peripherals.D.GPIO_Port = GPIOA;
    display_peripherals.D.GPIO_Pin = GPIO_PIN_3;
    display_peripherals.E.GPIO_Port = GPIOB;
    display_peripherals.E.GPIO_Pin = GPIO_PIN_5;
    display_peripherals.F.GPIO_Port = GPIOB;
    display_peripherals.F.GPIO_Pin = GPIO_PIN_6;
    display_peripherals.G.GPIO_Port = GPIOB;
    display_peripherals.G.GPIO_Pin = GPIO_PIN_7;
    display_peripherals.Dp.GPIO_Port = GPIOB;
    display_peripherals.Dp.GPIO_Pin = GPIO_PIN_8;
    display_peripherals.K1.GPIO_Port = GPIOB;
    display_peripherals.K1.GPIO_Pin = GPIO_PIN_9;
    display_peripherals.K2.GPIO_Port = GPIOB;
    display_peripherals.K2.GPIO_Pin = GPIO_PIN_12;
    display_peripherals.K3.GPIO_Port = GPIOB;
    display_peripherals.K3.GPIO_Pin = GPIO_PIN_13;
    display_peripherals.K4.GPIO_Port = GPIOB;
    display_peripherals.K4.GPIO_Pin = GPIO_PIN_14;

    /* Define the desired On-Time and Off-Time steps. */
    // NOTE:  For this particular case, I will consider a PWM Period of 10 Time steps, where the On-Time will be of 6 Time
    //        steps and, therefore, the Off-Time will be of 4 Time steps.
    // NOTE:  In consideration that the Timer to be used in non-interrupt mode by the 5641AS 7-segment Display Driver
    //        module will be triggered by this example each 4'807Hz approximately (meaning that each 7-segment display of
    //        that device will be refreshed at each 120Hz approximately), this will allow this module to output a
    //        sufficiently fast refresh rate at the 5641AS Device so that the human eye is able to see as if all the
    //        7-segment displays of that device are being simultaneously activated.
    /** <b>Local variable on_time_steps:</b> uint32_t Variable Type used to hold the desired On-Time-Steps for the Simulated PWM that we want the @ref display_5641as to generate whenever turning On the LEDs in the 5641AS 7-segment Display Device. */
    uint32_t on_time_steps = 6;
    /** <b>Local variable off_time_steps:</b> uint32_t Variable Type used to hold the desired Off-Time-Steps for the Simulated PWM that we want the @ref display_5641as to generate whenever turning On the LEDs in the 5641AS 7-segment Display Device. */
    uint32_t off_time_steps = 4;

    /* Initialize the 5641AS 7-segment Display Driver module. */
    init_5641as_display_module(&htim2, &display_peripherals, on_time_steps, off_time_steps);
}

static int convert_number_to_ASCII(float src, uint16_t *dst)
{
    if ((src>=100) || (src<=-10))
    {
        return -1;
    }

    if (src > 0)
    {
        uint8_t first_character = src/10;
        switch (first_character)
        {
            case 0:
                dst[0] = Command_NULL_in_ASCII;
                break;
            case 1:
                dst[0] = Number_1_in_ASCII;
                break;
            case 2:
                dst[0] = Number_2_in_ASCII;
                break;
            case 3:
                dst[0] = Number_3_in_ASCII;
                break;
            case 4:
                dst[0] = Number_4_in_ASCII;
                break;
            case 5:
                dst[0] = Number_5_in_ASCII;
                break;
            case 6:
                dst[0] = Number_6_in_ASCII;
                break;
            case 7:
                dst[0] = Number_7_in_ASCII;
                break;
            case 8:
                dst[0] = Number_8_in_ASCII;
                break;
            case 9:
                dst[0] = Number_9_in_ASCII;
                break;
            default:
                // This case should not give place ever.
        }

        uint8_t second_character = src - (first_character*10);
        switch (second_character)
        {
            case 0:
                dst[1] = Number_0Dp_in_ASCII;
                break;
            case 1:
                dst[1] = Number_1Dp_in_ASCII;
                break;
            case 2:
                dst[1] = Number_2Dp_in_ASCII;
                break;
            case 3:
                dst[1] = Number_3Dp_in_ASCII;
                break;
            case 4:
                dst[1] = Number_4Dp_in_ASCII;
                break;
            case 5:
                dst[1] = Number_5Dp_in_ASCII;
                break;
            case 6:
                dst[1] = Number_6Dp_in_ASCII;
                break;
            case 7:
                dst[1] = Number_7Dp_in_ASCII;
                break;
            case 8:
                dst[1] = Number_8Dp_in_ASCII;
                break;
            case 9:
                dst[1] = Number_9Dp_in_ASCII;
                break;
            default:
                // This case should not give place ever.
        }

        uint8_t third_character = (src - (first_character*10) - (second_character)) * 10;
        switch (third_character)
        {
            case 0:
                dst[2] = Number_0_in_ASCII;
                break;
            case 1:
                dst[2] = Number_1_in_ASCII;
                break;
            case 2:
                dst[2] = Number_2_in_ASCII;
                break;
            case 3:
                dst[2] = Number_3_in_ASCII;
                break;
            case 4:
                dst[2] = Number_4_in_ASCII;
                break;
            case 5:
                dst[2] = Number_5_in_ASCII;
                break;
            case 6:
                dst[2] = Number_6_in_ASCII;
                break;
            case 7:
                dst[2] = Number_7_in_ASCII;
                break;
            case 8:
                dst[2] = Number_8_in_ASCII;
                break;
            case 9:
                dst[2] = Number_9_in_ASCII;
                break;
            default:
                // This case should not give place ever.
        }
    }
    else if (src == 0)
    {
        dst[0] = Command_NULL_in_ASCII;
        dst[1] = Number_0Dp_in_ASCII;
        dst[2] = Number_0_in_ASCII;
    }
    else
    {
        dst[0] = Letter_minus_sign_in_ASCII;

        uint8_t second_character = (-1)*src;
        switch (second_character)
        {
            case 0:
                dst[1] = Number_0Dp_in_ASCII;
                break;
            case 1:
                dst[1] = Number_1Dp_in_ASCII;
                break;
            case 2:
                dst[1] = Number_2Dp_in_ASCII;
                break;
            case 3:
                dst[1] = Number_3Dp_in_ASCII;
                break;
            case 4:
                dst[1] = Number_4Dp_in_ASCII;
                break;
            case 5:
                dst[1] = Number_5Dp_in_ASCII;
                break;
            case 6:
                dst[1] = Number_6Dp_in_ASCII;
                break;
            case 7:
                dst[1] = Number_7Dp_in_ASCII;
                break;
            case 8:
                dst[1] = Number_8Dp_in_ASCII;
                break;
            case 9:
                dst[1] = Number_9Dp_in_ASCII;
                break;
            default:
                // This case should not give place ever.
        }

        uint8_t third_character = ((src*-1) - second_character) * 10;
        switch (third_character)
        {
            case 0:
                dst[2] = Number_0_in_ASCII;
                break;
            case 1:
                dst[2] = Number_1_in_ASCII;
                break;
            case 2:
                dst[2] = Number_2_in_ASCII;
                break;
            case 3:
                dst[2] = Number_3_in_ASCII;
                break;
            case 4:
                dst[2] = Number_4_in_ASCII;
                break;
            case 5:
                dst[2] = Number_5_in_ASCII;
                break;
            case 6:
                dst[2] = Number_6_in_ASCII;
                break;
            case 7:
                dst[2] = Number_7_in_ASCII;
                break;
            case 8:
                dst[2] = Number_8_in_ASCII;
                break;
            case 9:
                dst[2] = Number_9_in_ASCII;
                break;
            default:
                // This case should not give place ever.
        }
    }

    return 0;
}

static void custom_firmware_update_config_init(void)
{
    /** <b>Local variable ret:</b> Return value of a @ref FirmUpdConf_Status function type. */
    int16_t ret;
    /** <b>Local variable attempts:</b> Counter for the number of attempts to initialize the Firmware Update Configurations sub-module. */
    uint8_t attempts = 0;
    /** <b>Local variable end_tick:</b> Defines the HAL Tick that our MCU/MPU needs to reach so that 0.5 seconds have passed with respect to each attempt to initialize the @ref firmware_update_config . */
    uint32_t end_tick;
    /** <b>Local variable current_tick:</b> Current HAL Tick in our MCU/MPU. */
    uint32_t current_tick;

    #if ETX_OTA_VERBOSE
        printf("Initializing the Firmware Update Configurations sub-module...\r\n");
    #endif
    do
    {
        /* Delay of 500 milliseconds. */
        end_tick = HAL_GetTick() + 500;
        current_tick = 0;
        while (current_tick < end_tick)
        {
            current_tick = HAL_GetTick();
        }

        /* We attempt to initialize the Firmware Update Configurations sub-module. */
        ret = firmware_update_configurations_init();
        attempts++;
        if (ret == FIRM_UPDT_CONF_EC_OK)
        {
            /* We read the latest data that has been written into the Firmware Update Configurations sub-module. */
            firmware_update_configurations_read(&fw_config);
            #if ETX_OTA_VERBOSE
                printf("DONE: Firmware Update Configurations sub-module has been successfully initialized.\r\n");
            #endif

            return;
        }
        #if ETX_OTA_VERBOSE
            printf("WARNING: The Firmware Update Configurations sub-module could not be initialized at attempt %d...\r\n", attempts);
        #endif
    }
    while(attempts < 3);

    #if ETX_OTA_VERBOSE
        printf("ERROR: The Firmware Update Configurations sub-module could not be initialized. Our MCU/MPU will halt!.\r\n");
    #endif

    while (1)
    {
    	convert_number_to_ASCII(MTKATR001_EC_INIT_FW_UPDT_CONF_MODULE_ERR, ascii_error_code);
		ascii_error_code[3] = 0;
		display_output[0] = 'E';
		display_output[1] = 'r';
		display_output[2] = 'r';
		display_output[3] = '=';
        set_5641as_display_output(display_output);
        HAL_Delay(2000);
        set_5641as_display_output(ascii_error_code);
        HAL_Delay(2000);
    }
}

static void custom_init_etx_ota_protocol_module(ETX_OTA_hw_Protocol hw_protocol, UART_HandleTypeDef *p_huart)
{
    /** <b>Local variable ret:</b> Used to hold the exception code value returned by a @ref ETX_OTA_Status function type. */
    ETX_OTA_Status ret;

    #if ETX_OTA_VERBOSE
        printf("Initializing the ETX OTA Firmware Update Module.\r\n");
    #endif
    GPIO_is_hm10_default_settings.GPIO_Port = GPIO_is_hm10_default_settings_Port;
    GPIO_is_hm10_default_settings.GPIO_Pin = GPIO_is_hm10_default_settings_Pin;

    ret = init_firmware_update_module(hw_protocol, p_huart, &fw_config, &GPIO_is_hm10_default_settings);
    if (ret != ETX_OTA_EC_OK)
    {
        #if ETX_OTA_VERBOSE
            printf("ERROR: The ETX OTA Firmware Update Module could not be initialized. Our MCU/MPU will halt!.\r\n");
        #endif

        while (1)
        {
        	convert_number_to_ASCII(MTKATR001_EC_INIT_ETX_OTA_MODULE_ERR, ascii_error_code);
			ascii_error_code[3] = 0;
			display_output[0] = 'E';
			display_output[1] = 'r';
			display_output[2] = 'r';
			display_output[3] = '=';
            set_5641as_display_output(display_output);
            HAL_Delay(2000);
            set_5641as_display_output(ascii_error_code);
            HAL_Delay(2000);
        }
    }
    #if ETX_OTA_VERBOSE
        printf("DONE: The ETX OTA Firmware Update Module has been successfully initialized.\r\n");
    #endif
}

static void validate_bootloader_firmware(void)
{
    #if ETX_OTA_VERBOSE
        printf("Validating the Bootloader Firmware of our MCU/MPU...\r\n");
    #endif
    if (fw_config.BL_fw_rec_crc != DATA_BLOCK_32BIT_ERASED_VALUE)
    {
        /** <b>Local variable cal_crc:</b> Value holder for the calculated 32-bit CRC of our MCU/MPU's current Bootloader Firmware. */
        uint32_t cal_crc = crc32_mpeg2((uint8_t *) BOOTLOADER_FIRMWARE_ADDRESS, fw_config.BL_fw_size);

        if (cal_crc != fw_config.BL_fw_rec_crc)
        {
            #if ETX_OTA_VERBOSE
                printf("The recorded 32-bit CRC of the installed Bootloader Firmware mismatches with the calculated one: [Calculated CRC = 0x%08X] [Recorded CRC = 0x%08X]\r\n",
                       (unsigned int) cal_crc, (unsigned int) fw_config.BL_fw_rec_crc);
                printf("ERROR: The Bootloader Firmware currently installed in our MCU/MPU is corrupted. Our MCU/MPU will halt!.\r\n");
            #endif

            while (1)
            {
            	convert_number_to_ASCII(MTKATR001_BOOTLOADER_FIRMWARE_VALIDATION_ERR, ascii_error_code);
				ascii_error_code[3] = 0;
				display_output[0] = 'E';
				display_output[1] = 'r';
				display_output[2] = 'r';
				display_output[3] = '=';
                set_5641as_display_output(display_output);
                HAL_Delay(2000);
                set_5641as_display_output(ascii_error_code);
                HAL_Delay(2000);
            }
        }
    }
	#if ETX_OTA_VERBOSE
		printf("DONE: Bootloader Firmware of our MCU/MPU has been successfully validated.\r\n");
	#endif
}

static void validate_application_firmware(uint8_t *is_app_fw_validation_ok)
{
    #if ETX_OTA_VERBOSE
        printf("Validating the Application Firmware of our MCU/MPU...\r\n");
    #endif
    if ((fw_config.App_fw_rec_crc==DATA_BLOCK_32BIT_ERASED_VALUE) || (fw_config.App_fw_size==DATA_BLOCK_32BIT_ERASED_VALUE) || (fw_config.App_fw_size==0x00000000))
    {
        #if ETX_OTA_VERBOSE
            printf("WARNING: No Application Firmware has been identified to be installed in our MCU/MPU.\r\n");
        #endif
        *is_app_fw_validation_ok = 0;
    }
    else if (fw_config.is_bl_fw_stored_in_app_fw == BT_FW_STORED_IN_APP_FW)
    {
        #if ETX_OTA_VERBOSE
            printf("WARNING: A Bootloader Firmware is currently stored at the designated Flash Memory of the Application Firmware of our MCU/MPU.\r\n");
        #endif
        *is_app_fw_validation_ok = 0;
    }
    else
    {
        /** <b>Local variable cal_crc:</b> Value holder for the calculated 32-bit CRC of our MCU/MPU's current Application Firmware. */
        uint32_t cal_crc = crc32_mpeg2((uint8_t *) APLICATION_FIRMWARE_ADDRESS, fw_config.App_fw_size);

        if (cal_crc != fw_config.App_fw_rec_crc)
        {
            #if ETX_OTA_VERBOSE
                printf("WARNING: The recorded 32-bit CRC of the installed Application Firmware Image mismatches with the calculated one: [Calculated CRC = 0x%08X] [Recorded CRC = 0x%08X]\r\n",
                       (unsigned int) cal_crc, (unsigned int) fw_config.App_fw_rec_crc);
            #endif
            *is_app_fw_validation_ok = 0;
        }
    }
    #if ETX_OTA_VERBOSE
		printf("DONE: Application Firmware of our MCU/MPU has been successfully validated.\r\n");
	#endif
}

static void HAL_uart_rx_flush(UART_HandleTypeDef *p_huart)
{
	/** <b>Local variable ret:</b> @ref HAL_StatusTypeDef Type variable used to hold the return value of a HAL function type. */
	HAL_StatusTypeDef  ret;
	/** <b>Local variable buff:</b> @ref uint8_t Type variable used to temporarily hold an available byte of data from the UART from which it is desired to flush its Rx. */
	uint8_t buff;

	/* Receive the HM-10 Device's BT data that is received Over the Air (OTA), if there is any. */
	ret = HAL_UART_Receive(p_huart, &buff, 1, 1);
	if (ret != HAL_TIMEOUT)
	{
		HAL_uart_rx_flush(p_huart);
	}
}

static void goto_application_firmware(void)
{
	/* Create function pointer with no arguments that points to the Memory Location Address of the Reset Handler of the Application Firmware. */
	#if ETX_OTA_VERBOSE
		printf("Jumping into Application Firmware\r\n");
	#endif
	void (*app_reset_handler) (void) = (void*) (*(volatile uint32_t *) (APPLICATION_FIRMWARE_RESET_HANDLER_ADDRESS));

	/* NOTE: Some MCUs might have already the ASM code available so that the Main Stack Pointer (MSP) is recycled, but this is not the case for all MCUs. */
	/* Therefore, if you were to need to do this from scratch, you would have to do the following: */
	/*
	HAL_RCC_DeInit();
	HAL_DeInit();
	__set_MSP(*(volatile uint32_t*) APLICATION_FIRMWARE_ADDRESS);
	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;
	*/

	/* Call the Application's Reset Handler. */
	app_reset_handler();
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */