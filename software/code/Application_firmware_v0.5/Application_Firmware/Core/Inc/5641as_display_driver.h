/**@file
 * @brief	5641AS 7-segment Display's driver Header file.
 *
 * @defgroup display_5641as 5641AS 7-segment Display Driver module
 * @{
 *
 * @brief   This module provides the functions, definitions, structures and variables that together work as the driver
 *          for the 5641AS 7-segment Display Device with the purpose of being used by the application.
 *
 * @details As for now, this @ref display_5641as provides the necessary things so that all the possible ASCII
 *          characters, except the for the Extended Characters, can be shown at the 5641AS 7-segment Display Device at
 *          will and in any custom way that the implementer desires.
 * @details The way that the @ref display_5641as works is that this module must first be initialized via the
 *          @ref init_5641as_display_module function in order to enable all the other functions to work properly. In
 *          addition, this initialization function is the means with which the implementer will designate to the
 *          @ref display_5641as the Timer of the MCU/MPU that this module will use in non-interrupt mode (remember to
 *          enable the given Timer and that work mode at the STM32CubeMX app) to execute the main function of this
 *          module over and over periodically at the time with which that Timer was configured to count at. Also, this
 *          initialization function is where the implementer will also tell what Output GPIO Pins are connected to the
 *          5641AS Device and, a very important thing, the On and Off Time Steps that will be used by the Callback
 *          function that will be executed due to the previously mentioned non-interrupt mode. For both of these Time
 *          Steps, each Step will equal to a single Callback of whenever that Timer's non interrupt mode is triggered,
 *          and where they will both be simulating a PWM on the Output terminals of the 5641AS Device at/via the
 *          previously mentioned Callback function. This way, any instructed valid ASCII characters to display at the
 *          5641AS Device will be shown one at a time, but where it is expected that the implementer requests and
 *          configures the timer of this module to work at a sufficiently fast refresh rate so that our human eyes can
 *          see as if all the instructed ASCII characters were being shown at the 5641AS Device simultaneously.
 * @details It is also important to note that the brightness of the 5641AS Device can be modified by assigning different
 *          values on the Total On and Off Time Steps in this module because that would mean that we would be changing
 *          the Duty Cycle of the Simulated PWM that is displaying the desired ASCII characters at the 5641AS Device.
 * @details One last aspect to mention is that whenever the implementer wants to update the ASCII characters shown at
 *          the 5641AS Device, all the available 7-segment display will have to be updated at the same time. However,
 *          the implement is free to request the previously displayed ASCII characters at any desired 7-segment display
 *          of the 5641AS Device and to only request to change the desired one(s).
 *
 * @note    Whenever this module is used as the driver for the 5641AS Device, that Device will always start operating
 *          with \c NULL ASCII values at all the 7-segment Displays that the Device has available.
 *
 * @note    If the @ref HAL_TIM_PeriodElapsedCallback function provided by the STMicroelectronic's Library is used in
 *          the main program or by another library, then this module will not work unless the code used in the
 *          @ref HAL_TIM_PeriodElapsedCallback function that lies inside this module is added into that other main
 *          program or external library (where you will probably have to erase the one entire
 *          @ref HAL_TIM_PeriodElapsedCallback function that lies inside this module to guarantee that the compiled
 *          program works as expected).
 *
 * @details <b><u>Code Example for displaying all the ASCII Characters that can be shown at the 5641AS 7-segment Display
 *          Device via the @ref display_5641as :</u></b>
 *
 * @code
  #include <stdio.h>	// Library from which "printf" is located at.
  #include <stdint.h> // This library contains the aliases: uint8_t, uint16_t, uint32_t, etc.
  #include "5641as_display_driver.h" // This custom Mortrack's library contains the functions, definitions and variables that together operate as the driver for the 5641AS 7-segment Display Device.

  // ################################################################# //
  // ##### INITIALIZATION OF THE 5641AS 7-SEGMENT DISPLAY MODULE ##### //
  // ################################################################# //
  // Populate the 5641AS 7-segment Display Device's Peripherals Definition parameters structure.
  Display_5641AS_peripherals_def_t display_peripherals;
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

  // Define the desired On-Time and Off-Time steps
  // NOTE:  For this particular case, I will consider a PWM Period of 10 Time steps, where the On-Time will be of 6 Time
  //        steps and, therefore, the Off-Time will be of 4 Time steps.
  // NOTE:  In consideration that the Timer to be used in non-interrupt mode by the 5641AS 7-segment Display Driver
  //        module will be triggered by this example each 4'807Hz approximately (meaning that each 7-segment display of
  //        that device will be refreshed at each 120Hz approximately), this will allow this module to output a
  //        sufficiently fast refresh rate at the 5641AS Device so that the human eye is able to see as if all the
  //        7-segment displays of that device are being simultaneously activated.
  uint32_t on_time_steps = 6;
  uint32_t off_time_steps = 4;

  // Initialize the 5641AS 7-segment Display Driver module.
  init_5641as_display_module(&htim2, &display_peripherals, on_time_steps, off_time_steps);

  // ############################################################################################################## //
  // ##### SHOWING ALL THE VALID ASCII CHARACTERS OF THE 5641AS 7-SEGMENT DISPLAY MODULE AT THE 5641AS DEVICE ##### //
  // ############################################################################################################## //
  Display_5641AS_Status ret;
  uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE];
  for (uint16_t i=0; i<266; i++)
  {
    display_output[0] = i;
    display_output[1] = i;
    display_output[2] = i;
    display_output[3] = i;
    ret = set_5641as_display_output(display_output);
    if (ret != Display_5641AS_EC_OK)
    {
      printf("DEBUG-ERROR: An invalid ASCII Character in its decimal value of %d has been given to the 5641AS 7-segment Display Driver module.\r\n", i);
    }
    else
   {
      printf("DEBUG-INFO: The ASCII Character in its decimal value of %d is valid in the 5641AS 7-segment Display Driver module.\r\n", i);
      HAL_Delay(5000);
    }
  }
  while(1); // Stop the program here.
 * @endcode
 *
 * @author 	Cesar Miranda Meza (cmirandameza3@hotmail.com)
 * @date	January 30, 2024.
 * @date    LAST UPDATE: February 06, 2024.
 */

