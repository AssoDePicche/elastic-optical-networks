import matplotlib.pyplot as plot
import numpy
import sys
import tensorflow

from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score,matthews_corrcoef,log_loss,roc_auc_score, confusion_matrix, ConfusionMatrixDisplay
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import StandardScaler
from tensorflow.keras.callbacks import  EarlyStopping
from tensorflow.keras.layers import Dense, Dropout
from tensorflow.keras.models import Sequential
from tensorflow.keras.optimizers import SGD

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('You must pass the .csv filename.')

        exit(1)

    features = numpy.genfromtxt(sys.argv[1], delimiter=',')

    expected = numpy.genfromtxt(sys.argv[2], delimiter=',')

    scaler = StandardScaler()

    x_train, x_val, y_train, y_val = train_test_split(features, expected, test_size=0.2, random_state=42)

    x_train = scaler.fit_transform(x_train)

    x_val = scaler.fit_transform(x_val)

    model = Sequential([
        Dense(64, input_dim=4, activation='relu'),
        Dropout(0.4),
        Dense(32, activation='relu'),
        Dense(16, activation='relu'),
        Dense(8, activation='relu'),
        Dense(1, activation='sigmoid')
    ])

    optimizer = SGD(learning_rate=0.01, momentum=0.9)

    model.compile(loss='binary_crossentropy', optimizer=optimizer, metrics=['accuracy', 'auc'])

    callbacks = [
        EarlyStopping(monitor='val_loss', patience=10, restore_best_weights=True)
    ]

    history = model.fit(x_train, y_train, validation_data=(x_val, y_val), epochs=50, batch_size=2, callbacks=callbacks)

    probabilities = model.predict(x_val).flatten()

    predictions = (probabilities > 0.5).astype(int)

    margins = numpy.abs(probabilities - 0.5)

    average_margin = numpy.mean(margins)

    accuracy = accuracy_score(y_val, predictions)

    precision = precision_score(y_val, predictions)

    recall = recall_score(y_val, predictions)

    f1 = f1_score(y_val, predictions)

    roc_auc = roc_auc_score(y_val, probabilities)

    mcc = matthews_corrcoef(y_val, predictions)

    log_loss_value = log_loss(y_val, probabilities)

    print(f'Accuracy: {accuracy:.2f}\n');

    print(f'Precision: {precision:.2f}\n');

    print(f'Recall:: {recall:.2f}\n');

    print(f'F1-Score: {f1:.2f}\n');

    print(f'ROC-AUC: {roc_auc:.2f}\n');

    print(f'MCC: {mcc:.2f}\n');

    print(f'Log-Loss: {log_loss_value:.2f}\n');

    print(f'Average Margin: {average_margin:.2f}\n');

    plot.plot(history.history['loss'], label='Training')

    plot.plot(history.history['val_loss'], label='Validation')

    plot.title('Loss Curve with Early Stopping')

    plot.xlabel('Epochs')

    plot.ylabel('Loss')

    plot.legend()

    plot.show()

    confusion_matrix = confusion_matrix(y_val, predictions)

    display = ConfusionMatrixDisplay(confusion_matrix=confusion_matrix)

    display.plot()

    plot.title('Confusion Matrix - Neural Network with SGD and Early Stopping')

    plot.show()

    plot.hist(margins, bins=10, edgecolor='black')

    plot.title('Margin Distribution')

    plot.xlabel('Margin')

    plot.ylabel('Frequency')

    plot.show()
