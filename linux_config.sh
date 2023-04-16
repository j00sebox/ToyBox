mkdir build
cp imgui.ini build
cmake -DGLFW_BUILD_DOCS=OFF -S . -B build
