TARGET = bin/phil-test
CXX = g++

SOURCE = $(shell ls src/*.cpp)
OBJS = $(SOURCE:.cpp=.o)

GTEST_URL = "http://googletest.googlecode.com/files/gtest-1.7.0.zip"
GTEST_ZIP = gtest-1.7.0.zip
GTEST_DIR = gtest/gtest-1.7.0
GTEST_SRC = $(GTEST_DIR)/src/gtest-all.cc
GTEST_OBJ = $(GTEST_DIR)/src/gtest-all.o
GTEST_LIB = $(GTEST_DIR)/build/libgtest.a

OPTS = -O2 -std=c++11
IDFLAGS = -I phillip/src
LDFLAGS = -L phillip/lib -lphil

# GOOGLE TEST
IDFLAGS += -I $(GTEST_DIR)/include
LDFLAGS += -L $(GTEST_DIR)/build -lgtest

# USE-LP-SOLVE
OPTS += -DUSE_LP_SOLVE
LDFLAGS += -llpsolve55

# USE-GUROBI
OPTS += -DUSE_GUROBI
LDFLAGS += -lgurobi_c++ -lgurobi60 -lpthread


$(TARGET): $(OBJS)
	mkdir -p bin
	$(CXX) $(OPTS) $(OBJS) $(IDFLAGS) $(LDFLAGS) -o $(TARGET)

.cpp.o:
	$(CXX) $(OPTS) $(IDFLAGS) -c -o $(<:.cpp=.o) $<

clean:
	rm -f $(TARGET)
	rm -f $(OBJS)

gtest:
	wget $(GTEST_URL) -O $(GTEST_ZIP)
	unzip $(GTEST_ZIP) -d gtest
	mkdir -p $(GTEST_DIR)/build
	$(CXX) -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread -c $(GTEST_DIR)/src/gtest-all.cc -o $(GTEST_OBJ)
	ar -rv $(GTEST_LIB) $(GTEST_OBJ)
	rm $(GTEST_ZIP)
