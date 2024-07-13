.DEFAULT_GOAL := all

BUILD_DIR := build
TARGET_PROJECT := GstQtExample

.PHONY: all
all: kill clean build run

.PHONY: build
build:
	@echo "Building project..."
	@mkdir -p $(BUILD_DIR)
	@if [ `uname` = "Linux" ]; then \
		cd $(BUILD_DIR) && cmake .. && make -j`nproc`; \
	elif [ `uname` = "Darwin" ]; then \
		cd $(BUILD_DIR) && cmake .. && make -j`sysctl -n hw.ncpu`; \
	else \
		echo "Not supported platform."; \
	fi

.PHONY: clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)

.PHONY: run
run: build
	@echo "Running project..."
	@if [ `uname` = "Linux" ]; then \
		./$(BUILD_DIR)/$(TARGET_PROJECT); \
	elif [ `uname` = "Darwin" ]; then \
		./$(BUILD_DIR)/$(TARGET_PROJECT).app/Contents/MacOS/$(TARGET_PROJECT); \
	else \
		echo "Not supported platform."; \
	fi

.PHONY: kill
kill:
	@pgrep $(TARGET_PROJECT) | xargs -r kill -9
	@pgrep gst-launch-1.0 | xargs -r kill -9

.PHONY: format
format:
	@find . -name '*.h' -o -name '*.cpp' | xargs clang-format -i

.PHONY: help
help:
	@echo "Usage: make [target]"
	@echo ""
	@echo "Targets:"
	@echo "  all      : Clean, build, and run the project (default target)"
	@echo "  build    : Build the project"
	@echo "  clean    : Clean up built files"
	@echo "  run      : Run the project"
	@echo "  kill     : Kill the running project"
	@echo "  format   : Format the source code"
	@echo "  help     : Show this help message"
