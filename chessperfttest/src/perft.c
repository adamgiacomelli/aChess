#include "perft.h"
#include <stdio.h>
#include <ctype.h>


#define LETTER 0
#define NUMBER 1
#define TOTAL_MOVES 4096

static long long unsigned int called_draw_chess_board = 0;
static long long unsigned int called_query_for_promotion = 0;
static long long unsigned int called_wait_for_input = 0;
static long long unsigned int called_print_message = 0;

static long long unsigned int 	opening_game_message = 0,
						white_is_on_the_move = 0, black_is_on_the_move = 0, castling_error = 0,	out_of_range_move = 0,
						missing_source_piece = 0, try_move_opponent = 0, blocked_destination_square = 0, illegal_move = 0,
						unknown_piece_type = 0,	results_in_check_error = 0, check = 0, checkmate = 0,unknown_input = 0,
						game_over = 0, undo_capped = 0,	 stack_underflow = 0, redo_capped = 0, print_error = 0;

static long long unsigned int valid_move_made=0;
static bool changed_to_white = false;
static bool changed_to_black = false;
static int current_turn = 0;
static int compute_to_depth = 0;
static int division_true = false;
static int failed_at_depth = 0;

static long long int valid[50] = {0};
static const long long int tested_values[50]= {20,400,8902,197281,4865609,119060324,3195901860,84998378956,2439530234167,69352859712417,0};

static long long int divide[100] = {0};
static int root_div = 0;

static int move_in_depth[6] = {0,0,0,0,0,0};

static short int states[TOTAL_MOVES];

static void setUndo(short int *move);
static void setQuit(short int *move);
static void print_move(short int state);
bool do_all_moves(short int *move, int turn, int depth,bool divide_node);



/*
 *	do_all_moves: iterates through all moves and in case of a legal move calls itself for the next level.
 *	do_all_moves: When it reaches the final level or finishes the iteration,
 *	do_all_moves: it counts the legal move for the previous depth, resets the move_counter, undo's and resumes the iteration.
 *	do_all_moves:
 *	do_all_moves: if "divide_node" is true, it divides the first level moves and shows how many legal moves result from each first level move;
 */
bool do_all_moves(short int *move, int turn, int depth,bool divide_node)
{
	if(current_turn != turn)
	{
		turn++;
		if(turn != depth)
		{
			return do_all_moves(move, turn, depth, divide_node);
		}
		else
		{
			if(divide_node) divide[root_div]++; //divide functions

			valid[turn-1]++;
			setUndo(move);
			return false;
		}
	}
	else if(move_in_depth[turn] < TOTAL_MOVES)
	{
		*move = states[move_in_depth[turn]];
		move_in_depth[turn]++;

		return true;
	}
	else if(move_in_depth[turn] == TOTAL_MOVES)
	{
		move_in_depth[turn] = 0;
	}

	if(current_turn == 0)
	{
		setQuit(move);
		for(int dep = 0; dep != depth;dep++)
		{
			printf("\nDepth %d:%lld",dep+1,valid[dep]);
			if(valid[dep] != tested_values[dep] && failed_at_depth == 0)
			{
				failed_at_depth = dep + 1;
			}
		}

		if(failed_at_depth != 0)
			printf("\nERROR:Failed at depth %d (should be: %lld)",failed_at_depth, tested_values[failed_at_depth-1]);

		return false;
	}

	if(divide_node && turn == 1) // divide functions
	{
		printf("\n");
		print_move(states[move_in_depth[turn-1]-1]);

		printf(" %lld", (divide[root_div]));
		root_div++;
	}
		valid[turn-1]++;
		setUndo(move);
		return false;

}

void print_move(short int state)
{
	printf("%hx", state);
}

bool wait_for_input(short int *move)
{
	//printf("there was");
	//print_results_of_test(0);
	called_wait_for_input++;

	/*for debuging later nodes*/

	static bool first = false;
	static bool second = false;
	static bool third = false;
	static bool fourth = false;
	if(first)
	{
	*move = 0x6365;
	first = false;
	current_turn--;
	return true;
	}

	if(second)
	{
	*move = 0x6654;
	second = false;
	current_turn--;
	return true;
	}

	if(third)
	{
	*move = 0x6116;
	third = false;
	current_turn--;
	return true;
	}

	if(fourth)
	{
	*move = 0x1111;
	fourth = false;
	current_turn--;
	return true;
	}

	/*dbug*/

	return do_all_moves(move,0,compute_to_depth,division_true);

	printf("Exited from recursive function, error");
	return false;
}

void setUndo(short int *move)
{
	*move = 0x00DF;
}

void setQuit(short int *move)
{
	*move = 0x00FF;
}

void setDivideParameters(bool divide_true, int depth_of_calculation)
{
	compute_to_depth = depth_of_calculation;
	division_true = divide_true;
}
void draw_chess_board(c_board board)
{
	called_draw_chess_board++;
}

