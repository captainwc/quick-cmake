#include <skutils/stack_printer.h>

void fun() {
  PRINT_STACK_HERE;
}

void bar() {
  fun();
}

void foo() {
  bar();
}

int main() {
  foo();
  return 0;
}
