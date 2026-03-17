/*******************************************************************************
 * File Name 	: sensor_task.c
 *
 * Description 	: FreeRTOS task that simulates environment sensors (temperature,
 *                humidity), publishes values over MQTT at fixed interval.
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

#include "sensor_task.h"
#include "i2c_driver.h"
#include "imu_driver.h"
#include "mag_driver.h"
#include "sim_driver.h"
#include "cy_mqtt_api.h"
#include "cy_retarget_io.h"
#include "mqtt_client_config.h"
#include "mqtt_task.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

/*******************************************************************************
 * Macros
 ********************************************************************************/
/* Payload buffer sizes. */
#define SENSOR_PAYLOAD_BUF_SIZE (48u)
#define IMU_PAYLOAD_BUF_SIZE    (96u)
#define MAG_PAYLOAD_BUF_SIZE    (48u)

/*******************************************************************************
 * Global Variables
 ********************************************************************************/
TaskHandle_t sensor_task_handle;

/******************************************************************************
 * Function Name: sensor_task
 ******************************************************************************
 * Summary:
 *  FreeRTOS task that every SENSOR_PUBLISH_INTERVAL_MS (1 s) generates random
 *  temperature and humidity, formats a payload, and publishes it on
 *  MQTT_ENV_TOPIC, MQTT_IMU_TOPIC, and MQTT_MAG_TOPIC.
 *
 * Parameters:
 *  void *pvParameters : Task parameter (unused)
 *
 * Return:
 *  void
 ******************************************************************************/
void sensor_task(void *pvParameters) {
  cy_rslt_t result;
  char payload[SENSOR_PAYLOAD_BUF_SIZE];
  char imu_payload[IMU_PAYLOAD_BUF_SIZE];
  char mag_payload[MAG_PAYLOAD_BUF_SIZE];
  cy_mqtt_publish_info_t publish_info = {
      .qos = (cy_mqtt_qos_t)MQTT_MESSAGES_QOS,
      .topic = MQTT_ENV_TOPIC,
      .topic_len = (sizeof(MQTT_ENV_TOPIC) - 1u),
      .retain = false,
      .dup = false,
      .payload = payload,
      .payload_len = 0u};
  if (i2c_init() != 0) {
    printf("Sensors: I2C init failed, skipping IMU/MAG\n");
  }
  bool imu_ok = (i2c_get() != NULL && imu_init(i2c_get()) == 0);
  int mag_init_rc = -1;
  if (i2c_get() != NULL)
    mag_init_rc = mag_init(i2c_get());
  bool mag_ok = (mag_init_rc == 0) && mag_available();

  (void)pvParameters;

  printf("MAG init: %s (rc=%d)\n", mag_ok ? "ok" : "not found", mag_init_rc);

  sim_seed((unsigned)xTaskGetTickCount());

  for (;;) {
    vTaskDelay(pdMS_TO_TICKS(SENSOR_PUBLISH_INTERVAL_MS));

    /* Read and print environment: real temperature from IMU (BMI270) if available, else simulated; humidity simulated. */
    {
      float temp;
      if (imu_ok && imu_read_temperature(&temp) == 0)
        ; /* use real temp from BMI270 */
      else
        temp = sim_rand_temperature();
      unsigned int humidity = sim_rand_humidity();
      int len = snprintf(payload, sizeof(payload), "temperature=%.1f,humidity=%u",
                         (double)temp, humidity);
      if (len > 0 && (size_t)len < sizeof(payload)) {
        printf("\nENV: %s\n", payload);
        if (mqtt_is_connected()) {
          publish_info.topic     = MQTT_ENV_TOPIC;
          publish_info.topic_len = (sizeof(MQTT_ENV_TOPIC) - 1u);
          publish_info.payload   = payload;
          publish_info.payload_len = (size_t)len;
          result = cy_mqtt_publish(mqtt_connection, &publish_info);
          if (result == CY_RSLT_SUCCESS) {
            printf("  -> published on '%s'\n", MQTT_ENV_TOPIC);
          } else {
            printf("  -> MQTT publish failed 0x%08lX\n", (unsigned long)result);
          }
        } else {
          printf("  -> (not connected, skip MQTT)\n");
        }
      }
    }

    /* Read and always print IMU (accelerometer, gyroscope) if init succeeded. */
    if (imu_ok) {
      imu_accel_gyro_t ag;
      if (imu_read_accel_gyro(&ag) == 0) {
        int len = snprintf(imu_payload, sizeof(imu_payload),
                           "ax=%.2f,ay=%.2f,az=%.2f,gx=%.3f,gy=%.3f,gz=%.3f",
                           (double)ag.accel_x, (double)ag.accel_y, (double)ag.accel_z,
                           (double)ag.gyro_x, (double)ag.gyro_y, (double)ag.gyro_z);
        if (len > 0 && (size_t)len < sizeof(imu_payload)) {
          printf("IMU: %s\n", imu_payload);
          if (mqtt_is_connected()) {
            publish_info.topic     = MQTT_IMU_TOPIC;
            publish_info.topic_len = (sizeof(MQTT_IMU_TOPIC) - 1u);
            publish_info.payload   = imu_payload;
            publish_info.payload_len = (size_t)len;
            result = cy_mqtt_publish(mqtt_connection, &publish_info);
            if (result == CY_RSLT_SUCCESS) {
              printf("  -> published on '%s'\n", MQTT_IMU_TOPIC);
            } else {
              printf("  -> MQTT publish failed 0x%08lX\n", (unsigned long)result);
            }
          } else {
            printf("  -> (not connected, skip MQTT)\n");
          }
        }
      }
    }

    /* Read and print magnetometer (BMM150) if present. */
    if (mag_ok) {
      mag_data_t mag;
      int rc = mag_read(&mag);
      if (rc == 0) {
        int len = snprintf(mag_payload, sizeof(mag_payload),
                           "mx=%.1f,my=%.1f,mz=%.1f",
                           (double)mag.mag_x, (double)mag.mag_y, (double)mag.mag_z);
        if (len > 0 && (size_t)len < sizeof(mag_payload)) {
          printf("MAG: %s\n", mag_payload);
          if (mqtt_is_connected()) {
            publish_info.topic     = MQTT_MAG_TOPIC;
            publish_info.topic_len = (sizeof(MQTT_MAG_TOPIC) - 1u);
            publish_info.payload   = mag_payload;
            publish_info.payload_len = (size_t)len;
            result = cy_mqtt_publish(mqtt_connection, &publish_info);
            if (result == CY_RSLT_SUCCESS) {
              printf("  -> published on '%s'\n", MQTT_MAG_TOPIC);
            } else {
              printf("  -> MQTT publish failed 0x%08lX\n", (unsigned long)result);
            }
          } else {
            printf("  -> (not connected, skip MQTT)\n");
          }
        }
      } else {
        static unsigned mag_fail_print = 0;
        if ((mag_fail_print++ % 50u) == 0u) {
          printf("MAG: read failed rc=%d\n", rc);
        }
      }
    }
  }
}

/* [] END OF FILE */
