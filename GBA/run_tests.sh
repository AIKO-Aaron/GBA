cd ..
rm bin/gba
make bin/gba
cd GBA
LD_LIBRARY_PATH='../bin/:../libs/lib' ../bin/gba
