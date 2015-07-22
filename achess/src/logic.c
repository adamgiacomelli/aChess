#include "logic.h"
#include <string.h>

#define BOARD_WIDTH 8
#define BOARD_HEIGHT 8
#define LETTER 0
#define NUMBER 1
#define FEN_SIZE 100

static U64 one_bitboard = 1;
static const U64 not_A_file = 0xfefefefefefefefe;
static const U64 not_H_file = 0x7f7f7f7f7f7f7f7f;

static c_board board;
static char initial_board[FEN_SIZE]= "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; //original board
									//"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1"; //debug board 1
									//"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -"; //debug board 2
static c_board saved_board;
static chess_position position_list[64];
static struct chess_ui chess_ui; //ui

static void initialize_c_board(void);
static void create_bit_board(char *init_board);
static void create_move_list(void);

static void form_attacked_positions_bit(void);
static void create_masks(void);
static void initialize_attack_arrays(void);

static U64 knight_attacks[64];
static U64 white_pawn_attacks[64];
static U64 black_pawn_attacks[64];
static U64 king_attacks[64];

static U64 rank_attacks[64][64];
static U64 rank_attacks_90[64][64];
static U64 rank_attacks_left_45[64][64];
static U64 rank_attacks_right_45[64][64];

static U64 set_mask[64];
static U64 set_mask_rot_90[64];
static U64 set_mask_rot_left_45[64];
static U64 set_mask_rot_right_45[64];

static U64 diagonal_right_mask;
static U64 diagonal_left_mask;

static U64 eastOne (U64 b) {return (b << 1) & not_A_file;}
static U64 noEaOne (U64 b) {return (b >> 7) & not_A_file;}
static U64 soEaOne (U64 b) {return (b << 9) & not_A_file;}
static U64 westOne (U64 b) {return (b >> 1) & not_H_file;}
static U64 soWeOne (U64 b) {return (b << 7) & not_H_file;}
static U64 noWeOne (U64 b) {return (b >> 9) & not_H_file;}

static U64 rotate_90(U64 bit_board);
static U64 rotate_neg90(U64 bit_board);
static U64 rotate_45(U64 bit_board);
static U64 rotate_neg45(U64 bit_board);
static chess_position get_position_from_char(char letter_char, char number_char);
static void print_bit_board(U64 bit_board);
static U64 shift_bit(U64 input_bit, int shift);

static bool verify_legal_move(short int move, color player);
	static bool move_results_in_check(short int move, color player);
	static bool can_chess_piece_move(short int move, color player,bool comments_);
	static bool basic_movement_check(short int move,color player, bool comments_);
	static bool pawn_movement(short int move,color player, bool comments_);
	static bool knight_movement(short int move,color player, bool comments_);
	static bool bishop_movement(short int move,color player, bool comments_);
	static bool rook_movement(short int move,color player, bool comments_);
	static bool queen_movement(short int move,color player, bool comments_);
	static bool king_movement(short int move,color player, bool comments_);

	static U64 pawn_moves(U64 from_here, color player);
	static U64 knight_moves(U64 from_here);
	static U64 king_moves(U64 from_here);
	static U64 bishop_moves(int from_here,color player);
	static U64 rook_moves(int from_here,color player);

static void move_piece(short int move,color player, piece which_piece);
	void update_boards(int move_src,int move_dst, color player);
	void remove_eaten_figures(int move_dst,color color);

static void switch_player(void);
static int check_verify(color player);
static bool check(color player);
static bool mate(color player);
static void pawn_promotion(short int move);
static void save_board(void);
static void load_board(void);

/*commands*/
static void undo_move(void);
static void redo_move(void);
static bool save_current_game(void);
static void start_new_game(void);


/*
 * Circle buffer
 */
#define BUFFER_SIZE 100
typedef struct  {
	c_board board;
}Game_state;

Game_state my_buffer[BUFFER_SIZE];
Game_state *buffer_start = my_buffer;
Game_state *buffer_end = my_buffer;
Game_state *buffer_undo = my_buffer;
unsigned int buffer_count = 0;


static void add_to_buffer(Game_state added_state);
static Game_state get_from_buffer_end();
static Game_state get_next_state();
static bool buffer_is_empty();
static void buffer_make_empty();

/*
 * Circle buffer end
 */

void attach_ui(struct chess_ui *ui)
{
	chess_ui = *ui;
}

void game(void)
{
	short int move = 0x0000;
	chess_position command;
	bool quit = false;
	bool chess_move;

	create_move_list();
	create_masks();
	initialize_attack_arrays();
	start_new_game();

	while(1)
	{
		/* UI - title & drawing */
		chess_ui.print_message(0); //title
		chess_ui.draw_chess_board(board);

		/* shranimo trenutno postavitev */
		save_board();

		/*1. Pogledamo input in ga kličemo ponovno dokler ni legalen  */
		do {
			chess_ui.print_message(board.player == WHITE ? 1 : 2); //players turn info
			chess_move = chess_ui.wait_for_input(&move);

		} while (chess_move && !verify_legal_move(move, board.player));

		if(chess_move) //če je vnesena šahovska poteza
		{
			/*2. Premaknemo figuro	*/
			move_piece(move, board.player, SOURCE);

			/*3.1 Povišamo kmeta če je možno	*/
			pawn_promotion(move);

			/*4. preverimo, če je nad igralcem šah ali mat	*/
			switch(check_verify(board.player == WHITE ? BLACK : WHITE))
			{
				case 0: break;
				case 1: chess_ui.print_message(11); break;
				case 2: chess_ui.print_message(12); break; //quit = 1; chess_ui.draw_chess_board(board);
			}

			/*5. zamenjamo igralca */
			switch_player();

			/*6.shranimo igro*/
			save_current_game();

			chess_ui.print_message(18);

		}
		else //če je vnesena programska poteza
		{
			command = move;
			switch(command)
			{
			case QUIT:	quit = true; break;
			case UNDO: undo_move();break;
			case REDO: redo_move(); break;
			case NEWG: 	start_new_game(); break;
			default:  chess_ui.print_message(13);
			}
		}
		if(quit) break;

	}
	chess_ui.print_message(14);
}

static void start_new_game(void)
{
	initialize_c_board();
	create_bit_board(initial_board);
	buffer_make_empty();
	save_current_game();
}

bool save_current_game(void)
{
	Game_state saving;
	saving.board = board;
	add_to_buffer(saving);
	return true;
}

