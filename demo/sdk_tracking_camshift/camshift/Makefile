CXX     = g++
TARGET  = camshiftDemo 
OBJECTS = camshiftDepth.o DJI_utility.o
CFLAGS  = -g -Wall -I../include `pkg-config --cflags opencv` 
LDFLAGS = -Wl,-rpath,./ -lpthread -lrt -L./ -L/usr/local/lib/ -lDJI_guidance -lusb-1.0 `pkg-config --libs opencv` 

$(TARGET) : $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
camshiftDepth.o : camshiftDepth.cpp DJI_utility.h
	$(CXX) $(CFLAGS) -c camshiftDepth.cpp DJI_utility.h
DJI_utility.o : DJI_utility.cpp DJI_utility.h
	$(CXX) $(CFLAGS) -c DJI_utility.cpp DJI_utility.h
clean:
	rm -rf *.o *.gch *.avi $(TARGET)
