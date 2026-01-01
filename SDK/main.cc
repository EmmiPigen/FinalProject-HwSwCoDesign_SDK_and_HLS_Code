/*
 * Alarm System Application with Pipelining
 */

#include <cstdlib>
#include <sleep.h>
#include <xgpio.h>
#include <xil_printf.h>
#include <xintrusion_checker.h>
#include <xparameters.h>
#include <xtime_l.h>
#include <stdio.h>

// User defined includes
#include "Utilities/systemDefinitions.h"
#include "camera/cameraModule.h"
#include "sensors/sensorsModule.h"

sensorData sensor_data1 = {0, true};
sensorData sensor_data2 = {0, true};
cameraData camera_data = {{0}, true};

XIntrusion_checker intrusion_checker;
XIntrusion_checker_Config *intrusion_checker_cfg;

XGpio buttons_gpio;
XGpio leds_gpio;

unsigned int manual_ms_counter = 0;
u32 prev_buttons = 0;
u32 last_sensors_read_ms = 0;
u32 last_camera_read_ms = 0;
bool auto_advance_time = false;

XTime Start_cycle, End_cycle;
double time_diff_add = 0.0;
int time_diff_count = 0;

typedef enum {
  STATE_INACTIVE = 0,
  STATE_ACTIVE,
  STATE_ALARMED
} AlarmState;

AlarmState current_state = STATE_INACTIVE;

void log_state_transistion(AlarmState from, AlarmState to) {
  xil_printf("STATE TRANSITION: %d -> %d\r\n\n", from, to);
}

unsigned int now() {
  return manual_ms_counter;
}

void advance_time(unsigned int ms) {
  manual_ms_counter += ms;
  xil_printf("------- Current time: %u ms -------\r\n", now());
}

void write_camera_data_to_hardware(XIntrusion_checker *dev, int i, int j, u32 value) {
  if (i < 0 || i >= N || j < 0 || j >= N) {
    xil_printf("Error: Camera data index out of bounds\r\n");
    return;
  }

  int linear_index = (i * N) + j;                     // Convert 2D index to linear index
  UINTPTR offset = 0x028 + (UINTPTR)linear_index * 8; // Each entry is 8 bytes apart

  XIntrusion_checker_WriteReg(dev->Ctrl_bus_BaseAddress, offset, value); // Write the value to the calculated offset
}

void run_sensor_module() {
  sensor_Module(&sensor_data1);
  sensor_Module(&sensor_data2);
  xil_printf("Generated Sensor Data: vS1=%d, vS2=%d\r\n", sensor_data1.vS, sensor_data2.vS);

  // Write sensor data to hardware
  XIntrusion_checker_Set_vS1(&intrusion_checker, (u32)sensor_data1.vS);
  XIntrusion_checker_Set_vS2(&intrusion_checker, (u32)sensor_data2.vS);
}

void run_camera_module() {
  camera_Module(&camera_data);
  xil_printf("Generated Camera Data:\r\n");

  for (int i = 0; i < N; i++) {
    for (int j = 0; j < N; j++) {
      write_camera_data_to_hardware(&intrusion_checker, i, j, (u32)camera_data.vC[i][j]);
    }
  }
}

void run_intrusion_checker() {
  xil_printf("Running Intrusion Checker Hardware Module...\r\n");

  // Mesure the start cycle count
  XTime_GetTime(&Start_cycle);

  XIntrusion_checker_Start(&intrusion_checker);
  while (!XIntrusion_checker_IsDone(&intrusion_checker)) {
  }

  // Retrieve results if needed
  u32 result = XIntrusion_checker_Get_return(&intrusion_checker);
  xil_printf("      Intrusion Checker Result: %u\r\n", result);

  // Mesure the end cycle count
  XTime_GetTime(&End_cycle);
  double total_time_ms = (double)(End_cycle - Start_cycle) / (COUNTS_PER_SECOND);
  time_diff_add += total_time_ms;
  time_diff_count++;
  printf("      Intrusion Checker Execution Time (ms): %f ms\r\n", total_time_ms * 1000);
  printf("      Average Execution Time over %d runs (ms): %f ms\r\n", time_diff_count, (time_diff_add / time_diff_count) * 1000);

  if (result) {
    xil_printf("ALARM! Intrusion Detected!\r\n");
    log_state_transistion(STATE_ACTIVE, STATE_ALARMED);
    current_state = STATE_ALARMED;
  } else {
    xil_printf("\n ------- No Intrusion Detected. -------\r\n");
  }
}

int read_pin_code() {
  xil_printf("Reading Pin Code...\r\n");
  return rand() % 10; // Simulate reading a pin code (0-9)
}

bool isValid(int P) {
  if (P % 2 == 0) {
    return true;
  } else {
    return false;
  }
}

