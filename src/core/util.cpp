#include "util.h"

float min(float a, float b) {
  return a < b ? a : b;
}
float max(float a, float b) {
  return a >= b ? a : b;
}

float clamp(float low, float v, float high) {
  return min(high, max(low, v));
}

int min_int(int a, int b) {
  return a < b ? a : b;
}
int max_int(int a, int b) {
  return a >= b ? a : b;
}

int clamp_int(int low, int value, int high) {
  return min_int(high, max_int(low, value));
}
