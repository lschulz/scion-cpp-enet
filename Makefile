PYTHON ?= python3

SRC_ROOT := $(dir $(realpath $(lastword $(MAKEFILE_LIST))))
PYTHONPATH := $(PYTHONPATH):$(SRC_ROOT)/python

TEST_DATA=$(addsuffix .bin,$(basename $(shell find tests -name '*.py')))

# Build library and examples

.PHONY: release
release:
	cmake --build build --config Release

.PHONY: debug
debug:
	cmake --build build --config Debug

# Run tests

.PHONY: test
test:
	TEST_BASE_PATH=$(realpath tests) build/Debug/unit-tests

# Make test data

.PHONY: test-data clean-test-data
test-data : $(TEST_DATA)

clean-test-data : $(TEST_DATA)
	rm $^

$(TEST_DATA): %.bin: %.py
	PYTHONPATH=$(PYTHONPATH) $(PYTHON) $<
