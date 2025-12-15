#include <iostream>
#include <stdio.h>
#include <chrono>

#include "IC.h"

#define INPUT_FILE "ic_input.dat"
#define GOLDEN_FILE "ic_golden.dat"
#define N 9

int main(){
  int vS1_tb_1;
  int vS2_tb_1;
  int vC_tb_1[N][N];
  int total_Im_sum_golden_1;
  bool alarm_flag_golden_1;

  int vS1_tb_2;
  int vS2_tb_2;
  int vC_tb_2[N][N];
  int total_Im_sum_golden_2;
  bool alarm_flag_golden_2;

  FILE *fp_in;
  FILE *fp_golden;

  int pass = -1;

  // Read input and golden files
  std::cout << "Reading input and golden files..." << std::endl;
  fp_in = fopen(INPUT_FILE, "r");
  fp_golden = fopen(GOLDEN_FILE, "r");

  fscanf(fp_in, "%d %d", &vS1_tb_1, &vS2_tb_1);
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      fscanf(fp_in, "%d", &vC_tb_1[i][j]);
    }
  }
  fscanf(fp_golden, "%d", &alarm_flag_golden_1);
  fscanf(fp_golden, "%d", &total_Im_sum_golden_1);

  fscanf(fp_in, "%d %d", &vS1_tb_2, &vS2_tb_2);
  for(int i = 0; i < N; i++){
    for(int j = 0; j < N; j++){
      fscanf(fp_in, "%d", &vC_tb_2[i][j]);
    }
  }

  fscanf(fp_golden, "%d", &alarm_flag_golden_2);
  fscanf(fp_golden, "%d", &total_Im_sum_golden_2);

  fclose(fp_in);
  fclose(fp_golden);

  std::cout << "Starting testbench..." << std::endl;
  // Test case 1
  std::cout << "Running test case 1..." << std::endl;
  std::cout << "Expected total_Im_sum: " << total_Im_sum_golden_1 << std::endl;
  bool alarm_flag_tb_1 = intrusion_checker(vS1_tb_1, vS2_tb_1, vC_tb_1);
  std::cout << "Flag: " << alarm_flag_tb_1 << std::endl;

  if (alarm_flag_tb_1 == alarm_flag_golden_1){
    std::cout << "Test case 1 PASSED!" << std::endl;
    pass++;
  } else {
    std::cout << "Test case 1 FAILED!" << std::endl;
  }

  // Test case 2
  std::cout << "Running test case 2..." << std::endl;
  std::cout << "Expected total_Im_sum: " << total_Im_sum_golden_2 << std::endl;
  bool alarm_flag_tb_2 = intrusion_checker(vS1_tb_2, vS2_tb_2, vC_tb_2);
  std::cout << "Flag: " << alarm_flag_tb_2 << std::endl;
  if (alarm_flag_tb_2 == alarm_flag_golden_2){
    std::cout << "Test case 2 PASSED!" << std::endl;
    pass++;
  } else
  {
    std::cout << "Test case 2 FAILED!" << std::endl;
  }

  if (pass == 1){
    std::cout << "All test cases PASSED!" << std::endl;
  } else {
    std::cout << "Some test cases FAILED!" << std::endl;
  }
  
}
