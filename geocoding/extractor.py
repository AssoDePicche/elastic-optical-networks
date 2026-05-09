import csv
import sys
import os

def extract_unique_cities(input_file, output_file):
    if not os.path.exists(input_file):
        print(f"Error: {input_file} not found.")

        sys.exit(1)

    cities = set()

    try:
        with open(input_file, mode='r', encoding='utf-8') as f:
            reader = csv.DictReader(f)

            for row in reader:
                cities.add(row['Origem'].strip())

                cities.add(row['Destino'].strip())

        sorted_cities = sorted(list(cities))

        with open(output_file, mode='w', encoding='utf-8') as f:
            for city in sorted_cities:
                f.write(f"{city}\n")

        print(f"Done! {len(sorted_cities)} cities saved to {output_file}")

        return sorted_cities
    except Exception as e:
        print(f"An error occurred: {e}")

        sys.exit(1)
