#ifndef CCCP_CFG_H
#define CCCP_CFG_H

/* ---------- CP PWM ---------- */
#define CCCP_CP_FREQ_MIN_HZ             900.0f
#define CCCP_CP_FREQ_MAX_HZ            1100.0f

#define CCCP_CP_PWM_5PCT_MIN             0.045f
#define CCCP_CP_PWM_5PCT_MAX             0.055f
#define CCCP_CP_PWM_MIN_VALID            0.03f
#define CCCP_CP_PWM_MAX_VALID            0.97f

/* ---------- CP voltage windows ----------
 * Example engineering windows only.
 * Replace with OEM/project calibrated values.
 */
#define CCCP_CP_A_MIN_V                 10.5f
#define CCCP_CP_A_MAX_V                 13.5f
#define CCCP_CP_B_MIN_V                  7.5f
#define CCCP_CP_B_MAX_V                 10.5f
#define CCCP_CP_C_MIN_V                  4.5f
#define CCCP_CP_C_MAX_V                  7.5f
#define CCCP_CP_0V_MIN_V                -1.5f
#define CCCP_CP_0V_MAX_V                 1.5f
#define CCCP_CP_NEG12_MAX_V             -1.5f

/* ---------- state timing ---------- */
#define CCCP_TASK_1MS                    1U
#define CCCP_TASK_10MS                  10U
#define CCCP_CP_CONFIRM_MS              30U
#define CCCP_READY_CONFIRM_MS           30U
#define CCCP_CP_INVALID_TIMEOUT_MS     100U

/* ---------- OBC project defaults ---------- */
#define CCCP_OBC_RATED_CURRENT_A        30.0f
#define CCCP_OBC_RATED_POWER_W        6600.0f

/* ---------- CP ADC scaling example ---------- */
#define CCCP_ADC_VREF_V                  3.3f
#define CCCP_ADC_COUNTS_MAX           4095.0f
#define CCCP_CP_R_TOP_OHM           30000.0f
#define CCCP_CP_R_BOTTOM_OHM        10000.0f

/* ---------- CC ADC scaling example ----------
 * Replace with actual CC front-end.
 */
#define CCCP_CC_R_TOP_OHM            10000.0f
#define CCCP_CC_R_BOTTOM_OHM         10000.0f

/* Example cable resistor table.
 * Replace by actual project/OEM table.
 */
#define CCCP_CC_R_680_OHM              680.0f
#define CCCP_CC_R_220_OHM              220.0f
#define CCCP_CC_R_100_OHM              100.0f
#define CCCP_CC_TOL_PCT                 10.0f

#endif
