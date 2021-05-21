import numpy as np
import cv2
import math
import sys

if len(sys.argv) < 2:
    print("Usage: compress.py [video]")
    print("Note: video must have a resolution lower than or equal to 384x216 px.")
    exit()

totalBytes = 0
repLengths = []
valLengths = []

video = cv2.VideoCapture(sys.argv[1])
frameCount = int(video.get(cv2.CAP_PROP_FRAME_COUNT))
w = int(video.get(cv2.CAP_PROP_FRAME_WIDTH))
h = int(video.get(cv2.CAP_PROP_FRAME_HEIGHT))
fps = int(video.get(cv2.CAP_PROP_FPS))
scale = int(216 / h)

if w > 384 or h > 216:
    print("Video must have a resolution lower than or equal to 384x216 px.")
    exit()

print(f"H:{h}, W:{w}, S:{scale}, FPS:{fps}")

frameCounter = 0
continuing = True

f = open("data.bin", "wb")
f.write(frameCount.to_bytes(2, 'big'))
f.write(w.to_bytes(2, 'big'))
f.write(h.to_bytes(2, 'big'))
f.write(fps.to_bytes(2, 'big'))
f.write(scale.to_bytes(1, 'big'))

while (frameCounter < frameCount and continuing):
    continuing, frame = video.read()
    frameCounter += 1

    frameArray = []

    for y in range(h):
        for x in range(w):
            brightness = int(np.sum(frame[y, x])/3)
            white = True if brightness > 127 else False
            frameArray.append(white)
    frameArray.append(None)

    rep = []
    valuesBinary = []
    previous = None
    numRep = 0
    for i in frameArray:
        if i != previous and previous != None:
            rep.append(numRep)
            valuesBinary.append(1 if previous == True else 0)
            numRep = 0
            previous = i
        if i == previous:
            numRep += 1
        if previous == None:
            numRep = 0
            previous = i

    rep[len(rep) - 1] = rep[len(rep) - 1] + 1

    valuesInt = []
    for i in range(math.ceil(len(valuesBinary) / 32)):
        values = []
        if ((i + 1) * 32) + 1 <= len(valuesBinary):
            values = valuesBinary[i * 32:((i + 1) * 32)]
        else:
            values = valuesBinary[i * 32:len(valuesBinary)]

        integer = int("".join(str(x) for x in values[::-1]), 2)
        valuesInt.append(integer)

    f.write(len(rep).to_bytes(2, 'big'))
    for i in range(len(rep)):
        f.write(rep[i].to_bytes(2, 'big'))

    f.write(len(valuesInt).to_bytes(1, 'big'))
    for i in range(len(valuesInt)):
        f.write(valuesInt[i].to_bytes(4, 'big'))

    repLengths.append(len(rep))
    valLengths.append(len(valuesInt))

    print(f"Frame #{frameCounter} -> {len(rep) * 2 + len(valuesInt) * 4} + 3 bytes")
    totalBytes += (len(rep) * 2 + len(valuesInt) * 4) + 3

print(f"TOTAL BYTES: {totalBytes}. Output at data.bin.")
video.release()
f.close()
