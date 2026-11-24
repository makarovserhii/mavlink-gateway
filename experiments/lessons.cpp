#include <iostream>

int main()
{
  std::cout << "Enter a two numbers separated by a space: ";

  int x{};
  int y{};

  std::cin >> x;
  std::cin >> y;

  std::cout << "x = " << x << " and " << "y = " << y << '\n';

  return 0;
}
