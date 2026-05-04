# obj-viewer

![Stanford Dragon](/images/dragon.png)

An OBJ file viewer that uses software rasterization. Texture support is 
planned, but not yet implemented. This was my first time making an actual 
project in C, and I tried to make the code good. This includes error checking 
(SO MUCH ERROR CHECKING), thread safety, and decent memory management.

## Features
- Supports OBJ files with convex faces up to four vertices (most models)
- Backface culling for performance
- Flat shading using dot product lighting

## Building from Source
To build this project from source, run these commands in your favorite terminal
emulator:

```bash
git clone https://github.com/tomatofoo/obj-viewer
cd obj-viewer
mkdir build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make
```

The output file is `main.out`. Move this to the root directory of the 
repository for it to run properly.

## Usage
Drag an OBJ file to the app window. The program will open the file.

### Keys
- W: Move forward
- A: Strafe left
- S: Move backward
- D: Strafe right
- Space: Move up
- Left Shift: Move down
- Left: Look left
- Right: Look right
- Up: Look up
- Down: Look down
- F2: Take a screenshot (supports PNG and JPEG, default PNG)

