CXX = g++
LN = g++
ODIR = build

CPP_FLAGS += --std=c++17
CPP_FLAGS += -MMD -MP
CPP_FILES := $(shell find src -type f -name '*.cpp')

FLAGS += -Og -g
# FLAGS += -O3

LN_FLAGS += -lm

OBJ = $(patsubst %,$(ODIR)/%.o,$(CPP_FILES))
DEP := $(OBJ:.o=.d)

$(ODIR)/%.cpp.o: %.cpp
	@mkdir -p "$$(dirname "$@")"
	$(CXX) $(FLAGS) $(CPP_FLAGS) -c $< -o $@

libresprite: $(OBJ)
	$(LN) $(FLAGS) $(LN_FLAGS) $^ -o $@

.PHONY: clean

clean:
	rm -rf $(ODIR)

-include $(DEP)
