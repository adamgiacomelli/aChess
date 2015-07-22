/*

  HELPER.C
  ========
  (c) Paul Griffiths, 1999
  Email: mail@paulgriffiths.net

  Implementation of sockets helper functions.

  Many of these functions are adapted from, inspired by, or 
  otherwise shamelessly plagiarised from "Unix Network 
  Programming", W Richard Stevens (Prentice Hall).

*/

#include "helper.h"
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>


#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define LETTER 0
#define NUMBER 1
#define MAX_SIZE 1000

const char Letters[8]= {'a','b','c','d','e','f','g','h'};
const char Numbers[8]= {'1','2','3','4','5','6','7','8'};

static char which_chess_piece(int index,c_board board);
static int connection_handle;

bool initGame(int conn)
{
	connection_handle = conn;
	
	struct chess_ui ui_ascii;

	ui_ascii.wait_for_input = wait_for_input;
	ui_ascii.draw_chess_board = draw_chess_board;
	ui_ascii.print_message = print_message;
	ui_ascii.query_for_promotion = query_for_promotion;

	attach_ui(&ui_ascii);

	game();
	return true;
}



/************************************************************************************
 * draw_chess_board: nariše ASCII šahovnico glede na to, kaj je v board strukturi	*
 * 																					*
 ***********************************************************************************/
void draw_chess_board(c_board board)
{
	Writeline(connection_handle, "\r\n   ", MAX_SIZE);
	char temp[30];
	for(int k = 0; k < BOARD_WIDTH; k++)
	{
		sprintf(temp, " %c ", (char)k+97); 
		Writeline(connection_handle, temp, MAX_SIZE); //napiše črke
	}


	for(int i = 0; i < BOARD_WIDTH; i++)
	{
		sprintf(temp, "\r\n %d ", BOARD_HEIGHT - i);
		Writeline(connection_handle, temp, MAX_SIZE);

		for(int j = 0; j < BOARD_HEIGHT; j++)
		{
			sprintf(temp, "[%c]", which_chess_piece((i*8 + j),board));
			Writeline(connection_handle, temp, MAX_SIZE);
		}
		sprintf(temp, " %d",BOARD_HEIGHT - i);
		Writeline(connection_handle, temp, MAX_SIZE);
	}

	Writeline(connection_handle, "\r\n   ", MAX_SIZE);
	
	for(int k = 0;k<BOARD_WIDTH;k++) 
	{
		sprintf(temp, " %c ", (char)k+97);
		Writeline(connection_handle, temp, MAX_SIZE); //napiše črke
	}

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
	case 0: 	Writeline(connection_handle,  "\r\n##Chess. \"quit\" to quit. \"newg\" to start new game. \"undo\" to undo last move", MAX_SIZE);
			break;
	case 1:
			Writeline(connection_handle, "\r\n#NOTIFY_01#White is on the move. Enter coordinates:\n\n", MAX_SIZE);
			break;
	case 2:
			Writeline(connection_handle, "\r\n#NOTIFY_02#Black is on the move. Enter coordinates:\n\n", MAX_SIZE);
			break;
	case 3:
			Writeline(connection_handle, "\r\n#ERROR_03#Castling error", MAX_SIZE);
			break;
	case 4:
			Writeline(connection_handle, "\r\n#ERROR_04#Move out of range.", MAX_SIZE);
			break;
	case 5:
			Writeline(connection_handle, "\r\n#ERROR_05#No piece to move", MAX_SIZE);
			break;
	case 6:
			Writeline(connection_handle, "\r\n#ERROR_06#You can't move opponents chess piece!", MAX_SIZE);
			break;
	case 7:
			Writeline(connection_handle, "\r\n#ERROR_07#Illegal move. Your piece on destination square", MAX_SIZE);
			break;
	case 8:
			Writeline(connection_handle, "\r\n#ERROR_08#Illegal move", MAX_SIZE);
			break;
	case 9:
			Writeline(connection_handle, "\r\n#ERROR_09#Unknown piece type.", MAX_SIZE);
			break;
	case 10:
			Writeline(connection_handle, "\r\n#ERROR_10#Move results in check, try again!", MAX_SIZE);
			break;
	case 11:
			Writeline(connection_handle, "\r\n#NOTIFY_11#**Check**", MAX_SIZE);
			break;
	case 12:
			Writeline(connection_handle, "\r\n#NOTIFY_12#**Checkmate**", MAX_SIZE);
			break;
	case 13:
			Writeline(connection_handle, "\r\n#ERROR_13#unknown input##", MAX_SIZE);
			break;
	case 14:
			Writeline(connection_handle, "\r\n#NOTIFY_14#Game over. Goodbye.", MAX_SIZE);
			break;
	case 15:
			Writeline(connection_handle, "\r\n#ERROR_14#Cannot undo further!", MAX_SIZE);
			break;
	case 16:
			Writeline(connection_handle, "\r\n#ERROR_15##Stack underflow###\r\n", MAX_SIZE);
			break;
	case 17:
			Writeline(connection_handle, "\r\n#ERROR_16#Cannot redo further!", MAX_SIZE);
			break;
	case 18:
			//Writeline(connection_handle, "\r\nValid move!");
			break;
	case 19:
			//Writeline(connection_handle, "\r\nChange to white!");
			break;
	case 20:
			//Writeline(connection_handle, "\r\nChange to black!");
			break;
	case 21:
			//Writeline(connection_handle, "\n\nundo sucess!");
			break;
	default: Writeline(connection_handle, "#ERROR_99#printing", MAX_SIZE);
	}

}

