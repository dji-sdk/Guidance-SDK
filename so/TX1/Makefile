TARGET = ./TX1/libDJI_guidance.so
OBJECTS = config.o frame.o imagetransfer.o log.o usb.o protocal.o linux_wrapper.o DJI_guidance.o 
CFLAGS = -rdynamic -Wall -fPIC -I./libusb-1.0
LDFLAGS = -shared -Wl,-rpath,./ -L -lusb-1.0.so

CXX = g++

$(TARGET) : $(OBJECTS)
	$(CXX) -o $(TARGET) $(OBJECTS) $(LDFLAGS)
	
bmp.o : bmp.cpp bmp.h
	$(CXX) $(CFLAGS) -c bmp.cpp bmp.h
	
config.o : config.cpp config.h
	$(CXX) $(CFLAGS) -c config.cpp config.h
	
frame.o : frame.cpp frame.h
	$(CXX) $(CFLAGS) -c frame.cpp frame.h
	
framecontainer.o : framecontainer.cpp framecontainer.h
	$(CXX) $(CFLAGS) -c framecontainer.cpp framecontainer.h
	
imagetransfer.o : imagetransfer.cpp imagetransfer.h
	$(CXX) $(CFLAGS) -c imagetransfer.cpp imagetransfer.h
	
DJI_guidance.o : DJI_guidance.cpp DJI_guidance.h
	$(CXX) $(CFLAGS) -c DJI_guidance.cpp DJI_guidance.h
	
log.o : log.cpp log.h
	$(CXX) $(CFLAGS) -c log.cpp log.h
	
protocal.o : protocal.cpp protocal.h
	$(CXX) $(CFLAGS) -c protocal.cpp protocal.h
	
stack.o : stack.cpp stack.h
	$(CXX) $(CFLAGS) -c stack.cpp stack.h
	
usb.o : usb.cpp usb.h
	$(CXX) $(CFLAGS) -c usb.cpp usb.h 
	
linux_wrapper.o : ./os/linux_wrapper.cpp platform.h
	$(CXX) $(CFLAGS) -c ./os/linux_wrapper.cpp platform.h 

clean:
	rm *.o *.gch $(TARGET)
