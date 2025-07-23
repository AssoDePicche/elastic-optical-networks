This repository contains an event-driven simulator for elastic optical networks to study the problem of routing, modulation, and spectrum allocation (RMSA), as well as quality of service metrics.

> More detailed documentation is currently being written. Please wait. Consider visiting the [official page](https://assodepicche.github.io/elastic-optical-networks/) for this application.
## Scope and Features

The scope of this project consists of analyzing network behavior in different configuration contexts, such as topology, traffic, routing policies, and resource allocation. Multiple topologies are available by default, however, it is possible to use any desired topology, as long as it follows the application standards.

The algorithms available are:
- **Quality of Service:** Absolute Fragmentation, External Fragmentation and Entropy-based Fragmentation.
- **Routing:** Breadth-first Search, Depth-first Search, Dijkstra's algorithm and K Shortest Paths.
- **Spectrum Allocation:** Best fit, First fit, Last fit, Random fit and Worst fit.

## Citation

If you use this application in your research, please cite:

```bibtext
@misc{rodrigues2025eon-simulator,
  author       = {Samuel do Prado Rodrigues and Rodrigo Campos Bortoletto},
  title        = {Elastic Optical Networks Simulator},
  year         = {2025},
  howpublished = {\url{https://github.com/samuel-rodrigues/elastic-optical-networks}},
  note         = {Accessed: \today},
  orcid        = {Samuel do Prado Rodrigues: https://orcid.org/0009-0002-0169-5804, Rodrigo Campos Bortoletto: https://orcid.org/0000-0002-9570-5833}
}
```

## Building From Source

First install and configure all necessary dependencies, then clone this repository:

```bash
git clone https://github.com/AssoDePicche/elastic-optical-networks.git --recursive && cd elastic-optical-networks
```

After that, use the available scripts to perform the build, tests, and profiling.

```bash
./scripts/build.sh
```

## Running

Once the application has been built, you can run it. To modify the simulation parameters, edit the [settings.json](resources/configuration/settings.json) file.

```bash
./build/App
```

When the simulation ends, it saves a .csv file with the simulation data and a report.txt file is created with the results.

## A Bit Of History

In August 2023, I began my scientific initiation program under the guidance of [Dr. Rodrigo Campos Bortoletto](https://orcid.org/0000-0002-9570-5833) ([ORCID](https://orcid.org/0000-0002-9570-5833)) in the LABCOM³ research group for the study of Elastic Optical Networks. Since then, I have sought to contribute to the academic community through the development of this software.

## License

This project is licensed under the [MIT License](LICENSE), see [LICENSE](LICENSE) for more information.

## Support This Project

This application is developed and maintained by me, [Samuel do Prado Rodrigues](https://orcid.org/0009-0002-0169-5804) ([ORCID](https://orcid.org/0009-0002-0169-5804)) under the guidance of my research advisor, [Dr. Rodrigo Campos Bortoletto](https://orcid.org/0000-0002-9570-5833) ([ORCID](https://orcid.org/0000-0002-9570-5833)), to support academic research on Elastic Optical Networks. If you find it useful, please consider supporting our work through [GitHub Sponsors](https://github.com/sponsors/AssoDePicche)

## Acknowledgments

I would like to thank my research advisor for all his support and my colleagues at LABCOM³ for guiding me throughout my studies.
