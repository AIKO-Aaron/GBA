mkdir libs
cd libs

SDL2_BASE="SDL2-2.0.10"
SDL2_TTF="SDL2_ttf-2.0.15"

PREFIX_PATH="$(pwd)"

echo "Downloading required Libraries..."

wget "https://www.libsdl.org/release/$SDL2_BASE.tar.gz" # SDL2
wget "https://www.libsdl.org/projects/SDL_ttf/release/$SDL2_TTF.tar.gz" # SDL2_ttf

echo "Unpacking required Libraries..."
tar -xf "$SDL2_BASE.tar.gz"
tar -xf "$SDL2_TTF.tar.gz"

cd "$SDL2_BASE"
./configure --prefix="$PREFIX_PATH"
make
make install
cd ..
rm -f "$SDL2_BASE.tar.gz"
rm -rf "$SDL2_BASE/"

cd "$SDL2_TTF"
./configure --prefix="$PREFIX_PATH"
make
make install
cd ..
rm -f "$SDL2_TTF.tar.gz"
rm -rf "$SDL2_TTF/"

cd ..