/*loads previous turn*/
void undo_move(void)
{
	Game_state loaded;

	if(buffer_count > 1 && buffer_undo != buffer_start)
	{
		loaded = get_from_buffer_end();
		board = loaded.board;
		chess_ui.print_message(21);
	}
	else
	{
		chess_ui.print_message(15);
	}
}

/*redos previously undoed move*/
void redo_move(void)
{
	Game_state loaded;

	if((buffer_end - buffer_undo) > 1)
	{
		loaded = get_next_state();
		board = loaded.board;
	}
	else
	{
		chess_ui.print_message(17);
	}
}


/****************************************
 * switch_player: zamenjamo igralca		*
 ***************************************/
void switch_player(void)
{
	if(board.player == BLACK)
	{
		chess_ui.print_message(19);
		board.player=WHITE;
	}
	else
	{
		chess_ui.print_message(20);
		board.player=BLACK;
	}
}

/********************************************************************************
 * verify_legal_move : nam potrdi, če je poteza, ki se izvaja legalna ali ne	*
 ********************************************************************************/
bool verify_legal_move(short int move, color player)
{
	if(!can_chess_piece_move(move, player, true) || move_results_in_check(move, player))
		return false;
	else
		return true;
}

/********************************************************************************************
 * 																							*
 *  can_chess_piece_move: 	funkcija vrne "true", če se lahko figura na mestu "move_source"	*
 *							premakne na mesto "move_destination". 							*
 * 		player		- WHITE ali BLACK - igralec, ki hoče premakniti figuro					*
 * 		comments_ 	- true ali false  - ali funkcija izpiše komentarje na nelegalne poteze 	*
 *******************************************************************************************/
bool can_chess_piece_move(short int move, color player,bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	//preverimo osnovne zakonitosti premikanja - obstoj figure, poteza na plošči, ali je igralec lastnik figure, ali je na destinaciji njegova figura
	if(!basic_movement_check(move, player, comments_)) return false;

	//preverimo zakonitosti vezane na premikajočo se figuro
	if(	((set_mask[move_src] & board.white_pawns) != 0) ||	((set_mask[move_src] & board.black_pawns) != 0)) return pawn_movement(move, player, comments_);
	if( ((set_mask[move_src] & board.white_knights) != 0) || ((set_mask[move_src] & board.black_knights) != 0)) return knight_movement(move, player, comments_);
	if( ((set_mask[move_src] & board.white_bishops) != 0) || ((set_mask[move_src] & board.black_bishops) != 0)) return bishop_movement(move, player, comments_);
	if( ((set_mask[move_src] & board.white_rooks) != 0) || ((set_mask[move_src] & board.black_rooks) != 0)) return rook_movement(move, player, comments_);
	if( ((set_mask[move_src] & board.white_queens) != 0) || ((set_mask[move_src] & board.black_queens) != 0)) return queen_movement(move, player, comments_);
	if( ((set_mask[move_src] & board.white_king) != 0) || ((set_mask[move_src] & board.black_king) != 0)) return king_movement(move, player, comments_);

	if(comments_) chess_ui.print_message(9); return false;

	return false;
}

bool basic_movement_check(short int move,color player, bool comments_)
{
	//pogledamo, če je poteza na igralni plošči
	if(MOVE_SRC_LETTER(move) < 1 || MOVE_SRC_LETTER(move) > 8
			||
	   MOVE_SRC_NUMBER(move) < 1 || MOVE_SRC_NUMBER(move) > 8
		   ||
	   MOVE_DST_LETTER(move) < 1 || MOVE_DST_LETTER(move) > 8
		   ||
	   MOVE_DST_NUMBER(move) < 1 || MOVE_DST_NUMBER(move) > 8)
	{
		if(comments_) chess_ui.print_message(4);
		return false;
	}

	U64 move_source_bit = set_mask[(8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1)];
	U64 move_destination_bit = set_mask[(8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1)];

	//ali obstaja figura prave barve za premik in ali se premika figura na polje kjer je figura iste barve?
	if( player == WHITE
			&&
	(( ((board.white_pieces_bit) & move_source_bit) == 0 )
			||
	( ((board.white_pieces_bit) & move_destination_bit) != 0)) )
	{
		if(comments_) chess_ui.print_message(5);
		return false;
	}
	else if(player == BLACK
			&&
	(( ((board.black_pieces_bit) & move_source_bit) == 0 )
			||
	( ((board.black_pieces_bit) & move_destination_bit) != 0)) )
	{
		if(comments_) chess_ui.print_message(5);
		return false;
	}
	return true;
}

bool pawn_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	U64 init_black_pawns = 0x000000000000FF00;
	U64 init_white_pawns = 0x00FF000000000000;

	int direction;
	if(player == WHITE) direction = 8;
	else direction = 2;


	//premik naravnost naprej na prazno mesto
	if(((set_mask[move_dst] & (player == WHITE ? board.black_pieces_bit : board.white_pieces_bit)) == 0) && (shift_bit(set_mask[move_src], direction) == set_mask[move_dst]))
	{
			return true;
	}
	//premik za dve naprej preko praznega mesta na prazno mesto(enpas se določi šele pri premiku)
	else if(shift_bit(shift_bit(set_mask[move_src], direction),direction) == set_mask[move_dst])
	{
		if(((set_mask[move_src] & (player == WHITE ? init_white_pawns : init_black_pawns)) != 0)
				&&
			((shift_bit(set_mask[move_src], direction) & board.all_pieces_bit) == 0 )
				&&
			(set_mask[move_dst] & (player == WHITE ? board.black_pieces_bit : board.white_pieces_bit)) == 0)
		{
			return true;
		}
	}
	//premik v diagonali na mesto z nasprotnikovo figuro ali z nastavljenim enpasantom
	else if((shift_bit(set_mask[move_src],(direction -1)) == set_mask[move_dst]) || (shift_bit(set_mask[move_src],(direction +1)) == set_mask[move_dst]) )
	{
		if((set_mask[move_dst] & (player == WHITE ? board.black_pieces_bit : board.white_pieces_bit)) != 0)
			return true;
		else if(( (set_mask[move_dst] & board.enpassant) != 0 ))
			return true;
	}

	if(comments_)chess_ui.print_message(8);
	return false;
}

U64 pawn_moves(U64 from_here, color player)
{
	U64 allowed_moves_bit = 0;
	int direction;

	if(player == WHITE) direction = 8;
		else direction = 2;

	allowed_moves_bit |= shift_bit(from_here,(direction -1)) ;
	allowed_moves_bit |= shift_bit(from_here,(direction +1)) ;

	return allowed_moves_bit;
}

