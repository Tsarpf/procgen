cppfile="$1"
outfile="${cppfile%.cpp}.out"
cpppath="$cppfile"
g++ -Wall -Wextra -Wshadow -fsanitize=undefined -std=c++11 $cpppath -o $outfile -O3