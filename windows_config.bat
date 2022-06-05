call mkdir build
call copy imgui.ini build
call cmake -DGLFW_BUILD_DOCS=OFF -S . -B build

PAUSE