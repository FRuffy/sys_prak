app = client

srcExt = c
srcdir = src
objDir = obj
binDir = bin
errdir = $(srcdir)/errhandler



CC = gcc
CFlags += -c -Wextra -Werror -Wall -O2
sources := $(shell find $(srcDir) -name '*.$(srcExt)') 
srcDirs := $(shell find . -name '*.$(srcExt)' -exec dirname {} \; | uniq)
objects := $(patsubst %.$(srcExt),$(objDir)/%.o,$(sources))


all: $(binDir)/$(app)

$(binDir)/$(app):  buildrepo  $(objects) 
	@mkdir -p `dirname $@`
	@$(CC) $(objects) $(LDFlags) -o $@

$(objDir)/%.o: %.$(srcExt)
	@echo "Building $@ ... "
	@$(CC) $(CFlags) $< -o $@

clean:
	rm -r $(objDir) 
	rm $(binDir)/$(app)
	rm $(binDir)/log.txt
	@echo "Cleaning..."


buildrepo:
	@$(call make-repo)

define make-repo
   for dir in $(srcDirs); \
   do \
	mkdir -p $(objDir)/$$dir; \
   done
endef




