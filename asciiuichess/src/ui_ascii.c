#include "ui_ascii.h"
#include <string.h>

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define LETTER 0
#define NUMBER 1

const char Letters[8]= {'a','b','c','d','e','f','g','h'};
const char Numbers[8]= {'1','2','3','4','5','6','7','8'};

static char which_chess_piece(int index,c_board board);

/************************************************************************************
 * draw_chess_board: nariše ASCII šahovnico glede na to, kaj je v board strukturi	*
 * 																					*
 ***********************************************************************************/
void draw_chess_board(c_board board)
{
	printf("\n   ");
	for(int k = 0;k<BOARD_WIDTH;k++) printf(" %c ", (char)k+97); //napiše črke


	for(int i = 0; i < BOARD_WIDTH; i++)
	{
		printf("\n %d ", BOARD_HEIGHT - i);

		for(int j = 0; j < BOARD_HEIGHT; j++)
		{
			printf("[%c]", which_chess_piece((i*8 + j),board));
		}

		printf(" %d",BOARD_HEIGHT - i);
	}

	printf("\n   ");
	for(int k = 0;k<BOARD_WIDTH;k++) printf(" %c ", (char)k+97); //napiše črke

 return;
}

/********************************************************************************
 * 	which_chess_piece: 	vrne črko, ki ustreza tipu (barva in figura) figure, 	*
 * 						zato, da jo lahko draw_chess_board nariše				*
 *																				*
 *******************************************************************************/
char which_chess_piece(int index,c_board board)
{
	U64 piece_bitboard = 1;
	piece_bitboard = piece_bitboard << index;


	if((piece_bitboard & board.white_pawns) != 0)
		return 'P';
	if((piece_bitboard & board.black_pawns) != 0)
		return 'p';
	if((piece_bitboard & board.white_rooks) != 0)
		return 'R';
	if((piece_bitboard & board.black_rooks) != 0)
		return 'r';
	if((piece_bitboard & board.white_bishops) != 0)
		return 'B';
	if((piece_bitboard & board.black_bishops) != 0)
		return 'b';
	if((piece_bitboard & board.white_knights) != 0)
		return 'N';
	if((piece_bitboard & board.black_knights) != 0)
		return 'n';
	if((piece_bitboard & board.white_queens) != 0)
		return 'Q';
	if((piece_bitboard & board.black_queens) != 0)
		return 'q';
	if((piece_bitboard & board.white_king) != 0)
		return 'K';
	if((piece_bitboard & board.black_king) != 0)
		return 'k';

	return ' ';
}

void print_message(int which)
{
	switch(which)
	{
	case 0: printf( "\nChess. \"quit\" to quit. \"newg\" to start new game. \"undo\" to undo last move");
			break;
	case 1:
			printf("\nWhite is on the move. Enter coordinates:");
			break;
	case 2:
			printf("\nBlack is on the move. Enter coordinates:");
			break;
	case 3:
			printf("\nCastling error");
			break;
	case 4:
			printf("\nMove out of range.");
			break;
	case 5:
			printf("\nNo piece to move");
			break;
	case 6:
			printf("\nYou can't move opponents chess piece!");
			break;
	case 7:
			printf("\nIllegal move. Your piece on destination square");
			break;
	case 8:
			printf("\nIllegal move");
			break;
	case 9:
			printf("\nUnknown piece type.");
			break;
	case 10:
			printf("\nMove results in check, try again!");
			break;
	case 11:
			printf("\n**Check**");
			break;
	case 12:
			printf("\n**Checkmate**");
			break;
	case 13:
			printf("\n##unknown input##");
			break;
	case 14:
			printf("\nGame over. Goodbye.");
			break;
	case 15:
			printf("\nCannot undo further!");
			break;
	case 16:
			printf("###Stack underflow###\n");
			break;
	case 17:
			printf("\nCannot redo further!");
			break;
	case 18:
			//printf("\nValid move!");
			break;
	case 19:
			//printf("\nChange to white!");
			break;
	case 20:
			//printf("\nChange to black!");
			break;
	case 21:
			//printf("\nundo sucess!");
			break;
	default: printf("#printing error#");
	}

}

char query_for_promotion(void)
{
	char ch, selected = 'Q';

	while(1)
	{
		printf("\n Pawn promotion: Q - queen, N - knight, R - rook, B - bishop:");
		while((ch = getchar()) != '\n')
		{
			selected = ch;
		}

		if(selected == 'Q' || selected == 'N' || selected == 'R' || selected == 'B')
			return selected;
		else
			printf("\n Selection not accepted, try again");
	}
}

/************************************************************************************
 * wait_for_input: sprejme ukaz uporabnika in ga pretvori v programu uporabnega 	*
 * 																					*
 ***********************************************************************************/
bool wait_for_input(short int *move)
{
	char ch, input[5];
	int max_char = 0;
	short int wait_move = 0x0000;
	short int init_move = 0x0001;


	while((ch = getchar()) != '\n')
	{
		if(max_char != 4)
			{
			input[max_char] = ch;
			max_char++;
			}
	}
	input[4] = '\0';

	if( isupper(input[0])) input[0] = tolower(input[0]);
	if( isupper(input[2])) input[2] = tolower(input[2]);

	//imamo input z 4mi chari

	if(!strcmp(input, "quit"))
	{
		*move = 0x00FF;
		return false;
	}
	else if(!strcmp(input, "newg"))
	{
		*move = 0x00EF;
		return false;
	}
	else if(!strcmp(input, "undo"))
	{
		*move = 0x00DF;
		return false;
	}
	else if(!strcmp(input, "redo"))
	{
		*move = 0x00CF;
		return false;
	}

	for(int i = 0; i < 8; i++)
	{
		if(input[0] == 97+i)
		{
			wait_move |= (((init_move + i) << 12) & 0xffff);
		}

		if(input[1] == 49+i)
		{
			wait_move |= (((init_move + i) << 8) & 0xffff);
		}

		if(input[2] == 97+i)
		{
			wait_move |= (((init_move + i) << 4) & 0xffff);
		}


		if(input[3] == 49+i)
		{
			wait_move |= ((init_move + i) & 0xffff);
		}
	}

	*move = wait_move;
	return true;
}

