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


def fetch_coordinates(cities):
    cities.sort()

    geolocator = Nominatim(user_agent="topology_generator_v2")

    coords_map = {}

    for city in cities:
        coords = get_lat_long(city, geolocator)

        coords_map[city] = coords

        print(f"-> {city}: {coords}")

        time.sleep(1.2)

    return coords_map


def fetch_distance_km(coord_map, source, destination):
    source_coords = coord_map[source]

    destination_coords = coord_map[source]

    if source == destination:
        return 0;
    elif source_coords and destination_coords:
        return geodesic(source_coords, destination_coords).km
    else:
        return 0
