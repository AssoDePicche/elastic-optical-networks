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
- Best fit
- First fit
- Last fit
- Random fit
- Worst fit

**Routing**
- Breadth-first Search
- Depth-first Search
- Dijkstra's algorithm

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
- `--calls`: the number of calls
- `--channels`: the number of resources available per link
- `--erlangs`: the desired traffic intensity in Erlangs
- `--lambda`: the call arrival rate per unit of time
- `--topology`: the file with the number of nodes and the adjacency matrix of the network

For example:

```bash
./build/source/App --calls 1000 --channels 30 --erlangs 25 --lambda 1 --topology topology.txt
```

With the topology.txt file as follows:

```txt
2
0 1
1 0
```

Output:

```txt
Execution time: 35s
Simulation time: 100266.000000
Channels (C): 30
Calls (n): 100000
Traffic Intensity (E): 25.000000
Arrival rate (λ): 1.000000
Service rate (μ): 25.000000
Duration of service (1/μ): 0.040000
Utilization (ρ): 0.040000
Idle rate (1-ρ): 0.960000
Grade of Service (ε): 0.005140
Busy Channels (1-ε): 24.871500
Occupancy ((1-ε)/C): 0.829050
Type 3
Ratio: 0.498140
BP(ε): 0.001150
Type 7
Ratio: 0.501860
BP(ε): 0.003990
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
