.PHONY: all

all: test_quad_tree

test_quad_tree: test_quad_tree.cc
	g++ -Wall -Werror -I . test_quad_tree.cc -o test_quad_tree
