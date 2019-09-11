to determine if a pixel is in the background, count the number of samples in the model that are a short enough distance away, in color space, from the pixel. If the number of samples close enough are greater than or equal to the min threshold, the pixel is a backgound pixel.

to initialize the model for each pixel when the program starts, take 20 samples randomly from the neighboring pixels.

to update the model, if a pixel has been classified as background, on a random frame ( not on every frame ), discard a sample randomly and replace it with the new background sample. If a pixel has been classified as foreground, on a random frame, discard a sample randomly and replace it with the background sample of a neighbor.
