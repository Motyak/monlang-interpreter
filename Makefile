include utils.mk # askmake, not, shell_onrun, shouldrebuild

export CXX := ccache g++

SHELL := /bin/bash
RM := rm -rf
CXXFLAGS := --std=c++23 -Wall -Wextra -Og -ggdb3 -I include
DEPFLAGS = -MMD -MP -MF .deps/$*.d
ARFLAGS = D -M < <(tools/aggregate-libs.mri.sh $@ $^); :

###########################################################

OBJS = \
obj/interpret.o \
obj/monlang_parser.o \
obj/Environment.o \
obj/MapKeyCmp.o \
$(BUILTIN_OBJS) \

BUILTIN_OBJS = \
obj/builtin/print.o \
obj/builtin/getline.o \
obj/builtin/getchar.o \
obj/builtin/putstr.o \
obj/builtin/slurpfile.o \
obj/builtin/sleep.o \
obj/builtin/exit.o \
obj/builtin/die.o \
obj/builtin/len.o \
$(OPERATORS_OBJS) \
$(PRIM_CTORS_OBJS) \

OPERATORS_OBJS := \
obj/builtin/operators/logical_and.o \
obj/builtin/operators/logical_or.o \
obj/builtin/operators/eq.o \
obj/builtin/operators/gt.o \
obj/builtin/operators/plus.o \
obj/builtin/operators/mul.o \
obj/builtin/operators/div.o \
obj/builtin/operators/intdiv.o \
obj/builtin/operators/mod.o \
obj/builtin/operators/pow.o \
obj/builtin/operators/leftshift.o \
obj/builtin/operators/rightshift.o \
obj/builtin/operators/bitwise_not.o \
obj/builtin/operators/bitwise_and.o \
obj/builtin/operators/bitwise_or.o \
obj/builtin/operators/bitwise_xor.o \

PRIM_CTORS_OBJS := \
obj/builtin/prim_ctors/Byte.o \
obj/builtin/prim_ctors/Bool.o \
obj/builtin/prim_ctors/Int.o \
obj/builtin/prim_ctors/Float.o \
obj/builtin/prim_ctors/Char.o \
obj/builtin/prim_ctors/Str.o \
obj/builtin/prim_ctors/List.o \
obj/builtin/prim_ctors/Lambda.o \

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

obj/main.o obj/monlang_parser.o: CXXFLAGS += -I monlang-parser/include
obj/main.o: CXXFLAGS += -Wno-unused-label
obj/builtin/prim_ctors/Bool.o: CXXFLAGS += -D TOGGLE_NIL_CAST_TO_BOOL
# obj/interpret.o: CXXFLAGS += -D TOGGLE_UNBOUND_SYM_AS_STR
# obj/interpret.o: CXXFLAGS += -D TOGGLE_PASS_BY_VALUE
$(OBJS) obj/main.o: obj/%.o: src/%.cpp
	$(CXX) -o $@ -c $< $(CXXFLAGS) $(DEPFLAGS)

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
${call shell_onrun, mkdir -p {.deps,obj}/builtin/{operators,prim_ctors} bin dist}

## shall not rely on these ##
# .DELETE_ON_ERROR:
.SUFFIXES:
