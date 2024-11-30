BUILD_DIR := $(PWD)/build

all:
	@echo "[MAKE] BEGIN BUILD ALL TARGET ..."
	cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=0
	cmake --build $(BUILD_DIR) -j10
	@echo "[MAKE] BUILD DONE !"

%:
	@echo "[MAKE] Building Target $@ ..."
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=0 > /dev/null
	@cmake --build $(BUILD_DIR) --target=$@ -j2 >/dev/null
	@echo -en "\033[A\033[2K"
	@$(BUILD_DIR)/bin/$@

clean:
	cmake --build $(BUILD_DIR) --target=clean

delete:
	rm -rf $(BUILD_DIR)/*


