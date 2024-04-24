/** @addtogroup display_5641as
 * @{
 */

#include "5641as_display_driver.h"
#include <string.h>	// Library from which "memset()" and "memcpy()" are located at.

#define SEVEN_SEGMENT_DISPLAY_5641AS_LEDS_SIZE      (8)        /**< @brief Total maximum LEDs available in single 7-segment Display of the 5641AS Device (including the Dp LED). */

static TIM_HandleTypeDef *p_htim;                                                                    /**< @brief Pointer to the Timer Handle Structure of the Timer that will be used in this @ref display_5641as to refresh/update the value shown at the 5641AS 7-segment Display Module. @details This pointer's value is defined in the @ref init_5641as_display_module function. */
static Display_5641AS_peripherals_def_t *p_display_peripherals;                                      /**< @brief Pointer to the 5641AS 7-segment Display Device's Peripherals Definition Structure that will be used in this @ref display_5641as to control the GPIO Peripherals towards which the terminals of the 5641AS 7-segment Display device are connected to. @details This pointer's value is defined in the @ref init_5641as_display_module function. */
static uint16_t display_5641as_output[DISPLAY_5641AS_CHARACTERS_SIZE] = {'\0', '\0', '\0', '\0'};    /**< @brief Pointer towards where the the desired ASCII characters currently shown at the 5641AS 7-segment Display is stored in the Memory. */
static uint8_t currently_enabled_5641as_7segment_display = 0;                                        /**< @brief Global Variable that indicates the currently enabled 7-segment Display from the 5641AS Device. @details The values contained in this variable should be the following:<br><br>* 0 = K1 5641AS 7-segment display is currently enabled.<br>* 1 = K2 5641AS 7-segment display is currently enabled.<br>* 2 = K3 5641AS 7-segment display is currently enabled.<br>* 3 = K4 5641AS 7-segment display is currently enabled. */
static uint32_t display_on_time_steps;                                                               /**< @brief Global Variable that will hold the desired number of steps during which each Display Character will be electrically turned On. @note One step equals the elapsed time at which the Interrupt Callback of the Timer of the @ref p_htim pointer is called. @details This pointer's value is defined in the @ref init_5641as_display_module function. */
static uint32_t display_off_time_steps;                                                              /**< @brief Global Variable that will hold the desired number of steps during which each Display Character will be electrically turned Off. @note One step equals the elapsed time at which the Interrupt Callback of the Timer of the @ref p_htim pointer is called. @details This pointer's value is defined in the @ref init_5641as_display_module function. */
static uint32_t current_display_on_time_step = 0;                                                    /**< @brief Global Variable that will hold the current time step, with respect to @ref display_on_time_steps , at which the @ref display_5641as is at whenever turning On the corresponding LEDs from the 5641AS 7-segment Display Device as stated at @ref display_5641as_output . @note The idea of this Global Variable, together with the @ref current_display_off_time_step Global Variable is to simulate/generate a PWM output for each of the 7-segment Displays contained in the 5641AS Device. */
static uint32_t current_display_off_time_step = 0;                                                   /**< @brief Global Variable that will hold the current time step, with respect to @ref display_off_time_steps , at which the @ref display_5641as is at whenever turning Off all the LEDs from the 5641AS 7-segment Display Device. @note The idea of this Global Variable, together with the @ref current_display_on_time_step Global Variable is to simulate/generate a PWM output for each of the 7-segment Displays contained in the 5641AS Device. */

/**@brief	ASCII code character definitions that are available in the @ref display_5641as .
 *
 * @note    These definitions are defined with respect to the decimal value that corresponds for each of the available
 *          ASCII code characters.
 */
