import time

from geopy.distance import geodesic

from geopy.geocoders import Nominatimz

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
    cities = [
        'Palo Alto', 'Seattle', 'Salt Lake City', 'Champaign', 'San Diego',
        'Houston', 'Boulder', 'Chicago', 'Lincoln', 'Pittsburgh',
        'Atlanta', 'College', 'Ann Arbor', 'Ithaca', 'Princeton', 'Cambridge'
    ]

    cities.sort()

    geolocator = Nominatim(user_agent="topology_generator_v1")

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

    output_filename = "nsfnet.txt"
    with open(output_filename, "w") as f:
        f.write(f"{len(cities)}\n")

        for row in matrix:
            line = " ".join(str(int(val)) for val in row)
            f.write(f"{line}\n")

    print(f"\nSuccess! Matrix saved to {output_filename}")
