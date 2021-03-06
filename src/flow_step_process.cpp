#include "flow_step_process.h"

using namespace flow;

FlowStepProcess::FlowStepProcess(std::wstring _cmd, std::wstring _name) : FlowStep(_name), cmd(_cmd)
{
  if (!_name.length())
  {
    name = cmd;
  }
};


/*
explicit FlowStepProcess::FlowStepProcess(FlowStepProcess&& other) noexcept
{
  std::swap(cmd, other.cmd);
  std::swap(process, other.process);
  std::swap(process_io.stdin_read, other.process_io.stdin_read);
  std::swap(process_io.stdin_write, other.process_io.stdin_write);
  std::swap(process_io.stdout_read, other.process_io.stdout_read);
  std::swap(process_io.stdout_write, other.process_io.stdout_write);
  std::swap(io.reader_thread, other.io.reader_thread);
  std::swap(io.eof, other.io.eof);
  io.should_stop = other.io.should_stop.load();
  other.io.should_stop = false;
}
*/


FlowStepProcess::~FlowStepProcess() noexcept
{
  std::wcout << "Destroying process " << name << std::endl;

  // Stop reader thread
  if (io.reader_thread.joinable())
  {
    io.should_stop = true;
    io.reader_thread.join();
  }

  // If the process is running, stop it
  if (is_running() && process.info.hProcess)
  {
    std::wcout << "Killing process " << cmd << std::endl;
    TerminateProcess(process.info.hProcess, 1);
  }
}


void FlowStepProcess::init()
{
  // Create pipes
  SECURITY_ATTRIBUTES security_attributes = { 0 };
  security_attributes.nLength = sizeof(SECURITY_ATTRIBUTES);
  security_attributes.bInheritHandle = TRUE;
  security_attributes.lpSecurityDescriptor = nullptr;

  if (!CreatePipe(&process_io.stdout_read, &process_io.stdout_write, &security_attributes, 0))
  {
    throw std::runtime_error("Failed to create STDOUT pipe");
  }

  if (!CreatePipe(&process_io.stdin_read, &process_io.stdin_write, &security_attributes, 0))
  {
    throw std::runtime_error("Failed to create STDIN pipe");
  }

  if (!SetHandleInformation(process_io.stdout_read, HANDLE_FLAG_INHERIT, 0))
  {
    throw std::runtime_error("Failed to set information for STDOUT pipe");
  }

  if (!SetHandleInformation(process_io.stdin_write, HANDLE_FLAG_INHERIT, 0))
  {
    throw std::runtime_error("Failed to set information for STDIN pipe");
  }


  // Create process
  process.startup_info.cb = sizeof(process.startup_info);
  process.startup_info.hStdError = process_io.stdout_write;
  process.startup_info.hStdOutput = process_io.stdout_write;
  process.startup_info.hStdInput = process_io.stdin_read;
  process.startup_info.dwFlags |= STARTF_USESTDHANDLES;

  const auto create_process_success = CreateProcess(
    nullptr,
    cmd.data(),
    nullptr,
    nullptr,
    true,
    0,
    nullptr,
    nullptr,
    &process.startup_info,
    &process.info
  );


  if (!create_process_success)
  {
    throw std::runtime_error("Failed to create the process");
  }

  CloseHandle(process_io.stdout_write);
  process_io.stdout_write = nullptr;

  CloseHandle(process_io.stdin_read);
  process_io.stdin_read = nullptr;
}


void FlowStepProcess::wait_to_end()
{
  if (process.info.hProcess)
  {
    WaitForSingleObject(process.info.hProcess, INFINITE);

    CloseHandle(process.info.hProcess);
    process.info.hProcess = nullptr;

    CloseHandle(process.info.hThread);
    process.info.hThread = nullptr;
  }
}


bool FlowStepProcess::is_running() const
{
  if (!process.info.hProcess)
  {
    return false;
  }

  DWORD exit_code = 0;
  GetExitCodeProcess(process.info.hProcess, &exit_code);
  if (exit_code == STILL_ACTIVE)
  {
    return true;
  }

  return false;
}


BufferedData<FlowStep::buffer_size> FlowStepProcess::read()
{
  BufferedData<buffer_size> data;
  DWORD read_bytes = 0;

  if (process_io.stdout_read)
  {
    FlushFileBuffers(process_io.stdout_read);
    const auto read_success = ReadFile(process_io.stdout_read, data.buffer.data(), buffer_size, &read_bytes, nullptr);
    if (!read_success)
    {
      // No need to do anything here, the caller handles 0 data / EOF.
    }
    data.count = static_cast<size_t>(read_bytes);
  }

  return data;
}


size_t FlowStepProcess::write(const BufferedData<FlowStep::buffer_size>& data)
{
  DWORD wrote_bytes = 0;
  if (process_io.stdin_write)
  {
    const auto write_success = WriteFile(process_io.stdin_write, data.buffer.data() + data.start, data.count, &wrote_bytes, nullptr);
  }
  return static_cast<size_t>(wrote_bytes);
}


void FlowStepProcess::handle_parent_eof()
{
  std::wcout << name; std::cout << " parent EOF" << std::endl;
  if (process_io.stdin_write)
  {
    FlushFileBuffers(process_io.stdin_write);
    CloseHandle(process_io.stdin_write);
    process_io.stdin_write = nullptr;
  }
}

