# obj-viewer

![Stanford Dragon](/images/dragon.png)

An OBJ file viewer that uses software rasterization. Texture support is 
planned, but not yet implemented. This was my first time making an actual 
project in C, and I tried to make the code good. This includes error checking 
(SO MUCH ERROR CHECKING), thread safety, and decent memory management. The code
is still pretty shitty though.

## Support
This viewer supports OBJ files with convex faces up to four vertices. Only 
basic geometry is supported. Only the basic material properties are supported 
(Ka, Kd, Ks, Ns, map_Ka, map_Kd, map_Ks, map_Ns) In addition, in files with 
textures, spaces in filenames are not supported. Materials with spaces in their
name will be equivalent to materials without spaces (e.g. a meterial called 
"123" will be the same as the material called "1 23"). Despite all these 
constraints, most OBJ files would be fully supported as these are advanced 
features.

## Features
- Screenshots
- Backface culling for performance
- Flat and smooth shading with Phong illumination 

## Building from Source
To build this project from source, install `SDL3`, `SDL3_image`, and 
`SDL3_ttf`, and run these commands in your favorite terminal emulator:

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