#ifndef DISPLAY_5641AS_DRIVER_H_
#define DISPLAY_5641AS_DRIVER_H_

#include "stm32f1xx_hal.h" // This is the HAL Driver Library for the STM32F1 series devices. If yours is from a different type, then you will have to substitute the right one here for your particular STMicroelectronics device. However, if you cant figure out what the name of that header file is, then simply substitute this line of code by: #include "main.h"
#include <stdint.h> // This library contains the aliases: uint8_t, uint16_t, uint32_t, etc.

#define DISPLAY_5641AS_CHARACTERS_SIZE      (4)        /**< @brief Total maximum characters that can be shown in the 5641AS 7-segment Display Device simultaneously (i.e., the total number of available 7-segment displays in the device). */

/**@brief	5641AS 7-segment Display Driver Exception codes.
 *
 * @details	These Exception Codes are returned by the functions of the @ref display_5641as to indicate the resulting
 *          status of having executed the process contained in each of those functions. For example, to indicate that
 *          the process executed by a certain function was successful or that it has failed.
 */
typedef enum
{
    Display_5641AS_EC_OK    = 0U,    //!< 5641AS Display Driver Process was successful.
    //Display_5641AS_EC_STOP  = 1U,    //!< 5641AS Display Driver Process has been stopped.
    //Display_5641AS_EC_NR	= 2U,	 //!< 5641AS Display Driver Process has concluded with no response.
    //Display_5641AS_EC_NA    = 3U,    //!< 5641AS Display Driver Data received or to be received Not Applicable.
    Display_5641AS_EC_ERR   = 4U     //!< 5641AS Display Driver Process has failed.
} Display_5641AS_Status;

/**@brief	GPIO Definition parameters structure of the @ref display_5641as .
 *
 * @details This structure is used in the @ref Display_5641AS_peripherals_def_t structure for persisting, in the
 *          @ref display_5641as , the GPIO peripherals of our MCU/MPU at which the terminals of the 5641AS 7-segment
 *          Display device will be connected to.
 */
