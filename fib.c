#include <stdio.h>
#include <stdlib.h>

int fib(int n) {
  if(n<2) return n;
  else return fib(n-1)+fib(n-2);
}

int main(int argc, char *argv[]) {
  if(argc != 2) {
    printf("too many arguments: %d \n", argc);
    exit(1);
  }
  char *input = argv[1]; // Get the input as a string
  int arg = atoi(input); // Convert the string argument to an integer

  int val = fib(arg);
  printf("%d\n",val);
  return 0;
}