char query_for_promotion(void)
{
	char ch[2], selected = 'Q';

	Writeline(connection_handle, "\n\n#PAWNPROMOTION_QUERY# Pawn promotion: Q - queen, N - knight, R - rook, B - bishop:", MAX_SIZE);
	
	while(1)
	{		
		Readline(connection_handle, ch, 2);		
		selected = ch[0];
		
		if(selected == 'Q' || selected == 'N' || selected == 'R' || selected == 'B')
			return selected;
		else
			Writeline(connection_handle, "\n\n#PAWNPROMOTION_ERROR# Selection not accepted, try again", MAX_SIZE);
	}
}

/************************************************************************************
 * wait_for_input: sprejme ukaz uporabnika in ga pretvori v programu uporabnega 	*
 * 																					*
 ***********************************************************************************/
bool wait_for_input(short int *move)
{
	char input[5];
	short int wait_move = 0x0000;
	short int init_move = 0x0001;

	Readline(connection_handle, input, 5);
	
	/*avoid accepting trailing carriage return*/	
	if(input[0] == 0xd)
		Readline(connection_handle, input, 5);

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

/*  Read a line from a socket  */

ssize_t Readline(int sockd, void *vptr, size_t maxlen) 
{

    ssize_t n, rc;
    char    c, *buffer;

    buffer = vptr;

    for ( n = 1; n < maxlen; n++ ) {
	
	if ( (rc = read(sockd, &c, 1)) == 1 ) {
	    *buffer++ = c;
	    if ( c == '\n' )
		break;
	}
	else if ( rc == 0 ) {
	    if ( n == 1 )
		return 0;
	    else
		break;
	}
	else {
	    if ( errno == EINTR )
		continue;
	    return -1;
	}
    }

    *buffer = 0;
    return n;
}


/*  Write a line to a socket  */

ssize_t Writeline(int sockd, const void *vptr, size_t n) {
    size_t      nleft = n;
    ssize_t     nwritten;
    const char *buffer;
    char ch = ' ';
    buffer = vptr;
	
    for(int i = 0; ch != '\0'; i++)
    {
	ch = buffer[i];
        nleft  = (size_t)i;
    }

    while ( nleft > 0 ) {
	if ( (nwritten = write(sockd, buffer, nleft)) <= 0 ) {
	    if ( errno == EINTR )
		nwritten = 0;
	    else
		return -1;
	}
	nleft  -= nwritten;
	buffer += nwritten;
    }

    return n;
}




