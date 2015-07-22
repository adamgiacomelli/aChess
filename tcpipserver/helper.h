/*

  HELPER.H
  ========
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net

  Interface to socket helper functions. 

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/


#ifndef PG_SOCK_HELP
#define PG_SOCK_HELP

#include <unistd.h>             /*  for ssize_t data type  */
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "enums.h"
#include "logic.h"

#define LISTENQ        (1024)   /*  Backlog for listen()   */

/*  Function declarations  */

ssize_t Readline(int fd, void *vptr, size_t maxlen);
ssize_t Writeline(int fc, const void *vptr, size_t maxlen);
bool initGame(int conn);

/*UI*/
bool wait_for_input(short int *move);
void draw_chess_board(c_board board);
void print_message(int which);
char query_for_promotion(void);
/*UI*/
#endif  /*  PG_SOCK_HELP  */

