import numpy as np
import cv2
import math
import sys

if len(sys.argv) < 2:
    print("Usage: compress.py [video]")
    print("Note: video must have a resolution lower than or equal to 384x216 px.")
    exit()

totalBytes = 0

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
    previous = None
    numRep = 0
    for i in frameArray:
        if i != previous and previous != None:
            rep.append(numRep)
            numRep = 0
            previous = i
        if i == previous:
            numRep += 1
        if previous == None:
            numRep = 0
            previous = i

    rep[len(rep) - 1] = rep[len(rep) - 1] + 1

    f.write(len(rep).to_bytes(2, 'big'))
    for i in range(len(rep)):
        f.write(rep[i].to_bytes(2, 'big'))

    if frameArray[0] == True:
        f.write(int(1).to_bytes(1, 'big'))
    else:
        f.write(int(0).to_bytes(1, 'big'))

    print(f"Frame #{frameCounter} -> {len(rep) * 2} + 3 bytes")
    totalBytes += (len(rep) * 2) + 3

print(f"TOTAL BYTES: {totalBytes}. Output at data.bin.")
video.release()
f.close()
