NAME		:= webserv
UNIT_NAME	:= unit_tests

CXX			:= c++
CXXSTD		:= -std=c++98

SRC_DIR		:= src
INC_DIR		:= include
BUILD_DIR	:= build
TEST_DIR	:= tests
CONF_DIR	:= conf
WWW_DIR		:= www

VERBOSE_MAKE ?= 0
ifeq ($(VERBOSE_MAKE),0)
	Q := @
	MAKEFLAGS += --no-print-directory
else
	Q := 
endif

NPROCS_MAKE ?= $(shell echo $$(( $$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2) / 2 )))
ifeq ($(NPROCS_MAKE),0)
	NPROCS_MAKE := 1
endif

ifeq ($(MAKELEVEL),0)
	MAKEFLAGS += -j$(NPROCS_MAKE)
endif

MODE_MAKE		?= debug
VALID_MODES		:= debug release sanitize valgrind

ifeq ($(filter $(MODE_MAKE),$(VALID_MODES)),)
$(error Invalid MODE_MAKE: $(MODE_MAKE). Valid modes: $(VALID_MODES))
endif

ARGS_MAKE ?=

GREEN		:= \033[0;32m
YELLOW		:= \033[1;33m
RED			:= \033[0;31m
BLUE		:= \033[0;34m
MAGENTA		:= \033[0;35m
CYAN		:= \033[0;36m
BOLD		:= \033[1m
RESET		:= \033[0m


BASE_FLAGS	:= -Wall -Wextra -Werror $(CXXSTD)
INCFLAGS	:= -I$(INC_DIR)
DEPFLAGS	:= -MMD -MP

ifeq ($(MODE_MAKE),debug)
	CXXFLAGS := $(BASE_FLAGS) -g3 -O0 -DDEBUG=true
	MODE_DIR := debug
	MODE_EXEC := $(BUILD_DIR)/$(NAME)_debug
	BUILD_DESC := Debug Build (with symbols and no optimization)
else ifeq ($(MODE_MAKE),release)
	CXXFLAGS := $(BASE_FLAGS)
	MODE_DIR := release
	MODE_EXEC := $(BUILD_DIR)/$(NAME)_release
	BUILD_DESC := Release Build (optimized for production)
else ifeq ($(MODE_MAKE),sanitize)
	CXXFLAGS := $(BASE_FLAGS) -g3 -O1 -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer
	LDFLAGS := -fsanitize=address -fsanitize=undefined
	MODE_DIR := sanitize
	MODE_EXEC := $(BUILD_DIR)/$(NAME)_sanitize
	BUILD_DESC := Sanitizer Build (address and undefined behavior detection)
else ifeq ($(MODE_MAKE),valgrind)
	CXXFLAGS := $(BASE_FLAGS) -g3 -O0 -DDEBUG=true
	MODE_DIR := debug
	MODE_EXEC := $(BUILD_DIR)/$(NAME)_debug
	BUILD_DESC := Valgrind Build (reuses debug build for memory debugging)
endif

OBJ_DIR_MODE := $(BUILD_DIR)/$(MODE_DIR)
UNIT_EXEC := $(BUILD_DIR)/$(UNIT_NAME)_$(MODE_DIR)

SRCS := $(patsubst $(SRC_DIR)/%,%,$(wildcard $(SRC_DIR)/*.cpp $(SRC_DIR)/*/*.cpp))

UNIT_SRCS := $(filter-out main.cpp, $(SRCS))
TEST_SRCS := $(patsubst $(TEST_DIR)/%.cpp,%.cpp,$(wildcard $(TEST_DIR)/*.cpp))

OBJS		:= $(addprefix $(OBJ_DIR_MODE)/, $(SRCS:.cpp=.o))
UNIT_OBJS	:= $(addprefix $(OBJ_DIR_MODE)/, $(UNIT_SRCS:.cpp=.o)) \
			   $(addprefix $(OBJ_DIR_MODE)/tests/, $(TEST_SRCS:.cpp=.o))

DEPS		:= $(OBJS:.o=.d)
UNIT_DEPS	:= $(UNIT_OBJS:.o=.d)


# Disable implicit rules
# .SUFFIXES:

all: $(NAME)

$(NAME): $(MODE_EXEC)
	@echo "$(CYAN)Building in $(BOLD)$(MODE_MAKE)$(RESET)$(CYAN) mode...$(RESET)"
	$(Q)if [ -L $(NAME) ]; then \
		CURRENT_TARGET=$$(readlink $(NAME)); \
		if [ "$$CURRENT_TARGET" != "$(MODE_EXEC)" ]; then \
			echo "$(YELLOW)Updating symlink: $(NAME) -> $(MODE_EXEC)$(RESET)"; \
			rm -f $(NAME); \
			ln -sf $(MODE_EXEC) $(NAME); \
		else \
			echo "$(GREEN)Symlink already points to correct target: $(MODE_EXEC)$(RESET)"; \
		fi; \
	else \
		echo "$(GREEN)Creating symlink: $(NAME) -> $(MODE_EXEC)$(RESET)"; \
		ln -sf $(MODE_EXEC) $(NAME); \
	fi

unit: $(UNIT_EXEC)
	@echo "$(CYAN)Building unit tests in $(BOLD)$(MODE_MAKE)$(RESET)$(CYAN) mode...$(RESET)"
	$(Q)if [ -L unit_test ]; then \
		CURRENT_TARGET=$$(readlink unit_test); \
		if [ "$$CURRENT_TARGET" != "$(UNIT_EXEC)" ]; then \
			echo "$(YELLOW)Updating symlink: unit_test -> $(UNIT_EXEC)$(RESET)"; \
			rm -f unit_test; \
			ln -sf $(UNIT_EXEC) unit_test; \
		else \
			echo "$(GREEN)Symlink already points to correct target: $(UNIT_EXEC)$(RESET)"; \
		fi; \
	else \
		echo "$(GREEN)Creating symlink: unit_test -> $(UNIT_EXEC)$(RESET)"; \
		ln -sf $(UNIT_EXEC) unit_test; \
	fi

unit_test: unit

debug: 
	$(Q)$(MAKE) MODE_MAKE=debug $(BUILD_DIR)/$(NAME)_debug

release:
	$(Q)$(MAKE) MODE_MAKE=release $(BUILD_DIR)/$(NAME)_release

sanitize:
	$(Q)$(MAKE) MODE_MAKE=sanitize $(BUILD_DIR)/$(NAME)_sanitize

valgrind:
	$(Q)$(MAKE) MODE_MAKE=debug $(BUILD_DIR)/$(NAME)_debug
	$(Q)echo "$(GREEN)Valgrind build ready (using debug executable)$(RESET)"

debug-unit:
	$(Q)$(MAKE) MODE_MAKE=debug unit

release-unit:
	$(Q)$(MAKE) MODE_MAKE=release unit

sanitize-unit:
	$(Q)$(MAKE) MODE_MAKE=sanitize unit

valgrind-unit:
	$(Q)$(MAKE) MODE_MAKE=valgrind unit

debug-tests:
	$(Q)$(MAKE) MODE_MAKE=debug tests

release-tests:
	$(Q)$(MAKE) MODE_MAKE=release tests

sanitize-tests:
	$(Q)$(MAKE) MODE_MAKE=sanitize tests

valgrind-tests:
	$(Q)$(MAKE) MODE_MAKE=valgrind tests

help:
	@echo "$(BOLD)$(CYAN)WebServ Makefile Help$(RESET)"
	@echo "$(YELLOW)Basic Build Targets:$(RESET)"
	@echo "  $(GREEN)all$(RESET)               - Build main application (current mode: $(MODE_MAKE))"
	@echo "  $(GREEN)$(NAME)$(RESET)           - Build and create webserv symlink to current mode"
	@echo "  $(GREEN)debug$(RESET)             - Build debug version with symbols (-g3 -O0)"
	@echo "  $(GREEN)release$(RESET)           - Build optimized release version" 
	@echo "  $(GREEN)sanitize$(RESET)          - Build with address/undefined sanitizers"
	@echo "  $(GREEN)valgrind$(RESET)          - Build version optimized for valgrind debugging"
	@echo ""
	@echo "$(YELLOW)Test Targets:$(RESET)"
	@echo "  $(GREEN)unit$(RESET)              - Build unit tests and create unit_test symlink"
	@echo "  $(GREEN)tests$(RESET)             - Build and run all unit tests"
	@echo "  $(GREEN)build-tests$(RESET)       - Build unit tests without running them"
	@echo "  $(GREEN)debug-unit$(RESET)        - Build unit tests in debug mode"
	@echo "  $(GREEN)release-unit$(RESET)      - Build unit tests in release mode"
	@echo "  $(GREEN)sanitize-unit$(RESET)     - Build unit tests in sanitize mode"
	@echo "  $(GREEN)valgrind-unit$(RESET)     - Build unit tests in valgrind mode"
	@echo "  $(GREEN)debug-tests$(RESET)       - Run unit tests in debug mode"
	@echo "  $(GREEN)release-tests$(RESET)     - Run unit tests in release mode"
	@echo "  $(GREEN)sanitize-tests$(RESET)    - Run unit tests in sanitize mode"
	@echo "  $(GREEN)valgrind-tests$(RESET)    - Run unit tests in valgrind mode"
	@echo ""
	@echo "$(YELLOW)Execution Targets:$(RESET)"
	@echo "  $(GREEN)run$(RESET)               - Run webserv with default or custom config"
	@echo "  $(GREEN)run-bg$(RESET)            - Run webserv in background"
	@echo "  $(GREEN)stop$(RESET)              - Stop all running webserv processes"
	@echo ""
	@echo "$(YELLOW)Memory Debugging:$(RESET)"
	@echo "  $(GREEN)valgrind-run$(RESET)      - Run valgrind memory check on executable"
	@echo "  $(GREEN)valgrind-full$(RESET)     - Run comprehensive valgrind analysis"
	@echo "  $(GREEN)valgrind-tests$(RESET)    - Run unit tests with valgrind"
	@echo ""
	@echo "$(YELLOW)Analysis & Utilities:$(RESET)"
	@echo "  $(GREEN)stats$(RESET)             - Show project statistics (files, lines, etc.)"
	@echo "  $(GREEN)info$(RESET)              - Show current build configuration"
	@echo ""
	@echo "$(YELLOW)Clean Targets:$(RESET)"
	@echo "  $(GREEN)clean$(RESET)             - Remove object files for current mode only"
	@echo "  $(GREEN)fclean$(RESET)            - Remove all generated files for current mode"
	@echo "  $(GREEN)cleanall$(RESET)          - Remove all build artifacts for all modes"
	@echo "  $(GREEN)re$(RESET)                - Clean and rebuild current mode"
	@echo "  $(GREEN)rebuild-all$(RESET)       - Clean and rebuild all modes"
	@echo ""
	@echo "$(YELLOW)Configuration:$(RESET)"
	@echo "  $(GREEN)MODE_MAKE=<mode>$(RESET)  - Set build mode: debug, release, sanitize, valgrind"
	@echo "  $(GREEN)VERBOSE_MAKE=1$(RESET)    - Enable verbose output (show all commands)"
	@echo "  $(GREEN)NPROCS_MAKE=<num>$(RESET) - Set number of parallel jobs (default: half cores)"
	@echo "  $(GREEN)ARGS_MAKE='...'$(RESET)   - Pass custom arguments to run targets"
	@echo ""
	@echo "$(YELLOW)Examples:$(RESET)"
	@echo "  $(CYAN)make debug$(RESET)                                     - Build debug version"
	@echo "  $(CYAN)make run ARGS_MAKE='conf/custom.conf'$(RESET)          - Run with custom config"
	@echo "  $(CYAN)make VERBOSE_MAKE=1 release$(RESET)                    - Build release with verbose output"
	@echo "  $(CYAN)make valgrind-run ARGS_MAKE='conf/custom.conf'$(RESET) - Run valgrind with custom config"
	@echo "  $(CYAN)make NPROCS_MAKE=4 sanitize$(RESET)                    - Build sanitize with 4 jobs"
	@echo "  $(CYAN)make release-tests$(RESET)                             - Run tests in release mode"

man: help

info:
	@echo "$(BOLD)$(CYAN)Build Configuration$(RESET)"
	@echo "$(YELLOW)Mode:$(RESET)         $(MODE_MAKE)"
	@echo "$(YELLOW)Description:$(RESET)  $(BUILD_DESC)"
	@echo "$(YELLOW)Executable:$(RESET)   $(MODE_EXEC)"
	@echo "$(YELLOW)Compiler:$(RESET)     $(CXX)"
	@echo "$(YELLOW)Flags:$(RESET)        $(CXXFLAGS)"
	@echo "$(YELLOW)Object Dir:$(RESET)   $(OBJ_DIR_MODE)"
	@echo "$(YELLOW)Sources:$(RESET)      $(words $(SRCS)) files"
	@echo "$(YELLOW)Test Sources:$(RESET) $(words $(TEST_SRCS)) files"
	@echo "$(YELLOW)Parallel Jobs:$(RESET) $(NPROCS_MAKE)"

stats:
	$(Q)echo "$(BOLD)$(CYAN)WebServ Project Statistics$(RESET)"
	$(Q)echo "$(YELLOW)Source Files:$(RESET)"
	$(Q)find src/ -name "*.cpp" | wc -l | xargs printf "  C++ files:        %s\n"
	$(Q)find include/ -name "*.hpp" | wc -l | xargs printf "  Header files:     %s\n"
	$(Q)echo "$(YELLOW)Lines of Code:$(RESET)"
	$(Q)find src/ -name "*.cpp" -exec cat {} \; | wc -l | xargs printf "  C++ source:       %s lines\n"
	$(Q)find include/ -name "*.hpp" -exec cat {} \; | wc -l | xargs printf "  Headers:          %s lines\n"
	$(Q)echo "$(YELLOW)Configuration:$(RESET)"
	$(Q)find conf/ -name "*.conf" | wc -l | xargs printf "  Config files:     %s\n"
	$(Q)find tests/ -name "*.cpp" | wc -l | xargs printf "  Test files:       %s\n"
	$(Q)echo "$(YELLOW)Build artifacts (current mode: $(MODE_MAKE)):$(RESET)"
	$(Q)if [ -d "$(BUILD_DIR)" ]; then \
		find $(BUILD_DIR) -name "*.o" 2>/dev/null | wc -l | xargs printf "  Object files:     %s\n"; \
		find $(BUILD_DIR) -type f -executable 2>/dev/null | wc -l | xargs printf "  Executables:      %s\n"; \
		du -sh $(BUILD_DIR) 2>/dev/null | awk '{print "  Build dir size:   " $$1}' || echo "  Build dir size:   0"; \
	else \
		echo "  No build artifacts found"; \
	fi

$(OBJ_DIR_MODE):
	@echo "$(YELLOW)Creating build directory: $(OBJ_DIR_MODE)$(RESET)"
	@mkdir -p $(OBJ_DIR_MODE)/config $(OBJ_DIR_MODE)/server $(OBJ_DIR_MODE)/raii \
			  $(OBJ_DIR_MODE)/cgi $(OBJ_DIR_MODE)/response $(OBJ_DIR_MODE)/tests

$(OBJ_DIR_MODE)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR_MODE)
	$(Q)echo "$(YELLOW)Compiling [$(MODE_MAKE)]: $<$(RESET)"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@

$(OBJ_DIR_MODE)/tests/%.o: $(TEST_DIR)/%.cpp | $(OBJ_DIR_MODE)
	$(Q)echo "$(YELLOW)Compiling test [$(MODE_MAKE)]: $<$(RESET)"
	$(Q)mkdir -p $(dir $@)
	$(Q)$(CXX) $(CXXFLAGS) $(INCFLAGS) $(DEPFLAGS) -c $< -o $@

$(MODE_EXEC): $(OBJS) | $(BUILD_DIR)
	$(Q)echo "$(GREEN)Linking $(MODE_EXEC) [$(MODE_MAKE)]...$(RESET)"
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) $(OBJS) -o $(MODE_EXEC)
	$(Q)echo "$(BOLD)$(GREEN)✓ $(MODE_EXEC) built successfully!$(RESET)"

$(UNIT_EXEC): $(UNIT_OBJS) | $(BUILD_DIR)
	$(Q)echo "$(GREEN)Linking $(UNIT_EXEC) [$(MODE_MAKE)]...$(RESET)"
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) $(UNIT_OBJS) -o $(UNIT_EXEC)
	$(Q)echo "$(BOLD)$(GREEN)✓ $(UNIT_EXEC) built successfully!$(RESET)"

$(BUILD_DIR):
	$(Q)echo "$(YELLOW)Creating build directory: $(BUILD_DIR)$(RESET)"
	$(Q)mkdir -p $(BUILD_DIR)


tests: unit_test
	@echo "$(BOLD)$(CYAN)Running Unit Tests [$(MODE_MAKE)]...$(RESET)"
	@./unit_test && echo "$(BOLD)$(GREEN)✓ All tests passed!$(RESET)" || echo "$(BOLD)$(RED)✗ Some tests failed!$(RESET)"

build-tests: unit_test

valgrind-run: 
	@if [ ! -f $(BUILD_DIR)/$(NAME)_debug ]; then \
		echo "$(YELLOW)Building debug version for valgrind analysis...$(RESET)"; \
		$(MAKE) MODE_MAKE=debug; \
	fi
	@EXEC=$(BUILD_DIR)/$(NAME)_debug; \
	echo "$(BOLD)$(MAGENTA)Running Valgrind Memory Check on $$EXEC...$(RESET)"; \
	if [ -n "$(ARGS_MAKE)" ]; then \
		echo "$(CYAN)Using custom arguments: $(ARGS_MAKE)$(RESET)"; \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
				  --verbose --log-file=valgrind.log ./$$EXEC $(ARGS_MAKE); \
	else \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
				  --verbose --log-file=valgrind.log ./$$EXEC ; \
	fi; \
	echo "$(GREEN)Valgrind log saved to valgrind.log$(RESET)"

valgrind-full:
	@if [ ! -f $(BUILD_DIR)/$(NAME)_debug ]; then \
		echo "$(YELLOW)Building debug version for comprehensive valgrind analysis...$(RESET)"; \
		$(MAKE) MODE_MAKE=debug; \
	fi
	@EXEC=$(BUILD_DIR)/$(NAME)_debug; \
	echo "$(BOLD)$(MAGENTA)Running Comprehensive Valgrind Analysis on $$EXEC...$(RESET)"; \
	if [ -n "$(ARGS_MAKE)" ]; then \
		echo "$(CYAN)Using custom arguments: $(ARGS_MAKE)$(RESET)"; \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
				  --track-fds=yes --show-reachable=yes --trace-children=yes --verbose \
				  --log-file=valgrind-full.log ./$$EXEC $(ARGS_MAKE) ; \
	else \
		valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
				  --track-fds=yes --show-reachable=yes --trace-children=yes --verbose \
				  --log-file=valgrind-full.log ./$$EXEC ; \
	fi; \
	echo "$(GREEN)Comprehensive valgrind log saved to valgrind-full.log$(RESET)"


clean:
	@echo "$(RED)Cleaning $(MODE_MAKE) mode objects...$(RESET)"
	@rm -rf $(OBJ_DIR_MODE)
	@rm -f $(DEPS) $(UNIT_DEPS)

fclean:  clean
	@echo "$(RED)Removing $(MODE_MAKE) mode executables and logs...$(RESET)"
	@rm -f $(MODE_EXEC) $(UNIT_EXEC) $(NAME) unit_test
	@rm -f valgrind*.log *.log
	@rm -f compile_commands.json

cleanall:
	@echo "$(RED)Cleaning all build modes...$(RESET)"
	@rm -rf $(BUILD_DIR)
	@rm -f $(NAME) unit_test
	@rm -f valgrind*.log *.log
	@rm -f compile_commands.json

re: fclean 
	@$(MAKE) MODE_MAKE=$(MODE_MAKE) all

rebuild-all: cleanall
	@$(MAKE) debug
	@$(MAKE) release
	@$(MAKE) sanitize
	@$(MAKE) valgrind


run: $(NAME)
	@echo "$(BOLD)$(CYAN)Starting WebServ...$(RESET)"
	@if [ -n "$(ARGS_MAKE)" ]; then \
		echo "$(YELLOW)Running with args: $(ARGS_MAKE)$(RESET)"; \
		trap 'echo "$(YELLOW)WebServ stopped by user$(RESET)"' INT TERM; \
		./$(NAME) $(ARGS_MAKE) || true; \
	else \
		trap 'echo "$(YELLOW)WebServ stopped by user$(RESET)"' INT TERM; \
		./$(NAME) || true; \
	fi

run-bg: $(NAME)
	@echo "$(BOLD)$(CYAN)Starting WebServ in background...$(RESET)"
	@if [ -n "$(ARGS_MAKE)" ]; then \
		echo "$(YELLOW)Running with args: $(ARGS_MAKE)$(RESET)"; \
		./$(NAME) $(ARGS_MAKE) & \
		echo "$(GREEN)WebServ started with PID: $$!$(RESET)"; \
	else \
		./$(NAME) & \
		echo "$(GREEN)WebServ started with PID: $$!$(RESET)"; \
	fi

stop:
	@echo "$(YELLOW)Stopping WebServ...$(RESET)"
	@pgrep -f "^\\./$(NAME)" > /dev/null && pkill -f "^\\./$(NAME)" && echo "$(GREEN)WebServ stopped$(RESET)" || echo "$(YELLOW)No WebServ process found$(RESET)"


-include $(DEPS)
-include $(UNIT_DEPS)

.PHONY: all clean fclean cleanall re rebuild-all help info tests build-tests unit \
		debug release sanitize valgrind debug-unit release-unit sanitize-unit valgrind-unit \
		debug-tests release-tests sanitize-tests valgrind-tests valgrind-run valgrind-full \
		run run-bg stop stats man $(NAME)

.SECONDARY: $(OBJS) $(UNIT_OBJS)
