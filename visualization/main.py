import numpy
import pandas
import sys

import matplotlib

matplotlib.use("Agg")

from matplotlib import pyplot
from scipy.interpolate import make_interp_spline
from scipy.optimize import bisect
from scipy.signal import savgol_filter
from scipy.special import factorial
from sklearn.preprocessing import MinMaxScaler

def erlang_b(grade_of_service, resources):
    return (grade_of_service**resources) / factorial(resources) / sum((grade_of_service**k) / factorial(k) for k in range(resources + 1))

def fn(grade_of_service, resources, desired_grade_of_service):
    return erlang_b(grade_of_service, resources) - desired_grade_of_service

def inverse_erlang_b():
  if len(sys.argv) != 4:
    print(f'You must inform the number of resources and desired grade of service')

    exit(1)

  resources = int(sys.argv[2])

  desired_grade_of_service = float(sys.argv[3])

  erlangs = bisect(fn, 0.1, 100, args=(resources, desired_grade_of_service))

  print(f"{erlangs:.4f} E")

def dataset_plotting():
  if len(sys.argv) != 3:
    print(f'You must inform the path to the dataset')

    exit(1)

  dataframe = pandas.read_csv(sys.argv[2])

  min_max_scale = ['external_fragmentation']

  z_score_scale = []

  y = ['grade_of_service', 'slot_blocking_probability', 'active_requests', 'absolute_fragmentation', 'entropy', 'external_fragmentation']

  translation: dict[str, dict[str, str]] = {
      'pt_BR': {
        'time': 'Tempo [t]',
        'grade_of_service': 'Probabilidade de Bloqueio',
        'slot_blocking_probability': 'Probabilidade de Bloqueio por FSU',
        'active_requests': 'Conexões Ativas',
        'absolute_fragmentation': 'Fragmentação Absoluta',
        'entropy': 'Fragmentação Ponderada (WF)',
        'external_fragmentation': 'Fragmentação Externa (EF)'
    }
  }

  for column in dataframe.columns:
    if dataframe[column].isna().sum() != 0:
      dataframe[column] = dataframe[column].fillna(dataframe[column].mean())
    if column in min_max_scale:
      dataframe[column] = MinMaxScaler().fit_transform(numpy.array(dataframe[column]).reshape(-1,1))
    if column in z_score_scale:
      dataframe[column] = (dataframe[column] - dataframe[column].mean()) / dataframe[column].std()

  x = dataframe['time']

  x_ = numpy.linspace(x.min(), x.max(), 500)

  for column in y:
    spline = make_interp_spline(x, dataframe[column])

    y_ = spline(x_)

    pyplot.plot(x_, y_)

    pyplot.xlabel(translation['pt_BR']['time'])

    pyplot.ylabel(translation['pt_BR'][column])

    pyplot.yscale('log')

    pyplot.grid()

    pyplot.savefig(f"{column}.png")

    pyplot.clf()

if __name__ == '__main__':
  if len(sys.argv) == 1:
    print(f'You must inform what application to call (try: --erlang or --plot)')

    exit(1)

  command = sys.argv[1]

  commands = {
    '--erlang': inverse_erlang_b,
    '--plot': dataset_plotting
  }

  try:
    if command not in commands.keys():
      raise Exception('Command not implemented')
    else:
      commands[command]()
  except Exception as exception:
    print(f'Error: {exception}')