typedef enum
{
    Command_NULL_in_ASCII                   = 0,     //!< \f$[NULL]_{ASCII} = 0_d\f$.
    Command_space_in_ASCII                  = 32,    //!< \f$[SPACE]_{ASCII} = 32_d\f$.
    //Letter_exclamation_mark_in_ASCII        = 33,    //!< \f$!_{ASCII} = 48_d\f$.
    Letter_double_quotation_mark_in_ASCII   = 34,    //!< \f$"_{ASCII} = 34_d\f$.
    //Letter_number_sign_in_ASCII             = 35,
    //Letter_dollar_sign_in_ASCII             = 36,
    //Letter_percentage_sign_in_ASCII         = 37,
    //Letter_ampersand_in_ASCII               = 38,
    Letter_single_quotation_mark_in_ASCII   = 39,    //!< \f$'_{ASCII} = 39_d\f$.
    //Letter_open_parenthesis_in_ASCII        = 40,
    //Letter_close_parenthesis_in_ASCII       = 41,
    //Letter_asterisk_in_ASCII                = 42,
    //Letter_plus_sign_in_ASCII               = 43,
    //Letter_comma_sign_in_ASCII              = 44,
    Letter_minus_sign_in_ASCII              = 45,    //!< \f$-_{ASCII} = 45_d\f$.
    Letter_dot_sign_in_ASCII                = 46,    //!< \f$._{ASCII} = 46_d\f$.
    //Letter_forward_slash_in_ASCII           = 47,
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
    //Letter_colon_mark_in_ASCII              = 58,
    //Letter_semicolon_mark_in_ASCII          = 59,
    //Letter_less_than_sign_in_ASCII          = 60,
    Letter_equals_sign_in_ASCII             = 61,    //!< \f$=_{ASCII} = 61_d\f$.
    //Letter_greater_than_sign_in_ASCII       = 62,
    //Letter_question_mark_in_ASCII           = 63,
    //Letter_At_sign_in_ASCII                 = 64,
    Letter_A_in_ASCII                       = 65,    //!< \f$A_{ASCII} = 65_d\f$.
    Letter_B_in_ASCII                       = 66,    //!< \f$B_{ASCII} = 66_d\f$.
    Letter_C_in_ASCII                       = 67,    //!< \f$C_{ASCII} = 67_d\f$.
    Letter_D_in_ASCII                       = 68,    //!< \f$D_{ASCII} = 68_d\f$.
    Letter_E_in_ASCII                       = 69,    //!< \f$E_{ASCII} = 69_d\f$.
    Letter_F_in_ASCII                       = 70,    //!< \f$F_{ASCII} = 70_d\f$.
    //Letter_G_in_ASCII                       = 71,    //!< \f$G_{ASCII} = 71_d\f$.
    Letter_H_in_ASCII                       = 72,    //!< \f$H_{ASCII} = 72_d\f$.
    //Letter_I_in_ASCII                       = 73,    //!< \f$I_{ASCII} = 73_d\f$.
    //Letter_J_in_ASCII                       = 74,    //!< \f$J_{ASCII} = 74_d\f$.
    //Letter_K_in_ASCII                       = 75,
    Letter_L_in_ASCII                       = 76,    //!< \f$L_{ASCII} = 76_d\f$.
    //Letter_M_in_ASCII                       = 77,
    //Letter_N_in_ASCII                       = 78,
    Letter_O_in_ASCII                       = 79,    //!< \f$O_{ASCII} = 79_d\f$.
    Letter_P_in_ASCII                       = 80,    //!< \f$P_{ASCII} = 80_d\f$.
    //Letter_Q_in_ASCII                       = 81,
    //Letter_R_in_ASCII                       = 82,
    Letter_S_in_ASCII                       = 83,    //!< \f$S_{ASCII} = 83_d\f$.
    //Letter_T_in_ASCII                       = 84,
    Letter_U_in_ASCII                       = 85,    //!< \f$U_{ASCII} = 85_d\f$.
    //Letter_V_in_ASCII                       = 86,
    //Letter_W_in_ASCII                       = 87,
    //Letter_X_in_ASCII                       = 88,
    //Letter_Y_in_ASCII                       = 89,
    //Letter_Z_in_ASCII                       = 90,
    Letter_opening_bracket_in_ASCII         = 91,    //!< \f$[_{ASCII} = 91_d\f$.
    //Letter_reverse_slash_in_ASCII           = 92,
    Letter_closing_bracket_in_ASCII         = 93,    //!< \f$]_{ASCII} = 93_d\f$.
    //Letter_caret_in_ASCII                   = 94,
    Letter_underscore_in_ASCII              = 95,    //!< \f$\__{ASCII} = 95_d\f$.
    //Letter_grave_accent_in_ASCII            = 96,
    //Letter_a_in_ASCII                       = 97,
    Letter_b_in_ASCII                       = 98,    //!< \f$b_{ASCII} = 98_d\f$.
    Letter_c_in_ASCII                       = 99,    //!< \f$c_{ASCII} = 99_d\f$.
    Letter_d_in_ASCII                       = 100,   //!< \f$d_{ASCII} = 100_d\f$.
    //Letter_e_in_ASCII                       = 101,
    Letter_f_in_ASCII                       = 102,   //!< \f$f_{ASCII} = 102_d\f$.
    //Letter_g_in_ASCII                       = 103,
    Letter_h_in_ASCII                       = 104,   //!< \f$h_{ASCII} = 104_d\f$.
    Letter_i_in_ASCII                       = 105,   //!< \f$i_{ASCII} = 105_d\f$.
    //Letter_j_in_ASCII                       = 106,
    //Letter_k_in_ASCII                       = 107,
    Letter_l_in_ASCII                       = 108,   //!< \f$l_{ASCII} = 108_d\f$.
    //Letter_m_in_ASCII                       = 109,
    Letter_n_in_ASCII                       = 110,   //!< \f$n_{ASCII} = 110_d\f$.
    Letter_o_in_ASCII                       = 111,   //!< \f$o_{ASCII} = 111_d\f$.
    //Letter_p_in_ASCII                       = 112,
    //Letter_q_in_ASCII                       = 113,
    Letter_r_in_ASCII                       = 114,   //!< \f$r_{ASCII} = 114_d\f$.
    //Letter_s_in_ASCII                       = 115,
    Letter_t_in_ASCII                       = 116,   //!< \f$t_{ASCII} = 116_d\f$.
    Letter_u_in_ASCII                       = 117,   //!< \f$u_{ASCII} = 117_d\f$.
    //Letter_v_in_ASCII                       = 118,
    //Letter_w_in_ASCII                       = 119,
    //Letter_x_in_ASCII                       = 120,
    //Letter_y_in_ASCII                       = 121,
    //Letter_z_in_ASCII                       = 122,
    //Letter_opening_curly_brackets_in_ASCII  = 123,
    Letter_vertical_bar_in_ASCII            = 124,   //!< \f$|_{ASCII} = 124_d\f$.
    //Letter_closing_curly_brackets_in_ASCII  = 125,
    //Letter_tilde_in_ASCII                   = 126
    Number_0Dp_in_ASCII	                    = 256,    //!< \f$0._{ASCII} = 256_d\f$ custom value.
    Number_1Dp_in_ASCII	                    = 257,    //!< \f$1._{ASCII} = 257_d\f$ custom value.
    Number_2Dp_in_ASCII	                    = 258,    //!< \f$2._{ASCII} = 258_d\f$ custom value.
    Number_3Dp_in_ASCII	                    = 259,    //!< \f$3._{ASCII} = 259_d\f$ custom value.
    Number_4Dp_in_ASCII	                    = 260,    //!< \f$4._{ASCII} = 260_d\f$ custom value.
    Number_5Dp_in_ASCII	                    = 261,    //!< \f$5._{ASCII} = 261_d\f$ custom value.
    Number_6Dp_in_ASCII	                    = 262,    //!< \f$6._{ASCII} = 262_d\f$ custom value.
    Number_7Dp_in_ASCII	                    = 263,    //!< \f$7._{ASCII} = 263_d\f$ custom value.
    Number_8Dp_in_ASCII	                    = 264,    //!< \f$8._{ASCII} = 264_d\f$ custom value.
    Number_9Dp_in_ASCII	                    = 265     //!< \f$9._{ASCII} = 265_d\f$ custom value.
} Display_5641AS_Supported_ASCII_Characters;

