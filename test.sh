echo 'building project'
gcc src/test.c src/vector.c src/keymap.c src/assert.c -o test
./test
rm test
echo 'removing test buld files'