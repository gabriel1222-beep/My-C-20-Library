CXX ?= g++
AR ?= ar
CXXFLAGS ?= -O3 -std=c++11
CPPFLAGS += -Iinclude

LIB_NAMES := $(notdir $(patsubst %/,%,$(wildcard src/*/)))
LIB_NAMES := $(foreach n,$(LIB_NAMES),$(if $(wildcard src/$(n)/*.cpp),$(n)))
LIBRARIES := $(addprefix lib/lib,$(addsuffix .a,$(LIB_NAMES)))

.PHONY: all clean
all: $(LIBRARIES)

define make_library
SOURCES_$(1) := $$(wildcard src/$(1)/*.cpp)
OBJECTS_$(1) := $$(patsubst src/$(1)/%.cpp,build/$(1)/%.o,$$(SOURCES_$(1)))

lib/lib$(1).a: $$(OBJECTS_$(1))
	mkdir -p $$(@D)
	rm -f $$@
	$$(AR) rcs $$@ $$^

build/$(1)/%.o: src/$(1)/%.cpp
	mkdir -p $$(@D)
	$$(CXX) $$(CPPFLAGS) $$(CXXFLAGS) -MMD -MP -c $$< -o $$@
endef

$(foreach n,$(LIB_NAMES),$(eval $(call make_library,$(n))))

-include $(wildcard build/*/*.d)

clean:
	rm -rf build
	rm -f lib/*.a
