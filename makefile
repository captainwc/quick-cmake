BUILD_DIR := $(PWD)/build

all:
	@echo "[MAKE] BEGIN BUILD ALL TARGET ..."
	cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=0
	cmake --build $(BUILD_DIR) -j10
	@echo "[MAKE] BUILD DONE !"

ctest:
	@echo "[MAKE] ReBuilding Tests ..."
	@rm -rf ${BUILD_DIR}/*
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=0 > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) -j10 > /dev/null 2>&1
	@echo -en "\033[A\033[2K"
	@cd ${BUILD_DIR} && ctest

%:
	@echo "[MAKE] Building Target $@ ..."
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE=0 > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) --target=$@ -j2 >/dev/null 2>&1
	@echo -en "\033[A\033[2K"
	@$(BUILD_DIR)/bin/$@

format:
	@fd -e cpp -e h -e hpp -x clang-format -i
	@echo "Done!"

clean:
	cmake --build $(BUILD_DIR) --target=clean

delete:
	rm -rf $(BUILD_DIR)/*


