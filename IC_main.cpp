#include "IC.h"
#include <iostream>

int intrusion_checker(int vS1, int vS2, data_t vC[N][N]) {
#pragma HLS INTERFACE s_axilite port=vC
#pragma HLS INTERFACE s_axilite port=vS2 bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=vS1 bundle=CTRL_BUS
#pragma HLS INTERFACE s_axilite port=return bundle=CTRL_BUS


  int total_Im_sum = 0;
  int s_sum = vS1 + vS2;

  loop_i: for(int i = 0; i < N; i++) {
    loop_j: for(int j = 0; j < N; j++) {
      total_Im_sum += s_sum * vC[i][j];
  }
}

#ifndef __SYNTHESIS__
  std::cout << "Computed total_Im_sum: " << total_Im_sum << std::endl;
#endif

  return (total_Im_sum >= 1900);
}
