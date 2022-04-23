/**
 * mad_mad_access_patterns
 * CS 241 - Spring 2022
 */
#include "tree.h"
#include "utils.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

int binary_search(char*, char*, uint32_t);

int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }
  
  int fd = open(argv[1], O_RDONLY);
  if (fd < 0) {
    openFail(argv[1]);
    exit(1);
  }

  struct stat s;
  fstat(fd, &s);
  void *addr = mmap(NULL, s.st_size, PROT_READ, MAP_SHARED, fd, 0);
  if (addr == MAP_FAILED) {
    mmapFail(argv[1]);
    exit(3);
  }

  if (strncmp((char*)addr, BINTREE_HEADER_STRING, 4)) {
    formatFail(argv[1]);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    char *word = argv[i];
    int s = binary_search((char*)addr, word, BINTREE_ROOT_NODE_OFFSET);
    if (s == 0) printNotFound(word);
  }
  close(fd);
  return 0;
}


int binary_search(char *addr, char *target, uint32_t off) {
  BinaryTreeNode *node = (BinaryTreeNode*) (addr + off);
  int res = strcmp(node->word, target);
  if (res == 0) {
    printFound(node->word, node->count, node->price);
    return 1;
  } else if (res > 0) {
    if (node->left_child) {
      if (binary_search(addr, target, node->left_child)) return 1; 
    }
  } else if (res < 0) {
    if (node->right_child) {
      if (binary_search(addr, target, node->left_child)) return 1;
    }
  }
  return 0;
}