TITLE_COLOR = \033[33m
NO_COLOR = \033[0m

# when executing make, compile all exe's
all: sensor_gateway sensor_node file_creator
zip:
	zip lab_final.zip main.c main.h connmgr.c connmgr.h datamgr.c datamgr.h errors.h sbuffer.c sbuffer.h sensor_db.c sensor_db.h config.h lib/dplist.c lib/dplist.h lib/tcpsock.c lib/tcpsock.h

# When trying to compile one of the executables, first look for its .c files
# Then check if the libraries are in the lib folder
sensor_gateway : main.c connmgr.c datamgr.c sensor_db.c sbuffer.c lib/libdplist.so lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** CPPCHECK *****$(NO_COLOR)"
	cppcheck --enable=all --suppress=missingIncludeSystem main.c connmgr.c datamgr.c sensor_db.c sbuffer.c
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_gateway *****$(NO_COLOR)"
	gcc -c main.c      -Wall -std=c11 -Werror -DDEBUG -DSET_MIN_TEMP=19.9 -DSET_MAX_TEMP=20.1 -DTIMEOUT=5 -o main.o      -fdiagnostics-color=auto
	gcc -c connmgr.c   -Wall -std=c11 -Werror -DDEBUG -DSET_MIN_TEMP=19.9 -DSET_MAX_TEMP=20.1 -DTIMEOUT=5 -o connmgr.o   -fdiagnostics-color=auto
	gcc -c datamgr.c   -Wall -std=c11 -Werror -DDEBUG -DSET_MIN_TEMP=19.9 -DSET_MAX_TEMP=20.1 -DTIMEOUT=5 -o datamgr.o   -fdiagnostics-color=auto
	gcc -c sensor_db.c -Wall -std=c11 -Werror -DDEBUG -DSET_MIN_TEMP=19.9 -DSET_MAX_TEMP=20.1 -DTIMEOUT=5 -o sensor_db.o -fdiagnostics-color=auto
	gcc -c sbuffer.c   -Wall -std=c11 -Werror -DDEBUG -DSET_MIN_TEMP=19.9 -DSET_MAX_TEMP=20.1 -DTIMEOUT=5 -o sbuffer.o   -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_gateway *****$(NO_COLOR)"
	gcc -fcommon -g main.o connmgr.o datamgr.o sensor_db.o sbuffer.o -ldplist -ltcpsock -lpthread -o sensor_gateway -Wall -L./lib -Wl,-rpath=./lib  -ltcpsock -ldplist -lpthread -lsqlite3 -fdiagnostics-color=auto

sensor_node : sensor_node.c lib/libtcpsock.so
	@echo "$(TITLE_COLOR)\n***** COMPILING sensor_node *****$(NO_COLOR)"
	gcc -c sensor_node.c  -Wall -std=gnu99  -Werror -o sensor_node.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING sensor_node *****$(NO_COLOR)"
	gcc sensor_node.o -ltcpsock -o sensor_node -Wall -L./lib -Wl,-rpath=./lib  -ltcpsock -ldplist -lpthread -lsqlite3 -fdiagnostics-color=auto

file_creator : file_creator.c
	@echo "$(TITLE_COLOR)\n***** COMPILE & LINKING file_creator *****$(NO_COLOR)"
	gcc file_creator.c -o file_creator -Wall -fdiagnostics-color=auto

# If you only want to compile one of the libs, this target will match (e.g. make liblist)
library : libdplist libsbuffer libtcpsock
libdplist : lib/libdplist.so
libsbuffer : lib/libsbuffer.so
libtcpsock : lib/libtcpsock.so

lib/libdplist.so : lib/dplist.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB dplist *****$(NO_COLOR)"
	gcc -c lib/dplist.c -Wall -std=c11 -Werror -fPIC -o lib/dplist.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB dplist< *****$(NO_COLOR)"
	gcc lib/dplist.o -o lib/libdplist.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libsbuffer.so : lib/sbuffer.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB sbuffer *****$(NO_COLOR)"
	gcc -c lib/sbuffer.c -Wall -std=c11 -Werror -fPIC -o lib/sbuffer.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB sbuffer *****$(NO_COLOR)"
	gcc lib/sbuffer.o -o lib/libsbuffer.so -Wall -shared -lm -fdiagnostics-color=auto

lib/libtcpsock.so : lib/tcpsock.c
	@echo "$(TITLE_COLOR)\n***** COMPILING LIB tcpsock *****$(NO_COLOR)"
	gcc -c lib/tcpsock.c -Wall -std=c11 -Werror -fPIC -o lib/tcpsock.o -fdiagnostics-color=auto
	@echo "$(TITLE_COLOR)\n***** LINKING LIB tcpsock *****$(NO_COLOR)"
	gcc lib/tcpsock.o -o lib/libtcpsock.so -Wall -shared -lm -fdiagnostics-color=auto

# do not look for files called clean, clean-all or this will be always a target
.PHONY : clean clean-all

clean:
	rm -rf *.o sensor_gateway sensor_node file_creator *~

clean-all: clean
	rm -rf lib/*.so
	rm -rf *.PID
remove:
	rm sensor_gateway
	rm sensor_node
	rm build/val
remove_generated_files:
	rm node*
	rm DS_NAME
	rm gateway.log
	rm sensor_data_recv*
	rm logFifo
add:
	make sensor_gateway
	make sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway

list:
	gcc -c -o dplist.o lib/dplist.c
	gcc -c -o dplist_h.o lib/dplist.h
	ar rs lib/lib_arch_dplist dplist.o dplist_h.o
	ar rs lib_arch_dplist dplist.o dplist_h.o
	gcc -c -o tcpsock.o lib/tcpsock.c
	gcc -c -o tcpsock_h.o lib/tcpsock.h
	ar rs lib/lib_arch_tcpsock tcpsock.o tcpsock_h.o
	ar rs lib_arch_tcpsock tcpsock.o tcpsock_h.o
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway

scalability : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./sensor_gateway 1234		#Start the server
	sleep 2
	-- ./sensor_node 15 1000z 127.0.0.1 1234 & echo $$! > node1.PID
	-- ./sensor_node 21 2000 127.0.0.1 1234 & echo $$! > node2.PID
	sleep 2
	-- ./sensor_node 37 3000 127.0.0.1 1234 & echo $$! > node3.PID
	-- ./sensor_node 49 4000 127.0.0.1 1234 & echo $$! > node4.PID
	-- ./sensor_node 112 6000 127.0.0.1 1234 & echo $$! > node5.PID
	-- ./sensor_node 28 7000 127.0.0.1 1234 & echo $$! > node6.PID
	-- ./sensor_node 129 3000 127.0.0.1 1234 & echo $$! > node7.PID
	@echo "All nodes are running"
	sleep 3600
	if [ -e node7.PID ]; then \
	kill -TERM $$(cat node7.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node6.PID) || true; \
	fi;
	if [ -e node5.PID ]; then \
	kill -TERM $$(cat node5.PID) || true; \
	fi;
	sleep 5
	if [ -e node4.PID ]; then \
	kill -TERM $$(cat node4.PID) || true; \
	fi;
	if [ -e node3.PID ]; then \
	kill -TERM $$(cat node3.PID) || true; \
	fi;
	sleep 5
	if [ -e node2.PID ]; then \
	kill -TERM $$(cat node2.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node1.PID) || true; \
	fi;

durability : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./sensor_gateway 3756		#Start the server
	sleep 2
	-- ./sensor_node 15 1000 127.0.0.1 3756 & echo $$! > node1.PID
	-- ./sensor_node 21 2000 127.0.0.1 3756 & echo $$! > node2.PID
	sleep 2
	-- ./sensor_node 37 3000 127.0.0.1 3756 & echo $$! > node3.PID
	-- ./sensor_node 49 4000 127.0.0.1 3756 & echo $$! > node4.PID
	-- ./sensor_node 112 8000 127.0.0.1 3756 & echo $$! > node5.PID
	-- ./sensor_node 28 6000 127.0.0.1 3756 & echo $$! > node6.PID
	-- ./sensor_node 129 3000 127.0.0.1 3756 & echo $$! > node7.PID
	@echo "All nodes are running"
	sleep 3600
	if [ -e node7.PID ]; then \
	kill -TERM $$(cat node7.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node6.PID) || true; \
	fi;
	if [ -e node5.PID ]; then \
	kill -TERM $$(cat node5.PID) || true; \
	fi;
	sleep 5
	if [ -e node4.PID ]; then \
	kill -TERM $$(cat node4.PID) || true; \
	fi;
	if [ -e node3.PID ]; then \
	kill -TERM $$(cat node3.PID) || true; \
	fi;
	sleep 5
	if [ -e node2.PID ]; then \
	kill -TERM $$(cat node2.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node1.PID) || true; \
	fi;

stress : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./sensor_gateway 1234		#Start the server
	sleep 2
	-- ./sensor_node 15 10 127.0.0.1 1234 & echo $$! > node1.PID
	-- ./sensor_node 21 25 127.0.0.1 1234 & echo $$! > node2.PID
	sleep 1
	-- ./sensor_node 37 45 127.0.0.1 1234 & echo $$! > node3.PID
	-- ./sensor_node 49 55 127.0.0.1 1234 & echo $$! > node4.PID
	-- ./sensor_node 112 1300 127.0.0.1 1234 & echo $$! > node5.PID
	-- ./sensor_node 28 1200 127.0.0.1 1234 & echo $$! > node6.PID
	-- ./sensor_node 129 75 127.0.0.1 1234 & echo $$! > node7.PID
	@echo "All nodes are running"
	sleep 5
	if [ -e node7.PID ]; then \
	kill -TERM $$(cat node7.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node6.PID) || true; \
	fi;
	if [ -e node5.PID ]; then \
	kill -TERM $$(cat node5.PID) || true; \
	fi;
	sleep 1
	if [ -e node4.PID ]; then \
	kill -TERM $$(cat node4.PID) || true; \
	fi;
	if [ -e node3.PID ]; then \
	kill -TERM $$(cat node3.PID) || true; \
	fi;
	sleep 1
	if [ -e node2.PID ]; then \
	kill -TERM $$(cat node2.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node1.PID) || true; \
	fi;

run : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o sensor_gateway
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./sensor_gateway 3756		#Start the server
	sleep 2
	-- ./sensor_node 15 1 127.0.0.1 3756 &	echo $$! > node1.PID
	-- ./sensor_node 21 2 127.0.0.1 3756 &	echo $$! > node2.PID
	-- ./sensor_node 28 1 127.0.0.1 3756 &	echo $$! > node3.PID
	-- ./sensor_node 37 7 127.0.0.1 3756 &	echo $$! > node4.PID
	-- ./sensor_node 49 1 127.0.0.1 3756 &	echo $$! > node5.PID
	-- ./sensor_node 112 2 127.0.0.1 3756 &	echo $$! > node6.PID
	-- ./sensor_node 129 3 127.0.0.1 3756 &	echo $$! > node7.PID
	-- ./sensor_node 132 8 127.0.0.1 3756 &	echo $$! > node8.PID
	-- ./sensor_node 142 4 127.0.0.1 3756 &	echo $$! > node9.PID
	-- ./sensor_node 145 3 127.0.0.1 3756 &	echo $$! > node10.PID
	-- ./sensor_node 148 4 127.0.0.1 3756 &	echo $$! > node11.PID
	-- ./sensor_node 151 2 127.0.0.1 3756 &	echo $$! > node12.PID
	-- ./sensor_node 154 1 127.0.0.1 3756 &	echo $$! > node13.PID
	-- ./sensor_node 157 6 127.0.0.1 3756 &	echo $$! > node14.PID
	-- ./sensor_node 160 7 127.0.0.1 3756 &	echo $$! > node15.PID
	@echo "Started the sensors"
	sleep 30
	pkill -f -e -c sensor_node

run_hardcore : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./build/val 3756		#Start the server
	sleep 2
	-- ./sensor_node 15 1 127.0.0.1 3756 & echo $$! > node1.PID
	-- ./sensor_node 21 1 127.0.0.1 3756 &	echo $$! > node2.PID
	-- ./sensor_node 28 1 127.0.0.1 3756 &	echo $$! > node3.PID
	-- ./sensor_node 37 1 127.0.0.1 3756 &	echo $$! > node4.PID
	-- ./sensor_node 49 1 127.0.0.1 3756 &	echo $$! > node5.PID
	-- ./sensor_node 112 1 127.0.0.1 3756 &	echo $$! > node6.PID
	-- ./sensor_node 129 1 127.0.0.1 3756 &	echo $$! > node7.PID
	-- ./sensor_node 132 1 127.0.0.1 3756 &	echo $$! > node8.PID
	-- ./sensor_node 142 1 127.0.0.1 3756 &	echo $$! > node9.PID
	-- ./sensor_node 145 1 127.0.0.1 3756 &	echo $$! > node10.PID
	-- ./sensor_node 148 1 127.0.0.1 3756 &	echo $$! > node11.PID
	-- ./sensor_node 151 1 127.0.0.1 3756 &	echo $$! > node12.PID
	-- ./sensor_node 154 1 127.0.0.1 3756 &	echo $$! > node13.PID
	-- ./sensor_node 157 1 127.0.0.1 3756 &	echo $$! > node14.PID
	-- ./sensor_node 160 1 127.0.0.1 3756 &	echo $$! > node15.PID
	sleep 2
	-- ./sensor_node 163 1 127.0.0.1 3756 &	echo $$! > node16.PID
	-- ./sensor_node 166 1 127.0.0.1 3756 &	echo $$! > node17.PID
	-- ./sensor_node 169 1 127.0.0.1 3756 &	echo $$! > node18.PID
	-- ./sensor_node 172 1 127.0.0.1 3756 &	echo $$! > node19.PID
	-- ./sensor_node 175 1 127.0.0.1 3756 &	echo $$! > node20.PID
	-- ./sensor_node 178 1 127.0.0.1 3756 &	echo $$! > node21.PID
	-- ./sensor_node 181 1 127.0.0.1 3756 &	echo $$! > node22.PID
	-- ./sensor_node 184 1 127.0.0.1 3756 &	echo $$! > node23.PID
	-- ./sensor_node 187 1 127.0.0.1 3756 &	echo $$! > node24.PID
	-- ./sensor_node 190 1 127.0.0.1 3756 &	echo $$! > node25.PID
	-- ./sensor_node 193 1 127.0.0.1 3756 &	echo $$! > node26.PID
	-- ./sensor_node 196 1 127.0.0.1 3756 &	echo $$! > node27.PID
	-- ./sensor_node 199 1 127.0.0.1 3756 &	echo $$! > node28.PID
	-- ./sensor_node 202 1 127.0.0.1 3756 &	echo $$! > node29.PID
	-- ./sensor_node 205 1 127.0.0.1 3756 &	echo $$! > node30.PID
	@echo "Started the sensors"
	sleep 30
	pkill -f -e -c sensor_node

run_hardcore_2 : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./build/val 3756		#Start the server
	sleep 3
	-- ./sensor_node 15 1 127.0.0.1 3756 & echo $$! > node1.PID
	-- ./sensor_node 21 1 127.0.0.1 3756 &	echo $$! > node2.PID
	-- ./sensor_node 28 1 127.0.0.1 3756 &	echo $$! > node3.PID
	-- ./sensor_node 37 1 127.0.0.1 3756 &	echo $$! > node4.PID
	-- ./sensor_node 49 1 127.0.0.1 3756 &	echo $$! > node5.PID
	-- ./sensor_node 112 1 127.0.0.1 3756 &	echo $$! > node6.PID
	-- ./sensor_node 129 1 127.0.0.1 3756 &	echo $$! > node7.PID
	-- ./sensor_node 132 1 127.0.0.1 3756 &	echo $$! > node8.PID
	-- ./sensor_node 142 1 127.0.0.1 3756 &	echo $$! > node9.PID
	-- ./sensor_node 145 1 127.0.0.1 3756 &	echo $$! > node10.PID
	-- ./sensor_node 148 1 127.0.0.1 3756 &	echo $$! > node11.PID
	-- ./sensor_node 151 1 127.0.0.1 3756 &	echo $$! > node12.PID
	-- ./sensor_node 154 1 127.0.0.1 3756 &	echo $$! > node13.PID
	-- ./sensor_node 157 1 127.0.0.1 3756 &	echo $$! > node14.PID
	-- ./sensor_node 160 1 127.0.0.1 3756 &	echo $$! > node15.PID
	sleep 2
	-- ./sensor_node 163 1 127.0.0.1 3756 &	echo $$! > node16.PID
	-- ./sensor_node 166 1 127.0.0.1 3756 &	echo $$! > node17.PID
	-- ./sensor_node 169 1 127.0.0.1 3756 &	echo $$! > node18.PID
	-- ./sensor_node 172 1 127.0.0.1 3756 &	echo $$! > node19.PID
	-- ./sensor_node 175 1 127.0.0.1 3756 &	echo $$! > node20.PID
	-- ./sensor_node 178 1 127.0.0.1 3756 &	echo $$! > node21.PID
	-- ./sensor_node 181 1 127.0.0.1 3756 &	echo $$! > node22.PID
	-- ./sensor_node 184 1 127.0.0.1 3756 &	echo $$! > node23.PID
	-- ./sensor_node 187 1 127.0.0.1 3756 &	echo $$! > node24.PID
	-- ./sensor_node 190 1 127.0.0.1 3756 &	echo $$! > node25.PID
	-- ./sensor_node 193 1 127.0.0.1 3756 &	echo $$! > node26.PID
	-- ./sensor_node 196 1 127.0.0.1 3756 &	echo $$! > node27.PID
	-- ./sensor_node 199 1 127.0.0.1 3756 &	echo $$! > node28.PID
	-- ./sensor_node 202 1 127.0.0.1 3756 &	echo $$! > node29.PID
	-- ./sensor_node 205 1 127.0.0.1 3756 &	echo $$! > node30.PID
	@echo "Started the sensors"
	sleep 30
	pkill -f -e -c sensor_node
valgrind : sensor_gateway sensor_node
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./build/val 1234		#Start the server
	sleep 2
	-- ./sensor_node 15 4000 127.0.0.1 1234 & echo $$! > node1.PID
	-- ./sensor_node 21 4500 127.0.0.1 1234 & echo $$! > node2.PID
	sleep 2
	-- ./sensor_node 37 3000 127.0.0.1 1234 & echo $$! > node3.PID
	-- ./sensor_node 49 4000 127.0.0.1 1234 & echo $$! > node4.PID
	-- ./sensor_node 112 8000 127.0.0.1 1234 & echo $$! > node5.PID
	-- ./sensor_node 28 6000 127.0.0.1 1234 & echo $$! > node6.PID
	-- ./sensor_node 129 3000 127.0.0.1 1234 & echo $$! > node7.PID
	@echo "All nodes are running"
	sleep 25
	if [ -e node2.PID ]; then \
	kill -TERM $$(cat node2.PID) || true; \
	fi;
	if [ -e node6.PID ]; then \
	kill -TERM $$(cat node1.PID) || true; \
	fi;

valgrind_gnome:
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./build/val 1234		#Start the server
	sleep 2
	-- ./sensor_node 15 1000 127.0.0.1 1234 & echo $$! > node1.PID
	-- ./sensor_node 21 2000 127.0.0.1 1234 & echo $$! > node2.PID
	sleep 2
	-- ./sensor_node 37 3000 127.0.0.1 1234 & echo $$! > node3.PID
	-- ./sensor_node 49 4000 127.0.0.1 1234 & echo $$! > node4.PID
	-- ./sensor_node 112 6000 127.0.0.1 1234 & echo $$! > node5.PID
	-- ./sensor_node 28 7000 127.0.0.1 1234 & echo $$! > node6.PID
	-- ./sensor_node 129 3000 127.0.0.1 1234 & echo $$! > node7.PID
	@echo "Started the sensors"
	sleep 30
	pkill -f -e -c sensor_node

valgrind_standart:
	mkdir -p build
	gcc -fcommon -g main.c sbuffer.c datamgr.c connmgr.c sensor_db.c -L./lib lib_arch_dplist -L./lib lib_arch_tcpsock -ltcpsock -ldplist -lpthread -lsqlite3 $(VLAGGN) -DTIMEOUT=5 -DSET_MAX_TEMP=90 -DSET_MIN_TEMP=89 -o build/val
	gnome-terminal -- valgrind --tool=memcheck --leak-check=yes --track-origins=yes -s --show-leak-kinds=all --show-reachable=yes --num-callers=20 --track-fds=yes --show-error-list=yes ./build/val 1234		#Start the server
	sleep 2
	-- ./sensor_node 15 1000 127.0.0.1 1234 & echo $$! > node1.PID
	-- ./sensor_node 21 2000 127.0.0.1 1234 & echo $$! > node2.PID
	sleep 2
	-- ./sensor_node 37 3000 127.0.0.1 1234 & echo $$! > node3.PID
	-- ./sensor_node 49 4000 127.0.0.1 1234 & echo $$! > node4.PID
	-- ./sensor_node 112 6000 127.0.0.1 1234 & echo $$! > node5.PID
	-- ./sensor_node 28 7000 127.0.0.1 1234 & echo $$! > node6.PID
	-- ./sensor_node 129 3000 127.0.0.1 1234 & echo $$! > node7.PID
	@echo "Started the sensors"
	sleep 30
	pkill -f -e -c sensor_node
