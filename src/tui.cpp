#include "tui.h"

extern "C"
{
  #define PDC_WIDE
  #include <curses.h>
  #include <panel.h>
}

#include <string>
#include <memory>
#include <mutex>
#include <exception>
#include <iostream>


TuiPanel::TuiPanel(int x, int y, int width, int height)
{
  window = newwin(height, width, y, x);
  if (!window)
  {
    throw std::runtime_error("TuiPanel - Couldn't create the window");
  }

  panel = new_panel(window);
  if (!panel)
  {
    throw std::runtime_error("TuiPanel - Couldn't create the panel");
  }
}


void TuiPanel::attron(chtype attr)
{
  wattron(window, attr);
}


void TuiPanel::attroff(chtype attr)
{
  wattroff(window, attr);
}


TuiPanel::~TuiPanel() noexcept
{
  del_panel(panel);
  delwin(window);
}


TuiViewOption::TuiViewOption(std::wstring name, int key) : name(name), key(key), active(false) {};


TuiViewMenu::TuiViewMenu(const Tui *tui) : main_tui(tui)
{
  const int width = COLS;
  panel = std::make_unique<TuiPanel>(
    0, LINES - height,
    width, height
  );
  top_panel(panel->panel);

  wborder(panel->window, ' ', ' ', ACS_HLINE, ' ', ACS_HLINE, ACS_HLINE, ' ', ' ');

  options.emplace_back(L"First view", KEY_F(1));
  options.emplace_back(L"Second view", KEY_F(2));
  options.emplace_back(L"Third view", KEY_F(3));

  // Mark the first option as selected
  options[0].active = true;
}


void TuiViewMenu::render() {

  int option_index = 0;
  int option_width = COLS / options.size();
  for (auto& option : options)
  {
    std::wstring key_str = L"";
    switch (option.key)
    {
      case KEY_F(1): key_str = L"F1"; break;
      case KEY_F(2): key_str = L"F2"; break;
      case KEY_F(3): key_str = L"F3"; break;
      default:       key_str = L"?"; ;
    }

    auto offset_x = option_width * option_index++;

    if (offset_x > 0)
    {
      mvwaddch(panel->window, 0, offset_x, ACS_TTEE);

      for (auto row = 0; row < 3; row++)
      {
        mvwaddch(panel->window, row + 1, offset_x, ACS_VLINE);
      }
    }


    auto key_padding_left = option_width / 2 - key_str.length() / 2;
    auto key_offset_x = key_padding_left + offset_x;
    panel->attron(A_UNDERLINE);
    panel->attron(COLOR_PAIR(1));
    mvwaddwstr(panel->window, 1, key_offset_x, key_str.c_str());
    panel->attroff(COLOR_PAIR(1));
    panel->attroff(A_UNDERLINE);

    auto name_padding_left = option_width / 2 - option.name.length() / 2;
    auto name_offset_x = name_padding_left + offset_x;

    if (option.active)
    {
      panel->attron(A_BOLD);
      mvwaddwstr(panel->window, 2, name_offset_x, option.name.c_str());
      panel->attroff(A_BOLD);
    }
    else
    {
      mvwaddwstr(panel->window, 2, name_offset_x, option.name.c_str());
    }
  }

  top_panel(panel->panel);

  update_panels();
  doupdate();
}


TuiViewFlow::TuiViewFlow(const Tui *tui) : main_tui(tui)
{
  const int width = COLS;
  panel = std::make_unique<TuiPanel>(
    0, 0,
    width, LINES - menu_height
  );
  top_panel(panel->panel);
}

void TuiViewFlow::render() {

  panel->attron(A_UNDERLINE);
  panel->attron(COLOR_PAIR(1));
  std::wstring t_msg = L"Test";
  mvwaddwstr(panel->window, 1, 1, t_msg.c_str());
  panel->attroff(COLOR_PAIR(1));
  panel->attroff(A_UNDERLINE);

  top_panel(panel->panel);

  if (!current_flow || !current_flow->get_root_step())
  {
    return;
  }

  // Render the visible portion of the flow grid

  const auto grid_size = current_flow->get_grid_size();

  /* TODO
   * - Keep track of the view offset / coordinate of the active cell
   * - Calculate visible portion based on the UI dimensions
   * - Render step names and borders
   *  - Highlight active one
   * - Render lines connecting the steps together
   */

  update_panels();
  doupdate();
}


Tui::Tui()
{
  const int width = COLS;
  main_panel = std::make_unique<TuiPanel>(
    0, 0,
    width, LINES - 3
  );
  scrollok(main_panel->window, true);
  curs_set(0);
}


void Tui::render()
{
  view_flow.render();
  view_menu.render();
}

