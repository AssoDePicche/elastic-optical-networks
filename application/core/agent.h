#pragma once

#include <cstdint>
#include <memory>

#include "request.h"
#include "spectrum.h"

namespace core {
struct Environment final {
  Request request;
  Carriers carriers;
  uint64_t activeRequests;
  uint64_t FSUsPerLink;
};

class Agent {
 public:
  ~Agent(void) = default;

  [[nodiscard]] virtual bool ShouldAccept(Environment&) = 0;
};

class ClassicAgent : public Agent {
 public:
  ClassicAgent();

  ~ClassicAgent();

  [[nodiscard]] bool ShouldAccept(Environment&) override;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};

class QLearningAgent : public Agent {
  public:
    QLearningAgent();

    ~QLearningAgent();

    [[nodiscard]] bool ShouldAccept(Environment&) override;

  private:
    struct Implementation;
    std::unique_ptr<Implementation> pImpl;
};

class AgentFactory final {
  public:
    [[nodiscard]] static std::unique_ptr<Agent> CreateAgent(const std::string&);
};
}  // namespace core
