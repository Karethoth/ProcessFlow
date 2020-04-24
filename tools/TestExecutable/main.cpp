#include <Windows.h>
#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

int main(int argc, char **argv)
{

  // Consumer
  if (argc > 1 && std::string{ argv[1] }.compare("consume") == 0)
  {
    std::string line;
    while (!std::getline(std::cin, line).eof())
    {
      try
      {
        if (line.length())
        {
          for (char c : line)
          {
            std::cout << (char)(std::isalpha(c) ? (c + 1) : c);
          }
          std::cout << std::endl;
        }
        else
        {
          break;
        }
      }
      catch (std::exception &e)
      {

      }
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "Reached end" << std::endl;
    return 0;
  }


  // Producer
  for (auto i=0; i<10; i++)
  {
    std::cout << i << "a\tTest\t" << i << "b" << std::endl;
    std::flush(std::cout);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }
  return 0;
}
