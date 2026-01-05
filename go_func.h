#pragma once
#include "raylib.h"

#define LIGHTBEIGE  ((Color){ 241, 226, 161, 255 })
#define DARKPINK    ((Color){ 205, 59, 144, 255 } )
#define LIGHTGREEN  ((Color){ 27, 255, 75, 255 }  )
#define DDARKGREEN  ((Color){ 10, 80, 20, 255 }   )
#define DARKGRAY2   ((Color){ 40, 40, 40, 64 }    )
#define SEESLBLACK  ((Color){ 0, 0, 0, 127 }      )
#define SEESLWHITE  ((Color){ 255, 255, 255, 127 })
#define CURSOR_B    ((Color){ 0, 0, 0, 96 }       )
#define CURSOR_W    ((Color){ 255, 255, 255, 96 } )
#define DARKYELLOW  ((Color){ 255, 255, 0, 64 }   )

#define WINDOW_SIZE 860
#define TEMP_MARGIN 30

// 碁盤交点の要素
#define EMPTy 0		// 空点		
#define BLACk 1		// 黒石
#define WHITe 2		// 白石
#define WALL3 3		// 盤外

#define BArea 4		// 黒地マーク
#define WArea 5		// 白地マーク

#define BDead 6		// 黒：死んだ石のマーク
#define WDead 7		// 白：死んだ石のマーク

#define NUM_ELEM 8	// 要素数

#define NUM_BOARD_SIZE 16					// 碁盤サイズの数
#define B_SIZE_MAX 19						// 碁盤の最大路数
#define WIDTH_MAX (B_SIZE_MAX + 2)			// 枠を含めた最大横幅
#define BOARD_MAX (WIDTH_MAX * WIDTH_MAX)	// 枠を含めた盤面配列サイズ

#define PASS	0
#define RESIGN	BOARD_MAX
#define KEEP_THINKING		-1
#define ERR_KEEP_THINKING	-999

#define SLOW_MOVE 0
#define MIDL_MOVE 1
#define FAST_MOVE 2
#define NUM_MOVE_SPEED 3
extern const float move_interval[NUM_MOVE_SPEED];
extern float current_interval;
extern float move_timer;
extern int   move_speed_switch;

#define NUM_AUTOGAMES		100

extern const int FILL_EYE_ERR;
extern const int FILL_EYE_O;

// Player type : 思考種類
#define HUMAN 0
#define COMPUTER 1
#define HAMBOT 2
#define TYPE_NUM 3
// 思考別の一手を決める関数
int youMove();
int comMove();
int hamMove();
typedef int (*ThinkFunc)();	// 思考関数を入れる関数ポインタ
extern ThinkFunc movesByType[TYPE_NUM];

extern Sound seBtnClick;
extern Sound sePikon;
extern Sound sePutStone;
extern bool is_mVolume;

// 画像
//Texture2D title_background;
extern Texture2D typeImg[TYPE_NUM];
extern Texture2D stoneImg[3];

extern int board[BOARD_MAX];
extern int check_board[BOARD_MAX];		// 検索済みフラグ用
extern int check_group[BOARD_MAX];		// 検索済み連フラグ用
extern int list_random_z[BOARD_MAX];	// init_boardで座標zをランダム入れる配列
extern int count[NUM_ELEM];				// 要素別のカウンター ex : count[BLACk]は黒石のカウンター

//
extern int cell_size;
extern int board_margin;
extern int tesuu;
extern int hama[2];
extern int kifu[1000];
extern int ko_z;
extern int all_playouts;
extern int last_move_was_pass;			// パスした石色
extern bool game_ended_by_pass;			// 連続パスで終了したか
extern bool ended_by_moves_lim;			// 手数制限により終了
extern int game_resigned;				// 投了した石色
extern int winner;						// 勝者


// 星の位置
extern const int starPos[ /*盤サイズindex*/][3];

// 置き碁の黒石位置
extern const int hcStonePos[10][9];

// 対局設定項目の選択肢 と インデックスの初期値
extern const char* bdsize[NUM_BOARD_SIZE];
extern int size_Idx;
extern char* player_type[TYPE_NUM];
extern int pBtype_Idx;
extern int pWtype_Idx;
extern const char* handicap_stones[9];
extern int         handicap_Idx;
extern int		   backup_hcIdx;
extern const char* komi_list[3];
extern int         komi_Idx;
extern const char* move_speed[NUM_MOVE_SPEED];
extern int		   move_interval_Idx;

// 方向をfor文で回す用
extern int dir4[4] /* = { +1, -1, +WIDTH, -WIDTH }*/;	// 右、左、下、上への移動量
extern int dirX[4] /* = { +WIDTH, -WIDTH, -1, +1 }*/;	// dir4 に dirX を加算することで斜め方向への移動量となる

// 方向を個別に扱う用
extern int toR;		// 右
extern int toL;		// 左
extern int toD;		// 下
extern int toU;		// 上

extern int toR_toD;	// 右下
extern int toL_toU;	// 左上
extern int toD_toL;	// 左下
extern int toU_toR;	// 右上


// プレイヤー
typedef struct {
	int type;
	char* name;
	Texture2D* img;
	ThinkFunc move;
} Player;
extern Player playerB;
extern Player playerW;

// 対局設定
typedef struct {
	int size;			// 碁盤の路数 9,13,19
	int width;			// = size + 2
	int bd_max;			// = width * width
	int moves_limit;	// bd_max+width*4 終了までの手数

	int handicap_stones;
	float komi;
	int turn_Color;
	int cur_move;

	int score;
	int b_score;
	int w_score;
	int b_stones;
	int w_stones;
	int b_dead_stones;
	int w_dead_stones;
	int winner;

	Player curPlayer;
} GameInfo;
extern GameInfo gm;

// シーン管理用
typedef void (*SceneFunc)();
extern SceneFunc current_scene;
void title_scene();
void init_scene();
void play_scene();
void end_scene();

void init_board();                          // 盤面配列を0(empty)で埋める
void init_players();
void init_game();
void set_handicap_stones();
void draw_board();
void draw_stones();
void draw_game_info();
void draw_players();
int get_z(int x, int y);
int get_y(int z);
int get_x(int z);
int get_xy_disp(int z);
int flip_color(int color);
void kesu(int tz, int color);			// 石を消す
void change_elements(int z, int elem);	// 座標zのグループ要素を 指定要素elem に変更する
void switch_turn();
int get_board_coord(int mouseX);
int get_grid2screen_point(int board_coord);
void DrawCursorStone();
void draw_move_mark(int z);
void draw_result();

bool BtnF(int x, int y, int w, int h, const char* label, const char* label2, int font_size, Color fgColor, Color bgColor);
bool BtnV(int x, int y, const char* label, int font_size, Color fgColor, Color bgColor);
void Label(int x, int y, const char* label, int font_size, Color fgColor, Color bgColor);

void clear_count();
void clear_check_board();
void elem_counter(int tz, int elem, int* count, int num_elems);
bool is_near_center(int z);
bool is_edge(int z);
bool is_1_2_line(int z);
bool is_stone_1around(int z, int color);
bool is_on_board(int x, int y);
int get_num_stones(int x1, int y1, int x2, int y2, int color);

void count_dame_sub(int tz, int color, int* p_dame, int* p_ishi);
void count_dame(int tz, int* p_dame, int* p_ishi);
void count_elem_sub(int tz, int* elem, int* num);
int count_elem(int tz);
int count_score2();
int move(int tz, int color, int fill_eye_err);
int get_random_z(int color);
int get_z_from_random_list(int color);