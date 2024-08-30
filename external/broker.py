import csv
import numpy
import sys
import tensorflow
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense

if __name__ == '__main__':
    data = numpy.array([])

    if len(sys.argv) < 2:
        print('You must pass the .csv filename.')

        exit(1)

    with open(sys.argv[1], 'r') as csv_file:
        reader = csv.reader(csv_file)

        data = numpy.append(data, list(reader))

    labels = numpy.array([])

    for connection in data:
        labels = numpy.append(labels, connection[0])

    model = Sequential()

    model.add(Dense(64, input_dim=5, activation='relu'))

    model.add(Dense(32, activation='relu'))

    model.add(Dense(1, activation='sigmoid'))

    model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])

    model.fit(data, labels, epochs=10, batch_size=2)

    loss, accuracy = model.evaluate(data, labels)

    predictions = model.predict(data)

    print(f'Loss: {loss}, Accuracy: {accuracy}')

    print(f'Predictions:\n', predictions)
