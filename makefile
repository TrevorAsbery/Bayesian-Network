CC = g++
CFLAGS = -O3
EXEC = NewsgroupClassifier
	
all:
	rm -f $(EXEC)
	$(CC) $(CFLAGS) -o $(EXEC) main.cpp
	./$(EXEC)