/********************************
 * Achess.c						*
 *								*
 *  Created on: Jan 18, 2011	*
 *      Author: adam			*
 *******************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include "logic.h"
#include "perft.h"



clock_t start,finish;

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define LETTER 0
#define NUMBER 1


int main(int argc, char *argv[])
{


	int depth_argument;
	bool divide_valid_argument;

	if(argc == 3)
	{
		depth_argument = (int)(*argv[1]) - 48;
		if(*argv[2]-48 > 0)
		{
		divide_valid_argument = true;
		printf("Depth = %d, divide on\n",depth_argument);
		}
		else
		{
			divide_valid_argument = false;
			printf("Depth = %d, divide off\n",depth_argument);
		}
	}
	else if(argc == 2)
	{
		depth_argument = (int)(*argv[1]) - 48;
		divide_valid_argument = 0;
		printf("Depth = %d, divide off\n",depth_argument);
	}
	else
	{
		printf("Default depth = 3, divide off\n");
		depth_argument = 3;
		divide_valid_argument = 0;
	}

	setDivideParameters(divide_valid_argument,depth_argument);

	generate_moves();

	struct chess_ui ui_ascii;

	ui_ascii.wait_for_input = wait_for_input;
	ui_ascii.draw_chess_board = draw_chess_board;
	ui_ascii.print_message = print_message;
	ui_ascii.query_for_promotion = query_for_promotion;

	attach_ui(&ui_ascii);

	start = clock();

	game();

	finish = clock();

	print_results_of_test(1);
	printf("Runtime: %ldcsec\n",((finish-start)*100)/CLOCKS_PER_SEC);

	return 0;
}
