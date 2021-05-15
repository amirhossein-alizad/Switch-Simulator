CC = g++
BUILD_DIR = build
SRC_DIR = src
INCLUDE_DIR = include
CFLAGS = -std=c++11 -I$(INCLUDE_DIR)

EXECUTABLE_FILE = simulator.out

OBJECTS = \
	$(BUILD_DIR)/Main.o \
	$(BUILD_DIR)/Switch.o \
	$(BUILD_DIR)/System.o \
	$(BUILD_DIR)/Manager.o \
	$(BUILD_DIR)/Frame.o \

FrameSensitivityList = \
	$(SRC_DIR)/Frame.cpp \
	$(INCLUDE_DIR)/Frame.hpp \

SwitchSensitivityList = \
	$(SRC_DIR)/Switch.cpp \
	$(INCLUDE_DIR)/Switch.hpp \
	$(INCLUDE_DIR)/Frame.hpp \

SystemSensitivityList = \
	$(SRC_DIR)/System.cpp \
	$(INCLUDE_DIR)/System.hpp \
	$(INCLUDE_DIR)/Frame.hpp \

ManagerSensitivityList = \
	$(SRC_DIR)/Manager.cpp \
	$(INCLUDE_DIR)/Manager.hpp \
	$(INCLUDE_DIR)/Switch.hpp \
	$(INCLUDE_DIR)/System.hpp \

MainSensitivityList = \
	$(SRC_DIR)/Main.cpp \
	$(INCLUDE_DIR)/Manager.hpp \

all: $(BUILD_DIR) $(EXECUTABLE_FILE)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/Frame.o: $(FrameSensitivityList)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Frame.cpp -o $(BUILD_DIR)/Frame.o

$(BUILD_DIR)/Switch.o: $(SwitchSensitivityList)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Switch.cpp -o $(BUILD_DIR)/Switch.o

$(BUILD_DIR)/System.o: $(SystemSensitivityList)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/System.cpp -o $(BUILD_DIR)/System.o

$(BUILD_DIR)/Manager.o: $(ManagerSensitivityList)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Manager.cpp -o $(BUILD_DIR)/Manager.o

$(BUILD_DIR)/Main.o: $(MainSensitivityList)
	$(CC) $(CFLAGS) -c $(SRC_DIR)/Main.cpp -o $(BUILD_DIR)/Main.o

$(EXECUTABLE_FILE): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(EXECUTABLE_FILE)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) *.o *.out  system* switch* STPlog.txt

.PHONY: cleandir
cleandir:
	rm -rf system* switch* STPlog.txt