typedef struct __attribute__ ((__packed__)) {
    GPIO_TypeDef *GPIO_Port;	//!< Type Definition of the GPIO peripheral port to which this @ref HM10_GPIO_def_t structure will be associated with.
    uint16_t GPIO_Pin;			//!< Pin number of the GPIO peripheral from to this @ref HM10_GPIO_def_t structure will be associated with.
} Display_5641AS_GPIO_def_t;

/**@brief	5641AS 7-segment Display Device's Peripherals Definition parameters structure.
 *
 * @details This contains all the fields required to associate the corresponding GPIO pins of our MCU towards which the
 *          terminals of the 5641AS 7-segment Display Device are connected to.
 */
typedef struct __attribute__ ((__packed__)) {
    Display_5641AS_GPIO_def_t A;	//!< Type Definition of the GPIO peripheral port to which the A terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t B;	//!< Type Definition of the GPIO peripheral port to which the B terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t C;	//!< Type Definition of the GPIO peripheral port to which the C terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t D;	//!< Type Definition of the GPIO peripheral port to which the D terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t E;	//!< Type Definition of the GPIO peripheral port to which the E terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t F;	//!< Type Definition of the GPIO peripheral port to which the F terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t G;	//!< Type Definition of the GPIO peripheral port to which the G terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t Dp;	//!< Type Definition of the GPIO peripheral port to which the Dp terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t K1;	//!< Type Definition of the GPIO peripheral port to which the K1 terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t K2;	//!< Type Definition of the GPIO peripheral port to which the K2 terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t K3;	//!< Type Definition of the GPIO peripheral port to which the K3 terminal of the 5641AS 7-segment Display device is connected to.
    Display_5641AS_GPIO_def_t K4;	//!< Type Definition of the GPIO peripheral port to which the K4 terminal of the 5641AS 7-segment Display device is connected to.
} Display_5641AS_peripherals_def_t;

/**@brief	Gets the ASCII characters that are expected to be currently shown at the 5641AS 7-segment Display Device.
 *
 * @param[out] display_output   Pointer towards where ASCII characters expected to be currently shown at the 5641AS
 *                              7-segment Display Device, will be stored.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 * @date    LAST UPDATE: February 03, 2024.
 */
void get_5641as_display_output(uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE]);

/**@brief	Sets the ASCII characters that is desired to be currently shown at the 5641AS 7-segment Display Device.
 *
 * @param[out] display_output   Pointer towards where ASCII characters that wants to be currently shown at the 5641AS
 *                              7-segment Display Device are stored at. Note that the ASCII characters has a size of
 *                              @ref DISPLAY_5641AS_CHARACTERS_SIZE bytes.
 *
 * @retval  Display_5641AS_EC_OK    If the desired ASCII Characters to be shown at the 5641AS 7-segment Display Device
 *                                  are successfully persisted into the @ref display_5641as .
 * @retval  Display_5641AS_EC_ERR   If the \p display_output param contains ASCII Characters not supported by the
 *                                  @ref display_5641as (To learn about the valid ASCII Characters, see
 *                                  @ref Display_5641AS_Supported_ASCII_Characters ).
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 * @date    LAST UPDATE: February 03, 2024.
 */
Display_5641AS_Status set_5641as_display_output(uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE]);