bool knight_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	if((knight_attacks[move_src] & set_mask[move_dst]) != 0)
		return true;

	if(comments_)chess_ui.print_message(8);
	return false;
}

U64 knight_moves(U64 from_here)
{
	return (  noWeOne(from_here >> 8)
			| noEaOne(from_here >> 8)
			| westOne(noWeOne(from_here))
			| eastOne(noEaOne(from_here))
			| westOne(soWeOne(from_here))
			| eastOne(soEaOne(from_here))
			| soWeOne(from_here << 8)
			| soEaOne(from_here << 8));
}

bool bishop_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	if((bishop_moves(move_src,player) & set_mask[move_dst]) != 0)
		return true;

	if(comments_)chess_ui.print_message(8);
	return false;
}

U64 bishop_moves(int from_here,color player)
{
	U64 allowed_moves_bit;
	int move_src = from_here;

	int move_src_rot_right45 =  move_src + 8*(move_src % 8) +((move_src + 8*(move_src % 8)) > 63 ? -64 : 0 );
	int move_src_rot_left45 = move_src - 8*(move_src % 8) +((move_src - 8*(move_src % 8)) < 0 ?  64 : 0 );

	U64 r_mask_remove = ((move_src % 8 >= (8-move_src / 8)) ?  diagonal_left_mask : ~diagonal_left_mask);
	U64 l_mask_remove = ((move_src % 8 > move_src / 8) ?  diagonal_right_mask : ~diagonal_right_mask);

	U64 diagonal_right =  ((((board.all_pieces_bit_rot45_right) >> ((move_src_rot_right45/8)*8))) & 0x7E) >> 1 ;
	U64 diagonal_left = ((((board.all_pieces_bit_rot45_left) >> ((move_src_rot_left45/8)*8))) & 0x7E) >> 1 ; ;

	allowed_moves_bit = (rank_attacks_left_45[move_src_rot_left45][diagonal_left] & ~l_mask_remove )| (rank_attacks_right_45[move_src_rot_right45][diagonal_right] & ~r_mask_remove);

	return (allowed_moves_bit & (player == WHITE ? ~board.white_pieces_bit : ~board.black_pieces_bit)) ;
}

bool rook_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	if( (rook_moves(move_src, player) & set_mask[move_dst]) != 0)
		return true;

	if(comments_)chess_ui.print_message(8);
	return false;
}

U64 rook_moves(int from_here,color player)
{
	int move_src = from_here;
	int move_src_rot_90 = (move_src % 8)*8 + 7 - (move_src/8);

	U64 rank =((board.all_pieces_bit >> (move_src/8)*8) & 0x07E) >> 1;
	U64 file =((board.all_pieces_bit_rot90 >> (move_src_rot_90/8)*8) & 0x07E) >> 1;

	return (((rank_attacks[move_src][rank] | rank_attacks_90[move_src_rot_90][file])) & (player == WHITE ? ~board.white_pieces_bit : ~board.black_pieces_bit)) ;

}

bool queen_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	U64 queen_move_bit = (rook_moves(move_src,player) | bishop_moves(move_src,player));

	if((queen_move_bit & set_mask[move_dst]) != 0)
		return true;

	if(comments_)chess_ui.print_message(8);
	return false;
}

