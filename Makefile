# Compiler and flags
CXX := g++

BINDIR = bin

ifeq ($(OS),Windows_NT)
#WINDOWS
	CXXFLAGS := -std=c++17 -Wall -D_WIN32_WINNT=0x0A00 -DWINVER=0x0A00 -Iutils -I"C:\Program Files\MySQL\mysql-connector-c-6.1.11-winx64\include"
	LDFLAGS := -lws2_32 -lwinmm -lwsock32 -lpthread -L"C:\Program Files\MySQL\mysql-connector-c-6.1.11-winx64\lib" -lmysql 
	TARGET := bin\QMonitor.exe
	RM = del /Q
else
# LINUX
	CXXFLAGS := -std=c++17 -Wall -Iutils
	LDFLAGS := -lmysqlcppconn -lpthread
	TARGET := bin/QMonitor
	RM = rm -f
endif
# Sources
SRC := src/main.cpp src/Monitor.cpp src/Config.cpp src/RestServer.cpp src/Logger.cpp

# Build rule
$(TARGET): $(SRC)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

MONITORLOGFILE=/var/log/monitor_service.log
SERVERLOGFILE=/var/log/server_service.log
.PHONY: install-log
install-log:
	sudo touch $(MONITORLOGFILE)
	sudo chown $(USER):$(USER) $(MONITORLOGFILE)
	sudo touch $(SERVERLOGFILE)
	sudo chown $(USER):$(USER) $(SERVERLOGFILE)

# Clean rule
clean:
	$(RM) *.o $(TARGET)