/**@brief	Gets the currently configured number of steps during which each 7-segment display of the 5641AS Device is
 *          being electrically turned On at the moment.
 *
 * @note    Each step equals the elapsed time at which the Interrupt Callback of the Timer used by @ref display_5641as
 *          will be called.
 *
 * @return  The currently configured number of steps during which each 7-segment display of the 5641AS Device is
 *          being electrically turned On at the moment.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
uint32_t get_display_on_time_steps();

/**@brief	Sets the desired number of steps during which it is wanted that each 7-segment display of the 5641AS Device
 *          is electrically turned On at the moment.
 *
 * @note    Each step equals the elapsed time at which the Interrupt Callback of the Timer used by @ref display_5641as
 *          will be called.
 *
 * @param on_time_steps     Desired number of steps during which each 7-segment display of the 5641AS Device will be
 *                          electrically turned On, where each step will equal the elapsed time at which the Interrupt
 *                          Callback of the \p htim param Timer will be called.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
void set_display_on_time_steps(uint32_t on_time_steps);

/**@brief	Gets the currently configured number of steps during which each 7-segment display of the 5641AS Device is
 *          being electrically turned Off at the moment.
 *
 * @note    Each step equals the elapsed time at which the Interrupt Callback of the Timer used by @ref display_5641as
 *          will be called.
 *
 * @return  The currently configured number of steps during which each 7-segment display of the 5641AS Device is
 *          being electrically turned Off at the moment.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
uint32_t get_display_off_time_steps();

/**@brief	Sets the desired number of steps during which it is wanted that each 7-segment display of the 5641AS Device
 *          is electrically turned Off at the moment.
 *
 * @note    Each step equals the elapsed time at which the Interrupt Callback of the Timer used by @ref display_5641as
 *          will be called.
 *
 * @param off_time_steps    Desired number of steps during which each 7-segment display of the 5641AS Device will be
 *                          electrically turned Off, where each step will equal the elapsed time at which the Interrupt
 *                          Callback of the \p htim param Timer will be called.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
void set_display_off_time_steps(uint32_t off_time_steps);

/**@brief   Start the Timer's Base generation in Interrupt Mode of the given Timer that the @ref display_5641as has been
 *          configured with when initializing it via the @ref init_5641as_display_module function.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    February 06, 2024.
 */
void start_5641as_display_module(void);

/**@brief   Stops the Timer's Base generation in Interrupt Mode of the given Timer that the @ref display_5641as has been
 *          configured with when initializing it via the @ref init_5641as_display_module function.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    February 06, 2024.
 */
void stop_5641as_display_module(void);

/**@brief   Initializes the @ref display_5641as in order to be able to use its provided functions and also starts that
 *          module.
 *
 * @details This function will also update several of its Global Static Pointers and Variables that are used in the
 *          @ref display_5641as and will then start the Timer's Base generation in Interrupt Mode of the given Timer via
 *         the \p htim param.
 *
 * @note    This Global Static Pointers and variables that will be persisted in the @ref display_5641as by this function
 *          are the following:<br>
 *          - The @ref p_htim Global Static Pointer to point to the address towards which the \p htim param points to.
 *          - The @ref p_display_peripherals Global Static Pointer to point to the address towards which the \p peripherals param points to.
 *          - The @ref display_on_time_steps Global Static Variable with the value of the \p on_time_steps param.
 *          - The @ref display_off_time_steps Global Static Variable with the value of the \p off_time_steps param.
 *
 * @note    <b>This function must be called only once</b> before calling any other function of the @ref display_5641as
 *          and does not require to call the @ref start_5641as_display_module after that since this
 *          @ref init_5641as_display_module function already takes care of that.
 *
 * @param[in] htim          Pointer to the Timer is desired for the @ref display_5641as to use for starting and using
 *                          its corresponding Timer's Base generation in Interrupt Mode in order for the
 *                          @ref display_5641as to count and simulate the On and Off times a PWM for each of the
 *                          7-segment Displays of the 5641AS Device.
 * @param[in] peripherals   Pointer to the 5641AS 7-segment Display Device's Peripherals Definition parameters structure
 *                          that should contain the required data of the GPIO Peripheral Pins at which the 5641AS
 *                          7-segment Display Device is expected to be connected at.
 * @param on_time_steps     Desired number of steps during which each 7-segment display of the 5641AS Device will be
 *                          electrically turned On, where each step will equal the elapsed time at which the Interrupt
 *                          Callback of the \p htim param Timer will be called.
 * @param off_time_steps    Desired number of steps during which each 7-segment display of the 5641AS Device will be
 *                          electrically turned Off, where each step will equal the elapsed time at which the Interrupt
 *                          Callback of the \p htim param Timer will be called.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 * @date    LAST UPDATE: February 06, 2024.
 */
void init_5641as_display_module(TIM_HandleTypeDef *htim, Display_5641AS_peripherals_def_t *peripherals, uint32_t on_time_steps, uint32_t off_time_steps);

#endif /* DISPLAY_5641AS_DRIVER_H_ */

/** @} */
