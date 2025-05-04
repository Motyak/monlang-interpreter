include utils.mk # askmake, not, shell_onrun, shouldrebuild

export CXX := ccache g++

SHELL := /bin/bash
RM := rm -rf
CXXFLAGS = --std=c++23 -Wall -Wextra -Og -ggdb3 -I include
DEPFLAGS = -MMD -MP -MF .deps/$*.d
ARFLAGS = D -M < <(tools/aggregate-libs.mri.sh $@ $^); :

###########################################################

OBJS := obj/interpret.o \
obj/monlang_parser.o \
obj/builtin.o \
obj/Environment.o
DEPS := $(OBJS:obj/%.o=.deps/%.d)

LIB_ARTIFACT_DIRS := ${foreach lib,${wildcard lib/*/},$(lib:%/=%)/{.deps,obj,dist,bin}}# for cleaning

###########################################################

all: dist/monlang-interpreter.a

main: $(OBJS)

clean:
	$(RM) obj/* .deps/*

mrproper:
	$(RM) obj .deps bin/*.elf bin/out bin/test dist lib/libs.a $(LIB_ARTIFACT_DIRS)

.PHONY: all main dist clean mrproper

###########################################################

$(OBJS) obj/main.o: obj/%.o: src/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(DEPFLAGS)

obj/monlang_parser.o: CXXFLAGS += -I monlang-parser/include
obj/main.o: CXXFLAGS += -Wno-unused-label

dist/monlang-interpreter.a: ARFLAGS = rcsvD
dist/monlang-interpreter.a: $(OBJS) lib/libs.a
	$(AR) $(ARFLAGS) $@ $^

bin/main.elf: obj/main.o $(OBJS) lib/libs.a
	$(CXX) -o $@ $^ $(LDFLAGS) $(LDLIBS)

-include $(DEPS) .deps/main.d

############################################################
# libs
############################################################

## aggregate all main code dependencies (.o, .a) into one static lib ##
.SECONDEXPANSION:
lib/libs.a: $$(libs)
	$(if $(call shouldrebuild, $@, $^), \
		$(AR) $(ARFLAGS) $@ $^)

## aggregate all test code dependencies (.o, .a) into one static lib ##
.SECONDEXPANSION:
lib/test-libs.a: $$(test_libs)
	$(if $(call shouldrebuild, $@, $^), \
		$(AR) $(ARFLAGS) $@ $^)

## build lib monlang-parser ##
libs += lib/monlang-parser/dist/monlang-parser.a
$(if $(call askmake, lib/monlang-parser), \
	.PHONY: lib/monlang-parser/dist/monlang-parser.a)
lib/monlang-parser/dist/monlang-parser.a:
	$(MAKE) -C lib/monlang-parser dist/monlang-parser.a

###########################################################

# will create all necessary directories after the Makefile is parsed
$(call shell_onrun, mkdir -p .deps obj bin dist)

## shall not rely on these ##
# .DELETE_ON_ERROR:
.SUFFIXES:
