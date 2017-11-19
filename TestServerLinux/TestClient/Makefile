BOOST_ROOT = /home/arthur/lib/boost

CXXFLAGS =	-I$(BOOST_ROOT)/include -std=c++11 -pthread -O2 -g
OBJS =		TestClient.o ClientFn.o	

LIBS = -L$(BOOST_ROOT)/lib -static -lboost_system

TARGET =	testc

$(TARGET):	$(OBJS)
	$(CXX) -o $(TARGET) $(OBJS) $(LIBS) $(CXXFLAGS)

all:	clean	$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
