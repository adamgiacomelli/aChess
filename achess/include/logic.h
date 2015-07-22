/** @file logic.h @addtogroup achess A Chess Library
  * @{ */

#ifndef LOGIC_H_
#define LOGIC_H_

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "enums.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
  WELCOME = 0,     /**< Welcome message intended for printing the commands and game title */
  WHITE_MOVE = 1,  /**< A message asking the white player to make a move. */
  BLACK_MOVE = 2,	 /**< A message asking the black player to make a move.*/
  ERROR_CASTLING = 3, /**< A castling error.*/
  ERROR_OUT_OF_RANGE = 4, /**< Out of range move error.*/
  ERROR_NO_APPROPRIATE_PIECE = 5, /**< Cannot find players piece error. */
  ERROR_OPPONENTS_PIECE = 6, /**< Attempt to move opponents piece error.*/
  ERROR_YOUR_PIECE_ON_DESTINATION = 7, /**< Attempt to move on square occupied by your piece error.*/
  ERROR_ILLEGAL_MOVE = 8, /**< Illegal move error.*/
  ERROR_UNKNOWN_PIECE = 9, /**< Unknown piece error. Used for debugging.*/
  ERROR_RESULT_IN_CHECK = 10, /**< The current move would result in check error. */
  NOTIFY_CHECK = 11, /**< Check notification. */
  NOTIFY_CHECKMATE = 12, /**< Checkmate notification. */
  ERROR_UNKNOWN_INPUT = 13, /**< Unknown input error. */
  NOTIFY_QUIT = 14, /**< Quit notification. */
  ERROR_CANT_UNDO = 15, /**< Unable to undo error. */
  ERROR_STACK_UNDERFLOW = 16, /**< Stack underflow error. Used for debugging. */
  ERROR_CANT_REDO = 17, /**< Unable to redo error. */
  NOTIFY_VALID_MOVE = 18, /**< Valid move made notification. Used for debugging. */
  NOTIFY_CHANGE_TO_WHITE = 19, /**< Player has been changed to white notification. Used for debugging. */
  NOTIFY_CHANGE_TO_BLACK = 20, /**< Player has been changed to black notification. Used for debugging. */
  NOTIFY_UNDO_SUCCESS = 21 /**< Successfully undo-ed move. Used for debugging. */  
} chess_message;

/** Logic calls a function of this type to get input.
 *	This is the main input function of the game. Logic runs without interruption until it calls this function,
 *  and expects the command variable to be either a valid chess move (of the type 0xA1B2, where A1 is the move source and
 *  B2 is the move destination) with a return value of "true" or a valid command of the same type with a return value of "false".
 *  All valid inputs are described in the "enums.h" header.
 *
 * @param command The address into which a command is to be written.
 * @return The function is expected to return "true" if the command is a chess move or "false" if the command is not a chess move
 */
typedef bool (*wait_for_input_t)(short int *command);


/** Logic calls a function of this type when it needs to print the chessboard.
 *	This function is called every time the board changes, e.g. when a move was made or undo was called.
 *	UI is expected to extract the appropriate information from the c_board structure it is passed.
 *	Details about the structure are explained in the "enums.h" header.
 *
 * @param board This structure has all the information about the chessboard at the current turn of the game.
 *
 */
typedef void (*draw_chess_board_t)(c_board board);

/** Logic calls a function of this type when it needs to print a message
 *	This function is called when the logic need to pass information about the game (e.g. errors, checks, mates).
 *	The type of message is chess_message.
 *
 * @param which The type of the message logic needs to print.
 *
 */
typedef void (*print_message_t)(int which);


/** Logic calls a function of this type when a pawn needs to be promoted.
 *	This is the remaining input function that is called only when a pawn needs to be promoted.
 *	The function is expected to return one of the following uppercase characters:
 *	'Q' - to promote the pawn to a queen
 *	'N' - to promote the pawn to a knight
 *	'B' - to promote the pawn to a bishop
 *	'R' - to promote the pawn to a rook
 * @return The char value that defines which piece the pawn is to be promoted to.
 */
typedef char (*query_for_promotion_t)(void);


/** The structure that has the addresses of the functions logic needs to call.
 *	Before starting the game the user should create all four functions and
 *	reference them in this type of structure. They are explained in detail
 *	in their typedef declarations in enums.h.
 */
struct chess_ui
{
	wait_for_input_t wait_for_input; 		/**< main input function */
	draw_chess_board_t draw_chess_board;    /**< function with the chessboard structure*/
	print_message_t print_message;			/**< function for printing errors and notifications*/
	query_for_promotion_t query_for_promotion; /**< function that returns the type of figure to promote a pawn to*/
};

/** This function attaches a structure containing the necessary ui functions to the game logic.
 * After the structure with the appropriate functions(the chess_ui structure) has been created use this
 * function to attach the functions to the game logic.
 *
 * @param ui location of the structure.
 */
void attach_ui(struct chess_ui *ui);

/** This function starts the game of chess.
 *	After the functions have been attached run this function to start the game.
 *	The game runs until the quit command is entered.
 */
void game(void);


#ifdef __cplusplus
}
#endif

#endif /* LOGIC_H_ */

/** }@ */

