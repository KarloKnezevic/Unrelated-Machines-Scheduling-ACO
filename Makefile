CC ?= cc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
CPPFLAGS := -Iinclude $(CPPFLAGS)
LDFLAGS ?=
LDLIBS ?= -lm

SRC_CORE = \
	src/instance.c \
	src/input_loader.c \
	src/evaluator.c \
	src/rng.c \
	src/minmin.c \
	src/aco.c \
	src/dynamic_scheduler.c \
	src/benchmark.c

SRC_MAIN = src/main.c
SRC_TEST = tests/test_runner.c

OBJ_CORE = $(patsubst %.c,build/%.o,$(SRC_CORE))
OBJ_MAIN = $(patsubst %.c,build/%.o,$(SRC_MAIN))
OBJ_TEST = $(patsubst %.c,build/%.o,$(SRC_TEST))

TARGET = bin/scheduler
TEST_TARGET = bin/scheduler_tests

.PHONY: all test benchmark clean

all: $(TARGET)

$(TARGET): $(OBJ_CORE) $(OBJ_MAIN)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_CORE) $(OBJ_MAIN) $(LDLIBS)

$(TEST_TARGET): $(OBJ_CORE) $(OBJ_TEST)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $(OBJ_CORE) $(OBJ_TEST) $(LDLIBS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(TEST_TARGET)
	./$(TEST_TARGET)

benchmark: $(TARGET)
	@mkdir -p results
	./$(TARGET) --input-root data/input --verbose --csv results/comparison.csv

clean:
	rm -rf build $(TARGET) $(TEST_TARGET)
