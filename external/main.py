import matplotlib.pyplot as pyplot
import numpy
import pandas
import seaborn
from imblearn.over_sampling import SMOTE
from pandas import DataFrame
from sklearn.cluster import KMeans
from sklearn.linear_model import LogisticRegression, SGDClassifier
from sklearn.metrics import (classification_report, f1_score,
                             matthews_corrcoef, roc_auc_score)
from sklearn.model_selection import GridSearchCV, KFold
from sklearn.naive_bayes import GaussianNB
from sklearn.preprocessing import LabelEncoder, StandardScaler

if __name__ == "__main__":
    RANDOM_STATE: int = 42

    # (7.51 + 14.68 + 29.36) E | 10 bandwidth | Seed 0
    dataframe: DataFrame = pandas.concat(
        [
            pandas.read_csv("dataset.csv"),
        ]
    )

    dataframe.reset_index(inplace=True, drop=True)

    dataframe["mean_fragmentation"] = dataframe[
        ["from_1_to_0_fragmentation", "from_0_to_1_fragmentation"]
    ].mean(axis=1)

    dataframe["mean_entropy"] = dataframe[
        ["from_1_to_0_entropy", "from_0_to_1_entropy"]
    ].mean(axis=1)

    print(dataframe.head())

    print(dataframe.describe())

    print(dataframe.isnull().sum())

    print(dataframe.dtypes)

    histogram = dataframe.hist(figsize=(10, 10))

    correlation = dataframe.corr()

    pyplot.figure(figsize=(12, 8))

    seaborn.heatmap(correlation, annot=True, cmap="coolwarm", fmt=".2f")

    pyplot.title("Correlação")

    pyplot.show()

    for column in ["mean_entropy", "mean_fragmentation"]:
        pearson_correlation = dataframe["blocking"].corr(dataframe[column])

        print(
            f"Correlação de Pearson (blocking,{column}): {pearson_correlation}"
        )

    print(dataframe["accepted"].value_counts())

    seaborn.countplot(x="accepted", data=dataframe)

    pyplot.title("Distribuição de requisições")

    pyplot.show()

    seaborn.boxplot(x="accepted", y="blocking", data=dataframe)

    pyplot.title("Boxplot accepted x blocking")

    pyplot.show()

    seaborn.boxplot(x=dataframe["blocking"])

    pyplot.title("Boxplot blocking")

    pyplot.show()

    stratification = [
        dataframe[
            (dataframe["accepted"] == True) & (dataframe["bandwidth"] == 3)
        ],
        dataframe[
            (dataframe["accepted"] == True) & (dataframe["bandwidth"] == 5)
        ],
        dataframe[
            (dataframe["accepted"] == False) & (dataframe["bandwidth"] == 3)
        ],
        dataframe[
            (dataframe["accepted"] == False) & (dataframe["bandwidth"] == 5)
        ],
    ]

    for data in stratification:
        pyplot.figure(figsize=(8, 6))

        seaborn.heatmap(
            data.corr(), annot=True, cmap="coolwarm", fmt=".2f", cbar=True
        )

    x, y = SMOTE(random_state=RANDOM_STATE).fit_resample(
        dataframe.drop(columns=["accepted"]), dataframe["accepted"]
    )

    x = StandardScaler().fit_transform(x)

    dataframe["accepted"] = LabelEncoder().fit_transform(dataframe["accepted"])

    print(dataframe.head())

    kfold: KFold = KFold(n_splits=10, shuffle=True, random_state=RANDOM_STATE)

    classifiers = [
        (GaussianNB(), {}),
        (SGDClassifier(), {}),
        (LogisticRegression(max_iter=5000, random_state=RANDOM_STATE), {}),
    ]

    for classifier, param_grid in classifiers:
        grid_search = GridSearchCV(
            classifier, param_grid=param_grid, scoring="f1", cv=kfold
        )

        grid_search.fit(x, y)

        best_classifier = grid_search.best_estimator_

        f1_scores = []

        roc_auc_scores = []

        mcc_scores = []

        y_trues = []

        y_predictions = []

        for train_index, test_index in kfold.split(x, y):
            x_train, x_test = x[train_index], x[test_index]

            y_train, y_test = y[train_index], y[test_index]

            y_predicted = best_classifier.fit(x_train, y_train).predict(x_test)

            y_trues.extend(y_test)

            y_predictions.extend(y_predicted)

            f1_scores.append(f1_score(y_test, y_predicted, zero_division=0.0))

            roc_auc_scores.append(roc_auc_score(y_test, y_predicted))

            mcc_scores.append(matthews_corrcoef(y_test, y_predicted))

        print(f"Classifier: {best_classifier.__class__.__name__}")

        print(f"AVG F1 Score: {numpy.mean(f1_scores): .4f}")

        print(f"AVG ROC-AUC Score: {numpy.mean(roc_auc_scores): .4f}")

        print(f"AVG MCC: {numpy.mean(mcc_scores): .4f}")

        print(classification_report(y_trues, y_predictions, zero_division=0.0))
