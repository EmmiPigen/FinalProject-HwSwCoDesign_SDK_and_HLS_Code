#include "../utilities/systemDefinitions.h"

struct sensorData {
  int vS;
  bool is_active;
};

void sensor_Module(sensorData *sensor_data);
