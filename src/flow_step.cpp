#include "flow_step.h"

using namespace flow;

FlowStep::FlowStep(std::wstring name) : name(name)
{
};

FlowStep::~FlowStep() noexcept
{
  std::wcout << "Destroying " << name << std::endl;

  // Stop reader thread
  if (io.reader_thread.joinable())
  {
    io.should_stop = true;
    io.reader_thread.join();
  }
}

std::string FlowStep::transform_data(std::string data)
{
  std::string current = data;
  for (auto& transformer : data_transformers)
  {
    current = transformer(current);
  }
  return current;
}


void FlowStep::start()
{
  init();

  io.reader_thread = std::thread([&]()
    {
      while (!io.should_stop)
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
            auto start = data_to_send.start;
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
        catch (std::exception & e)
        {
        }
      }

      handle_eof();
    });

  //io.reader_thread.detach();
}


void FlowStep::handle_eof()
{
  io.should_stop = true;
  io.eof = true;

  for (auto& next_step : next_steps)
  {
    next_step->handle_parent_eof();
  }
}


void FlowStep::handle_parent_eof()
{
  io.should_stop = true;
}


[[nodiscard]]
FlowSize calc_flow_grid_size(const flow::FlowStep* step)
{
  //std::lock_guard step_lock{step->next_steps_mutex};

  if (!step)
  {
    return { 1, 1 };
  }

  FlowSize dim;
  dim.w = 1;
  dim.h = step->next_steps.size();
  if (!dim.h)
  {
    dim.h = 1;
  }

  for (const auto& next_step : step->next_steps)
  {
    const auto next_dim = calc_flow_grid_size(&(*next_step));
    if (next_dim.w >= dim.w)
    {
      dim.w = next_dim.w + 1;
    }
    
    if (next_dim.h > 1)
    {
      dim.h += next_dim.h - 1;
    }
  }

  return dim;
}


FlowSize FlowStep::get_branch_flow_grid_size() const
{
  return calc_flow_grid_size(this);
}
