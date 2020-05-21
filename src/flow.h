#pragma once

#include "flow_step.h"

namespace flow
{
  class Flow
  {
    std::vector<std::shared_ptr<FlowStep>> steps{};


  public:
    std::shared_ptr<FlowStep> add_step(std::shared_ptr<FlowStep> step);

    [[nodiscard]]
    std::shared_ptr<FlowStep> get_root_step();

    void connect_steps(
      const std::shared_ptr<FlowStep>& parent,
      const std::shared_ptr<FlowStep>& child
    );

    void start();
    bool is_running();


    // Returns the grid dimensions required to render the flow
    [[nodiscard]]
    FlowSize get_grid_size() const;
  };

};

