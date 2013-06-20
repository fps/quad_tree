.PHONY: all

all: test_quad_tree

test_quad_tree: test_quad_tree.cc quad_tree/quad_tree.h
	g++ -g -O0 -Wall -Werror -I . test_quad_tree.cc -o test_quad_tree
