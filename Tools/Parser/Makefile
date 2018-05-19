PARSER = $(DEBUG)/Parser

PARSER_SRCS = $(PARSER_DIR)/ArabicShaping.cpp

PARSER_OBJS = $(PARSER_SRCS:$(PARSER_DIR)/%.cpp=$(PARSER)/%.o)

$(PARSER):
	mkdir $(PARSER)

$(PARSER)/%.o: $(PARSER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) -c $< -o $@

$(PARSER_TARGET): $(PARSER_OBJS)
	$(AR) $(ARFLAGS) $(PARSER_TARGET) $(PARSER_OBJS)

parser: $(PARSER) $(PARSER_TARGET)

parser_clean:
	$(RM) $(PARSER)/*.o
	$(RM) $(PARSER_TARGET)
