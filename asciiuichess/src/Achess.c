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

#include "logic.h"
#include "ui_ascii.h"

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define LETTER 0
#define NUMBER 1


int main()
{

	struct chess_ui ui_ascii;

	ui_ascii.wait_for_input = wait_for_input;
	ui_ascii.draw_chess_board = draw_chess_board;
	ui_ascii.print_message = print_message;
	ui_ascii.query_for_promotion = query_for_promotion;

	attach_ui(&ui_ascii);

	game();

	return 0;
}
