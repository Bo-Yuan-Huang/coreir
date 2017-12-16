#include "multiply_shift_16.h"

#include <iostream>
#include <bitset>

using namespace std;

int main() {
  circuit_state state;
  state.self_A[0] = 288;
  state.self_A[1] = 288;

  simulate(&state);

  cout << "state.self_out = " << bitset<16>(state.self_out) << endl;
  assert(state.self_out == 0b0000010001000000);

  return 0;
}
