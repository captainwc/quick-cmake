Generator := Ninja

ANSI_CLEAR := \033[0m
ANSI_INFO_COLOR := \033[36;3;108m
ANSI_CHANGE_LINE := \033[A\033[2K

BUILD_DIR := $(PWD)/build

all:
	@echo "${ANSI_INFO_COLOR}[MAKE] BEGIN BUILD ALL TARGET ...${ANSI_CLEAR}"
	cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -G"${Generator}"
	cmake --build $(BUILD_DIR) -j10
	@echo "${ANSI_INFO_COLOR}[MAKE] BUILD DONE !${ANSI_CLEAR}"

ctest:
	@echo "${ANSI_INFO_COLOR}[MAKE] ReBuilding Tests ...${ANSI_CLEAR}"
	@rm -rf ${BUILD_DIR}/*
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -G"${Generator}" > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) -j10 > /dev/null 2>&1
	@echo -en "${ANSI_CHANGE_LINE}"
	@cd ${BUILD_DIR} && ctest

%:
	@echo "${ANSI_INFO_COLOR}[MAKE] Building Target $@ ...${ANSI_CLEAR}"
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -G"${Generator}" > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) --target=$@ -j2 >/dev/null 2>&1
	@echo -en "${ANSI_CHANGE_LINE}"
	@$(BUILD_DIR)/bin/$@

format:
	@fd -e cpp -e h -e hpp -x clang-format -i
	@echo "${ANSI_INFO_COLOR}Done!${ANSI_CLEAR}"

line:
	@fd -e h -e cpp -x wc -l | awk '{ line += $$1 } END { print "Total Lines: " line }'

clean:
	cmake --build $(BUILD_DIR) --target=clean

delete:
	rm -rf $(BUILD_DIR)/*


