#include "cameraModule.h"
#include "cstdlib"

void camera_Module(cameraData *camera_data) {
  if (camera_data->is_active) {
    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        camera_data->vC[i][j] = rand() % 10; // Generate random value between 0 and 9
      }
    }
  }
}