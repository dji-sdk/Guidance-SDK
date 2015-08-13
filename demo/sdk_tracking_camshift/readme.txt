This is the package of depth-based tracking with camshift algorithm. It uses one grayscale image and one corresponding depth image to track an object.

This package has been tested on Windows 7, 2010 and 2013.

BUILD =========================
- Modified the path and lib versions for OpenCV in the property sheets use_opencv_debug.props and use_opencv_release.props. 
- Open the solution file camshift.2013.sln or camshift.2010.sln, and build.

RUN ===========================
- Connect computer with Guidance.
- Run the program with mouse-click:
	-- Run the binary file without parameters (e.g. camshift.2010.exe).
	-- Select rectangle surrounding the interested object on the tracking window with mouse.
