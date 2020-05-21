#include "flow.h"
#include "flow_step.h"
#include "flow_step_process.h"
#include "tui.h"

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


static void finish(int sig)
{
  endwin();
  exit(0);
}


int main()
{
  setlocale(LC_ALL, "");
  //std::wcout.sync_with_stdio(false);
  //std::wcout.imbue(std::locale("en_US.utf8"));

  signal(SIGINT, finish);

  auto win = initscr();
  resize_term(50, 150);
  start_color();
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_WHITE, COLOR_BLACK);

  Tui tui{};

  tui.main_panel->attron(COLOR_PAIR(2));

  tui.render();
  flow::Flow test_flow;
  {
    auto step1 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable produce" ));
    auto step2 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #1" ));
    auto step3 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #2" ));

    auto step4 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #3" ));
    auto step5 = test_flow.add_step(std::make_shared<flow::FlowStepProcess>( L"TestExecutable consume", L"Consumer #4" ));

    test_flow.connect_steps(step1, step2);
    test_flow.connect_steps(step1, step4);

    test_flow.connect_steps(step2, step3);
    test_flow.connect_steps(step2, step5);

    const auto data_cb = [&](const std::string& data, const flow::FlowStep &context)
    {
      static std::mutex cout_mutex{};
      std::lock_guard cout_lock{ cout_mutex };
      waddwstr(tui.main_panel->window, context.name.c_str());
      waddstr(tui.main_panel->window, ": ");
      waddstr(tui.main_panel->window, data.c_str());
      tui.render();
      refresh();
    };

    step1->set_data_callback(data_cb);
    step2->set_data_callback(data_cb);
    step3->set_data_callback(data_cb);
    step4->set_data_callback(data_cb);
    step5->set_data_callback(data_cb);

    test_flow.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  tui.view_flow.set_current_flow(std::make_shared<flow::Flow>(test_flow));

  for (;;)
  {
    if (!test_flow.is_running())
    {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  waddwstr(tui.main_panel->window, L"EOF! Press any key to exit.");
  tui.main_panel->attroff(COLOR_PAIR(2));
  getch();

  endwin();
}

