## CSV2GV

This module contains a GV file generator, which reads a CSV file with columns named origin, destination and distance. For example:

| Source | Destination | Distance |
| ------ | ----------- | -------- |
| A      | B           | 1        |
| A      | C           | 1        |
| B      | C           | 1        |

A GV file is a document that includes descriptions about graphs and is written using the DOT language. You can read more about it [here](https://graphviz.org/).
## Usage

1. Compile the source code

```bash
g++ -std=c++23 csv_to_graphviz.cpp -o csv_to_graphviz
```

2. Pass the CSV filename (*without the extension*) as an argument and a GV file will be created

```bash
csv_to_graphviz ./filename
```

## Creating a SVG from a GV

```bash
dot -Tsvg filename.gv -o filename.svg
```