/**@brief	Turns On and Off the desired LEDs of the 7-segment Display from the 5641AS Device that corresponds according
 *          to @ref currently_enabled_5641as_7segment_display .
 *
 * @param A_pin_state   GPIO Pin state to be assigned to the A Terminal of the 5641AS 7-segment Display Device.
 * @param B_pin_state   GPIO Pin state to be assigned to the B Terminal of the 5641AS 7-segment Display Device.
 * @param C_pin_state   GPIO Pin state to be assigned to the C Terminal of the 5641AS 7-segment Display Device.
 * @param D_pin_state   GPIO Pin state to be assigned to the D Terminal of the 5641AS 7-segment Display Device.
 * @param E_pin_state   GPIO Pin state to be assigned to the E Terminal of the 5641AS 7-segment Display Device.
 * @param F_pin_state   GPIO Pin state to be assigned to the F Terminal of the 5641AS 7-segment Display Device.
 * @param G_pin_state   GPIO Pin state to be assigned to the G Terminal of the 5641AS 7-segment Display Device.
 * @param Dp_pin_state  GPIO Pin state to be assigned to the Dp Terminal of the 5641AS 7-segment Display Device.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
static void show_custom_display_output(GPIO_PinState A_pin_state, GPIO_PinState B_pin_state, GPIO_PinState C_pin_state, GPIO_PinState D_pin_state, GPIO_PinState E_pin_state, GPIO_PinState F_pin_state, GPIO_PinState G_pin_state, GPIO_PinState Dp_pin_state);

/**@brief	Turns Off all the LEDs of the 7-segment Display from the 5641AS Device that corresponds according to
 *          @ref currently_enabled_5641as_7segment_display .
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 * @date    LAST UPDATE: February 08, 2024.
 */
