# Building project to build folder
bld:
	 cmake build -S . -B ./build -DTd_DIR="td" -DCMAKE_BUILD_TYPE=Release && cd ./build && make

# Running builded project from build folder
run:
	cd ./build && ./main
