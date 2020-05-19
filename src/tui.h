#pragma once

#include "flow.h"

extern "C"
{
  #define PDC_WIDE
  #include <curses.h>
  #include <panel.h>
}

#include <vector>
#include <string>
#include <memory>

struct Tui;

struct TuiPanel
{
  WINDOW *window;
  PANEL  *panel;

  TuiPanel(int x, int y, int width, int height);

  void attron(chtype attr);
  void attroff(chtype attr);

  ~TuiPanel() noexcept;
};


struct TuiViewOption
{
  std::wstring name;
  int          key;
  bool         active;

  TuiViewOption(std::wstring name, int key);
};


struct TuiViewMenu
{
  constexpr static int height = 4;

  std::unique_ptr<TuiPanel> panel;
  std::vector<TuiViewOption> options;

  const Tui *main_tui;

  TuiViewMenu(const Tui* tui);

  void render();
};


struct TuiViewFlow
{
  constexpr static int menu_height = 4;

  std::unique_ptr<TuiPanel> panel;
  const Tui *main_tui;

  std::shared_ptr<flow::Flow> current_flow;

  TuiViewFlow(const Tui* tui);
  void render();

  void set_current_flow(std::shared_ptr<flow::Flow> flow)
  {
    current_flow = flow;
  }
};


struct Tui
{
  std::unique_ptr<TuiPanel> main_panel;
  TuiViewMenu               view_menu{this};
  TuiViewFlow               view_flow{this};

  Tui();
  void render();
};

