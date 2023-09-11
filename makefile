OBJ = main.o ui.o callbacks.o helpers.o
CC = tcc
IUPINCLUDE = -IC:/tcc/iup/include/
IUPLIB = -LC:/tcc/iup/

atomo.exe: $(OBJ) librarian.exe
	$(CC) -o atomo.exe -Wall -Wl,-subsystem=windows $(OBJ) $(IUPINCLUDE) $(IUPLIB) -liup

main.o: main.c
	$(CC) -c main.c $(IUPINCLUDE)

ui.o: ui.c
	$(CC) -c ui.c $(IUPINCLUDE)

callbacks.o: callbacks.c
	$(CC) -c callbacks.c $(IUPINCLUDE)

helpers.o: helpers.c
	$(CC) -c helpers.c $(IUPINCLUDE)

librarian.exe: librarian/librarian.go
	cd librarian; go build; mv librarian.exe ..

clean:
	rm *.o *.exe *~
