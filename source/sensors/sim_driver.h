/*******************************************************************************
 * File Name 	: sim_driver.h
 *
 * Description 	: Simulated environment sensors (temperature, humidity) for
 *                testing and when real sensors are unavailable. Call
 *                sim_seed() once before using the random generators.
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

#ifndef SIM_DRIVER_H_
#define SIM_DRIVER_H_

/*******************************************************************************
* Function Prototypes
********************************************************************************/
/** Seed the simulator PRNG. Call once (e.g. at task start) before using the rand functions. */
void sim_seed(unsigned int seed);

/** Random temperature in °C (default range 15–35). */
float sim_rand_temperature(void);

/** Random humidity in percent (default range 30–90). */
unsigned int sim_rand_humidity(void);

#endif /* SIM_DRIVER_H_ */
