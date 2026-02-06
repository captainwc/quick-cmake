#include <vector>

#include "skutils/logger.h"
#include "skutils/random.h"
#include "skutils/test.h"

void random_printer() {
  int size = 5;
  LINE_BREAKER("int (0, 1)");
  for (int i = 0; i < size; i++) {
    std::cout << RANDTOOL.getRandomInt(0, 1) << ", ";
  }
  std::cout << std::endl;
  LINE_BREAKER("double (-1, 1)");
  for (int i = 0; i < size; i++) {
    std::cout << RANDTOOL.getRandomDouble(-1, 1) << ", ";
  }
  std::cout << std::endl;
  LINE_BREAKER("int vector (-2, 3)")
  for (int i = 0; i < size; i++) {
    std::cout << sk::utils::toString(RANDTOOL.getRandomIntVector(2, -2, 3)) << ", ";
  }
  std::cout << std::endl;
  LINE_BREAKER("double vector (-0.5, 0.5)")
  for (int i = 0; i < size; i++) {
    std::cout << sk::utils::toString(RANDTOOL.getRandomDoubleVector(2, -0.5, 0.5)) << ", ";
  }
  std::cout << std::endl;
  LINE_BREAKER("String upperAlphaNumeric")
  for (int i = 0; i < size; i++) {
    std::cout << RANDTOOL.getRandomString(5, CHARSET_UPPER + CHARSET_NUMBER) << ", ";
  }
  std::cout << std::endl;
}

int main() {
  random_printer();
}