static void show_null_display_output();

/**@brief	Turns Off all the terminals of the 5641AS 7-segment Display Device.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
static void turn_off_all_5641as_display_terminals();

void init_5641as_display_module(TIM_HandleTypeDef *htim, Display_5641AS_peripherals_def_t *peripherals, uint32_t on_time_steps, uint32_t off_time_steps)
{
    /* Persist the pointer to the Timer that is desired for the 5641AS 7-segment Display module to use. */
    p_htim = htim;

    /* Persist the pointer to the 5641AS 7-segment Display Device's Peripherals Definition Structure. */
    p_display_peripherals = peripherals;

    /* Persist the desired On-time steps during which each Display Character will be electrically turned On. */
    display_on_time_steps = on_time_steps;

    /* Persist the desired Off-time steps during which each Display Character will be electrically turned Off. */
    display_off_time_steps = off_time_steps;

    /* Starts the given Timer's Base generation in Interrupt mode. */
    start_5641as_display_module();
}

void start_5641as_display_module(void)
{
    HAL_TIM_Base_Start_IT(p_htim);
}

void stop_5641as_display_module(void)
{
    HAL_TIM_Base_Stop_IT(p_htim);
}

void get_5641as_display_output(uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE])
{
    memcpy(display_output, display_5641as_output, DISPLAY_5641AS_CHARACTERS_SIZE);
}

