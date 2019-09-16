# ViBe Implementation
## Paper Implemented From [https://services.lib.mtu.edu:2194/document/5672785](https://services.lib.mtu.edu:2194/document/5672785)
This is an implementation of the ViBe algorithm used to segment the background and foreground with several modifications made most importantly this implementation doesn't use a strictly conservative background model update method and will sometimes include foreground samples into the background

The input must be in the same folder as the compiled binary and the output will be put in an output folder. The input must be in P3 ppm format, the output will be in the same format.

to compile `gcc vibe.c -lm`
