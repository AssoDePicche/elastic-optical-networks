#include "agent.h"

#include <algorithm>
#include <vector>

#include "spectrum.h"

namespace core {
struct ClassicAgent::Implementation {
  bool ShouldAccept(Environment& environment) {
    if (environment.FSUsPerLink <= environment.activeRequests) {
      return false;
    }

    const auto keys = environment.request.path.keys();

    const auto first = *keys.begin();

    const auto slice = environment.request.type.allocator(
        environment.carriers[first], environment.request.type.FSUs);

    if (!slice.has_value()) {
      return false;
    }

    environment.request.slice = slice.value();

    for (const auto& key : keys) {
      if (environment.carriers.at(key).available() <
              environment.request.type.FSUs ||
          !environment.carriers.at(key).available_at(
              environment.request.slice)) {
        return false;
      }
    }

    return true;
  }
};

ClassicAgent::ClassicAgent() { pImpl = std::make_unique<Implementation>(); }

ClassicAgent::~ClassicAgent() {}

bool ClassicAgent::ShouldAccept(Environment& environment) {
  return pImpl->ShouldAccept(environment);
}

struct QLearningAgent::Implementation {
  double learningRate;
  double eagerness;
  double reward;
  double penalty;
  std::vector<std::vector<int>> qRewards;
  std::vector<std::vector<double>> qStates;

  bool ShouldAccept(Environment& environment) {
    if (environment.FSUsPerLink <= environment.activeRequests) {
      return false;
    }

    const auto keys = environment.request.path.keys();

    const auto first = *keys.begin();

    const auto slice = environment.request.type.allocator(
        environment.carriers[first], environment.request.type.FSUs);

    if (!slice.has_value()) {
      return false;
    }

    for (const auto& key : keys) {
      if (environment.carriers.at(key).available() <
              environment.request.type.FSUs ||
          !environment.carriers.at(key).available_at(
              environment.request.slice)) {
        return false;
      }
    }

    auto carriers = environment.carriers;

    const auto allocate = [&](const auto key) {
      carriers[key].allocate(environment.request.slice);
    };

    std::for_each(keys.begin(), keys.end(), allocate);

    AbsoluteFragmentation fragmentation;

    double meanFragmentation = 0.0;

    for (const auto& key : keys) {
      meanFragmentation += fragmentation(carriers.at(key));
    }

    meanFragmentation /= keys.size();

    return meanFragmentation < 0.75;
  }
};

QLearningAgent::QLearningAgent() { pImpl = std::make_unique<Implementation>(); }

QLearningAgent::~QLearningAgent() {}

bool QLearningAgent::ShouldAccept(Environment& environment) {
  return pImpl->ShouldAccept(environment);
}

std::unique_ptr<Agent> AgentFactory::CreateAgent(const std::string& type) {
  if (type == "classic") {
    return std::make_unique<ClassicAgent>();
  }

  if (type == "qlearning") {
    return std::make_unique<QLearningAgent>();
  }

  return nullptr;
}
}  // namespace core
