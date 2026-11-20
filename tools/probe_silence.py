import struct
import sys


def read(path):
    data = open(path, "rb").read()
    return struct.unpack("<%dh" % (len(data) // 2), data)


for name in sys.argv[1:]:
    signal = read(name)
    window = 110
    print(name, len(signal))
    quiet = False
    start = 0
    for i in range(0, len(signal) - window, window):
        frame = signal[i:i + window]
        level = sum(abs(x) for x in frame) / len(frame)
        is_quiet = level < 200
        if is_quiet and not quiet:
            start = i
            quiet = True
        elif not is_quiet and quiet:
            print("silence", start // window, (i - start) * 1000 // 11025, "ms")
            quiet = False
    if quiet:
        print("silence", start // window, (len(signal) - start) * 1000 // 11025, "ms")
