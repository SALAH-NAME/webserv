#!/bin/bash

set -o nounset
set -o pipefail

GREEN="\033[1;32m"
RED="\033[1;31m"
YELLOW="\033[1;33m"
CYAN="\033[1;36m"
NC="\033[0m"

pass_count=0
fail_count=0
test_count=0

PRINT_OUTPUT=false

VALID_DIR="./conf"
ERRORS_DIR="./conf/errors"
EXECUTABLE="./webserv"

usage() {
    cat <<EOF
Usage: $0 [OPTIONS]

Options:
  -p, --print-output    Print the full output for each test run.
  -h, --help            Display this help message and exit.
EOF
}

parse_args() {
    while [[ "$#" -gt 0 ]]; do
        case $1 in
            -p|--print-output)
                PRINT_OUTPUT=true
                ;;
            -h|--help)
                usage
                exit 0
                ;;
            *)
                echo "Unknown option: $1"
                usage
                exit 1
                ;;
        esac
        shift
    done
}

print_header() {
    echo -e "${CYAN}======================================="
    echo -e "          Running Unit Tests          "
    echo -e "=======================================${NC}"
}

print_summary() {
    echo -e "\n${CYAN}======== Test Summary ========${NC}"
    echo -e "Total tests executed: ${test_count}"
    echo -e "${GREEN}Passed: ${pass_count}${NC}"
    echo -e "${RED}Failed: ${fail_count}${NC}"

    if [ ${fail_count} -ne 0 ]; then
        echo -e "\n${RED}Some tests failed. Re-run with -p if you need to see detailed output.${NC}"
        exit 1
    else
        echo -e "\n${GREEN}All tests passed successfully!${NC}"
        exit 0
    fi
}

run_valid_test() {
    local config="$1"
    test_count=$((test_count + 1))
    echo -e "\nTest ${test_count}: ${CYAN}${config}${NC}"
    echo "Expected Outcome: PASS (The configuration should be accepted)"

    local output
    output=$("${EXECUTABLE}" "${config}" 2>&1)
    local exit_code=$?

    if [ ${exit_code} -eq 0 ]; then
        echo -e "${GREEN}PASS:${NC} Expected PASS and got PASS. (exit code: ${exit_code})"
        pass_count=$((pass_count + 1))
        if [ "$PRINT_OUTPUT" = true ]; then
            echo -e "${YELLOW}Output:${NC}"
            echo "${output}"
        fi
    else
        echo -e "${RED}FAIL:${NC} Expected PASS but got FAIL. (exit code: ${exit_code})"
        if [ "$PRINT_OUTPUT" = true ]; then
            echo -e "${RED}Output:${NC}"
            echo "${output}"
        fi
        fail_count=$((fail_count + 1))
    fi
}

run_error_test() {
    local config="$1"
    test_count=$((test_count + 1))
    echo -e "\nTest ${test_count}: ${CYAN}${config}${NC}"
    echo "Expected Outcome: FAIL (The configuration should be rejected)"

    local output
    output=$("${EXECUTABLE}" "${config}" 2>&1)
    local exit_code=$?

    if [ ${exit_code} -ne 0 ]; then
        echo -e "${GREEN}PASS:${NC} Expected FAIL and got FAIL. (exit code: ${exit_code})"
        pass_count=$((pass_count + 1))
        if [ "$PRINT_OUTPUT" = true ]; then
            echo -e "${YELLOW}Output:${NC}"
            echo "${output}"
        fi
    else
        echo -e "${RED}FAIL:${NC} Expected FAIL but got PASS. (exit code: ${exit_code})"
        if [ "$PRINT_OUTPUT" = true ]; then
            echo -e "${RED}Output:${NC}"
            echo "${output}"
        fi
        fail_count=$((fail_count + 1))
    fi
}

run_valid_tests() {
    echo -e "\n${YELLOW}Testing valid configuration files in ${VALID_DIR}:${NC}"
    local found_valid=false
    for config in "${VALID_DIR}"/*.conf; do
        if [ ! -e "$config" ]; then
            continue
        fi
        found_valid=true
        run_valid_test "$config"
    done

    if [ "$found_valid" = false ]; then
        echo -e "${RED}No valid .conf files found in ${VALID_DIR}.${NC}"
    fi
}

run_error_tests() {
    echo -e "\n${YELLOW}Testing error configuration files in ${ERRORS_DIR}:${NC}"
    local found_error=false
    for config in "${ERRORS_DIR}"/*.conf; do
        if [ ! -e "$config" ]; then
            continue
        fi
        found_error=true
        run_error_test "$config"
    done

    if [ "$found_error" = false ]; then
        echo -e "${RED}No error .conf files found in ${ERRORS_DIR}.${NC}"
    fi
}

build_executable() {
    if [ ! -f "$EXECUTABLE" ]; then
        echo -e "${YELLOW}Executable not found. Building it...${NC}"
        make
        if [ $? -ne 0 ]; then
            echo -e "${RED}Build failed. Exiting.${NC}"
            exit 1
        fi
    fi
}

main() {
    parse_args "$@"
    print_header

		build_executable

    run_valid_tests
    run_error_tests

    print_summary
}

shopt -s nullglob

main "$@"
