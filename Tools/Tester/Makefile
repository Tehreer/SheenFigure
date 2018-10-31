TESTER_INCLUDES = -I$(ROOT_DIR) -I$(HEADERS_DIR) -I$(TOOLS_DIR) -I$(SHEENBIDI_DIR)
TESTER_FLAGS = $(TESTER_INCLUDES)
TESTER_LIBS = -L$(DEBUG) -l$(LIB_SHEENFIGURE) -l$(LIB_SHEENBIDI) -l$(LIB_PARSER)

TESTER      = $(DEBUG)/Tester
TESTER_OT   = $(TESTER)/OpenType
TESTER_UTIL = $(TESTER)/Utilities

TESTER_SRCS = $(TESTER_DIR)/AlbumTester.cpp \
              $(TESTER_DIR)/FontTester.cpp \
              $(TESTER_DIR)/GlyphManipulationTester.cpp \
              $(TESTER_DIR)/GlyphPositioningTester.cpp \
              $(TESTER_DIR)/GlyphSubstitutionTester.cpp \
              $(TESTER_DIR)/JoiningTypeLookupTester.cpp \
              $(TESTER_DIR)/ListTester.cpp \
              $(TESTER_DIR)/LocatorTester.cpp \
              $(TESTER_DIR)/MiscTester.cpp \
              $(TESTER_DIR)/main.cpp \
              $(TESTER_DIR)/PatternTester.cpp \
              $(TESTER_DIR)/SchemeTester.cpp \
              $(TESTER_DIR)/TextProcessorTester.cpp \
              $(TESTER_DIR)/OpenType/Builder.cpp \
              $(TESTER_DIR)/OpenType/Writer.cpp \
              $(TESTER_DIR)/Utilities/Convert.cpp \
              $(TESTER_DIR)/Utilities/SFPattern+Testing.cpp \
              $(TESTER_DIR)/Utilities/Unicode.cpp

TESTER_OBJS = $(TESTER_SRCS:$(TESTER_DIR)/%.cpp=$(TESTER)/%.o)

$(TESTER):
	mkdir $(TESTER)
	mkdir $(TESTER_OT)
	mkdir $(TESTER_UTIL)

$(TESTER)/%.o: $(TESTER_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) $(EXTRA_FLAGS) $(TESTER_FLAGS) -c $< -o $@

$(TESTER_TARGET): $(TESTER_OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(EXTRA_FLAGS) $(TESTER_FLAGS) $(EXTRA_LIBS) $(TESTER_LIBS)

tester: debug parser $(TESTER) $(TESTER_TARGET)

tester_clean:
	$(RM) $(TESTER)/*.o
	$(RM) $(TESTER_OT)/*.o
	$(RM) $(TESTER_UTIL)/*.o
	$(RM) $(TESTER_TARGET)
