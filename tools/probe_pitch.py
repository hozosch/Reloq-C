import math
import struct
import sys


def read(path):
    data = open(path, "rb").read()
    return struct.unpack("<%dh" % (len(data) // 2), data)


def rms(frame):
    return math.sqrt(sum(x * x for x in frame) / len(frame))


def pitch(frame, rate=11025):
    n = len(frame)
    if rms(frame) < 500:
        return 0
    best_lag = 0
    best_corr = 0
    for lag in range(40, 160):
        num = sum(frame[i] * frame[i + lag] for i in range(n - lag))
        denominator = math.sqrt(
            sum(frame[i] * frame[i] for i in range(n - lag))
            * sum(frame[i + lag] * frame[i + lag] for i in range(n - lag))
        )
        if denominator == 0:
            continue
        corr = num / denominator
        if corr > best_corr:
            best_corr = corr
            best_lag = lag
    if best_lag == 0 or best_corr < 0.4:
        return 0
    return rate / best_lag


for name in sys.argv[1:]:
    signal = read(name)
    print(name, len(signal))
    step = 110
    for i in range(0, len(signal) - step, step):
        frame = signal[i:i + step]
        value = pitch(frame)
        level = rms(frame)
        if level > 500:
            print(i // step, round(value, 1), round(level))
