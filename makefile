COMPILE_CMD = x86_64-w64-mingw32-g++
LINK_CMD = x86_64-w64-mingw32-g++
OBJ_DIR = bin/obj
OUT_DIR = bin/out
DEBUG_CC_FLAGS = -ggdb -c -Wall -D cdwDebugMode -D cdwTestBuild
RELEASE_CC_FLAGS = -O3 -c -Wall
DEBUG_LNK_FLAGS_POST = -ggdb -static-libgcc -static-libstdc++ -static
RELEASE_LNK_FLAGS_POST = -static-libgcc -static-libstdc++ -static

test: debug
	bin/out/debug/test.exe

debug: \
	dirs \
	$(OUT_DIR)/debug/test.exe \

all: \
	debug \
	$(OUT_DIR)/release/test.exe \

clean:
	rm -rf bin

dirs:
	@mkdir -p $(OBJ_DIR)/debug/prattle
	@mkdir -p $(OBJ_DIR)/debug/test
	@mkdir -p $(OBJ_DIR)/release/prattle
	@mkdir -p $(OBJ_DIR)/release/test
	@mkdir -p $(OUT_DIR)/debug
	@mkdir -p $(OUT_DIR)/release

.PHONY: debug all clean dirs test

# ----------------------------------------------------------------------
# test

include prattle.mak

TEST_SRC = \
	$(PRATTLE_SRC) \
	src/test/lexor.test.cpp \
	src/test/main.cpp \

TEST_DEBUG_OBJ = $(subst src,$(OBJ_DIR)/debug,$(patsubst %.cpp,%.o,$(TEST_SRC)))

$(OUT_DIR)/debug/test.exe: $(TEST_DEBUG_OBJ)
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(TEST_DEBUG_OBJ) $(DEBUG_LNK_FLAGS_POST) -Lbin/out/debug

$(TEST_DEBUG_OBJ): $(OBJ_DIR)/debug/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(DEBUG_CC_FLAGS) $< -o $@

TEST_RELEASE_OBJ = $(subst src,$(OBJ_DIR)/release,$(patsubst %.cpp,%.o,$(TEST_SRC)))

$(OUT_DIR)/release/test.exe: $(TEST_RELEASE_OBJ)
	$(info $< --> $@)
	@$(LINK_CMD) -o $@ $(TEST_RELEASE_OBJ) $(RELEASE_LNK_FLAGS_POST) -Lbin/out/release

$(TEST_RELEASE_OBJ): $(OBJ_DIR)/release/%.o: src/%.cpp
	$(info $< --> $@)
	@$(COMPILE_CMD) $(RELEASE_CC_FLAGS) $< -o $@
