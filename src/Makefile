########### settings
BINDIR = ../bin/
PROGRAM = congeal
CC = g++ 
OBJDIR = ../obj/

########## patterns
HFILES = $(wildcard *.h)
CXXFILES = $(wildcard *.c++) $(wildcard *.cc)
OBJFILES = $(addprefix $(OBJDIR),$(patsubst %.c++,%.o,$(wildcard *.c++)))

ifeq ($(OSTYPE),darwin)
CLIBS = -lpthread 
#-llbfgs
else
CLIBS = -lpthread
endif


########## release
release : PROGRAM := $(PROGRAM)
ifeq ($(OSTYPE),darwin)
release : CFLAGS = \
			-DPTHREAD_BARRIER=1\
			-O3\
			-Wall \
			-Winline \
			-fast \
			--param inline-unit-growth=500000 \
			--param large-function-growth=500000 \
			--param max-inline-insns-single=500000 \
			-DPTHREAD_BARRIER=1

#			-DFEATURE_LBFGS=1\

else
release : CFLAGS = \
			-DFEATURE_LBFGS=0\
			-DPTHREAD_BARRIER=0\
			-O3 \
			-Wall\
			-finline-limit=2000 \
			--param inline-unit-growth=90000 \
			--param large-function-growth=90000 \
			--param max-inline-insns-single=20000 \
			--param large-stack-frame=90000 \
			--param large-stack-frame-growth=900 \
			-falign-loops=16 \
			-falign-jumps=16 \
			-falign-functions=16\
			-ffast-math \
			-fstrict-aliasing\
			-funroll-loops \
			-ftree-loop-linear\
			-fsched-interblock \
			-fgcse-sm 
endif
release : LINKFLAGS = 
release : OBJDIR = ../obj/release/
release : 
	$(MAKE) target OBJDIR="$(OBJDIR)" CFLAGS="$(CFLAGS)"  LINKFLAGS="$(LINKFLAGS)" PROGRAM="$(PROGRAM)"
.PHONY : release

########## debug
debug : PROGRAM := $(PROGRAM).debug
ifeq ($(OSTYPE),darwin)
debug : CFLAGS = -g -O0 -Wall -frepo \
			-DPTHREAD_BARRIER=1
else
debug : CFLAGS = -g -O0 -Wall -frepo \
			-DFEATURE_LBFGS=0
endif

debug : LINKFLAGS = 
debug : OBJDIR = ../obj/debug/
debug : 
	$(MAKE) target OBJDIR="$(OBJDIR)" CFLAGS="$(CFLAGS)"  LINKFLAGS="$(LINKFLAGS)" PROGRAM="$(PROGRAM)"
.PHONY : debug

########## profile
profile : PROGRAM := $(PROGRAM).profile
profile : CFLAGS = -g -O3 -Wall -frepo -finstrument-functions
profile : OBJDIR = ../obj/profile/
profile : LINKFLAGS = -lSaturn 
profile : 
	$(MAKE) target OBJDIR="$(OBJDIR)" CFLAGS="$(CFLAGS)"  LINKFLAGS="$(LINKFLAGS)" PROGRAM="$(PROGRAM)"
.PHONY : profile

########## test
test: $(OBJDIR)libRefCount.o $(OBJDIR)String.o 
	g++ -g -O0 $(OBJDIR)libRefCount.o $(OBJDIR)String.o test.cc -o TEST

########## rules
target : $(BINDIR)$(PROGRAM)

$(BINDIR)$(PROGRAM): Makefile $(OBJDIR)Makefile.dep $(OBJFILES) | $(BINDIR) 
	$(CC) $(CFLAGS) $(LINKFLAGS) $(CLIBS) $(OBJFILES) -o $(BINDIR)$(PROGRAM) 
	@echo compilation done.

.PHONY: $(OBJDIR)
$(OBJDIR):
	@[ -d $@ ] || mkdir -p $@

.PHONY: $(BINDIR)
$(BINDIR):
	@[ -d $@ ] || mkdir -p $@


$(OBJDIR)Makefile.dep: Makefile | $(OBJDIR)
	g++ -MM $(CXXFILES) $(HFILES) | sed "s-\(.*:\)-"$(OBJDIR)"\1-" > $(OBJDIR)Makefile.dep

.PHONY: depend
depend:
	g++ -MM $(CXXFILES) $(HFILES) | sed "s-\(.*:\)-"$(OBJDIR)"\1-" > $(OBJDIR)Makefile.dep

tags:
	etags *

$(OBJDIR)%.o : %.c++ | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@ 


clean:
	rm -f $(OBJDIR)*/* $(BINDIR)*
.PHONY: clean

# DO NOT DELETE
include $(OBJDIR)Makefile.dep
