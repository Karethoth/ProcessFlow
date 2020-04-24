#pragma once

#include "flow_step.h"

class Flow
{
  std::vector<std::shared_ptr<FlowStep>> steps{};


public:
  std::shared_ptr<FlowStep> add_step(std::shared_ptr<FlowStep> step);

  void connect_steps(
    const std::shared_ptr<FlowStep>& parent,
    const std::shared_ptr<FlowStep>& child
  );

  void start();
  bool is_running();
};

