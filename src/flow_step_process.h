#pragma once

#include "flow_step.h"

namespace flow
{

  class FlowStepProcess : public FlowStep
  {
  protected:
    struct
    {
      STARTUPINFO         startup_info = { 0 };
      PROCESS_INFORMATION info = { 0 };
    } process;

    struct
    {
      HANDLE stdin_read = nullptr;
      HANDLE stdin_write = nullptr;
      HANDLE stdout_read = nullptr;
      HANDLE stdout_write = nullptr;
    } process_io;


    [[nodiscard]] virtual BufferedData<buffer_size> read();
    [[nodiscard]] virtual size_t write(const BufferedData<buffer_size>& data);

    void virtual handle_parent_eof() override;

  public:
    std::wstring cmd;

    explicit FlowStepProcess(std::wstring _cmd, std::wstring _name = L"");

    explicit FlowStepProcess(FlowStepProcess&&) = delete;
    //explicit FlowStepProcess(FlowStepProcess&& other) noexcept;
    virtual ~FlowStepProcess() noexcept;


    virtual void init() override;
    void wait_to_end();

    [[nodiscard]] bool is_running() const;
  };

};
