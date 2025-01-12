# Define paths
BIN_DIR = bin
BUILD_DIR = build
CQC_CLI_DIR = cqc-cli
CQC_TUI_DIR = cqc-tui
CQC_TUI_BUILD_DIR = $(CQC_TUI_DIR)/$(BUILD_DIR)
TEST_PROJ_DIR = test-proj

.PHONY: all cqc-cli cqc-tui clean run

# Default target
all: bin-folder cqc-cli cqc-tui

# Create bin folder
bin-folder:
	@echo "Creating bin folder..."
	mkdir -p $(BIN_DIR)

# Target for cqc-cli
cqc-cli:
	@echo "Building cqc-cli..."
	cd $(CQC_CLI_DIR) && make
	mv $(CQC_CLI_DIR)/analyzer $(BIN_DIR)/analyzer

# Target for cqc-tui
cqc-tui:
	@echo "Building cqc-tui..."
	mkdir -p $(CQC_TUI_BUILD_DIR)
	cd $(CQC_TUI_BUILD_DIR) && cmake ..
	cd $(CQC_TUI_BUILD_DIR) && make -j
	mv $(CQC_TUI_BUILD_DIR)/cqc-tui $(BIN_DIR)/cqc-tui

# Clean build artifacts
clean:
	@echo "Cleaning build artifacts..."
	rm -rf $(CQC_CLI_BUILD_DIR)
	rm -rf $(CQC_TUI_BUILD_DIR)
	rm -rf $(BIN_DIR)

# Run cqc-tui with test-proj as parameter
run:
	@echo "Running cqc-tui with test-proj..."
	cp -r $(TEST_PROJ_DIR) $(BIN_DIR)/$(TEST_PROJ_DIR)
	$(BIN_DIR)/cqc-tui $(TEST_PROJ_DIR)

