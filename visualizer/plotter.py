# Copyright © 2021 Vladimir Erofeev. All rights reserved.

from matplotlib import style
from mpl_toolkits.mplot3d import axes3d
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.ticker import FuncFormatter
import matplotlib.patches as mpatches


def b_formatter(x, pos):
    x = float(x)
    kilo = 1024
    mega = kilo * kilo
    giga = kilo * kilo * kilo
    if x < kilo:
        return '%dB' % x
    elif x < mega:
        return '%dKB' % (x / kilo)
    elif x < giga:
        return '%dKB' % (x / mega)
    return '%dGB' % (x / giga)


def bps_formatter(x, pos):
    x = float(x)
    kilo = 1024
    mega = kilo * kilo
    giga = kilo * kilo * kilo
    if x < kilo:
        return '%dB/s' % x
    elif x < mega:
        if (x / kilo) >= 10:
            return '%dKB/s' % (x / kilo)
        return '%.1fKB/s' % (x / kilo)
    elif x < giga:
        if (x / mega) >= 10:
            return '%dKB/s' % (x / mega)
        return '%.1fMB/s' % (x / mega)
    if (x / giga) >= 10:
        return '%dGB/s' % (x / giga)
    return '%.1fGB/s' % (x / giga)


def get_colour(mean, std):
    if std >= 0.5 * mean:
        return 'k'
    elif std >= 0.20 * mean:
        return 'r'
    elif std >= 0.10 * mean:
        return 'y'
    else:
        return 'g'


def plot_2d_bars(result):
    factors = result.measurements[0].factors
    for key in factors.keys():
        factor = str(key)
        level = factors[key]

    msrCnt = len(result.measurements)
    fig, ax = plt.subplots()

    # X tick labels and their coordinates as [1, 2, 3, ..., msrCnt]
    xlabels = np.empty(msrCnt, dtype=object)
    xlevels = np.empty(msrCnt, dtype=int)
    xticks = np.arange(start=1, stop=msrCnt + 1, step=1)
    x = np.zeros(msrCnt)
    bs = 0.25

    for i, msr in enumerate(result.measurements):
        # + 0 ~ avg, + 1 ~ min, + 2 ~ max
        xlevels[i] = msr.factors[key]
        x[i] = xticks[i] - bs

    xlevels = np.sort(xlevels)
    for i in range(msrCnt):
        xlabels[i] = b_formatter(xlevels[i], pos=None) if factor == "RS" else xlevels[i]
        
    # Defining sizes of bars
    dx = np.ones(msrCnt)
    dy = np.ones(msrCnt)

    for i in range(msrCnt):
        dx[i] = bs * 2
        dy[i] = result.measurements[i].throughput.mean

    color = np.full(msrCnt, 'b')
    for i in range(msrCnt):
        color[i] = get_colour(dy[i], result.measurements[i].throughput.std)

    ax.yaxis.set_major_formatter(FuncFormatter(bps_formatter))

    # Set ticks to equal factor levels
    ax.set_xticks(xticks)
    ax.set_xticklabels(xlabels)

    ax.bar(x, dy, dx, color=color, align='edge')

    pattern = result.pattern;
    title = "IsConsecutive: {}, IsRead: {}\nMetric: {}".format(pattern.is_consecutive, pattern.is_read, "Throughput")

    ax.set_title(title)
    ax.set_xlabel(key)

    # Averages
    black_patch = mpatches.Patch(color='k', label='std >= 0.5 * mean')
    red_patch = mpatches.Patch(color='r', label='std >= 0.20 * mean')
    yellow_patch = mpatches.Patch(color='y', label='std >= 0.10 * mean')
    green_patch = mpatches.Patch(color='g', label='std < 0.10 * mean')

    patches = [black_patch, red_patch, yellow_patch, green_patch]

    plt.legend(handles=patches, loc='upper left')

    plt.show()