bool king_movement(short int move,color player, bool comments_)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	//če se kralj premika v katerokoli smer za eno polje

	if((king_attacks[move_src] & set_mask[move_dst]) != 0)
		return true;

	//queenside rošada
	if(westOne(westOne(set_mask[move_src])) == set_mask[move_dst])
	{
		if( (player == WHITE ? ((board.castling & 0x0F00) != 0) :((board.castling & 0x000F) != 0))
				&&
			((set_mask[move_src] & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0)
				&&
			((westOne(set_mask[move_src]) & board.all_pieces_bit) == 0) && ((westOne(set_mask[move_src]) & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0)
				&&
			((westOne(westOne(set_mask[move_src])) & board.all_pieces_bit) == 0) && ((westOne(westOne(set_mask[move_src])) & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0)
				&&
			((westOne(westOne(westOne(set_mask[move_src]))) & board.all_pieces_bit) == 0) )
		{
			return true;
		}
	}
	//kingside rošada
	else if(eastOne(eastOne(set_mask[move_src])) == set_mask[move_dst])
	{

		if( (player == WHITE ? ((board.castling & 0xF000) != 0) : ((board.castling & 0x00F0) != 0))
				&&
			((set_mask[move_src] & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0)
				&&
			((eastOne(set_mask[move_src]) & board.all_pieces_bit) == 0)
				&&
			((eastOne(set_mask[move_src]) & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0)
				&&
			((eastOne(eastOne(set_mask[move_src])) & board.all_pieces_bit) == 0)
				&&
			((eastOne(eastOne(set_mask[move_src])) & (player == WHITE ? board.black_attacked_positions : board.white_attacked_positions)) == 0) )
		{
			return true;
		}
	}

	if(comments_)chess_ui.print_message(8);
	return false;
}

U64 king_moves(U64 from_here)
{
	U64 allowed_moves_bit = 0;

	for(int i = 1; i < 10; i++)
	{
		allowed_moves_bit |= shift_bit(from_here, i);
	}

	return allowed_moves_bit;
}

/*
 * 	update_boards: izvede premik figure na vseh ploščah - tudi rotiranih
 */
void update_boards(int move_src,int move_dst, color player)
{
	if(player == WHITE)
	{
		board.white_pieces_bit ^= (set_mask[move_src] | set_mask[move_dst]);
		board.white_pieces_bit_rot90 ^= (set_mask_rot_90[move_src] | set_mask_rot_90[move_dst]);
		board.white_pieces_bit_rot45_left ^= (set_mask_rot_left_45[move_src] | set_mask_rot_left_45[move_dst]);
		board.white_pieces_bit_rot45_right ^= (set_mask_rot_right_45[move_src] | set_mask_rot_right_45[move_dst]);
	}
	else
	{
		board.black_pieces_bit ^= (set_mask[move_src] | set_mask[move_dst]);
		board.black_pieces_bit_rot90 ^= (set_mask_rot_90[move_src] | set_mask_rot_90[move_dst]);
		board.black_pieces_bit_rot45_left ^= (set_mask_rot_left_45[move_src] | set_mask_rot_left_45[move_dst]);
		board.black_pieces_bit_rot45_right ^= (set_mask_rot_right_45[move_src] | set_mask_rot_right_45[move_dst]);
	}
}


/****************************************************************************
 * move_piece: premakne figuro oz. izvede rošado							*
 * 																			*
 ***************************************************************************/
void move_piece(short int move, color player,piece which_piece)
{
	int move_src = (8*(8-MOVE_SRC_NUMBER(move)) + MOVE_SRC_LETTER(move) - 1);
	int move_dst = (8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1);

	if(which_piece == SOURCE)
	{
		if(	((set_mask[move_src] & (player == WHITE ? board.white_pawns : board.black_pawns)) != 0))
		{
			if(player == WHITE)
			{
				if((set_mask[move_dst] & board.enpassant) != 0) //if the pawn is eating enpassant
				{
					board.black_pawns &= ~(set_mask[move_dst+8]); //delete opponents pawn
					board.black_pieces_bit &= ~(set_mask[move_dst+8]); //update black figures
					board.black_pieces_bit_rot90 &= ~(set_mask_rot_90[move_dst + 8 ]);
					board.black_pieces_bit_rot45_left &= ~(set_mask_rot_left_45[move_dst + 8]);
					board.black_pieces_bit_rot45_right &= ~(set_mask_rot_right_45[move_dst + 8]);

					board.white_pawns ^= (set_mask[move_src] | set_mask[move_dst]); //move figure
					update_boards(move_src,move_dst,WHITE);

					board.enpassant = 0x0000000000000000;
				}
				else if((set_mask[move_dst] & board.all_pieces_bit) == 0)//if the pawn is moving to an empty place
				{

					if(( (set_mask[move_src] >> 16) & (set_mask[move_dst]) ) != 0 )
					{
						board.enpassant = (set_mask[move_src] >> 8); //we set the enpassant if the pawn is moving two spaces
					}
					else
					{
						board.enpassant = 0x0000000000000000;//we clear the enpassant if the pawn is moving one space
					}

					board.white_pawns ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,WHITE);
				}
				else
				{
					board.enpassant = 0x0000000000000000;
					board.white_pawns ^= (set_mask[move_src] | set_mask[move_dst]); // move figure
					update_boards(move_src,move_dst,WHITE);
					remove_eaten_figures(move_dst,WHITE); //remove eaten figure
				}
			}
			else
			{
				if((set_mask[move_dst] & board.enpassant) != 0)
				{
					board.white_pawns = board.white_pawns & ~(set_mask[move_dst-8] );
					board.white_pieces_bit &= ~(set_mask[move_dst-8]);
					board.white_pieces_bit_rot90 &= ~(set_mask_rot_90[move_dst-8]);
					board.white_pieces_bit_rot45_left &= ~(set_mask_rot_left_45[move_dst-8]);
					board.white_pieces_bit_rot45_right &= ~(set_mask_rot_right_45[move_dst-8]);//possible bug with rshift and lshift!!!

					board.black_pawns ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,BLACK);
					board.enpassant = 0x0000000000000000;
				}
				else if((set_mask[move_dst] & board.all_pieces_bit) == 0)
				{
					//enpassant setting
					if(( (set_mask[move_src] << 16) & (set_mask[move_dst]) ) != 0 )
					{
						board.enpassant = (set_mask[move_src] << 8);
					}
					else
					{
						board.enpassant = 0x0000000000000000;
					}
					board.black_pawns ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,BLACK);
				}
				else
				{
					board.enpassant = 0x0000000000000000;
					board.black_pawns ^= (set_mask[move_src] | set_mask[move_dst]); // move figure
					update_boards(move_src,move_dst,BLACK);

					remove_eaten_figures(move_dst,BLACK); //remove eaten figure

				}
			}
		}
		if( ((set_mask[move_src] & (player == WHITE ? board.white_knights : board.black_knights)) != 0))
		{
			board.enpassant = 0x0000000000000000;
			if(player == WHITE)
			{
			board.white_knights ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,WHITE);
			remove_eaten_figures(move_dst,WHITE);

			}
			else
			{
			board.black_knights ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,BLACK);
			remove_eaten_figures(move_dst, BLACK);
			}
		}
		if( ((set_mask[move_src] & (player == WHITE ? board.white_bishops : board.black_bishops)) != 0))
		{
			board.enpassant = 0x0000000000000000;
			if(player == WHITE)
			{
			board.white_bishops ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,WHITE);
			remove_eaten_figures(move_dst, WHITE);
			}
			else
			{
			board.black_bishops ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,BLACK);

			remove_eaten_figures(move_dst, BLACK);
			}
		}
		if( ((set_mask[move_src] & (player == WHITE ? board.white_rooks : board.black_rooks)) != 0))
		{
			board.enpassant = 0x0000000000000000;
			if(player == WHITE)
			{
				if((set_mask[move_src] & 0x8000000000000000) != 0)
					board.castling &= 0x0FFF;
				else if((set_mask[move_src] & 0x0100000000000000) != 0)
					board.castling &= 0xF0FF;

			board.white_rooks ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,WHITE);
			remove_eaten_figures(move_dst, WHITE);
			}
			else
			{
				if((set_mask[move_src] & 0x0000000000000001) != 0)
					board.castling &= 0xFFF0;
				else if((set_mask[move_src] & 0x0000000000000080) != 0)
					board.castling &= 0xFF0F;

			board.black_rooks ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,BLACK);
			remove_eaten_figures(move_dst, BLACK);
			}
		}
		if( ((set_mask[move_src] & (player == WHITE ? board.white_queens : board.black_queens)) != 0))
		{
			board.enpassant = 0x0000000000000000;

			if(player == WHITE)
			{
			board.white_queens ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,WHITE);

			remove_eaten_figures(move_dst, WHITE);
			}
			else
			{
			board.black_queens ^= (set_mask[move_src] | set_mask[move_dst]);
			update_boards(move_src,move_dst,BLACK);

			remove_eaten_figures(move_dst, BLACK);
			}
		}
		if( ((set_mask[move_src] & (player == WHITE ? board.white_king : board.black_king)) != 0))
		{
			board.enpassant = 0x0000000000000000;
			if(player == WHITE)
			{

				if(westOne(westOne(set_mask[move_src])) == set_mask[move_dst])
				{
					board.castling &= 0xF0FF;
					board.white_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,WHITE);
					board.white_rooks ^= (set_mask[56] | set_mask[59]);
					update_boards(56,59,WHITE);

				}
				else if(eastOne(eastOne(set_mask[move_src])) == set_mask[move_dst])
				{
					board.castling &= 0x0FFF;
					board.white_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,WHITE);
					board.white_rooks ^= (set_mask[63] | set_mask[61]);
					update_boards(63,61,WHITE);
				}
				else
				{
					board.castling &= 0x00FF;
					board.white_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,WHITE);
					remove_eaten_figures(move_dst,WHITE);
				}
			}
			else
			{
				if(westOne(westOne(set_mask[move_src])) == set_mask[move_dst])
				{
					board.castling &= 0xFFF0;
					board.black_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,BLACK);
					board.black_rooks ^= (set_mask[0] | set_mask[3]);
					update_boards(0,3,BLACK);
				}
				else if(eastOne(eastOne(set_mask[move_src])) == set_mask[move_dst])
				{
					board.castling &= 0xFF0F;
					board.black_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,BLACK);
					board.black_rooks ^= (set_mask[7] | set_mask[5]);
					update_boards(7,5,BLACK);
				}
				else
				{
					board.castling &= 0xFF00;
					board.black_king ^= (set_mask[move_src] | set_mask[move_dst]);
					update_boards(move_src,move_dst,BLACK);
					remove_eaten_figures(move_dst,BLACK);
				}
			}
		}
	}

	board.all_pieces_bit = (board.white_pieces_bit | board.black_pieces_bit);
	board.all_pieces_bit_rot90 = (board.white_pieces_bit_rot90 | board.black_pieces_bit_rot90);
	board.all_pieces_bit_rot45_right = (board.white_pieces_bit_rot45_right | board.black_pieces_bit_rot45_right);
	board.all_pieces_bit_rot45_left = (board.white_pieces_bit_rot45_left | board.black_pieces_bit_rot45_left);


	form_attacked_positions_bit();
}

