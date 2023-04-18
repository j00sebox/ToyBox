# Toy Box

## About

This project is a sandbox for learning more about rendering architecture and interfacing with graphics APIs like OpenGL. This is an ongoing project as there are many more features I want to add and improve upon.  

![Test_Scene](/screenshots/test_scene.png)

![Airplane](/screenshots/airplane.png)

![Asteroid_Field](/screenshots/asteroid_field.png)

This scene is rendering 10,000 objects with instances.

## Build

### Windows

If on Windows you can run the windows_config.bat script to do all the setup. It will create a Visual Studio solution file located in the build folder. Once opened you can set the toy_box project as the start up one. After that is done then you can build and run it from there.

There is also currently a Windows release version available.

### Linux

If using a Linux system there is currently no script to create all the makefiles but here is what you need:

```
mkdir build
cp imgui.ini build
cmake -DGLFW_BUILD_DOCS=OFF -S . -B build
```
The copying of the imgui.ini file is so that your window will launch with the windows in the same arrangement as seen in the picture above.

After running these commands to can build and run toy_box using the following commands:

```
cd build
make
./toy_box
```
## Controls

* WASD to move around
* By holding the right mouse button you can rotate the camera.
* R is to reset the camera to the origin.
* C can recompile shaders.
* ESC will shut the program down.
