import matplotlib.pyplot as pyplot
import numpy
import pandas
from sklearn.preprocessing import MinMaxScaler

if __name__ == "__main__":
    dataframe = pandas.read_csv("benchmark.csv")

    metrics: list = dataframe.columns[1:].tolist()

    for column in metrics:
        dataframe[column] = MinMaxScaler().fit_transform(dataframe[[column]])

    theta = numpy.linspace(0, 2 * numpy.pi, len(metrics), endpoint=False)

    theta = numpy.concatenate([theta, [theta[0]]])

    figure, axis = pyplot.subplots(
        figsize=(12, 12), subplot_kw=dict(polar=True)
    )

    axis.set_theta_zero_location("N")

    axis.set_theta_direction(-1)

    axis.set_rlabel_position(90)

    axis.spines["polar"].set_zorder(1)

    axis.spines["polar"].set_color("lightgrey")

    colors: list = [
        "#FF595E",
        "#FFCA3A",
        "#8AC926",
        "#1982C4",
        "#6A4C93",
        "#FF6721",
        "#606C38",
        "#BC6C25",
        "#177E89",
    ]

    highlight = dataframe.sort_values(
        by=["MCC", "Training (s)", "Inference (s)"],
        ascending=[False, True, True],
    ).index[0]

    for index, (_, row) in enumerate(dataframe.iterrows()):
        values = row[metrics].values.flatten().tolist()

        values.append(values[0])

        if index == highlight:
            linewidth = 2.5
            alpha = 0.6
            linestyle = "solid"
        else:
            linewidth = 1.5
            alpha = 0.3
            linestyle = "dashed"

        axis.plot(
            theta,
            values,
            label=row["Model"],
            linewidth=linewidth,
            linestyle=linestyle,
            marker="o",
            color=colors[index % len(colors)],
        )

        axis.fill(theta, values, alpha=alpha, color=colors[index % len(colors)])

    axis.set_xticks(theta, metrics + [metrics[0]], color="black", size=12)

    axis.set_yticks(
        numpy.linspace(0.0, 1.0, 5),
        [".0", ".25", ".50", ".75", "1.0"],
        color="black",
        size=12,
    )

    axis.legend(
        loc="upper right", bbox_to_anchor=(1.3, 1.1), fontsize=10, frameon=False
    )

    pyplot.show()
