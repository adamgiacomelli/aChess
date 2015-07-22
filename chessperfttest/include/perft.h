#ifndef PERFT_H_
#define PERFT_H_

#include <stdbool.h>
#include "enums.h"

/*UI*/
bool wait_for_input(short int *move);
void draw_chess_board(c_board board);
void print_message(int which);
char query_for_promotion(void);
/*UI*/
void print_results_of_test(int type);
void generate_moves();
void setDivideParameters(bool divide_true, int depth_of_calculation);

#endif /* PERFT_H_ */
