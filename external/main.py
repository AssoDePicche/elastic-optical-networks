import numpy
import pandas
from sklearn.linear_model import LogisticRegression, SGDClassifier
from sklearn.metrics import (classification_report, f1_score,
                             matthews_corrcoef, roc_auc_score)
from sklearn.model_selection import GridSearchCV, KFold
from sklearn.naive_bayes import GaussianNB
from sklearn.preprocessing import LabelEncoder, OneHotEncoder, StandardScaler


def make_report(classifier, x, y, kfold):
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

        f1_scores.append(f1_score(y_test, y_predicted))

        roc_auc_scores.append(roc_auc_score(y_test, y_predicted))

        mcc_scores.append(matthews_corrcoef(y_test, y_predicted))

    report = classification_report(y_trues, y_predictions)

    return {
        "f1_score": numpy.mean(f1_scores),
        "roc_auc": numpy.mean(roc_auc_scores),
        "mcc": numpy.mean(mcc_scores),
        "classification_report": report,
    }


if __name__ == "__main__":
    RANDOM_STATE: int = 42

    # (7.51 + 14.68 + 29.36) E | 10 channels | Seed 0
    dataframe: pandas.DataFrame = pandas.read_csv("dataset10.csv")

    dataframe2: pandas.DataFrame = pandas.read_csv("dataset40.csv")

    dataframe3: pandas.DataFrame = pandas.read_csv("dataset85.csv")

    dataframe = pandas.concat([dataframe, dataframe2, dataframe3])

    dataframe.reset_index(inplace=True, drop=True)

    x = dataframe.drop(columns=["accepted"])

    y = dataframe["accepted"]

    y = LabelEncoder().fit_transform(y)

    categorical_columns = x.select_dtypes(include=["object"]).columns.tolist()

    encoder = OneHotEncoder(sparse_output=False, drop="first")

    encoded = encoder.fit_transform(x[categorical_columns])

    encoded_dataframe = pandas.DataFrame(
        encoded, columns=encoder.get_feature_names_out(categorical_columns)
    )

    x = pandas.concat(
        [x.drop(columns=categorical_columns), encoded_dataframe], axis=1
    )

    x = StandardScaler().fit_transform(x)

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

        report = make_report(best_classifier, x, y, kfold)

        print(f"Classifier: {classifier.__class__.__name__}")

        print(f"AVG F1-Score: {report['f1_score']: .4f}")

        print(f"AVG ROC-AUC Score: {report['roc_auc']: .4f}")

        print(f"AVG MCC: {report['mcc']: .4f}")

        print(f"Classification Report:\n{report['classification_report']}")
