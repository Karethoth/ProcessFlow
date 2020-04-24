#pragma once

extern "C"
{
  #define PDC_WIDE
  #include <curses.h>
  #include <panel.h>
}

#include <string>
#include <memory>


struct TuiPanel
{
  WINDOW *window;
  PANEL  *panel;

  TuiPanel(int x, int y, int width, int height)
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

  ~TuiPanel() noexcept
  {
    del_panel(panel);
    delwin(window);
  }
};


struct TuiViewOption
{
  std::wstring name;
  int          key;
  bool         active;

  TuiViewOption(std::wstring name, int key) : name(name), key(key), active(false) {};
};


struct TuiViewMenu
{
  constexpr static int height = 4;

  std::unique_ptr<TuiPanel> panel;
  std::vector<TuiViewOption> options;

  TuiViewMenu()
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

  void render() {

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
      wattron(panel->window, A_UNDERLINE);
      wattron(panel->window, COLOR_PAIR(1));
      mvwaddwstr(panel->window, 1, key_offset_x, key_str.c_str());
      wattroff(panel->window, COLOR_PAIR(1));
      wattroff(panel->window, A_UNDERLINE);

      auto name_padding_left = option_width / 2 - option.name.length() / 2;
      auto name_offset_x = name_padding_left + offset_x;

      if (option.active)
      {
        wattron(panel->window, A_BOLD);
        mvwaddwstr(panel->window, 2, name_offset_x, option.name.c_str());
        wattroff(panel->window, A_BOLD);
      }
      else
      {
        wattron(panel->window, A_LOW);
        mvwaddwstr(panel->window, 2, name_offset_x, option.name.c_str());
        wattroff(panel->window, A_LOW);
      }
    }

    top_panel(panel->panel);

    update_panels();
    doupdate();
  }
};


struct Tui
{
  std::unique_ptr<TuiPanel> main_panel;
  TuiViewMenu               view_menu{};

  Tui()
  {
    const int width = COLS;
    main_panel = std::make_unique<TuiPanel>(
      0, 0,
      width, LINES - 3
    );
    scrollok(main_panel->window, true);
    curs_set(0);
  }

  void render()
  {
    view_menu.render();
  }
};
