# Bad Apple on a CASIO fx-CG50 calculator.

I made this program to play Bad Apple on my CASIO CG50 calculator, note that the calculator is very slow at reading files, that's the reason for the lag and the low resolution.

Also, I was lent this calculator for a year, and since I had to return it in a month, the code was made in a hurry, so the compression algorithm is very simple, but not very efficient.

This is the example [video](https://youtu.be/27PPHqZrRkE  "video").

Before playing the video, you can select if you want to show the frame counter, and if you want the video to be played in real time (skipping frames if it lags), or play all frames regardless of the playback speed.

If you press the MENU key in the middle of the playback, or if the video ended, you'll have to exit the add-in by going to another one and entering back again to start playing the video again.

## Compiling
I used the PrizmSDK (0.5.1), you can just place the contents of this repository inside a project folder in the SDK and run the `make.bat` file.

## Compressing a different video
You can just run the `compress.py` script with the first argument being the video. Requires OpenCV 2 and numpy. It will create a `data.bin` file, which you can just transfer to the calculator and change its name to `video.bin`.
```
Usage: compress.py [video]
```
Note: video must have a resolution lower than or equal to 384x216 px, as well as less than 60 fps.

## Errors
If the calculator says "Error reading video" when opening the add-in, make sure you place the binary file (`video.bin`) in the root of the storage memory of the calculator, and that it has that name.

If the calculator says "Invalid video", the resolution is higher that the screen of the calculator, the fps are above 60, or the scale at which the video is going to be played is haigher than 99.