Display_5641AS_Status set_5641as_display_output(uint16_t display_output[DISPLAY_5641AS_CHARACTERS_SIZE])
{
    /* Validate the desired output to be displayed at the 5641AS 7-segment Display Device. */
    for (uint8_t current_display_output=0; current_display_output<DISPLAY_5641AS_CHARACTERS_SIZE; current_display_output++)
    {
        switch (display_output[current_display_output])
        {
            case Command_NULL_in_ASCII:
            case Command_space_in_ASCII:
            case Letter_double_quotation_mark_in_ASCII:
            case Letter_single_quotation_mark_in_ASCII:
            case Letter_minus_sign_in_ASCII:
            case Letter_dot_sign_in_ASCII:
            case Number_0_in_ASCII:
            case Number_1_in_ASCII:
            case Number_2_in_ASCII:
            case Number_3_in_ASCII:
            case Number_4_in_ASCII:
            case Number_5_in_ASCII:
            case Number_6_in_ASCII:
            case Number_7_in_ASCII:
            case Number_8_in_ASCII:
            case Number_9_in_ASCII:
            case Letter_equals_sign_in_ASCII:
            case Letter_A_in_ASCII:
            case Letter_B_in_ASCII:
            case Letter_C_in_ASCII:
            case Letter_D_in_ASCII:
            case Letter_E_in_ASCII:
            case Letter_F_in_ASCII:
            case Letter_H_in_ASCII:
            case Letter_L_in_ASCII:
            case Letter_O_in_ASCII:
            case Letter_P_in_ASCII:
            case Letter_S_in_ASCII:
            case Letter_U_in_ASCII:
            case Letter_opening_bracket_in_ASCII:
            case Letter_closing_bracket_in_ASCII:
            case Letter_underscore_in_ASCII:
            case Letter_b_in_ASCII:
            case Letter_c_in_ASCII:
            case Letter_d_in_ASCII:
            case Letter_f_in_ASCII:
            case Letter_h_in_ASCII:
            case Letter_i_in_ASCII:
            case Letter_l_in_ASCII:
            case Letter_n_in_ASCII:
            case Letter_o_in_ASCII:
            case Letter_r_in_ASCII:
            case Letter_t_in_ASCII:
            case Letter_u_in_ASCII:
            case Letter_vertical_bar_in_ASCII:
            case Number_0Dp_in_ASCII:
            case Number_1Dp_in_ASCII:
            case Number_2Dp_in_ASCII:
            case Number_3Dp_in_ASCII:
            case Number_4Dp_in_ASCII:
            case Number_5Dp_in_ASCII:
            case Number_6Dp_in_ASCII:
            case Number_7Dp_in_ASCII:
            case Number_8Dp_in_ASCII:
            case Number_9Dp_in_ASCII:
                break;
            default:
                return Display_5641AS_EC_ERR;
        }
    }

    /* Persist the desired display output at the 5641AS 7-segment Display Device. */
    for (uint8_t current_display_output=0; current_display_output<DISPLAY_5641AS_CHARACTERS_SIZE; current_display_output++)
    {
        display_5641as_output[current_display_output] = display_output[current_display_output];
    }

    return Display_5641AS_EC_OK;
}

uint32_t get_display_on_time_steps()
{
    return display_on_time_steps;
}

void set_display_on_time_steps(uint32_t on_time_steps)
{
    display_on_time_steps = on_time_steps;
}

uint32_t get_display_off_time_steps()
{
    return display_off_time_steps;
}

void set_display_off_time_steps(uint32_t off_time_steps)
{
    display_off_time_steps = off_time_steps;
}