U64 shift_bit(U64 input_bit, int shift)
{
	switch(shift)
	{
	case 1:	return ((input_bit & not_A_file) << 7);
	case 2: return (input_bit  << 8);
	case 3: return ((input_bit & not_H_file) << 9);
	case 4: return ((input_bit & not_A_file) >> 1);
	case 5 : return 0; break;
	case 6: return ((input_bit & not_H_file) << 1);
	case 7: return ((input_bit & not_A_file) >> 9);
	case 8: return (input_bit >> 8);
	case 9: return ((input_bit & not_H_file) >> 7);
	default: break;
	}
	return 0;
}

void initialize_c_board(void)
{
	board.white_pawns =  0;
	board.white_knights = 0;
	board.white_bishops = 0;
	board.white_queens = 0;
	board.white_rooks = 0;
	board.white_king = 0;
	board.black_pawns =  0;
	board.black_knights = 0;
	board.black_bishops = 0;
	board.black_queens = 0;
	board.black_rooks = 0;
	board.black_king = 0;

	board.player = WHITE;
	board.enpassant = NONE;
	board.castling = 0;
	board.half_moves = 0;
	board.full_moves = 1;
}
/*
 * create_bit_board: pre-parsa iz FEN notacije v strukturo, kliče se samo na začetku igre
 *
 */
void create_bit_board(char *init_board)
{
	int i, slash = 0, space = 0;
	chess_position enpas_position;
	U64 enpas_bit = one_bitboard;
	//get pieces
	for(i = 0; init_board[i] != ' ';i++)
	{
		switch(init_board[i])
		{
		case 'P': board.white_pawns = (board.white_pawns | (one_bitboard << (i - slash + space))); break;
		case 'N': board.white_knights = (board.white_knights | (one_bitboard << (i - slash + space))); break;
		case 'B': board.white_bishops = (board.white_bishops | (one_bitboard << (i - slash + space))); break;
		case 'Q': board.white_queens = (board.white_queens | (one_bitboard << (i - slash + space))); break;
		case 'R': board.white_rooks = (board.white_rooks | (one_bitboard << (i - slash + space))); break;
		case 'K': board.white_king = (board.white_king | (one_bitboard << (i - slash + space))); break;
		case 'p': board.black_pawns =  (board.black_pawns | (one_bitboard << (i - slash + space))); break;
		case 'n': board.black_knights = (board.black_knights | (one_bitboard << (i - slash + space))); break;
		case 'b': board.black_bishops = (board.black_bishops | (one_bitboard << (i - slash + space))); break;
		case 'q': board.black_queens = (board.black_queens | (one_bitboard << (i - slash + space))); break;
		case 'r': board.black_rooks = (board.black_rooks | (one_bitboard << (i - slash + space))); break;
		case 'k': board.black_king = (board.black_king | (one_bitboard << (i - slash + space))); break;
		case '/': slash++;break;
		case '1': break;
		case '2': space +=1; break;
		case '3': space +=2; break;
		case '4': space +=3; break;
		case '5': space +=4; break;
		case '6': space +=5; break;
		case '7': space +=6; break;
		case '8': space +=7; break;
		default: chess_ui.print_message(13);break;
		}
	}
	i++;

	//get player
		if(init_board[i] == 'w')
			board.player = WHITE;
		else if(init_board[i] == 'b')
			board.player = BLACK;
		else chess_ui.print_message(13);

	i+=2;
	//get castling
	for(;init_board[i] != ' ';i++)
	{
		switch(init_board[i])
		{
		case 'K': board.castling = (board.castling | 0x1000);break;
		case 'Q': board.castling = (board.castling | 0x0100);break;
		case 'k': board.castling = (board.castling | 0x0010);break;
		case 'q': board.castling = (board.castling | 0x0001);break;
		case '-': break;
		}
	}
	i++;
	//get enpasant
	if(init_board[i] == '-')
	{
		board.enpassant = 0x0000000000000000;
		i++;
	}
	else
	{
		enpas_position = get_position_from_char(init_board[i], init_board[i+1]);
		for(int i = 0; i < 64;i++)
		{
			if(enpas_position == position_list[i])
				board.enpassant = enpas_bit;
			enpas_bit = enpas_bit << 1;
		}
		i+=2;
	}
	i++;
	board.half_moves = init_board[i] - 48;
	i+=2;
	board.full_moves = init_board[i] - 48;

	board.white_pieces_bit = board.white_pawns | board.white_knights | board.white_bishops | board.white_rooks | board.white_queens | board.white_king;
	board.black_pieces_bit = board.black_pawns | board.black_knights | board.black_bishops | board.black_rooks | board.black_queens | board.black_king;
	board.white_pieces_bit_rot90 = rotate_90(board.white_pieces_bit);
	board.black_pieces_bit_rot90 = rotate_90(board.black_pieces_bit);
	board.white_pieces_bit_rot45_right = rotate_45(board.white_pieces_bit);
	board.black_pieces_bit_rot45_right = rotate_45(board.black_pieces_bit);
	board.white_pieces_bit_rot45_left = rotate_neg45(board.white_pieces_bit);
	board.black_pieces_bit_rot45_left = rotate_neg45(board.black_pieces_bit);

	board.all_pieces_bit = (board.white_pieces_bit | board.black_pieces_bit);
	board.all_pieces_bit_rot90 = (board.white_pieces_bit_rot90 | board.black_pieces_bit_rot90);
	board.all_pieces_bit_rot45_right = (board.white_pieces_bit_rot45_right | board.black_pieces_bit_rot45_right);
	board.all_pieces_bit_rot45_left = (board.white_pieces_bit_rot45_left | board.black_pieces_bit_rot45_left);

	form_attacked_positions_bit();
}

