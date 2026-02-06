#include "driver.hpp"
#include "road.hpp"
#include "vehicle.hpp"

/**
TODOLIST:
1. Road中捕获按键，设置响应状态。
    1.1 设置值应该有个上限（考虑方向盘角度有限制），可以用栈
2. 然后调用vehicle方法计算位置
3. 在road中显示新的位置
*/

int main() {
  Road road;
  road.CreateDaoku();

  Vehicle v;

  Driver(v, road).StartDrive("Practice");

  return 0;
}