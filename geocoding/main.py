import sys

from extractor import extract_unique_cities

from geocode import fetch_coordinates, fetch_distance_km

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python main.py <filename> <output_filename>')

        sys.exit(1)

    input_file = sys.argv[1]

    cities = []

    if input_file.endswith('.csv'):
      cities = extract_unique_cities(input_file, 'cities.txt')
    else:
      try:
        with open(input_file, 'r') as stream:
          cities = [line.strip() for line in stream if line.strip()]
      except FileNotFoundError:
          print(f"Error: The file '{input_file}' was not found.")

          sys.exit(1)

    print(f"Fetching coordinates for {len(cities)} nodes...")

    coords_map = fetch_coordinates(cities)

    for city, coords in coords_map.items():
        print(f"-> {city}: {coords}")

    matrix = []

    for source in cities:
        costs = []

        for destination in cities:
            cost = fetch_distance_km(coords_map, source, destination)

            costs.append(cost)

        matrix.append(costs)

    output_filename = sys.argv[2]

    with open(output_filename, 'w') as stream:
        stream.write(f"{len(cities)}\n")

        for row in matrix:
            line = ' '.join(str(int(val)) for val in row)

            stream.write(f"{line}\n")

    print(f"\nSuccess! Matrix saved to {output_filename}")
