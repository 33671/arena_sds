CC      := gcc
CFLAGS  := -Wall -Wextra -std=c99

.PHONY: all test test-arena test-sds clean

all: test

# ── Run all tests ──────────────────────────────────────
test: test-sds test-arena

# ── Original SDS unit tests (no arena) ─────────────────
test-sds: build/sds-test
	@echo "=== Original SDS tests ==="
	@./build/sds-test
	@echo ""

build/sds-test: sds.c sds.h sdsalloc.h testhelp.h
	@mkdir -p build
	$(CC) $(CFLAGS) -DSDS_TEST_MAIN -o $@ sds.c

# ── Arena SDS tests ────────────────────────────────────
test-arena: build/arena-sds-test
	@echo "=== Arena SDS tests ==="
	@./build/arena-sds-test
	@echo ""

build/arena-sds-test: test_arena_sds.c sds.c sds.h arena.h
	@mkdir -p build
	$(CC) $(CFLAGS) -DSDS_USE_ARENA -o $@ test_arena_sds.c sds.c

# ── Clean ──────────────────────────────────────────────
clean:
	rm -rf build
