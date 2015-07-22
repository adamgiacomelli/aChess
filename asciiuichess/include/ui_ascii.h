#ifndef UI_ASCII_H_
#define UI_ASCII_H_

#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include "enums.h"

/*UI*/
bool wait_for_input(short int *move);
void draw_chess_board(c_board board);
void print_message(int which);
char query_for_promotion(void);
/*UI*/

#endif /* UI_ASCII_H_ */