/**@brief   This is the overridden function @ref HAL_TIM_PeriodElapsedCallback that the STMicroelectronics Library
 *          provides in order for the implementers of that function to state some desired actions for each time whenever
 *          the non-blocking mode is triggered by a desired Timer.
 *
 * @details In this particular case, this overridden function will be used to both count a step of time each moment that
 *          this Callback function is called/triggered and to also simulate a PWM controlled in such a way that the
 *          @ref display_5641as will display the last ASCII characters requested to be shown at the 5641AS 7-segment
 *          Display Device.
 *
 * @note    The ASCII Characters to be shown at the 5641AS 7-segment Display Device can be changed via the
 *          @ref set_5641as_display_output function.
 *
 * @note    This function must not be called by the implementer and the only reason it exists here is because it
 *          overrides the @ref HAL_TIM_PeriodElapsedCallback function provided by the STMicroelectronics Library.
 *
 * @param[in] htim	Pointer to the Timer struct of the actual Timer from which it is desired that the
 *                  @ref display_5641as uses in non-interrupt mode for counting steps of time and simulating a PWM in
 *                  order to controll the output shown at the 5641AS 7-segment Display Device.
 *
 * @author	César Miranda Meza (cmirandameza3@hotmail.com)
 * @date    January 28, 2024.
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    /* Proceed with executing the code for the @ref display_5641as only if the current callback corresponds to the Timer designated to this module. */
    if (htim->Instance == p_htim->Instance)
    {
        /* Generate the corresponding simulated PWM in order to show the lastly requested ASCII Characters at the 5641AS 7-segment Display Device. */
        if (current_display_on_time_step != display_on_time_steps) // If true, then the current time step of the @ref display_5641as is at the On time of the currently simulated PWM output at the 5641AS Device.
        {
            /* Only if this is the first On-Time step of the current PWM Period, then turn On the corresponding LEDs at the 5641AS 7-segment Display Device and maintain them that way in order to show the desired output in it. */
            if (current_display_on_time_step == 0)
            {
                switch (display_5641as_output[currently_enabled_5641as_7segment_display])
                {
                    case Command_NULL_in_ASCII:
                    case Command_space_in_ASCII:
                        show_null_display_output();
                        break;
                    case Letter_double_quotation_mark_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_single_quotation_mark_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_minus_sign_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_dot_sign_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
                        break;
                    case Number_0_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Number_1_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Number_2_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_3_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_4_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_5_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_6_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_7_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Number_8_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Number_9_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_equals_sign_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_A_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_B_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_C_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_D_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_E_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_F_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_H_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_L_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_O_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_P_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_S_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_U_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_opening_bracket_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_closing_bracket_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_underscore_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_b_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_c_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_d_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_f_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_h_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_i_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_l_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_n_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_o_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_r_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_t_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET);
                        break;
                    case Letter_u_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Letter_vertical_bar_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET);
                        break;
                    case Number_0Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET);
                        break;
                    case Number_1Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
                        break;
                    case Number_2Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_3Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_4Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_5Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_6Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_7Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET);
                        break;
                    case Number_8Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    case Number_9Dp_in_ASCII:
                        show_custom_display_output(GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_RESET, GPIO_PIN_RESET, GPIO_PIN_SET, GPIO_PIN_SET, GPIO_PIN_SET);
                        break;
                    default:
                        show_null_display_output(); // This case should not give place ever.
                }
            }

            /* Increase the On-Step-Time of the @ref display_5641as once. */
            current_display_on_time_step++;

            /* Reset the counter of the Current-On-Time-Step and change the currently chosen 7-segment Display of the 5641AS Device to the next one, but only if this is the last Time-Step for the On-Time of the simulated PWM and if that PWM has a 100% Duty Cycle. */
            if ((current_display_on_time_step>=display_on_time_steps) && (display_off_time_steps==0))
            {
                current_display_on_time_step = 0;
                currently_enabled_5641as_7segment_display++;
                if (currently_enabled_5641as_7segment_display == 4)
                {
                    currently_enabled_5641as_7segment_display = 0;
                }
            }
        }
        else // The current time step of the @ref display_5641as is at the Off time of the currently simulated PWM output at the 5641AS Device.
        {
            /* Only if this is the first Off-Time step of the current PWM Period, then turn Off all the LEDs at the 5641AS 7-segment Display Device and maintain them that way in order output the Off Time of the Simulated PWM. */
            if (current_display_off_time_step == 0)
            {
                show_null_display_output();
            }

            /* Increase the Off-Step-Time of the @ref display_5641as once. */
            current_display_off_time_step++;

            /* If this is the last Time-Step for the Off-Time of the simulated PWM, then Reset the counters of the Current-Time-Step (On and Off counters) and change the currently chosen 7-segment Display of the 5641AS Device to the next one. */
            if (current_display_off_time_step >= display_off_time_steps)
            {
                current_display_on_time_step = 0;
                current_display_off_time_step = 0;
                currently_enabled_5641as_7segment_display++;
                if (currently_enabled_5641as_7segment_display == 4)
                {
                    currently_enabled_5641as_7segment_display = 0;
                }
            }
        }
    }
}

