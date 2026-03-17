#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "utils.h"
#include "init.h"
#define COMMAND_OK 0
#define COMMAND_NOT_OK 1

int cmd_hash_object(int argc, char *argv[]){
  if(argc < 3){
    printf("Uso: irongit hash-object <file>\n");
    return COMMAND_NOT_OK;
  }
  char *hash_value = NULL;
  char *file_name = argv[2];
  if (file_exists(file_name)) {
    hash_object(file_name, &hash_value);
    printf("%s\n", hash_value); // Final hash
    free(hash_value); 
  } else {
    fprintf(stderr, "Error: The file %s does not exist\n", argv[2]);
  }
  
  return COMMAND_OK;
}

int cmd_init(int argc, char *argv[]){
  if(init(".") == COMMAND_OK){
    printf("Initialized empty Git repository.\n");
  }else{
    printf("Reinitialized empty Git repository.\n");
  }
  return COMMAND_OK;
}
