#include "sensorsModule.h"
#include "cstdlib"

void sensor_Module(sensorData *sensor_data) {
  if (sensor_data->is_active) {
    sensor_data->vS = rand() % 5; // Generate random value between 0 and 5
  }
}