char query_for_promotion(void)
{
	called_query_for_promotion++;

	valid[current_turn] += 3;
	return 'Q';
}

void print_message(int which)
{
	called_print_message++;

	switch(which)
	{
	case 0: opening_game_message++;break;
	case 1: white_is_on_the_move++;break;
	case 2: black_is_on_the_move++;break;
	case 3: castling_error++;break;
	case 4: out_of_range_move++;break;
	case 5: missing_source_piece++;break;
	case 6: try_move_opponent++;break;
	case 7: blocked_destination_square++;break;
	case 8: illegal_move++; break;
	case 9: unknown_piece_type++; break;
	case 10: results_in_check_error++; break;
	case 11: if(current_turn == compute_to_depth-1)check++; break;
	case 12: checkmate++;break;
	case 13: unknown_input++;break;
	case 14: game_over++;break;
	case 15: undo_capped++;break;
	case 16: stack_underflow++;break;
	case 17: redo_capped++;break;
	case 18: valid_move_made++;break;
	case 19: changed_to_white = true; current_turn++; break;
	case 20: changed_to_black = true; current_turn++; break;
	case 21: current_turn--;break;
	default: print_error++;break;
	}
}

void generate_moves()
{
	chess_position position_list[64];
	position_list[0]= A8; position_list[1]= B8; position_list[2]= C8; position_list[3]= D8;	position_list[4]= E8; position_list[5]= F8; position_list[6]= G8; position_list[7]= H8;
	position_list[8]= A7; position_list[9]= B7; position_list[10]= C7; position_list[11]= D7;	position_list[12]= E7; position_list[13]= F7; position_list[14]= G7; position_list[15]= H7;
	position_list[16]= A6; position_list[17]= B6; position_list[18]= C6; position_list[19]= D6;	position_list[20]= E6; position_list[21]= F6; position_list[22]= G6; position_list[23]= H6;
	position_list[24]= A5; position_list[25]= B5; position_list[26]= C5; position_list[27]= D5;	position_list[28]= E5; position_list[29]= F5; position_list[30]= G5; position_list[31]= H5;
	position_list[32]= A4; position_list[33]= B4; position_list[34]= C4; position_list[35]= D4;	position_list[36]= E4; position_list[37]= F4; position_list[38]= G4; position_list[39]= H4;
	position_list[40]= A3; position_list[41]= B3; position_list[42]= C3; position_list[43]= D3;	position_list[44]= E3; position_list[45]= F3; position_list[46]= G3; position_list[47]= H3;
	position_list[48]= A2; position_list[49]= B2; position_list[50]= C2; position_list[51]= D2;	position_list[52]= E2; position_list[53]= F2; position_list[54]= G2; position_list[55]= H2;
	position_list[56]= A1; position_list[57]= B1; position_list[58]= C1; position_list[59]= D1;	position_list[60]= E1; position_list[61]= F1; position_list[62]= G1; position_list[63]= H1;

	for(int i = 0; i < 64;i++)
	{
		for(int j = 0; j < 64;j++)
		{
			states[i*64+j] = 0;
			states[i*64+j] = (position_list[j] << 8);
			states[i*64+j] |= position_list[i];
		}
	}
}

void print_results_of_test(int type)
{
	switch(type)
	{
	case 0:	printf("\n###turn##\ncalled_draw_chess_board ##%lld##\nlong unsigned int called_query_for_promotion ##%lld##\nlong unsigned int called_wait_for_input ##%lld##\ncalled_print_message ##%lld##\
				\nopening_game_message ##%lld##\nwhite_is_on_the_move ##%lld##\nblack_is_on_the_move ##%lld##\ncastling_error ##%lld##\nout_of_range_move ##%lld## \
			\nmissing_source_piece ##%lld##\ntry_move_opponent ##%lld##\nblocked_destination_square ##%lld##\nillegal_move ##%lld## \
			\nunknown_piece_type ##%lld##\nresults_in_check_error ##%lld##\ncheck ##%lld##\ncheckmate ##%lld##\nunknown_input ##%lld##\
			\ngame_over ##%lld##\nundo_capped ##%lld##\nstack_underflow ##%lld##\nredo_capped ##%lld##\nprint_error ##%lld##\n"
						, called_draw_chess_board, called_query_for_promotion, called_wait_for_input, called_print_message,
						opening_game_message, white_is_on_the_move, black_is_on_the_move, castling_error,	out_of_range_move,
						missing_source_piece, try_move_opponent, blocked_destination_square, illegal_move,
						unknown_piece_type,	results_in_check_error, check, checkmate, unknown_input,
						game_over, undo_capped,stack_underflow, redo_capped, print_error );
	break;
	case 1:printf("\nValid moves:%lld", valid_move_made);
		   printf("\nPromotions:%lld", called_query_for_promotion*4);
		   printf("\nChecks:%lld", check + checkmate);
		   printf("\nMates:%lld\n", checkmate);break;
	default: printf("\n#$#unknown test print call#$#\n");
	}
}
