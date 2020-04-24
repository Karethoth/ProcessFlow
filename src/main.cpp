#include "flow.h"
#include "flow_step.h"
#include "flow_step_process.h"

extern "C"
{
#define PDC_WIDE
  #include <curses.h>
  #include <panel.h>
}
#include <signal.h>

#include <iostream>
#include <atomic>
#include <thread>
#include <future>
#include <chrono>
#include <vector>
#include <mutex>
#include <sstream>
#include <functional>

static void finish(int sig);

int main()
{
  setlocale(LC_ALL, "");
  //std::wcout.sync_with_stdio(false);
  //std::wcout.imbue(std::locale("en_US.utf8"));

  signal(SIGINT, finish);

  /*
  auto win = initscr();

  start_color();
  init_pair(1, COLOR_RED, COLOR_BLACK);
  attron(COLOR_PAIR(1));
  */

  flow::Flow test_flow;

  {
    auto step1 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable produce" ));
    auto step2 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #1" ));
    auto step3 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #2" ));

    test_flow.connect_steps(step1, step2);
    test_flow.connect_steps(step2, step3);

    const auto data_cb = [](const std::string& data, const flow::FlowStep &context)
    {
      static std::mutex cout_mutex{};
      std::lock_guard cout_lock{ cout_mutex };
      std::cout << "" << data;
    };

    step1->set_data_callback(data_cb);
    step2->set_data_callback(data_cb);
    step3->set_data_callback(data_cb);

    test_flow.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  for (;;)
  {
    if (!test_flow.is_running())
    {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  //printw(step.cmd.c_str());
  /*
  mvaddwstr(0, 0, step.cmd.c_str());
  refresh();
  attroff(COLOR_PAIR(1));

  step.wait_to_end();

  endwin();
  */
}
static void finish(int sig)
{
  //endwin();
  exit(0);
}
