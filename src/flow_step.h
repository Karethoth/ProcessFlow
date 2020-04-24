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


template<size_t t_buffer_size, typename T=char>
struct BufferedData
{
  std::array<T, t_buffer_size> buffer{};
  size_t count{ 0 };
  size_t start{ 0 };
};


template <typename T_in, typename T_out>
using FlowStepDataTransform = std::function<T_out(const T_in&)>;

struct FlowStep
{
  constexpr static const size_t buffer_size = 256;

  struct
  {
    std::thread      reader_thread{};
    std::atomic_bool should_stop  = false;
    bool             eof          = false;
  } io;

  std::vector<std::shared_ptr<FlowStep>>  next_steps;
  std::mutex                              next_steps_mutex;
  std::function<void(const std::string&, const FlowStep&)> data_cb;

  std::vector<FlowStepDataTransform<std::string_view, std::string>> data_transformers;


public:
  std::wstring name;

  explicit FlowStep(std::wstring name=L"Unnamed") : name(name)
  {
  };

  virtual ~FlowStep() noexcept
  {
    std::wcout << "Destroying " << name << std::endl;

    // Stop reader thread
    if (io.reader_thread.joinable())
    {
      io.should_stop = true;
      io.reader_thread.join();
    }
  }

  [[nodiscard]] virtual bool is_running() const = 0;

  virtual void init()
  {
  }

  [[nodiscard]] virtual BufferedData<buffer_size> read()
  {
    BufferedData<buffer_size> data;
    return data;
  }

  [[nodiscard]] virtual size_t write(const BufferedData<buffer_size> &data)
  {
    return data.count;
  }

  [[nodiscard]]
  virtual std::string transform_data(std::string data)
  {
    std::string current = data;
    for (auto& transformer : data_transformers)
    {
      current = transformer(current);
    }
    return current;
  }

  virtual void start()
  {
    init();

    io.reader_thread = std::thread([&]()
    {
      while(!io.should_stop)
      {
        try
        {
          // Read process output
          auto data_to_send = read();
          const auto received_byte_count = data_to_send.count;
          if (received_byte_count == 0)
          {
            io.should_stop = true;
            break;
          }

          if (data_cb)
          {
            const std::string data_str{ data_to_send.buffer.begin(), data_to_send.buffer.begin() + data_to_send.count };
            data_cb(data_str, *this);
          }


          // Pass it onwards to the next steps
          std::lock_guard next_steps_lock{ next_steps_mutex };
          for (auto& next_step : next_steps)
          {
            auto start         = data_to_send.start;
            data_to_send.count = received_byte_count;

            while (start < received_byte_count)
            {
              data_to_send.start = start;
              data_to_send.count = received_byte_count - start;

              const auto bytes_sent = next_step->write(data_to_send);
              if (!bytes_sent)
              {
                break;
              }

              start += bytes_sent;
            }
          }
        }
        catch (std::exception &e)
        {
        }
      }

      handle_eof();
    });

    //io.reader_thread.detach();
  }

  virtual void handle_eof()
  {
    io.should_stop = true;
    io.eof         = true;
  }

  virtual void handle_parent_eof()
  {
    io.should_stop = true;
  }
};

