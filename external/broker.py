import numpy
import sys
import tensorflow
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import Dense

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('You must pass the .csv filename.')

        exit(1)

    dataset = numpy.genfromtxt(sys.argv[1], delimiter=',')

    labels = numpy.array([])

    for row in dataset:
        labels = numpy.append(labels, row[1])

    model = Sequential()

    model.add(Dense(64, input_dim=5, activation='relu'))

    model.add(Dense(32, activation='relu'))

    model.add(Dense(1, activation='sigmoid'))

    model.compile(loss='binary_crossentropy', optimizer='adam', metrics=['accuracy'])

    model.fit(dataset, labels, epochs=10, batch_size=2)

    loss, accuracy = model.evaluate(dataset, labels)

    predictions = model.predict(dataset)

    print(f'Loss: {loss}, Accuracy: {accuracy}')

    print(f'Predictions:\n', predictions)
