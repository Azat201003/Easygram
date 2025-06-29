all: bld run # Build and start

bld: # Building project to build folder
	mkdir -p build && \
	cmake build -S . -B ./build -DCMAKE_BUILD_TYPE=Release && \
	make -C build

run: # Running builded project from build folder
	cd ./build && ./main
