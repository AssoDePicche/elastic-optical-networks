#pragma once

#include <graph/router.h>
#include <prng/prng.h>

#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "configuration.h"
#include "document.h"
#include "request.h"
#include "spectrum.h"

namespace core {
struct Event final {
  enum class Type { Arrival, Departure };

  double time;
  Type type;
  Request request;

  Event(void) = default;

  Event(const double, const Type&, const Request&);

  [[nodiscard]] bool operator<(const Event&) const noexcept;

  [[nodiscard]] static Event MakeArrival(const double, const Request&);

  [[nodiscard]] static Event MakeDeparture(const double, const Request&);
};

struct Statistics final {
  double absolute_fragmentation;
  double entropy_fragmentation;
  double external_fragmentation;
  double time;
  uint64_t active_requests;
  uint64_t total_FSUs_requested;
  uint64_t total_FSUs_blocked;
  uint64_t total_requests;
  uint64_t total_requests_blocked;

  Statistics(void) = default;

  void Reset(void);

  [[nodiscard]] double GradeOfService(void) const;

  [[nodiscard]] double SlotBlockingProbability(void) const;

  [[nodiscard]] std::string Serialize(void) const;
};

struct Trace final {
  std::string type;
  graph::Vertex source;
  graph::Vertex destination;
  uint64_t FSUs;
  bool accepted;
};

class Kernel final {
 public:
  Kernel(std::shared_ptr<Configuration>);

  ~Kernel();

  void Run(void);

  [[nodiscard]] Document GetReport(void) const;

  void ExportDataset(const std::string&) const;

  void Reset(void);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
