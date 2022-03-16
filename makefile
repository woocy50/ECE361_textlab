BUILD_DIR := ./build

# Find all the C files we want to compile
SRCS := $(shell find utils -name '*.c')

# String substitution for every C file.
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)

# String substitution (suffix version without %).
# As an example, ./build/hello.cpp.o turns into ./build/hello.cpp.d
DEPS := $(OBJS:.o=.d)

# Every folder in ./src will need to be passed to GCC so that it can find header files
INC_DIRS := $(shell find -name '*.h')
# Add a prefix to INC_DIRS. So moduleA would become -ImoduleA. GCC understands this -I flag
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

# The final build step.
all: server client

server: server.c $(OBJS)
	gcc server.c $(OBJS) -o $@ -pthread

client: client.c $(OBJS)
	gcc client.c $(OBJS) -o $@ -pthread

# Build step
$(BUILD_DIR)/%.c.o: %.c
	mkdir -p $(dir $@)
	gcc -c $< -o $@

.PHONY: clean

clean:
	rm -r $(BUILD_DIR) ||:
	rm -f server ||:
	rm -f client ||:
