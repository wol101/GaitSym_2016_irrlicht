# get the system and architecture
ifeq ($(OS),Windows_NT)
    SYSTEM := WIN32
    ifeq ($(PROCESSOR_ARCHITECTURE),AMD64) OR ($(PROCESSOR_ARCHITEW6432),AMD64)
	ARCH := AMD64
    endif
    ifeq ($(PROCESSOR_ARCHITECTURE),x86)
	ARCH := IA32
    endif
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
	SYSTEM := LINUX
    endif
    ifeq ($(UNAME_S),Darwin)
	SYSTEM := OSX
    endif
    UNAME_P := $(shell uname -p)
    ifeq ($(UNAME_P),x86_64)
	ARCH := AMD64
    endif
    ifneq ($(filter %86,$(UNAME_P)),)
	ARCH := IA32
    endif
    ifneq ($(filter arm%,$(UNAME_P)),)
	ARCH := ARM
    endif
    ifneq ($(filter ppc%,$(UNAME_P)),)
	ARCH := PPC
    endif
endif
HOST := $(shell hostname)

ifeq ($(SYSTEM),OSX)
    OPT_FLAGS = -g -O0 -DdDOUBLE 
    #OPT_FLAGS = -O3 -ffast-math -fast -DEXPERIMENTAL
    CXXFLAGS = -Wall -fexceptions $(OPT_FLAGS) -DdDOUBLE -DUSE_OLD_ODE
    CFLAGS = -Wall $(OPT_FLAGS) -DdDOUBLE
    # suggested by linker
    # LDFLAGS = -Xlinker -bind_at_load $(OPT_FLAGS) 
    LDFLAGS = $(OPT_FLAGS) 
    LIBS = $(HOME)/Unix/lib/libode.a ${HOME}/Unix/lib/libANN.a -lxml2 -lpthread -lm -lz -framework CoreServices
    OPENGL_LIBS = $(HOME)/Unix/lib/libglui.a -framework GLUT -framework OpenGL
    INC_DIRS = -I../GaitSym2016/rapidxml-1.13 -I../GaitSym2016/exprtk -I$(HOME)/Unix/include -I/usr/include/libxml2
    OPENGL_INC = -I/System/Library/Frameworks/GLUT.framework/Versions/A/Headers -I/System/Library/Frameworks/OpenGL.framework/Versions/A/Headers/
endif

ifeq ($(SYSTEM),LINUX)
    ifeq ($(ARCH),PPC) 
	CXXFLAGS = -O3 -DdDOUBLE -DUSE_OLD_ODE
	LDFLAGS  =  
	CXX      = mpic++
	CC       = mpicc
	LIBS = -L"$(HOME)/Unix/lib" -lode -lxml2 -lpthread -lm  
	INC_DIRS = -I"$(HOME)/Unix/include" -I"$(HOME)/Unix/include/libxml2"
    endif
    
    ifeq ($(ARCH),AMD64)
	#CXXFLAGS = -static -ffast-math -O3 -DdDOUBLE -DEXPERIMENTAL
	CXXFLAGS = -static -O3 -DdDOUBLE -DEXPERIMENTAL -std=c++11
	LDFLAGS  = -static 
	CXX      = CC
	CC       = cc
	LIBS = -L"$(HOME)/Unix/lib" -lode -lANN -lxml2 -lpthread -lm -lz 
	INC_DIRS = -I../GaitSym2016/rapidxml-1.13 -I"$(HOME)/Unix/include" -I/usr/include/libxml2 
    endif
endif


# vpath %.cpp src
# vpath %.c src 

GAITSYMSRC = \
AMotorJoint.cpp\
BallJoint.cpp\
Body.cpp\
BoxCarDriver.cpp\
BoxGeom.cpp\
ButterworthFilter.cpp\
CappedCylinderGeom.cpp\
Contact.cpp\
Controller.cpp\
CyclicDriver.cpp\
CylinderWrapStrap.cpp\
DampedSpringMuscle.cpp\
DataFile.cpp\
DataTarget.cpp\
DataTargetQuaternion.cpp\
DataTargetScalar.cpp\
DataTargetVector.cpp\
Drivable.cpp\
Driver.cpp\
Environment.cpp\
ErrorHandler.cpp\
Face.cpp\
FacetedBox.cpp\
FacetedCappedCylinder.cpp\
FacetedConicSegment.cpp\
FacetedObject.cpp\
FacetedPolyline.cpp\
FacetedRect.cpp\
FacetedSphere.cpp\
Filter.cpp\
FixedDriver.cpp\
FixedJoint.cpp\
FloatingHingeJoint.cpp\
Geom.cpp\
GLUtils.cpp\
HingeJoint.cpp\
Joint.cpp\
MAMuscleComplete.cpp\
MAMuscle.cpp\
MAMuscleExtended.cpp\
Marker.cpp\
MovingAverage.cpp\
Muscle.cpp\
NamedObject.cpp\
NPointStrap.cpp\
ObjectiveMain.cpp\
PCA.cpp\
PIDMuscleLength.cpp\
PIDTargetMatch.cpp\
PlaneGeom.cpp\
PositionReporter.cpp\
RayGeom.cpp\
Reporter.cpp\
Simulation.cpp\
SliderJoint.cpp\
SphereGeom.cpp\
StackedBoxCarDriver.cpp\
StepDriver.cpp\
Strap.cpp\
StrokeFont.cpp\
SwingClearanceAbortReporter.cpp\
TCP.cpp\
ThreePointStrap.cpp\
TIFFWrite.cpp\
TorqueReporter.cpp\
TrimeshGeom.cpp\
TwoCylinderWrapStrap.cpp\
TwoPointStrap.cpp\
UDP.cpp\
UGMMuscle.cpp\
UniversalJoint.cpp\
Util.cpp\
Warehouse.cpp\
XMLConverter.cpp

GAITSYMOBJ = $(addsuffix .o, $(basename $(GAITSYMSRC) ) )
GAITSYMHEADER = $(addsuffix .h, $(basename $(GAITSYMSRC) ) ) PGDMath.h DebugControl.h SimpleStrap.h

BINARIES = bin/gaitsym bin/gaitsym_opengl bin/gaitsym_udp bin/gaitsym_opengl_udp bin/gaitsym_tcp bin/gaitsym_opengl_tcp

BINARIES_NO_OPENGL = bin/gaitsym bin/gaitsym_udp bin/gaitsym_tcp

all: directories binaries 

no_opengl: directories binaries_no_opengl

directories: bin obj 

binaries: $(BINARIES)

binaries_no_opengl: $(BINARIES_NO_OPENGL)

obj: 
	-mkdir obj
	-mkdir obj/no_opengl
	-mkdir obj/opengl
	-mkdir obj/no_opengl_udp
	-mkdir obj/opengl_udp
	-mkdir obj/no_opengl_tcp
	-mkdir obj/opengl_tcp

bin:
	-mkdir bin
	 
obj/no_opengl/%.o : src/%.cpp
	$(CXX) $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/gaitsym: $(addprefix obj/no_opengl/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(LIBS)

obj/opengl/%.o : src/%.cpp
	$(CXX) -DUSE_OPENGL $(CXXFLAGS) $(INC_DIRS) $(OPENGL_INC)  -c $< -o $@

bin/gaitsym_opengl: $(addprefix obj/opengl/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(OPENGL_LIBS) $(LIBS) 

obj/no_opengl_socket/%.o : src/%.cpp
	$(CXX) -DUSE_SOCKETS $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/gaitsym_socket: $(addprefix obj/no_opengl_socket/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(SOCKET_LIBS) $(LIBS)

obj/opengl_socket/%.o : src/%.cpp
	$(CXX) -DUSE_OPENGL -DUSE_SOCKETS $(CXXFLAGS) $(INC_DIRS) $(OPENGL_INC)  -c $< -o $@

bin/gaitsym_opengl_socket: $(addprefix obj/opengl_socket/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(SOCKET_LIBS) $(OPENGL_LIBS) $(LIBS) 

obj/no_opengl_udp/%.o : src/%.cpp
	$(CXX) -DUSE_UDP $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/gaitsym_udp: $(addprefix obj/no_opengl_udp/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(UDP_LIBS) $(LIBS)

obj/opengl_udp/%.o : src/%.cpp
	$(CXX) -DUSE_OPENGL -DUSE_UDP $(CXXFLAGS) $(INC_DIRS) $(OPENGL_INC)  -c $< -o $@

bin/gaitsym_opengl_udp: $(addprefix obj/opengl_udp/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(UDP_LIBS) $(OPENGL_LIBS) $(LIBS) 

obj/no_opengl_tcp/%.o : src/%.cpp
	$(CXX) -DUSE_TCP $(CXXFLAGS) $(INC_DIRS)  -c $< -o $@

bin/gaitsym_tcp: $(addprefix obj/no_opengl_tcp/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(TCP_LIBS) $(LIBS)

obj/opengl_tcp/%.o : src/%.cpp
	$(CXX) -DUSE_OPENGL -DUSE_TCP $(CXXFLAGS) $(INC_DIRS) $(OPENGL_INC)  -c $< -o $@

bin/gaitsym_opengl_tcp: $(addprefix obj/opengl_tcp/, $(GAITSYMOBJ) ) 
	$(CXX) $(LDFLAGS) -o $@ $^ $(TCP_LIBS) $(OPENGL_LIBS) $(LIBS) 


clean:
	rm -rf obj bin
	rm -rf distribution
	rm -rf build*

superclean:
	rm -rf obj bin
	rm -rf distribution
	rm -rf build*
	rm -rf GaitSymQt/GaitSymQt.pro.user.*
	find . -name '.DS_Store' -exec rm -f {} \;
	find . -name '.gdb_history' -exec rm -f {} \;
	find . -name '.#*' -exec rm -f {} \;
	find . -name '*~' -exec rm -f {} \;
	find . -name '#*' -exec rm -f {} \;
	find . -name '*.bak' -exec rm -f {} \;
	find . -name '*.bck' -exec rm -f {} \;
	find . -name '*.tmp' -exec rm -f {} \;
	find . -name '*.o' -exec rm -f {} \;

distribution: distribution_dirs gaitsym_distribution gaitsym_distribution_extras

distribution_dirs:
	rm -rf distribution
	-mkdir distribution
	-mkdir distribution/src

gaitsym_distribution: $(addprefix distribution/src/, $(GAITSYMSRC)) $(addprefix distribution/src/, $(GAITSYMHEADER))

$(addprefix distribution/src/, $(GAITSYMSRC)):
	scripts/strip_ifdef.py EXPERIMENTAL $(addprefix src/, $(notdir $@)) $@ 

$(addprefix distribution/src/, $(GAITSYMHEADER)):
	scripts/strip_ifdef.py EXPERIMENTAL $(addprefix src/, $(notdir $@)) $@ 

gaitsym_distribution_extras:
	cp -rf ann_1.1.2 distribution/
	cp -rf exprtk distribution/
	cp -rf GaitSymQt distribution/
	cp -rf irrlicht-1.9 distribution/
	cp -rf ode-0.15 distribution/
	cp -rf rapidxml-1.13 distribution/
	cp makefile distribution/
	find distribution -type d -name CVS -print -exec rm -rf {} \;
	rm -rf distribution/GaitSymQt/GaitSymQt.pro.*


