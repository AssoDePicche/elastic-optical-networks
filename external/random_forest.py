import numpy
import sys

from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import confusion_matrix, precision_score, recall_score, f1_score, roc_auc_score

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('You must pass the .csv filename.')

        exit(1)

    features = numpy.genfromtxt(sys.argv[1], delimiter=',')

    labels = numpy.genfromtxt(sys.argv[2], delimiter=',')

    x_train, x_test, y_train, y_test = train_test_split(features, labels, test_size=0.2)

    classifier = RandomForestClassifier()

    classifier.fit(x_train, y_train)

    y_predictions = classifier.predict(x_test)

    matrix = confusion_matrix(y_test, y_predictions)

    precision = precision_score(y_test, y_predictions)

    recall = recall_score(y_test, y_predictions)

    f1 = f1_score(y_test, y_predictions)

    y_probabilities = classifier.predict_proba(x_test)[:, 1]

    auc = roc_auc_score(y_test, y_probabilities)

    print(f'Precision: {precision:.2f}\n')

    print(f'Recall: {recall:.2f}\n')

    print(f'F1-Score: {f1:.2f}\n')

    print(f'AUC-ROC: {auc:.2f}\n')
