cp system.c /usr/src/minix/kernel/system.c;
cd /usr/src;
make build MKUPDATE=yes > log.txt 2> log.txt
