import glob
import matplotlib.pyplot as pyplot
import numpy
import os
import pandas
import shutil
import subprocess
import sys
import tempfile

from concurrent.futures import ProcessPoolExecutor, as_completed
from scipy import stats

def execute(executable, load):
  service_rate = 1 / load

  configuration = './resources/configuration/configuration.json'

  with tempfile.TemporaryDirectory() as temp_dir:
    with subprocess.Popen([executable, str(service_rate), configuration, temp_dir], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as process:
      for line in process.stdout:
        print(f'[Load={load:03d}] {line}', end='')

    files = glob.glob(f'./{temp_dir}/*.csv')

    if not files:
      raise Exception('No CSV file found for {load} Erlangs')

    buffer = pandas.concat([pandas.read_csv(file) for file in files], ignore_index=True)

    rows = []

    for column in buffer.columns:
      data = buffer[column].dropna()

      if len(data) == 0:
        continue

      mean = data.mean()

      stddev = data.std()

      sem = stats.sem(data)

      if numpy.isnan(sem) or numpy.isinf(sem) or sem == 0:
        ci = [mean, mean]
      else:
        ci = stats.t.interval(confidence=.95, df=len(data) - 1, loc=mean, scale=sem)

      rows.append({
        'column': column,
        'mean': mean,
        'stddev': stddev,
        'ci_lower': ci[0],
        'ci_upper': ci[1],
      })

    dataframe = pandas.DataFrame(rows)

    dataframe['load'] = load

    return dataframe

def main(executable, min_load, max_load, offset) -> None:
  if not os.path.isfile(executable):
    subprocess.run(['bash', './scripts/build.sh'], check=True)

  loads = numpy.arange(min_load, max_load + 1, offset)

  if min_load == 0:
    loads[0] = 1

  print('Beginning')

  buffer = []

  with ProcessPoolExecutor() as executor:
    tasks = { executor.submit(execute, executable, load): load for load in loads }

    for task in as_completed(tasks):
      buffer.append(task.result())

  print('Simulations completed\nWriting dataset.csv')

  pandas.concat(buffer, ignore_index=True).to_csv('dataset.csv', index=False)

  print('Done')

if __name__ == '__main__':
  if len(sys.argv) != 4:
    print(f'You must inform min/max load and the offset')

    exit(1)

  executable = './build/App'

  min_load = int(sys.argv[1])

  max_load = int(sys.argv[2])

  offset = int(sys.argv[3])

  if min_load < 0:
    min_load = 0

  try:
    main(executable, min_load, max_load, offset)
  except subprocess.CalledProcessError as error:
    print(exception.stderr)
  except Exception as exception:
    print(exception)
