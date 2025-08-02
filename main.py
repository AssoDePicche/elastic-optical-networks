import glob
import os
import pandas
import subprocess

from scipy import stats

if __name__ == '__main__':
  executable = './build/App'

  try:
    if not os.path.isfile(executable):
      subprocess.run(['bash', './scripts/build.sh'], check=True)

    with subprocess.Popen([executable], stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True) as process:
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

    buffer.to_csv('dataset.csv', index=False)

    print('Wrote results in dataset.csv')

    print('Removing simulation datasets in resources/temp')

    for file in resources:
      os.remove(file)

    print('Done')
  except subprocess.CalledProcessError as error:
    print(error.stderr)
  except Exception as exception:
    print(exception)
