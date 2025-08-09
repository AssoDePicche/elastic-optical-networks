import glob
import numpy
import os
import pandas
import shutil
import subprocess

from scipy import stats

if __name__ == '__main__':
  executable = './build/App'

  args = numpy.arange(1, 101, 10)

  try:
    if not os.path.isfile(executable):
      subprocess.run(['bash', './scripts/build.sh'], check=True)

    for arg in args:
      with subprocess.Popen([executable, str(1 / arg)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as process:
        for line in process.stdout:
          print(line, end='')

      print('Reading simulation results...')

      resources = glob.glob('./resources/temp/*.csv')

      dataframes = []

      for file in resources:
          df = pandas.read_csv(file)

          dataframes.append(df)

      dataframe = pandas.concat(dataframes, ignore_index=True)

      buffer = pandas.DataFrame(columns=[
        'column',
        'mean',
        'stddev',
        'ci_lower',
        'ci_upper',
      ])

      for column in dataframe.columns:
        dataframe[column].dropna()

        if column != 'time':
          mean = dataframe[column].mean()

          stddev = dataframe[column].std()

          sem = stats.sem(dataframe[column])

          length = len(dataframe[column])

          confidence_interval = stats.t.interval(confidence=.95, df=length-1, loc=mean, scale=sem)

          buffer = pandas.concat([buffer, pandas.DataFrame({
            'column': [column],
            'mean': [mean],
            'stddev': [stddev],
            'ci_lower': [confidence_interval[0]],
            'ci_upper': [confidence_interval[1]],
          })], ignore_index=True)

      filename = f'{arg:03d}_dataset.csv'

      buffer.to_csv(filename, index=False)

      print(f'Wrote results in {filename}')

      print('Removing simulation datasets in resources/temp')

      for file in resources:
        os.remove(file)

      dirname = f'resources/results/{arg:03d}'

      os.makedirs(dirname)

      print(f'Moving simulation reports to {dirname}')

      for file in glob.glob('./resources/temp/*.txt'):
        shutil.move(file, dirname)

      print('Done')
  except subprocess.CalledProcessError as error:
      print(error.stderr)
  except Exception as exception:
      print(exception)
