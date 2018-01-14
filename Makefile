DIRS := test logger_uart logger_assert

DIRS := $(addprefix examples/,$(DIRS))

.PHONY: all
all: $(DIRS:=.build) doc

.PHONY: doc
doc:
	@echo "  DOC     doc/html"
	@$(MAKE) -C doc > /dev/null

.PHONY: clean
clean: $(DIRS:=.clean) doc.clean

%.build:
	@echo "  BUILD   $*"
	@$(MAKE) -C $*  > /dev/null

%.clean:
	@echo "  CLEAN   $*"
	@$(MAKE) -C $* distclean > /dev/null
