TARGET = binary-name
CXX = g++

SOURCE = $(shell ls src/*.cpp)
OBJS = $(SOURCE:.cpp=.o)

OPTS = -O2 -std=c++11
IDFLAGS = -I phillip/src -I src
LDFLAGS = -L phillip/lib -lphil

GTEST_URL = "http://googletest.googlecode.com/files/gtest-1.7.0.zip"
GTEST_ZIP = gtest-1.7.0.zip
GTEST_DIR = gtest/gtest-1.7.0
GTEST_SRC = $(GTEST_DIR)/src/gtest-all.cc
GTEST_OBJ = $(GTEST_DIR)/src/gtest-all.o
GTEST_LIB = $(GTEST_DIR)/build/libgtest.a

$(TARGET): $(OBJS)
	mkdir -p bin
	$(CXX) $(OPTS) $(OBJS) $(IDFLAGS) $(LDFLAGS) -o $(TARGET)

.cpp.o:
	$(CXX) -O2 -c $<

clean:
	rm -f $(TARGET)
	rm -f *.o

gtest:
	wget $(GTEST_URL) -O $(GTEST_ZIP)
	unzip $(GTEST_ZIP) -d gtest
	mkdir -p $(GTEST_DIR)/build
	$(CXX) -isystem $(GTEST_DIR)/include -I $(GTEST_DIR) -pthread -c $(GTEST_DIR)/src/gtest-all.cc -o $(GTEST_OBJ)
	ar -rv $(GTEST_LIB) $(GTEST_OBJ)
	rm $(GTEST_ZIP)
