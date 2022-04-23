/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2022
 */
#include "tree.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/
int binary_search(FILE*, char*, uint32_t);

int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }
  
  FILE *fd;
  fd = fopen(argv[1], "r");
  if (!fd) {
    openFail(argv[1]);
    exit(1);
  }

  char header[BINTREE_ROOT_NODE_OFFSET];
  fread(header, BINTREE_ROOT_NODE_OFFSET, 1, fd);
  
  if (strcmp(header, BINTREE_HEADER_STRING)) {
    formatFail(argv[1]);
    exit(1);
  }

  for (int i = 2; i < argc; i++) {
    char *word = argv[i];
    int s = binary_search(fd, word, BINTREE_ROOT_NODE_OFFSET);
    if (s == 0) printNotFound(word);
  }
  fclose(fd);
  return 0;
}

int binary_search(FILE *fd, char *target, uint32_t off) {
  if (off == 0) return 0;
  fseek(fd, off, SEEK_SET);
  BinaryTreeNode *node = calloc(1, sizeof(BinaryTreeNode));
  fread(node, sizeof(BinaryTreeNode), 1, fd);
  fseek(fd, off + sizeof(BinaryTreeNode), SEEK_SET);
  char word[100];
  fread(word, 100, 1, fd);
  int res = strcmp(word, target);
  if (res == 0) {
    printFound(word, node->count, node->price);
    return 1;
  } else if (res > 0) {
    if (node->left_child) {
      if (binary_search(fd, target, node->left_child)) return 1; 
    }
  } else if (res < 0) {
    if (node->right_child) {
      if (binary_search(fd, target, node->right_child)) return 1;
    }
  }
  return 0;
}