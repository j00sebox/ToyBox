# Toy Box

## About

This project is a sandbox for learning more about rendering architecture and interfacing with graphics APIs like OpenGL. This is an ongoing project as there are many more features I want to add and improve upon.  

![Test_Scene](/screenshots/test_scene.png)

![Airplane](/screenshots/airplane.png)

![Asteroid_Field](/screenshots/asteroid_field.png)

This scene is rendering 10,000 objects with instances!

## Requirements

You will need a GPU with drivers that supports at least up to OpenGL version 4.3 and a compiler that supports C++20 features. Also cmake will be required to create all the build files.

## Building for Windows and Linux

By running:

```
python project_config.py
```

It will do all the cmake configuration and create the build folder.

### Windows

If on Windows it will create a Visual Studio solution file located in the build folder. Once opened you can set the toy_box project as the start up one. After that is done then you can build and run it from there.

### Linux

On linux after configuring the project you can build and run using the following commands:

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
