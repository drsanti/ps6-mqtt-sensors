/*******************************************************************************
 * File Name 	: sim_driver.c
 *
 * Description 	: Simulated temperature and humidity for environment (ENV)
 *                when real sensors are not used.
 *
 * Author      	: Asst.Prof.Santi Nuratch, Ph.D
 *                INC AUTOMATION
 *                Department of Control Systems and Instrumentation Engineering
 *				  King Mongkut's University of Technology Thonburi (KMUTT)
 *
 * Version     	: 1.0
 * Date         : 17 March 2026
 * Target       : CY8CKIT-062S2-AI PSoC™ 6 AI Evaluation Board
 *
 *******************************************************************************/

#include "sim_driver.h"
#include <stdlib.h>

/*******************************************************************************
* Simulated ranges
********************************************************************************/
#define SIM_TEMP_MIN_C    (15.0f)
#define SIM_TEMP_MAX_C    (35.0f)
#define SIM_HUMIDITY_MIN  (30u)
#define SIM_HUMIDITY_MAX  (90u)

/******************************************************************************
 * Function Name: sim_seed
 ******************************************************************************/
void sim_seed(unsigned int seed)
{
    srand(seed);
}

/******************************************************************************
 * Function Name: sim_rand_temperature
 ******************************************************************************/
float sim_rand_temperature(void)
{
    float t = (float)(rand() % 1001) / 1000.0f; /* 0.0 .. 1.0 */
    return SIM_TEMP_MIN_C + t * (SIM_TEMP_MAX_C - SIM_TEMP_MIN_C);
}

/******************************************************************************
 * Function Name: sim_rand_humidity
 ******************************************************************************/
unsigned int sim_rand_humidity(void)
{
    unsigned int span = SIM_HUMIDITY_MAX - SIM_HUMIDITY_MIN + 1u;
    return SIM_HUMIDITY_MIN + (unsigned int)(rand() % (int)span);
}

/* [] END OF FILE */
