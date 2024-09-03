# Elastic Optical Networks

This repository contains an elastic optical network simulator developed for personal studies, initially as part of the first undergraduate research program in which I participated during September of 2023 and August of 2024, under the supervision of Dr. Rodrigo Campos Bortoletto.

The studies consisted mainly of machine learning algorithms applied to routing, modulation and spectrum assignment. The topologies adopted for analysis were Point-to-Point, Arpanet, Ipê, Germany 17, Abilene, JPN48, NSFNET and Pan-European.

## Table of Contents

- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [Contact](#contact)
- [License](#license)

## Features

The algorithms available for route and spectrum allocation are:

**Spectrum Allocation**
- [Best fit](https://en.wikipedia.org/wiki/Best-fit_bin_packing#:~:text=The%20best%2Dfit%20algorithm%20uses,before%20placing%20the%20new%20item.)
- [First fit](https://en.wikipedia.org/wiki/First-fit_bin_packing)
- [Last fit](https://en.wikipedia.org/wiki/First-fit-decreasing_bin_packing)
- Random fit
- [Worst fit](https://www.geeksforgeeks.org/worst-fit-allocation-in-operating-systems/)

**Routing**
- [Breadth-first Search](https://en.wikipedia.org/wiki/Breadth-first_search)
- [Depth-first Search](https://en.wikipedia.org/wiki/Depth-first_search)
- [Dijkstra's algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm)

## Installation

1. Install dependencies

Before cloning this repository, you must have installed all the dependencies for this project:

- [CMake](https://cmake.org/download/)

2. Clone the repository

```bash
git clone git@github.com:AssoDePicche/elastic-optical-networks.git
```

3. Build

```bash
cmake -S . -B build && cmake --build build
```

## Usage

Run the binary created in the [Installation section](#installation) with the arguments specified below. At the end of the simulation, in addition to the parameter information, the network grade of service (GoS) calculated using the [Erlang B formula](https://en.wikipedia.org/wiki/Erlang_(unit)), the proportion of request types (3-slot or 7-slot requests), the blocking probability (BP) per request type will be available and some other information.

You must pass the following arguments for the simulation:
- `--arrival-rate`: the call arrival rate per unit of time
- `--channels`: the number of resources available per link
- `--seed`: the seed number for random processes
- `--service-rate`: the service rate per unit of time
- `--spectrum-allocator`: the [spectrum allocation algorithm](#features)
- `--time-units`: the maximum time that should be simulated
- `--topology`: the file with the number of nodes and the adjacency matrix of the network

For example:

```bash
./build/source/App --time-units 100000 --seed 227953 --channels 10 --service-rate 0.133 --arrival-rate 1 --topology topology.txt --spectrum-allocator first-fit
```

With the topology.txt file as follows:

```txt
2
0 1
1 0
```

Output:

```txt
Execution time: 44s
Simulation time: 99999.888274
Seed: 0
Channels (C): 10
Calls (n): 100202
Arrival rate (λ): 1.000000
Service rate (μ): 0.133000
Traffic Intensity (ρ): 7.518797
Grade of Service (ε): 0.100966
Busy Channels (1-ε): 0.899034
Occupancy ((1-ε)/C): 0.089903
Type 1
Ratio: 1.000000
BP(ε): 0.100966
```

## Contributing

There are many ways to contribute to this project:
- You can contact me to suggest articles that might be useful in future resources.
- You can (please) cite this project and related articles published during my research in your work.
- You can contribute to the code through refactoring or implementation by following the steps below:

1. Fork the repository.

2. Create a new branch: `git checkout -b feature-name`.

3. Make your changes.

4. Push your branch: `git push origin feature-name`.

5. Create a pull request.

## Contact

Samuel do Prado Rodrigues - samuelprado730@gmail.com

## License

This project is licensed under the [MIT License](LICENSE), see [LICENSE](LICENSE) for more information.
