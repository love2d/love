TARGET = reshax-5million
BINDIR = .
OBJDIR = obj
SRCDIR = src
EXEC = $(BINDIR)/$(TARGET)
OBJ = $(OBJDIR)/main.o

CPP = g++
CFLAGS = -s -O2 -Wall
LDFLAGS =

default: $(EXEC)

$(EXEC): $(OBJ)
	$(CPP) $(CFLAGS) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CPP) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

cleanall: clean
	rm -f $(EXEC)
