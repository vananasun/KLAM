COMPILER = wcc
LINKER   = wlink
CFLAGS   = -zp1 -zq -ml -bt=DOS -3 -ot -oi -fpi87 -fp3      -oneatx
LFLAGS   = SYSTEM DOS OPTION QUIET 
LIBPATH  = C:\WATCOM\LIB386
TARGET   = BUILD\KLAM.EXE
SRCEXT   = .C

# Define basic commands.
mkdir    = mkdir $(subst /,\,$(1))>nul 2>&1||(exit 0)
rm       = $(wordlist 2,65535,$(foreach FILE,$(subst /,\,$(1)),& del $(FILE)>nul 2>&1))||(exit 0)
rmdir    = del /f/s/q $(subst /,\,$(1))>nul 2>&1&&rmdir /s/q $(subst /,\,$(1))>nul 2>&1||(exit 0)

# Define a recursive wildcard function.
# Usage: $(call rwildcard, <directory>, <file pattern>)
define rwildcard
	$(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2)$(filter $(subst *,%,$2),$d))
endef

# Gather list of source files.
srcfiles := $(call rwildcard, SRC/, %$(SRCEXT))

# Create list of object files from source files,
# every file in the list that does not exist yet
# will be compiled automatically.
objects  := $(patsubst %$(SRCEXT), %.OBJ, $(srcfiles))
objects  := $(objects:SRC/%=BUILD/%)


# TARGET := $(subst /,\,$(TARGET))


# Debug target.
all: debug
debug: CFLAGS += -oe=10
debug: $(objects)
	$(LINKER) $(LFLAGS) NAME $(TARGET) LIBP $(LIBPATH) FILE { $^ }
	@$(call rm, *.ERR)

# Release target.
release: CFLAGS += -fp2 -7 -ol+ -oe=30
release: $(objects)
	$(LINKER) $(LFLAGS) NAME $(TARGET) LIBP $(LIBPATH) FILE { $^ }
	@$(call rm, *.ERR)



# Target for every required object in the specified target.
$(objects): BUILD/%.OBJ: SRC/%$(SRCEXT)
	$(COMPILER) $(CFLAGS) $(subst /,\,$^) -fo=$(subst /,\,$@) -i=SRC\

# 'make clean' removes the specified target file
# and the specified object directory.
.PHONY: clean
clean:
	@$(call rm, $(TARGET))
	@$(call rm, BUILD/*.OBJ)
