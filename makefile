# bash support echo -e while sh not support. But you dont know which will be the default in different platform.
SHELL := /usr/bin/bash

Generator := Ninja

ANSI_CLEAR := \033[0m
ANSI_INFO_COLOR := \033[36;3;108m
ANSI_CHANGE_LINE := \033[A\033[2K

BUILD_DIR := $(PWD)/build

all:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] BEGIN BUILD ALL TARGET ...${ANSI_CLEAR}"
	cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -G"${Generator}"
	cmake --build $(BUILD_DIR) -j10
	@echo -e "${ANSI_INFO_COLOR}[MAKE] BUILD DONE !${ANSI_CLEAR}"

ctest:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] ReBuilding Tests ...${ANSI_CLEAR}"
	@rm -rf ${BUILD_DIR}/*
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -G"${Generator}" > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) -j10 > /dev/null 2>&1
	@echo -en "${ANSI_CHANGE_LINE}"
	@cd ${BUILD_DIR} && ctest

cpack:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Preparing To Pack And Install $@ ...${ANSI_CLEAR}"
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -G"${Generator}" > /dev/null 2>&1
	@cd $(BUILD_DIR) && cpack --config CPackConfig.cmake
	@cmake --install $(BUILD_DIR) --prefix $(BUILD_DIR)/installed
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Packed Successfully!${ANSI_CLEAR}"

coverage:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] ReBuilding Projects ...${ANSI_CLEAR}"
	@rm -rf $(BUILD_DIR)/*
	@cmake -S$(PWD) -B$(BUILD_DIR) -DBUILD_WITH_COVERAGE=ON -G"${Generator}" > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) -j10 > /dev/null 2>&1
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Calculating Coverage ...${ANSI_CLEAR}"
	@cmake --build $(BUILD_DIR) --target=coverage | tail -n 3
	@echo -e "${ANSI_INFO_COLOR}Done!${ANSI_CLEAR}"
	@echo -e "${ANSI_INFO_COLOR}[COVERAGE_FILE]:${ANSI_CLEAR} $(BUILD_DIR)/coverage_report/index.html"

%:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Building Target $@ ...${ANSI_CLEAR}"
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -G"${Generator}" > /dev/null 2>&1
	@cmake --build $(BUILD_DIR) --target=$@ -j2 >/dev/null 2>&1
	@echo -en "${ANSI_CHANGE_LINE}"
	@cd $(BUILD_DIR)/bin && ./$@
	@cd - > /dev/null

install:
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Preparing To Install $@ ...${ANSI_CLEAR}"
	@cmake -S$(PWD) -B$(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -G"${Generator}" > /dev/null 2>&1
	@sudo cmake --build $(BUILD_DIR) --target=install
	@echo -e "${ANSI_INFO_COLOR}[MAKE] Install Successfully!${ANSI_CLEAR}"

format:
	@fdfind -e cpp -e h -e hpp -x clang-format -i
	@echo -e "${ANSI_INFO_COLOR}Done!${ANSI_CLEAR}"

line:
	@fdfind -e h -e cpp -e txt -e cmake -x wc -l | awk '{ line += $$1 } END { print "Total Lines: " line }'

clean:
	cmake --build $(BUILD_DIR) --target=clean

delete:
	@rm -rf $(BUILD_DIR)
	@mkdir $(BUILD_DIR)
	@echo -e "${ANSI_INFO_COLOR}Done!${ANSI_CLEAR}"



