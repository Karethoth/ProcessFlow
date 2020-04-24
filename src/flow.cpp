#include "flow.h"

using namespace flow;

std::shared_ptr<FlowStep> Flow::add_step(std::shared_ptr<FlowStep> step)
{
  steps.emplace_back(step);
  return step;
}


void Flow::connect_steps(
  const std::shared_ptr<FlowStep>& parent,
  const std::shared_ptr<FlowStep>& child
)
{
  std::shared_ptr<FlowStep> matched_parent;
  std::shared_ptr<FlowStep> matched_child;

  for (auto& step : steps)
  {
    if (step == parent)
    {
      matched_parent = step;
    }
    else if (step == child)
    {
      matched_child = step;
    }
  }

  if (matched_parent && matched_child)
  {
    matched_parent->add_next_step(matched_child);
  }
  else
  {
    throw std::runtime_error("Flow::steps must contain both parent and child steps");
  }
}


void Flow::start()
{
  std::vector<std::shared_ptr<FlowStep>> started_steps{};

  const std::function<void(std::shared_ptr<FlowStep>&)> start_step_recursive = [&](std::shared_ptr<FlowStep> &step)
  {
    for (const auto& started_step : started_steps)
    {
      if (started_step == step)
      {
        return;
      }
    }

    started_steps.push_back(step);

    for (auto& child_step : step->next_steps)
    {
      start_step_recursive(child_step);
    }

    step->start();
  };


  for (auto& step : steps)
  {
    start_step_recursive(step);
  }
}


bool Flow::is_running()
{
  for (const auto& step : steps)
  {
    if (step->is_running())
    {
      return true;
    }
  }

  return false;
}