void initialize_platform() {
  // ------- Initialize Intrusion Checker Hardware Module -------
  intrusion_checker_cfg = XIntrusion_checker_LookupConfig(XPAR_INTRUSION_CHECKER_0_DEVICE_ID);
  XIntrusion_checker_CfgInitialize(&intrusion_checker, intrusion_checker_cfg);

  // ------- Initialize GPIO for buttons -------
  XGpio_Initialize(&buttons_gpio, BUTTONS_DEVICE_ID);
  XGpio_SetDataDirection(&buttons_gpio, BUTTONS_CHANNEL, 0xFF); // Set all bits as input

  // ------- Initialize GPIO for LEDs -------
  XGpio_Initialize(&leds_gpio, LEDS_DEVICE_ID);
  XGpio_SetDataDirection(&leds_gpio, LEDS_CHANNEL, 0x00); // Set all bits as output
}

void handle_inactive_state() {
  xil_printf("------- STATE: INACTIVE -------\r\n");
  int pin_code = read_pin_code();

  if (isValid(pin_code)) {
    xil_printf("      Valid Pin Code Entered. Transitioning to ACTIVE state.\r\n");
    log_state_transistion(STATE_INACTIVE, STATE_ACTIVE);
    current_state = STATE_ACTIVE;
  } else {
    xil_printf("      Invalid Pin Code. Remaining in INACTIVE state.\r\n");
  }
  usleep(10000 * 100); // Sleep for 1 second
}

void handle_active_state() {
  // The buttons are for advancing time and toggling auto-advance and is just for testing purposes
  u32 buttons = XGpio_DiscreteRead(&buttons_gpio, BUTTONS_CHANNEL);

  if ((buttons & 0x8) && !(prev_buttons & 0x8)) {
    xil_printf("\nButton Pressed: advancing time\r\n");
    advance_time(TIME_STEP_MS);
  }

  if ((buttons & 0x4) && !(prev_buttons & 0x4)) {
    if (!auto_advance_time) {
      xil_printf("------- Auto-advance time every %u ms ENABLED -------\r\n", TIME_STEP_MS);
      auto_advance_time = true;
    } else {
      xil_printf("------- Auto-advance time DISABLED -------\r\n");
      auto_advance_time = false;
    }
  }

  prev_buttons = buttons;
  if (auto_advance_time) {
    advance_time(TIME_STEP_MS);
  }

  // The actual alarm system logic starts here
  // Run the sensor if the sampling interval has passed
  if ((now() >= SENSOR_SAMPLING_INTERVAL_MS + last_sensors_read_ms) && (now() != last_sensors_read_ms)) {
    last_sensors_read_ms = now(); // Update the last read time
    run_sensor_module();          // Run the sensor module
    usleep(100);
  }
  // Run the camera if the sampling interval has passed
  if ((now()) >= CAMERA_SAMPLING_INTERVAL_MS + last_camera_read_ms && (now() != last_camera_read_ms)) {
    last_camera_read_ms = now(); // Update the last read time
    run_camera_module();         // Run the camera module
    usleep(100);
    run_intrusion_checker(); // Run the intrusion checker HLS IP core
  }

  // And ends here

  if (auto_advance_time) {
    usleep((TIME_STEP_MS + 100) * 1000);
    xil_printf("\r\n");
  }
}

void handle_alarmed_state() {
  xil_printf("------- STATE: ALARMED -------\r\n");
  usleep(500 * 1000);                                  // Sleep for 0.5 seconds
  XGpio_DiscreteWrite(&leds_gpio, LEDS_CHANNEL, 0xFF); // Turn on all LEDs to indicate alarm

  int t_start = now();
  while (now() - t_start <= 10000) { // Stay in ALARMED state for 10 seconds
    int pin_code = read_pin_code();
    if (isValid(pin_code)) {
      xil_printf("      Valid Pin Code Entered. Transitioning to ACTIVE state.\r\n");
      log_state_transistion(STATE_ALARMED, STATE_ACTIVE);
      current_state = STATE_ACTIVE;
      // Turn off LEDs
      XGpio_DiscreteWrite(&leds_gpio, LEDS_CHANNEL, 0x00);
      return;
    }

    xil_printf("      Invalid Pin Code. Remaining in ALARMED state.\r\n");
    usleep(1500 * 1000); // Sleep for 1.5 seconds
    advance_time(1500);
  }
  xil_printf("      Alarm duration elapsed. Transitioning to ACTIVE state.\r\n");
  log_state_transistion(STATE_ALARMED, STATE_ACTIVE);
  current_state = STATE_ACTIVE;

  // Turn off LEDs
  XGpio_DiscreteWrite(&leds_gpio, LEDS_CHANNEL, 0x00);
}

int main() {
  xil_printf("Initializing Alarm System Application with Pipelining...\r\n");
  initialize_platform();
  XTime_SetTime(0); // Initialize the cycle counter

  while (1) {
    switch (current_state) {
    case STATE_INACTIVE:
      handle_inactive_state();
      break;
    case STATE_ACTIVE:
      handle_active_state();
      break;
    case STATE_ALARMED:
      handle_alarmed_state();
      break;
    default:
      xil_printf("Error: Unknown State\r\n");
      break;
    }
  }
  return 0;
}
