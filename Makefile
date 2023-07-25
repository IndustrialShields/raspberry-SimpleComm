override CXX=g++
override CXXFLAGS=-Wall -Wextra -Werror -std=c++14 #-O2

LIBDIR = out
LIBNAME = $(LIBDIR)/libraspberry-SimpleComm

SRCDIR = src
BUILDDIR = $(SRCDIR)/build

HEADERS = $(wildcard src/*.h)
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
STATIC_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.o,$(SOURCES))
DYNAMIC_OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(BUILDDIR)/%.do,$(SOURCES))

all: static_objects $(LIBNAME).a dynamic_objects $(LIBNAME).so

install: $(LIBNAME).so
	sudo cp $< /usr/local/lib/
	sudo mkdir -p /usr/local/include/raspberry-SimpleComm
	sudo cp $(HEADERS) /usr/local/include/raspberry-SimpleComm/
	sudo ldconfig
uninstall:
	sudo rm /usr/local/lib/libraspberry-SimpleComm.so
	sudo rm -r /usr/local/include/raspberry-SimpleComm

# Build static objects
static_objects: $(STATIC_OBJECTS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	@mkdir -p $(BUILDDIR)
	$(CXX) $< -c -o $@ $(CXXFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $< -o $@ $(CXXFLAGS)

# Static library
$(LIBNAME).a: $(STATIC_OBJECTS)
	@mkdir -p $(LIBDIR)
	ar rcs $@ $(STATIC_OBJECTS)

# Build dynamic objects
dynamic_objects: $(DYNAMIC_OBJECTS)

$(BUILDDIR)/%.do: $(SRCDIR)/%.cpp $(SRCDIR)/%.h
	@mkdir -p $(BUILDDIR)
	$(CXX) $< -c -o $@ $(CXXFLAGS) -fPIC

$(BUILDDIR)/%.do: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $< -o $@ $(CXXFLAGS) -fPIC

# Dynamic library
$(LIBNAME).so: $(DYNAMIC_OBJECTS)
	@mkdir -p $(LIBDIR)
	$(CXX) -shared -o $@ $(DYNAMIC_OBJECTS)

EXAMPLESDIR = examples
EXAMPLES = $(patsubst $(EXAMPLESDIR)/%.cpp,$(EXAMPLESDIR)/%, $(wildcard $(EXAMPLESDIR)/*.cpp))

examples: $(EXAMPLES)

$(EXAMPLESDIR)/%:  $(EXAMPLESDIR)/%.cpp $(LIBNAME).a
	$(CXX) $< -o $@ $(CXXFLAGS) -I $(SRCDIR) -L $(LIBDIR)/ -lraspberry-SimpleComm -static

clean:
	\rm -rf $(BUILDDIR) $(LIBDIR) $(filter-out %.cpp,$(wildcard $(EXAMPLESDIR)/*))

