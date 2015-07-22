/** @file enums.h @addtogroup achess
  * @{ */

#ifndef ENUMS_H_
#define ENUMS_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum { EMPTY, PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, SOURCE } piece;
typedef enum { BLACK, WHITE } color;

/** The square locations and commands in the game logic are presented in a hexadecimal way.
 * Use this type of enumeration to enter chess moves and commands into the address provided when wait_for_input function is called.
 * A chess move is a combination of two chess positions - the move_source position and the move_destination position. E.g. to
 * create a move from A2 (0x12) to B6 (0x26), create a 16 bit hexadecimal number 0x1226.
 * The commands are self-explanatory - QUIT quits the game, NEWG starts a new game, UNDO and REDO undo a move and redo a move, respectively.
 */
typedef enum{
    A1 = 0x11,    A2 = 0x12,    A3 = 0x13,    A4 = 0x14,    A5 = 0x15,    A6 = 0x16,    A7 = 0x17,    A8 = 0x18,
    B1 = 0x21,    B2 = 0x22,    B3 = 0x23,    B4 = 0x24,    B5 = 0x25,    B6 = 0x26,    B7 = 0x27,    B8 = 0x28,
    C1 = 0x31,    C2 = 0x32,    C3 = 0x33,    C4 = 0x34,    C5 = 0x35,    C6 = 0x36,    C7 = 0x37,    C8 = 0x38,
    D1 = 0x41,    D2 = 0x42,    D3 = 0x43,    D4 = 0x44,    D5 = 0x45,    D6 = 0x46,    D7 = 0x47,    D8 = 0x48,
    E1 = 0x51,    E2 = 0x52,    E3 = 0x53,    E4 = 0x54,    E5 = 0x55,    E6 = 0x56,    E7 = 0x57,    E8 = 0x58,
    F1 = 0x61,    F2 = 0x62,    F3 = 0x63,    F4 = 0x64,    F5 = 0x65,    F6 = 0x66,    F7 = 0x67,    F8 = 0x68,
    G1 = 0x71,    G2 = 0x72,    G3 = 0x73,    G4 = 0x74,    G5 = 0x75,    G6 = 0x76,    G7 = 0x77,    G8 = 0x78,
    H1 = 0x81,    H2 = 0x82,    H3 = 0x83,    H4 = 0x84,    H5 = 0x85,    H6 = 0x86,    H7 = 0x87,    H8 = 0x88,
    NONE = 0x00, QUIT = 0xFF, NEWG = 0xEF, UNDO = 0xDF, REDO = 0xCF
}chess_position;

#define POS_LETTER(pos) (((pos) >> 4)& 0xf)	/**< Macro function for parsing a chess_position into a column letter */
#define POS_NUMBER(pos) ((pos) & 0xf)	/**< Macro function for parsing a chess_position into a file number */
#define MOVE_SRC_LETTER(move) (((move) >> 12) & 0xf) /**< Macro function for parsing a chess move into a source column letter */
#define MOVE_SRC_NUMBER(move) (((move) >> 8) & 0xf)	/**< Macro function for parsing a chess move into a source file number */
#define MOVE_DST_LETTER(move) (((move) >> 4) & 0xf) /**< Macro function for parsing a chess move into a destination column letter */
#define MOVE_DST_NUMBER(move) ((move) & 0xf) 	/**< Macro function for parsing a chess move into a destination file number */

typedef unsigned long long int U64; 	/**< The bitboard type used to present diffrent aspects of the chessboard*/

/** This is the structure with all the chessboard information in any given turn.
 *	Bitboards are structured so that the top-most left-most square is bit 0, and proceed right,
 *	skiping to the start of the next file every 8 bits up to bottom right-most square which is bit 63.
 */
typedef struct{
	U64 white_pawns; /**< The bitboard including locations of all the white pawns */
	U64 white_knights; /**< The bitboard including locations of all the white knights */
	U64 white_bishops; /**< The bitboard including locations of all the white bishops */
	U64 white_rooks; /**< The bitboard including locations of all the white rooks */
	U64 white_queens; /**< The bitboard including locations of all the white queens */
	U64 white_king; /**< The bitboard including locations of the white king */

	U64 black_pawns; /**< The bitboard including locations of all the black pawns */
	U64 black_knights; /**< The bitboard including locations of all the black knights */
	U64 black_bishops; /**< The bitboard including locations of all the black bishops */
	U64 black_rooks; /**< The bitboard including locations of all the black rooks */
	U64 black_queens; /**< The bitboard including locations of all the black queens */
	U64 black_king; /**< The bitboard including locations of the black king */

	U64 white_pieces_bit; /**< The bitboard including locations of all the white pieces */
	U64 white_pieces_bit_rot90; /**< The bitboard including locations of all the white pieces, rotated 90 degrees clockwise*/
	U64 white_pieces_bit_rot45_left; /**< The bitboard including locations of all the white pieces, rotated 45 degrees counter-clockwise */
	U64 white_pieces_bit_rot45_right; /**< The bitboard including locations of all the white pieces, rotated 45 degrees clockwise */

	U64 black_pieces_bit; /**< The bitboard including locations of all the black pieces */
	U64 black_pieces_bit_rot90; /**< The bitboard including locations of all the black pieces, rotated 90 degrees clockwise*/
	U64 black_pieces_bit_rot45_left; /**< The bitboard including locations of all the black pieces, rotated 45 degrees counter-clockwise */
	U64 black_pieces_bit_rot45_right; /**< The bitboard including locations of all the black pieces, rotated 45 degrees clockwise */

	U64 all_pieces_bit; /**< The bitboard including locations of all the pieces */
	U64 all_pieces_bit_rot90; /**< The bitboard including locations of all the pieces, rotated 90 degrees clockwise*/
	U64 all_pieces_bit_rot45_left;  /**< The bitboard including locations of all the pieces, rotated 45 degrees counter-clockwise */
	U64 all_pieces_bit_rot45_right; /**< The bitboard including locations of all the pieces, rotated 45 degrees clockwise */

	color player; /**< The variable indiciating which players turn it is */
	U64 enpassant; /**< The bitboard including the location of the en-passant square */
	short int castling; /**< The variable indicating the rules for castling - 0xFFFF -all castling legal, first hex number is white kingside casling, then white queenside castling, black kingside castling, black queenside castling respectively*/
	int half_moves; /**< The number of turns since last capture - not implemented yet */
	int full_moves; /**< The total number of turns*/

	U64 black_attacked_positions; /**< The bitboard indicating all positions attacked by black pieces */
	U64 white_attacked_positions; /**< The bitboard indicating all positions attacked by white pieces */

}c_board;

#ifdef __cplusplus
}
#endif

#endif /* ENUMS_H_ */

/** @} */

