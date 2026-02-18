# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# ChucK flags
CHUCK_SRC_DIR = chuck/src/core
FLAGS += -I$(CHUCK_SRC_DIR) -I$(CHUCK_SRC_DIR)/lo

# Platform-specific flags
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	FLAGS += -D__PLATFORM_APPLE__
else
	FLAGS += -D__PLATFORM_LINUX__
endif

# Common flags
FLAGS += -D__DISABLE_MIDI__ -D__DISABLE_HID__ -D__ALTER_HID__ -D__DISABLE_SERIAL__ -D__DISABLE_OTF_SERVER__ -D__DISABLE_WATCHDOG__ -D__DISABLE_SHELL__

# ChucK sources
SOURCES += $(CHUCK_SRC_DIR)/chuck.cpp \
	$(CHUCK_SRC_DIR)/chuck_absyn.cpp \
	$(CHUCK_SRC_DIR)/chuck_carrier.cpp \
	$(CHUCK_SRC_DIR)/chuck_compile.cpp \
	$(CHUCK_SRC_DIR)/chuck_dl.cpp \
	$(CHUCK_SRC_DIR)/chuck_emit.cpp \
	$(CHUCK_SRC_DIR)/chuck_errmsg.cpp \
	$(CHUCK_SRC_DIR)/chuck_frame.cpp \
	$(CHUCK_SRC_DIR)/chuck_globals.cpp \
	$(CHUCK_SRC_DIR)/chuck_instr.cpp \
	$(CHUCK_SRC_DIR)/chuck_io.cpp \
	$(CHUCK_SRC_DIR)/chuck_lang.cpp \
	$(CHUCK_SRC_DIR)/chuck_oo.cpp \
	$(CHUCK_SRC_DIR)/chuck_parse.cpp \
	$(CHUCK_SRC_DIR)/chuck_scan.cpp \
	$(CHUCK_SRC_DIR)/chuck_stats.cpp \
	$(CHUCK_SRC_DIR)/chuck_symbol.cpp \
	$(CHUCK_SRC_DIR)/chuck_table.cpp \
	$(CHUCK_SRC_DIR)/chuck_type.cpp \
	$(CHUCK_SRC_DIR)/chuck_ugen.cpp \
	$(CHUCK_SRC_DIR)/chuck_vm.cpp \
	$(CHUCK_SRC_DIR)/uana_extract.cpp \
	$(CHUCK_SRC_DIR)/uana_xform.cpp \
	$(CHUCK_SRC_DIR)/ugen_filter.cpp \
	$(CHUCK_SRC_DIR)/ugen_osc.cpp \
	$(CHUCK_SRC_DIR)/ugen_stk.cpp \
	$(CHUCK_SRC_DIR)/ugen_xxx.cpp \
	$(CHUCK_SRC_DIR)/ulib_ai.cpp \
	$(CHUCK_SRC_DIR)/ulib_doc.cpp \
	$(CHUCK_SRC_DIR)/ulib_machine.cpp \
	$(CHUCK_SRC_DIR)/ulib_math.cpp \
	$(CHUCK_SRC_DIR)/ulib_opsc.cpp \
	$(CHUCK_SRC_DIR)/ulib_std.cpp \
	$(CHUCK_SRC_DIR)/util_console.cpp \
	$(CHUCK_SRC_DIR)/util_opsc.cpp \
	$(CHUCK_SRC_DIR)/util_network.c \
	$(CHUCK_SRC_DIR)/util_serial.cpp \
	$(CHUCK_SRC_DIR)/util_buffers.cpp \
	$(CHUCK_SRC_DIR)/util_math.cpp \
	$(CHUCK_SRC_DIR)/util_platforms.cpp \
	$(CHUCK_SRC_DIR)/util_raw.c \
	$(CHUCK_SRC_DIR)/util_string.cpp \
	$(CHUCK_SRC_DIR)/util_thread.cpp \
	$(CHUCK_SRC_DIR)/util_xforms.c \
	$(CHUCK_SRC_DIR)/chuck_yacc.c \
	$(CHUCK_SRC_DIR)/util_sndfile.c \
	$(CHUCK_SRC_DIR)/lo/address.c \
	$(CHUCK_SRC_DIR)/lo/blob.c \
	$(CHUCK_SRC_DIR)/lo/bundle.c \
	$(CHUCK_SRC_DIR)/lo/message.c \
	$(CHUCK_SRC_DIR)/lo/method.c \
	$(CHUCK_SRC_DIR)/lo/pattern_match.c \
	$(CHUCK_SRC_DIR)/lo/send.c \
	$(CHUCK_SRC_DIR)/lo/server.c \
	$(CHUCK_SRC_DIR)/lo/server_thread.c \
	$(CHUCK_SRC_DIR)/lo/timetag.c

# Tools
LEX=flex
YACC=bison

# Generate Flex/Bison files
# Note: we need to use the upstream logic of concatenating the files
$(CHUCK_SRC_DIR)/chuck.tab.c $(CHUCK_SRC_DIR)/chuck.tab.h: $(CHUCK_SRC_DIR)/chuck.y
	$(YACC) -dv -b $(CHUCK_SRC_DIR)/chuck $(CHUCK_SRC_DIR)/chuck.y

$(CHUCK_SRC_DIR)/chuck.lex: $(CHUCK_SRC_DIR)/chuck.tab.h

$(CHUCK_SRC_DIR)/chuck.yy.c: $(CHUCK_SRC_DIR)/chuck.lex
	$(LEX) --nounistd -o$(CHUCK_SRC_DIR)/chuck.yy.c $(CHUCK_SRC_DIR)/chuck.lex

$(CHUCK_SRC_DIR)/chuck_yacc.c: $(CHUCK_SRC_DIR)/chuck.yy.c $(CHUCK_SRC_DIR)/chuck.tab.c
	cat $(CHUCK_SRC_DIR)/chuck.tab.c $(CHUCK_SRC_DIR)/chuck.yy.c > $@

$(CHUCK_SRC_DIR)/chuck_yacc.h: $(CHUCK_SRC_DIR)/chuck.tab.h
	cat $(CHUCK_SRC_DIR)/chuck.tab.h > $@

# Add generated files to clean
clean_generated:
	rm -f $(CHUCK_SRC_DIR)/chuck.tab.c $(CHUCK_SRC_DIR)/chuck.tab.h $(CHUCK_SRC_DIR)/chuck.yy.c $(CHUCK_SRC_DIR)/chuck.output

clean: clean_generated

# Add .cpp and .c files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)
DISTRIBUTABLES += $(wildcard presets)
DISTRIBUTABLES += autoload.txt
DISTRIBUTABLES += test.ck
# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

debug:
	@echo $(SOURCES)
