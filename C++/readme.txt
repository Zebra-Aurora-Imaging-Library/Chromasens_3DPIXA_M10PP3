Running this example will show how to grab images using the Chromasens 3dPixa
stereo color linescan camera and use the CS-3D api. Camera output images have 
been saved on disk and will be reloaded. 

By default, the application runs without using the CS-3D api to calculate the 
disparity map and the rectified color map. The CS3D I3DApi object, and all
the structures it is using, was redefined to simply load images from disk
to simulate the full computation that would have been done by the CS3D api.

To use the actual CS-3D api, set USE_CS3D_API to 1. The images will then 
be provided to the CS-3D api which will calculate the disparity map and its 
associated rectified color map.

To run the example with the CS-3D api, the Chromasens CS-3D api needs to be 
installed as well as a CUDA 2.0 capable GPU. 
See Chromasens CS-3D-Api-Manual.pdf for more information.
The location of the CS3DApi64.dll is harcoded in the application to 
C:\\Program Files\\Chromasens\\3D\\dlls.
This path should be adapted according to the Chromasens CS-3D installation directory.

To run the example using an actual 3dPixa camera, the camera needs to be hooked
to either a Solios or Radient board. Set the SYSTEM_TO_USE variable accordingly.
SYSTEM_TO_USE | SYSTEM
       0      | M_SYSTEM_HOST
       1      | M_SYSTEM_RADIENT
       2      | M_SYSTEM_SOLIOS
The dcf files are not installed and should be retrieved from the Interfacing Cameras 
section under the Support section of www.zebra.com.
Since the camera is a linescan camera, some motion of the object is required. 
The DCF files might need to be adjusted according to the behavior of the scanning
system in use. Add the necessary code in the StartScan() function to initiate the 
movement and, if required, send the trigger signal to the cameras/frame grabber.

Here is the configuration on which this example has been tested:
- 64 bits Windows OS
- Compiler: Microsoft Visual Studio 2017
- Chromasens CS-3D api: CS-3D v2.3e
- GPU: Quadro K2200
- Matrox Imaging Library: MIL 10.0 PP3 