def plot_3d_bars(result):
    factors = result.measurements[0].factors
    keys = []
    for key in factors.keys():
        keys.append(str(key))

    msrCnt = len(result.measurements)
    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    xlevels = np.empty(msrCnt, dtype=int)
    ylevels = np.empty(msrCnt, dtype=int)

    # Defining positions of bars
    x = np.zeros(msrCnt)
    y = np.zeros(msrCnt)
    z = np.zeros(msrCnt)

    bs = 0.25
    for i, msr in enumerate(result.measurements):
        xlevels[i] = msr.factors[keys[0]]
        ylevels[i] = msr.factors[keys[1]]

        # For now save here the factor levels to use later
        x[i] = msr.factors[keys[0]]
        y[i] = msr.factors[keys[1]]

    # np.unique returns a sorted array of unique values
    xlevels = np.unique(xlevels)
    ylevels = np.unique(ylevels)

    xticks = np.arange(start=1, stop=len(xlevels) + 1, step=1)
    yticks = np.arange(start=1, stop=len(ylevels) + 1, step=1)

    xlabels = np.empty(len(xlevels), dtype=object)
    ylabels = np.empty(len(ylevels), dtype=object)

    # Dicts to find ticks array index by factor level
    xdict = dict()
    ydict = dict()

    # Fill the dicts and labels
    for i in range(len(xlevels)):
        xdict[xlevels[i]] = i
        xlabels[i] = b_formatter(xlevels[i], pos=None) if keys[0] == "RS" else xlevels[i]
    for i in range(len(ylevels)):
        ydict[ylevels[i]] = i
        ylabels[i] = b_formatter(ylevels[i], pos=None) if keys[1] == "RS" else ylevels[i]

    # If, for example, len(xlevels) << len(ylevels), then bars would be too wide on one x axis.
    # Here we compensate for that.
    x_to_y = int(len(xlevels) / len(ylevels))
    y_to_x = int(len(ylevels) / len(xlevels))
    dx_div = 1
    dy_div = 1
    if x_to_y > 2:
        dy_div = x_to_y
    if y_to_x > 2:
        dx_div = y_to_x

    xbs = bs / dx_div
    ybs = bs / dy_div

    for i in range(msrCnt):
        j = xdict[x[i]]
        k = ydict[y[i]]
        x[i] = xticks[j] - xbs
        y[i] = yticks[k] - ybs
        
    # Defining sizes of bars
    dx = np.ones(msrCnt)
    dy = np.ones(msrCnt)
    dz = np.ones(msrCnt)

    for i in range(msrCnt):
        dx[i] = xbs * 2
        dy[i] = ybs * 2
        dz[i] = result.measurements[i].throughput.mean

    color = np.full(msrCnt, 'b')
    for i in range(msrCnt):
        color[i] = get_colour(dz[i], result.measurements[i].throughput.std)

    ax.zaxis.set_major_formatter(FuncFormatter(bps_formatter))

    # Set ticks to equal factor levels
    ax.set_xticks(xticks)
    ax.set_xticklabels(xlabels)
    ax.set_yticks(yticks)
    ax.set_yticklabels(ylabels)

    ax.bar3d(x, y, z, dx, dy, dz, color=color, shade=True)

    pattern = result.pattern;
    title = "IsConsecutive: {}, IsRead: {}\nMetric: {}".format(pattern.is_consecutive, pattern.is_read, "Throughput")

    ax.set_title(title)
    ax.set_xlabel(keys[0])
    ax.set_ylabel(keys[1])

    # Averages
    black_patch = mpatches.Patch(color='k', label='std >= 0.5 * mean')
    red_patch = mpatches.Patch(color='r', label='std >= 0.20 * mean')
    yellow_patch = mpatches.Patch(color='y', label='std >= 0.10 * mean')
    green_patch = mpatches.Patch(color='g', label='std < 0.10 * mean')

    patches = [black_patch, red_patch, yellow_patch, green_patch]

    plt.legend(handles=patches, bbox_to_anchor=(1.05, 1), loc='upper left')

    plt.show()


def plot_result(result):
    style.use('ggplot')
    factorsCnt = len(result.measurements[0].factors)
    if factorsCnt == 1:
        plot_2d_bars(result)
    elif factorsCnt == 2:
        plot_3d_bars(result)
    else:
        raise Exception('Plotting is not supported for {} factors'.format(factorsCnt))
