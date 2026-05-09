import sys

import time

from geopy.distance import geodesic

from geopy.geocoders import Nominatim

from geopy.exc import GeocoderTimedOut

def get_lat_long(city, geolocator):
    try:
        location = geolocator.geocode(city)
        if location:
            return (location.latitude, location.longitude)
        return None
    except GeocoderTimedOut:
        time.sleep(2)
        return get_lat_long(city, geolocator)

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: python main.py <filename> <output_filename>')

        sys.exit(1)

    input_file = sys.argv[1]

    try:
      with open(input_file, 'r') as stream:
        cities = [line.strip() for line in stream if line.strip()]
    except FileNotFoundError:
      print(f"Error: The file '{input_file}' was not found.")

      sys.exit(1)

    cities.sort()

    geolocator = Nominatim(user_agent="topology_generator_v2")

    coords_map = {}
    print(f"Fetching coordinates for {len(cities)} nodes...")

    for city in cities:
        coords = get_lat_long(city, geolocator)
        coords_map[city] = coords
        print(f"-> {city}: {coords}")
        time.sleep(1.2)

    matrix = []
    for source in cities:
        costs = []
        source_coords = coords_map[source]

        for destination in cities:
            dest_coords = coords_map[destination]

            if source == destination:
                cost = 0
            elif source_coords and dest_coords:
                cost = geodesic(source_coords, dest_coords).km
            else:
                cost = 0

            costs.append(cost)
        matrix.append(costs)

    output_filename = sys.argv[2]

    with open(output_filename, 'w') as stream:
        stream.write(f"{len(cities)}\n")

        for row in matrix:
            line = ' '.join(str(int(val)) for val in row)
            stream.write(f"{line}\n")

    print(f"\nSuccess! Matrix saved to {output_filename}")
