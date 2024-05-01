# GNU Makefile to build Kpass:

PACKAGE  :=  Kpass
VERSION  :=  1.0
ELFNAME  :=  kpass
TARGET   :=  $(ELFNAME).out

# Build options
CC       ?=  gcc
CFLAGS   := -g -MMD -Wall -Wno-unused-function #-Werror #-Wextra
CPPFLAGS :=
INCLUDE  :=
LDLIBS   += -pthread -lcrypto -lsqlite3

SRCDIR   :=  src
OBJDIR   :=  obj
DISTDIR  :=  dist
PREFIX   ?=  /usr/local

RM       :=  rm -f
BOLD     :=  $(shell tput bold)
NC       :=  $(shell tput sgr0)

# Source files
SRCS :=  $(SRCDIR)/main.c $(SRCDIR)/db.c $(SRCDIR)/entry.c $(SRCDIR)/import.c $(SRCDIR)/kpass.c $(SRCDIR)/utils.c $(SRCDIR)/vendor.c

# Build object files
OBJS  = $(SRCS:%.c=$(OBJDIR)/%.o)

# Build dependencies
DEPS  = $(OBJS:%.o=%.d)

# Makefile execution starts here
all: info $(TARGET)

info:
	@echo "\n$(BOLD)Building $(PACKAGE) v$(VERSION) $(DISTNAME) release:$(NC)\n"
	@echo "TARGET   =  $(TARGET)"
	@echo "COMPILER =  $(CC)"
	@echo "CFLAGS   = $(CFLAGS)"
	@echo "LDLIBS   = $(LDLIBS)"

# Link object files
$(TARGET): $(OBJS)
	@echo  "\n\nLinking   ... object-files"
	@$(CC) -o $(TARGET) $(OBJS) $(LDFLAGS) $(LDLIBS)
	@echo "$(BOLD)$(PACKAGE)$(NC) build completed [$(BOLD)$(TARGET)$(NC)]"

# Compile source files
$(OBJDIR)/%.o : %.c
	@mkdir -p $(@D)
	@printf "\nCompiling ... $<"
	@$(CC) $(CFLAGS) $(CPPFLAGS) $(INCLUDE) -c $< -o $@
	@printf "\33[2K\rCompiled  ... $<"

clean:
	$(RM) -r $(OBJDIR) $(TARGET)

# Include dependencies
-include $(DEPS)

# EOF