static void show_custom_display_output(GPIO_PinState A_pin_state, GPIO_PinState B_pin_state, GPIO_PinState C_pin_state, GPIO_PinState D_pin_state, GPIO_PinState E_pin_state, GPIO_PinState F_pin_state, GPIO_PinState G_pin_state, GPIO_PinState Dp_pin_state)
{
    /* Turn Off all the terminals of the 5641AS 7-segment Display Device. */
    turn_off_all_5641as_display_terminals();

    /* Turn On and Off the LEDs of the 5641AS Device as requested. */
    HAL_GPIO_WritePin(p_display_peripherals->A.GPIO_Port, p_display_peripherals->A.GPIO_Pin, A_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->B.GPIO_Port, p_display_peripherals->B.GPIO_Pin, B_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->C.GPIO_Port, p_display_peripherals->C.GPIO_Pin, C_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->D.GPIO_Port, p_display_peripherals->D.GPIO_Pin, D_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->E.GPIO_Port, p_display_peripherals->E.GPIO_Pin, E_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->F.GPIO_Port, p_display_peripherals->F.GPIO_Pin, F_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->G.GPIO_Port, p_display_peripherals->G.GPIO_Pin, G_pin_state);
    HAL_GPIO_WritePin(p_display_peripherals->Dp.GPIO_Port, p_display_peripherals->Dp.GPIO_Pin, Dp_pin_state);

    /* Enable the current K-Display Output of the 5641AS Device. */
    switch (currently_enabled_5641as_7segment_display)
    {
        case 0:
            HAL_GPIO_WritePin(p_display_peripherals->K1.GPIO_Port, p_display_peripherals->K1.GPIO_Pin, GPIO_PIN_RESET);
            break;
        case 1:
            HAL_GPIO_WritePin(p_display_peripherals->K2.GPIO_Port, p_display_peripherals->K2.GPIO_Pin, GPIO_PIN_RESET);
            break;
        case 2:
            HAL_GPIO_WritePin(p_display_peripherals->K3.GPIO_Port, p_display_peripherals->K3.GPIO_Pin, GPIO_PIN_RESET);
            break;
        default:
            HAL_GPIO_WritePin(p_display_peripherals->K4.GPIO_Port, p_display_peripherals->K4.GPIO_Pin, GPIO_PIN_RESET);
    }
}

static void show_null_display_output()
{
    /* Turn Off all the terminals of the 5641AS 7-segment Display Device. */
    turn_off_all_5641as_display_terminals();
}

static void turn_off_all_5641as_display_terminals()
{
    /* Turn Off all the terminals connected to the 5641AS 7-segment Display Device. */
    HAL_GPIO_WritePin(p_display_peripherals->K1.GPIO_Port, p_display_peripherals->K1.GPIO_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p_display_peripherals->K2.GPIO_Port, p_display_peripherals->K2.GPIO_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p_display_peripherals->K3.GPIO_Port, p_display_peripherals->K3.GPIO_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p_display_peripherals->K4.GPIO_Port, p_display_peripherals->K4.GPIO_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(p_display_peripherals->A.GPIO_Port, p_display_peripherals->A.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->B.GPIO_Port, p_display_peripherals->B.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->C.GPIO_Port, p_display_peripherals->C.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->D.GPIO_Port, p_display_peripherals->D.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->E.GPIO_Port, p_display_peripherals->E.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->F.GPIO_Port, p_display_peripherals->F.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->G.GPIO_Port, p_display_peripherals->G.GPIO_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(p_display_peripherals->Dp.GPIO_Port, p_display_peripherals->Dp.GPIO_Pin, GPIO_PIN_RESET);
}

/** @} */
