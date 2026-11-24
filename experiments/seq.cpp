#include <cstdint>  // подключает типы с точным размером, например uint8_t (1 байт, 0..255)
#include <iostream> // подключает вывод на экран (std::cout)

// Функция: принимает два числа по 1 байту, ничего не возвращает (void)
void check(uint8_t last, uint8_t received)
{
  // Вычитаем. C++ сначала превращает оба числа в int, поэтому
  // static_cast<uint8_t> «обрезает» результат обратно до 0..255 — это и есть «по модулю 256»
  uint8_t distance = static_cast<uint8_t>(received - last);

  // std::cout << ... — печать, как console.log / print.
  // static_cast<int> нужен, иначе uint8_t напечатается как символ, а не как число
  std::cout << "last=" << static_cast<int>(last)
            << " received=" << static_cast<int>(received)
            << " distance=" << static_cast<int>(distance) << " -> ";

  if (distance == 0)
  {
    std::cout << "дубль\n"; // \n — перенос строки
  }
  else if (distance < 128)
  {
    std::cout << "новый, потеряно: " << distance - 1 << "\n";
  }
  else
  {
    std::cout << "старый (опоздал)\n";
  }
}

// main — точка входа: с неё начинается выполнение программы
int main()
{
  check(10, 8);
  check(250, 8);
  check(254, 3);
  check(1, 10);
  check(5, 5);
  return 0; // 0 = «программа завершилась успешно»
}