/*
 * rotate_90 rotira bitboard v smeri urinega kazalca za 90 stopinj
 */
U64 rotate_90(U64 bit_board)
{
	U64 rotated_board = 0;

	for(int i = 0; i < 64; i++)
	{
		if((set_mask[i] & bit_board)!= 0)
		{
			rotated_board |= one_bitboard << (8*(i % 8)+(7-i/8));
		}
	}
	return rotated_board;
}

/*
 * rotate_neg90 rotira bitboard proti smeri urinega kazalca za 90 stopinj
 */
U64 rotate_neg90(U64 bit_board)
{
	U64 rotated_board = 0;

	for(int i = 0; i < 64; i++)
	{
		if((set_mask[i] & bit_board)!= 0)
		{
			rotated_board |= one_bitboard << ( (8*(7 - i % 8) + (i/8)));
		}
	}

	return rotated_board;
}
/*
 * rotate_45 ustvari pseudo rotiran board za 45 stopinj v smeri urinega kazalca
 */
U64 rotate_45(U64 bit_board)
{
	U64 rotated_board = 0;

	for(int i = 0; i < 64; i++)
	{
		if((set_mask[i] & bit_board)!= 0)
		{
			if(i%8 == 0)
				rotated_board |= one_bitboard << i;
			else
			{	if((i + 8*(i%8)) > 63)
					rotated_board |= one_bitboard << (i + 8*(i%8) - 64);
				else
					rotated_board |= one_bitboard << (i + 8*(i%8));
			}

		}
	}
	return rotated_board;
}


/*
 * rotate_45 ustvari pseudo rotiran board za 45 stopinj proti smeri urinega kazalca
 */
U64 rotate_neg45(U64 bit_board)
{
	U64 rotated_board = 0;

	for(int i = 0; i < 64; i++)
	{
		if((set_mask[i] & bit_board)!= 0)
		{
			if(i%8 == 0)
				rotated_board |= one_bitboard << i;
			else
			{	if((i + 8*(i%8)) < 0)
					rotated_board |= one_bitboard << (i - 8*(i%8) + 64);
				else
					rotated_board |= one_bitboard << (i - 8*(i%8));
			}

		}
	}
	return rotated_board;
}


/*
 *  form_attacked_positions_bit: formira bit-boarde ki vsebujejo vsa napadena polja
 *
 */
void form_attacked_positions_bit(void)
{
	board.white_attacked_positions = 0;
	board.black_attacked_positions = 0;

	for(int i = 0; i < 64;i++)
	{
		if((set_mask[i] & board.white_pieces_bit) != 0)
		{
			if((set_mask[i] & board.white_pawns) != 0) board.white_attacked_positions |= white_pawn_attacks[i];
			if((set_mask[i] & board.white_knights) != 0) board.white_attacked_positions |= knight_attacks[i];
			if((set_mask[i] & board.white_bishops) != 0) board.white_attacked_positions |= bishop_moves(i, WHITE);
			if((set_mask[i] & board.white_rooks) != 0) board.white_attacked_positions |= rook_moves(i, WHITE);
			if((set_mask[i] & board.white_queens) != 0) board.white_attacked_positions |= (rook_moves(i,WHITE) | bishop_moves(i,WHITE));
			if((set_mask[i] & board.white_king) != 0) board.white_attacked_positions |= king_attacks[i];
		}
		else if((set_mask[i] & board.black_pieces_bit) != 0)
		{
			if((set_mask[i] & board.black_pawns) != 0) board.black_attacked_positions |= black_pawn_attacks[i];
			if((set_mask[i] & board.black_knights) != 0) board.black_attacked_positions |= knight_attacks[i];
			if((set_mask[i] & board.black_bishops) != 0) board.black_attacked_positions |= bishop_moves(i, BLACK);
			if((set_mask[i] & board.black_rooks) != 0) board.black_attacked_positions |= rook_moves(i, BLACK);
			if((set_mask[i] & board.black_queens) != 0) board.black_attacked_positions |= (rook_moves(i,BLACK) | bishop_moves(i,BLACK));
			if((set_mask[i] & board.black_king) != 0) board.black_attacked_positions |= king_attacks[i];
		}
	}

}
/*
 * create_masks: kreira maske,potrebne za manipulacijo z bitboardi
 */
static void create_masks(void)
{
	diagonal_left_mask = 0 ;
	diagonal_right_mask = 0;
	U64 bit = 0x00000000000000FF;

	for(int i = 0; i < 64; i++)
	{
		set_mask[i] = one_bitboard << i;
		set_mask_rot_90[i] = rotate_90(set_mask[i]);
		set_mask_rot_left_45[i] = rotate_neg45(set_mask[i]);
		set_mask_rot_right_45[i] = rotate_45(set_mask[i]);
	}

	for(int i = 0;i < 8; i++)
	{
		diagonal_right_mask |= (bit >> i) << ((7-i)*8);
		diagonal_left_mask |= ((bit << (i+1)) & 0xFF) << ((7-i)*8);
	}
	diagonal_left_mask = ~diagonal_left_mask;

}

/*
 * initialize_attack_arrays: inicializira matrike ki prevajajo rotirane matrike nazaj v običajne
 *
 */
