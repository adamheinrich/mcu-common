DIRS := test logger_uart logger_assert

DIRS := $(addprefix examples/,$(DIRS))

.PHONY: all
all: $(DIRS:=.build)

.PHONY: clean
clean: $(DIRS:=.clean)

%.build:
	@echo "  BUILD   $*"
	@$(MAKE) -C $*  > /dev/null

%.clean:
	@echo "  CLEAN   $*"
	@$(MAKE) -C $* distclean > /dev/null
