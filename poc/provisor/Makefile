TARGET      := provisor

#The Directories, Source, Includes, Objects, Binary and Resources
SRCDIR      := src
BUILDDIR    := obj
TARGETDIR   := bin
RESDIR      := res

ifeq ($(PLATFORM),arm)
	CXX       := arm-linux-gnueabihf-g++
	STRIP     := /usr/arm-linux-gnueabihf/bin/strip
	LIBDIR    := ../lib/arm
else
	LIBDIR    := ../lib/x64 
endif

SRCEXT      := cpp
DEPEXT      := d
OBJEXT      := o

#Flags, Libraries and Includes
LIB         := -L$(LIBDIR)  -static-libstdc++ -lm -pthread -lstdc++fs \
				-liothub_client -liothub_client_mqtt_transport  \
				-lprov_device_client -lprov_device_ll_client -lprov_auth_client -lprov_mqtt_transport \
				 -lparson -lumqtt -laziotsharedutil   -lcrypto -lssl
#-L../torhsm/bin -ltorhsm

INCDIR      := -Isrc -I../inc/azure
INCDEP      := $(INC)
CXXFLAGS    := -g -std=c++17 -w $(INCDIR)

STRIP       ?= strip
CXX         ?= g++

#---------------------------------------------------------------------------------
#DO NOT EDIT BELOW THIS LINE
#---------------------------------------------------------------------------------
SOURCES     := $(shell find -L $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS     := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(SOURCES:.$(SRCEXT)=.$(OBJEXT)))

#Defauilt Make
all: resources $(TARGET)

#Copy Resources from Resources Directory to Target Directory
resources: directories
	if [ -d $(RESDIR) ]; then rsync -r $(RESDIR)/ $(TARGETDIR); fi

#Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

#Clean only Objects
clean:
	$(foreach obj, $(OBJECTS), $(RM) -f $(obj))
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -rf $(TARGETDIR)

#Pull in dependency info for *existing* .o files
-include $(OBJECTS:.$(OBJEXT)=.$(DEPEXT))

#Link
$(TARGET): $(OBJECTS)
	$(CXX) -o $(TARGETDIR)/$(TARGET) $^ $(LIB)
	$(STRIP) -vsg $(TARGETDIR)/$(TARGET)

#Compile
$(BUILDDIR)/%.$(OBJEXT): $(SRCDIR)/%.$(SRCEXT)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
	@$(CXX) $(CXXFLAGS) $(INCDEP) -MM $(SRCDIR)/$*.$(SRCEXT) > $(BUILDDIR)/$*.$(DEPEXT)
	@cp -f $(BUILDDIR)/$*.$(DEPEXT) $(BUILDDIR)/$*.$(DEPEXT).tmp
	@sed -e 's|.*:|$(BUILDDIR)/$*.$(OBJEXT):|' < $(BUILDDIR)/$*.$(DEPEXT).tmp > $(BUILDDIR)/$*.$(DEPEXT)
	@sed -e 's/.*://' -e 's/\\$$//' < $(BUILDDIR)/$*.$(DEPEXT).tmp | fmt -1 | sed -e 's/^ *//' -e 's/$$/:/' >> $(BUILDDIR)/$*.$(DEPEXT)
	@rm -f $(BUILDDIR)/$*.$(DEPEXT).tmp

print-%  : ; @echo $* = $($*)

#Non-File Targets
.PHONY: all rebuild clean cleaner resources
