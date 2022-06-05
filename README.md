# Toy Box

## About

This project is a sandbox for learning more about how rendering is done and OpenGL. 

![Airplane](/screenshots/airplane.png)

## Build

### Windows

If on Windows you can run the windows_config.bat script to do all the setup. It will create a Visual Studio solution file located in the build folder. Once opened you can set the toy_box project as the start up one. After that is done then you can build and run it from there.

### Linux

If using a Linux system there is currently no script to create all the makefiles but here is what you need:

```
mkdir build
cp imgui.ini build
cmake -DGLFW_BUILD_DOCS=OFF -S . -B build
```
The copying of the imgui.ini file is so that your window will launch with the windows in the same arrangement as seen in the picture above.

After running these commands to can build and run toy_box using the following commands
