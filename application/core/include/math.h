#pragma once

#include <functional>

using PairingFunction = std::function<unsigned(unsigned, unsigned)>;

[[nodiscard]] unsigned CantorPairingFunction(unsigned, unsigned);
