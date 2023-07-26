tetrashell: tetrashell.c
	clang-format -i --style=Google tetrashell.c
	gcc $^ -o $@
