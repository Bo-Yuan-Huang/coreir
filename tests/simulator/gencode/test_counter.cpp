#include <stdint.h>
#include <cstdio>

#include "counter.h"

int main() {
  uint8_t self_en = 1;
  uint16_t self_out;
  uint8_t self_clk = 0;
  uint8_t self_clk_last = 1;
  uint16_t ri_old_value = 0;

  //uint16_t ri_new_value = 0;

  circuit_state state;
  state.self_en = self_en;
  state.self_clk = self_clk;
  state.self_clk_last = 1;
  state.ri_old_value = 0;
  state.ri_new_value = 0; //&ri_new_value;
  state.self_out = self_out;
  
  for (int i = 0; i < 20; i++) {

    state.self_clk = i % 2;

    //simulate(&ri_new_value, &self_out, ri_old_value, self_clk, self_clk_last, self_en);

    simulate(&state);

    // Copy old values to new values
    state.self_clk_last = state.self_clk;
    state.ri_old_value = state.ri_new_value;
  }

  printf("output = %hu\n", state.self_out);
  printf("new_register value = %hu\n", state.ri_new_value);
  
  if ((state.ri_new_value != 10) || (state.self_out != 9)) {

    return 1;
  }
  

  return 0;
}