static void initialize_attack_arrays(void)
{
	U64 selecting_bit = one_bitboard;
	U64 rank_bit;
	U64 rank_input_bit = 0;
	U64 rank_entrance_bit = 0;

	for(int i = 0; i < 64; i++)
	{
		for(int j = 0; j < 64; j++)
		{
			rank_bit = one_bitboard << (i % 8); //rank bit: vsebuje polje za katerega računamo premik levo-desno
			rank_input_bit = (j << 1) & 0x7E; //rank input bit: je 8bitno polje z vsebino figur tiste vrstice
			rank_entrance_bit = 0; //postavimo nov rank entrance bit na nič
			//east search
			for(int k = 0; k < 8; k++)
			{
				rank_bit = eastOne(rank_bit); //premaknemo rank bit na vzhod(desno)
				rank_entrance_bit |= rank_bit; //ga dodamo bitu, ki bo vseboval napadena polja
				if((rank_bit & rank_input_bit) != 0) //če je polje zasedeno prekinemo zanko (rank_entrance potem vsebuje tudi prvo figuro)
					break;
			}

			rank_bit = one_bitboard << (i % 8);
			//west search
			for(int k = 0; k < 8; k++)
			{
				rank_bit = westOne(rank_bit);
				rank_entrance_bit |= rank_bit;
				if((rank_bit & rank_input_bit) != 0)
					break;
			}
			rank_entrance_bit &= ~(one_bitboard << (i % 8)); //odstranimo polje na katerem je figura
			rank_entrance_bit = rank_entrance_bit << (i - i%8); //zamaknemo entrance bit na pravilno vrstico

			rank_attacks[i][j] =  rank_entrance_bit;
			rank_attacks_90[i][j] = rotate_neg90(rank_attacks[i][j]);
			rank_attacks_right_45[i][j] = rotate_neg45(rank_attacks[i][j]);
			rank_attacks_left_45[i][j] = rotate_45(rank_attacks[i][j]);

		}
	}

	for(int i = 0; i < 64; i++)
	{
		knight_attacks[i] = knight_moves(selecting_bit);
		black_pawn_attacks[i] = pawn_moves(selecting_bit,BLACK);
		white_pawn_attacks[i] = pawn_moves(selecting_bit,WHITE);
		king_attacks[i] = king_moves(selecting_bit);

		selecting_bit = selecting_bit << 1;
	}


}

chess_position get_position_from_char(char letter_char, char number_char)
{
	int first, second;
	first = letter_char;
	second = number_char;
	switch(first)
	{
	case 'a': first = 0x1;break;
	case 'b': first = 0x2;break;
	case 'c': first = 0x3;break;
	case 'd': first = 0x4;break;
	case 'e': first = 0x5;break;
	case 'f': first = 0x6;break;
	case 'g': first = 0x7;break;
	case 'h': first = 0x8;break;
	default: break;
	}

	switch(second)
	{
	case '1': second = 0x01;break;
	case '2': second = 0x02;break;
	case '3': second = 0x03;break;
	case '4': second = 0x04;break;
	case '5': second = 0x05;break;
	case '6': second = 0x06;break;
	case '7': second = 0x07;break;
	case '8': second = 0x08;break;
	default: break;
	}

	return first || second;
}

/*
 * remove_eaten_figures: odstrani požre figure
 */
void remove_eaten_figures(int move_dst,color color)
{
	if(color == WHITE)
	{
		board.black_pawns &= ~(set_mask[move_dst]);;
		board.black_knights &= ~(set_mask[move_dst]);
		board.black_bishops &= ~(set_mask[move_dst]);
		board.black_queens &= ~(set_mask[move_dst]);
		board.black_rooks &= ~(set_mask[move_dst]);

		board.black_pieces_bit &= ~(set_mask[move_dst]);
		board.black_pieces_bit_rot90 &= ~(set_mask_rot_90[move_dst]);
		board.black_pieces_bit_rot45_left &= ~(set_mask_rot_left_45[move_dst]);
		board.black_pieces_bit_rot45_right &= ~(set_mask_rot_right_45[move_dst]);
	}
	else
	{
		board.white_pawns &= ~(set_mask[move_dst]);;
		board.white_knights &= ~(set_mask[move_dst]);
		board.white_bishops &= ~(set_mask[move_dst]);
		board.white_queens &= ~(set_mask[move_dst]);
		board.white_rooks &= ~(set_mask[move_dst]);

		board.white_pieces_bit &= ~(set_mask[move_dst]);
		board.white_pieces_bit_rot90 &= ~(set_mask_rot_90[move_dst]);
		board.white_pieces_bit_rot45_left &= ~(set_mask_rot_left_45[move_dst]);
		board.white_pieces_bit_rot45_right &= ~(set_mask_rot_right_45[move_dst]);
	}

}

/*
 * print_bit_board: debugging funkcija za sprotno gledanje bitboardov
 */
static void print_bit_board(U64 bit_board)
{
	U64 temp_bit = bit_board;

	printf("\n");
	for(int i = 1; i <= 64; i++)
	{
		if(temp_bit % 2 != 0)
			printf("1");
		else
			printf("0");

		if(i % 8 == 0)
			printf("\n");

		temp_bit /= 2;
	}
}


/*save_board: shrani trenutno postavitev figur*/
void save_board()
{
	saved_board = board;
}

/*load_board: naloži postavitev figur iz saved_board*/
void load_board()
{
	board =saved_board;
}



/****************************************************************
 * pawn_promotion: poviša kmeta, ki je prišel do konca			*
 *	 															*
 ***************************************************************/
void pawn_promotion(short int move)
{
	U64 move_destination_bit = set_mask[(8*(8-MOVE_DST_NUMBER(move)) + MOVE_DST_LETTER(move) - 1)];

	char pawn_promoted;

	if( ((move_destination_bit & board.black_pawns) != 0)
			&&
		((move_destination_bit & 0xFF00000000000000) != 0) )
	{
		pawn_promoted = chess_ui.query_for_promotion();

		board.black_pawns = board.black_pawns & ~move_destination_bit;

		switch(pawn_promoted)
		{
			case 'Q': board.black_queens = board.black_queens | move_destination_bit; break;
			case 'N': board.black_knights = board.black_knights | move_destination_bit; break;
			case 'R': board.black_rooks = board.black_rooks | move_destination_bit; break;
			case 'B': board.black_bishops = board.black_bishops | move_destination_bit; break;
		}
	}
	else if( ((move_destination_bit & board.white_pawns) != 0)
			&&
			((move_destination_bit & 0x00000000000000FF) != 0) )
	{
		pawn_promoted = chess_ui.query_for_promotion();

		board.white_pawns = board.white_pawns & ~move_destination_bit;

		switch(pawn_promoted)
		{
			case 'Q': board.white_queens = board.white_queens | move_destination_bit; break;
			case 'N': board.white_knights = board.white_knights | move_destination_bit; break;
			case 'R': board.white_rooks = board.white_rooks | move_destination_bit; break;
			case 'B': board.white_bishops = board.white_bishops | move_destination_bit; break;
		}
	}
	else
	{
		return;
	}
}

