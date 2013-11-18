# makefile for ttrack, based on opencv
CXX = g++
CXXFLAGS = `pkg-config --cflags opencv` -g3 -c
LIBS = `pkg-config --libs opencv` -lm

AllCpps = $(wildcard *.cpp)
AllObjects = $(AllCpps:.cpp=.o)

guidematting: $(AllObjects)
	@echo 'Building executable:'
	$(CXX) -o $@ $(AllObjects) $(LIBS)
	@echo 'Building finished.'

.PHONY:clean
clean:
	rm -rf $(AllObjects) $(AllObjects:.o=.d)
	@echo 'Clean finished.'

include $(AllObjects:.o=.d)

%.d: %.cpp
	set -e; rm -f $@; \
	$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; \
	sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$ 


