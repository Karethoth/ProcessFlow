#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <vector>
#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <functional>
#include <iostream>
#include <array>

namespace flow
{

  template<size_t t_buffer_size, typename T = char>
  struct BufferedData
  {
    std::array<T, t_buffer_size> buffer{};
    size_t count{ 0 };
    size_t start{ 0 };
  };


  template <typename T_in, typename T_out>
  using FlowStepDataTransform = std::function<T_out(const T_in&)>;

  class FlowStep
  {
  protected:
    constexpr static const size_t buffer_size = 256;

    using DataCallbackFunctor = std::function<void(const std::string&, const FlowStep&)>;

    struct
    {
      std::thread      reader_thread{};
      std::atomic_bool should_stop = false;
      bool             eof = false;
    } io;

    DataCallbackFunctor data_cb;
    std::vector<FlowStepDataTransform<std::string_view, std::string>> data_transformers;

    [[nodiscard]] virtual std::string transform_data(std::string data);

    [[nodiscard]] virtual BufferedData<buffer_size> read() = 0;
    [[nodiscard]] virtual size_t write(const BufferedData<buffer_size>& data) = 0;


  public:
    std::wstring name;
    std::vector<std::shared_ptr<FlowStep>>  next_steps;
    std::mutex                              next_steps_mutex;

    explicit FlowStep(std::wstring name = L"Unnamed");
    virtual ~FlowStep() noexcept;

    virtual void start();

    void add_next_step(std::shared_ptr<FlowStep> step)
    {
      std::lock_guard next_steps_lock{ next_steps_mutex };
      next_steps.push_back(step);
    }

    void set_data_callback(DataCallbackFunctor cb)
    {
      data_cb = cb;
    }

    virtual void init() = 0;
    [[nodiscard]] virtual bool is_running() const = 0;

    virtual void handle_eof();
    virtual void handle_parent_eof();
  };

};
