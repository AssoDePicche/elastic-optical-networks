import numpy
import pandas
from imblearn.over_sampling import SMOTE
from pandas import DataFrame
from sklearn.ensemble import (AdaBoostClassifier, BaggingClassifier,
                              RandomForestClassifier)
from sklearn.linear_model import LogisticRegressionCV, SGDClassifier
from sklearn.metrics import (accuracy_score, classification_report, f1_score,
                             log_loss, matthews_corrcoef, precision_score,
                             recall_score, roc_auc_score)
from sklearn.model_selection import GridSearchCV, KFold
from sklearn.naive_bayes import GaussianNB
from sklearn.neighbors import KNeighborsClassifier
from sklearn.neural_network import MLPClassifier
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.svm import LinearSVC
from sklearn.tree import DecisionTreeClassifier
from xgboost import XGBClassifier

if __name__ == "__main__":
    # (7.51 + 14.68 + 29.36) E | 10 bandwidth | Seed 0
    dataframe: DataFrame = pandas.concat(
        [
            pandas.read_csv("dataset.csv"),
        ]
    )

    dataframe.reset_index(inplace=True, drop=True)

    dataframe["accepted"] = LabelEncoder().fit_transform(dataframe["accepted"])

    x, y = SMOTE().fit_resample(
        dataframe.drop(columns=["accepted"]), dataframe["accepted"]
    )

    x = StandardScaler().fit_transform(x)

    classifiers = [
        (GaussianNB(), {}),
        (SGDClassifier(), {}),
        (LogisticRegressionCV(), {}),
        (DecisionTreeClassifier(), {}),
        (KNeighborsClassifier(), {}),
        (LinearSVC(), {}),
        (AdaBoostClassifier(), {}),
        (BaggingClassifier(), {}),
        (RandomForestClassifier(), {}),
        (
            MLPClassifier(
                solver="lbfgs", alpha=1e-5, hidden_layer_sizes=(5, 2)
            ),
            {},
        ),
        (XGBClassifier(), {}),
    ]

    kfold: KFold = KFold(n_splits=10, shuffle=True)

    for classifier, param_grid in classifiers:
        grid_search = GridSearchCV(
            classifier, param_grid=param_grid, scoring="f1", cv=kfold
        )

        grid_search.fit(x, y)

        best_classifier = grid_search.best_estimator_

        metrics = [
            (accuracy_score, []),
            (precision_score, []),
            (recall_score, []),
            (f1_score, []),
            (roc_auc_score, []),
            (matthews_corrcoef, []),
            (log_loss, []),
        ]

        y_trues = []

        y_predictions = []

        for train_index, test_index in kfold.split(x, y):
            x_train, x_test = x[train_index], x[test_index]

            y_train, y_test = y[train_index], y[test_index]

            y_predicted = best_classifier.fit(x_train, y_train).predict(x_test)

            y_trues.extend(y_test)

            y_predictions.extend(y_predicted)

            for metric, score in metrics:
                score.append(metric(y_test, y_predicted))

        print(f"{best_classifier.__class__.__name__}")

        for metric, score in metrics:
            print(f"avg {metric.__name__}: {numpy.mean(score):.4f}")

        print(classification_report(y_trues, y_predictions, zero_division=0.0))
