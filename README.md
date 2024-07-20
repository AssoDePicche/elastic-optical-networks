# Elastic Optical Networks

This repository contains an elastic optical network simulator developed for personal studies, initially as part of the first undergraduate research program in which I participated during September of 2023 and August of 2024, under the supervision of Dr. Rodrigo Campos Bortoletto.

The studies consisted mainly of machine learning algorithms applied to routing, modulation and spectrum assignment. The topologies adopted for analysis were Point-to-Point, Arpanet, Ipê, Germany 17, Abilene, JPN48, NSFNET and Pan-European.

## Table of Contents

- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [Contact](#contact)
- [License](#license)

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

To run the project, execute the binary created in the [Installation section](#installation). At the end of the simulation, the network blocking probability calculated by the [Erlang B formula](https://en.wikipedia.org/wiki/Erlang_(unit)) and the time spent on the simulation will be available.

```bash
./build/source/App
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