static void create_move_list(void)
{
	position_list[0]= A8; position_list[1]= B8; position_list[2]= C8; position_list[3]= D8;	position_list[4]= E8; position_list[5]= F8; position_list[6]= G8; position_list[7]= H8;
	position_list[8]= A7; position_list[9]= B7; position_list[10]= C7; position_list[11]= D7;	position_list[12]= E7; position_list[13]= F7; position_list[14]= G7; position_list[15]= H7;
	position_list[16]= A6; position_list[17]= B6; position_list[18]= C6; position_list[19]= D6;	position_list[20]= E6; position_list[21]= F6; position_list[22]= G6; position_list[23]= H6;
	position_list[24]= A5; position_list[25]= B5; position_list[26]= C5; position_list[27]= D5;	position_list[28]= E5; position_list[29]= F5; position_list[30]= G5; position_list[31]= H5;
	position_list[32]= A4; position_list[33]= B4; position_list[34]= C4; position_list[35]= D4;	position_list[36]= E4; position_list[37]= F4; position_list[38]= G4; position_list[39]= H4;
	position_list[40]= A3; position_list[41]= B3; position_list[42]= C3; position_list[43]= D3;	position_list[44]= E3; position_list[45]= F3; position_list[46]= G3; position_list[47]= H3;
	position_list[48]= A2; position_list[49]= B2; position_list[50]= C2; position_list[51]= D2;	position_list[52]= E2; position_list[53]= F2; position_list[54]= G2; position_list[55]= H2;
	position_list[56]= A1; position_list[57]= B1; position_list[58]= C1; position_list[59]= D1;	position_list[60]= E1; position_list[61]= F1; position_list[62]= G1; position_list[63]= H1;

}



/****************************************************************************
 * check_: preveri, če je nad "player"-jem šah.								*
 * 																			*
 ***************************************************************************/
bool check(color player)
{
	if(player == WHITE ? ((board.white_king & board.black_attacked_positions) != 0) : ((board.black_king & board.white_attacked_positions) != 0))
		return true;

	return false;
}

/************************************************************
 * mate: preveri, če je nad "player"-jem mat.				*
 * 															*
 * 		foreach (piece <- currentPlayer.pieces) do			*
 *			foreach (position <- piece.position) do			*
 *				check <- tryCheck(piece, position)			*
 *				if (!check) then return false				*
 *			end												*
 *		end													*
 *															*
 *		return true											*
 ***********************************************************/
bool mate(color player)
{
	short int try_move;
	U64 find_piece = one_bitboard;

//najdemo igralčevo figuro
	for(int i = 0; i < 64;i++)
	{
		if(((player == WHITE ? board.white_pieces_bit : board.black_pieces_bit) & find_piece) != 0)
		{
			try_move = position_list[i] << 8;

			for(int j = 0; j< 64; j++)
			{
				try_move &= 0xff00;
				try_move |= position_list[j];

				save_board();

				if(can_chess_piece_move(try_move,player,false))
				{
					move_piece(try_move,player,SOURCE);

					if(!check(player))
					{
						load_board();
						return false;
					}
					load_board();
				}
			}
		}
		try_move=0;
		find_piece = find_piece << 1;
	}

	return true;
}

/****************************************************************************************
 * move_results_in_check: preveri, če je po potezi šah nad igralcem, ki premika figuro	*
 ***************************************************************************************/
bool move_results_in_check(short int move,color player)
{
	save_board();

	move_piece(move, player, SOURCE);

	if(player == WHITE ? ((board.white_king & board.black_attacked_positions) != 0) : ((board.black_king & board.white_attacked_positions) != 0))
	{
		load_board();
		chess_ui.print_message(10);
		return true;
	}
	load_board();
	return false;
}

/*check_verify: preveri, če je šah ali mat*/
int check_verify(color player)
{
	if(check(player))
	{
		if(mate(player))
		{
			return 2;
		}
		return 1;
	}
	return 0;
}

/*circle buffer*/
void add_to_buffer(Game_state added_state)
{
	int dif;
	dif = (buffer_end - buffer_undo);

	if(dif < 2)
	{
		buffer_undo = buffer_end;
		*buffer_end = added_state;

		if(buffer_count < BUFFER_SIZE)	buffer_count++;
	}
	else if(dif > 1)
	{
		buffer_undo++;
		if(buffer_undo == &my_buffer[BUFFER_SIZE])
			buffer_undo = my_buffer;


		buffer_end = buffer_undo;
		*buffer_end = added_state;

		buffer_count -= dif;
		buffer_count +=2;
	}
	buffer_end++;
	if(buffer_end == &my_buffer[BUFFER_SIZE])
			buffer_end = my_buffer;

	buffer_undo++;
	if(buffer_undo == &my_buffer[BUFFER_SIZE])
			buffer_undo = my_buffer;

	if(buffer_start == buffer_end && buffer_count > 0)
		{
			buffer_start++;
			if(buffer_start == &my_buffer[BUFFER_SIZE])
				buffer_start = my_buffer;
		}

}

Game_state get_from_buffer_end()
{

	if(buffer_count < 2 || buffer_undo == buffer_start)
	{
		chess_ui.print_message(17); //error
	}
	else
	{
		if(buffer_undo == buffer_end) //prvi undo mora iti še preko trenutnega stanja
		{
			if(buffer_undo == my_buffer)
				buffer_undo = &my_buffer[BUFFER_SIZE];
			buffer_undo--;

			if(buffer_undo == my_buffer)
				buffer_undo = &my_buffer[BUFFER_SIZE];
			buffer_undo--;

		}
		else
		{
			if(buffer_undo == my_buffer)
				buffer_undo = &my_buffer[BUFFER_SIZE];
			buffer_undo--;
		}
	}
	return *buffer_undo;
}

Game_state get_next_state()
{
	if(buffer_end - buffer_undo < 2)//če sta poravnana ali če imata le en korak med njima (end kaže na prazno polje)
	{
		chess_ui.print_message(17); //error
	}
	else
	{
		buffer_undo++;
		if(buffer_undo == &my_buffer[BUFFER_SIZE])
			buffer_undo = my_buffer;
	}

	return *buffer_undo;
}

bool buffer_is_empty()
{
	return buffer_count == 0;
}

void buffer_make_empty()
{
	buffer_start = my_buffer;
	buffer_end = my_buffer;
	buffer_undo = my_buffer;
	buffer_count = 0;
}
