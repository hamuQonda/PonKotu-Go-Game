#include "go_func.h"

const float move_interval[] = { 1.0f, 0.30f, 0.0333f };
int move_interval_Idx = MIDL_MOVE;
float move_timer = 0.0f;
float current_interval = 0.0f;
int   move_speed_switch = 1;

int board[BOARD_MAX] = { 0 };
int check_board[BOARD_MAX] = { 0 };		// 検索済みフラグ用
int check_group[BOARD_MAX] = { 0 };		// 検索済み連フラグ用
int list_random_z[BOARD_MAX] = { 0 };	// init_boardで座標zをランダム入れる配列
int count[NUM_ELEM] = { 0 };			// 要素別のカウンター ex : count[BLACk]は黒石のカウンター

int dir4[4] = { 0 }; /* = { +1, -1, +WIDTH, -WIDTH }*/;	// 右、左、下、上への移動量
int dirX[4] = { 0 };/* = { +WIDTH, -WIDTH, -1, +1 }*/;	// dir4 に dirX を加算することで斜め方向への移動量となる

// 方向を個別に扱う用
int toR = 0;		// 右
int toL = 0;		// 左
int toD = 0;		// 下
int toU = 0;		// 上

int toR_toD = 0;	// 右下
int toL_toU = 0;	// 左上
int toD_toL = 0;	// 左下
int toU_toR = 0;	// 右上

int cell_size = 40;
int board_margin = 40;
int tesuu = 0;
int hama[2] = { 0 };
int kifu[1000] = { 0 };
int ko_z = 0;
int all_playouts = 0;
int last_move_was_pass = EMPTy;                 // パスした石色
bool game_ended_by_pass = false;                // 連続パスで終了したか
bool ended_by_moves_lim = false;				// 手数制限により終了
int game_resigned = EMPTy;						// 投了した石色
int winner = EMPTy;                             // 勝者
const int FILL_EYE_ERR = 1;
const int FILL_EYE_OK = 0;

Player playerB;
Player playerW;

Sound seBtnClick;
Sound sePikon;
Sound sePutStone;
bool is_mVolume = true;

Texture2D typeImg[TYPE_NUM] = { 0 };
Texture2D stoneImg[3] = { 0 };

SceneFunc current_scene = title_scene;

ThinkFunc movesByType[TYPE_NUM] = { youMove, comMove, hamMove };
GameInfo gm;

// 星の位置
const int starPos[NUM_BOARD_SIZE
			 /*盤サイズindex*/][3] = {
			 /*  4          */{ 1, 1, 2},   // 
			 /*  5          */{ 2, 2, 2},   // 
			 /*  6          */{ 1, 1, 4},   // 
			 /*  7          */{ 1, 3, 5},   // 
			 /*  8          */{ 2, 2, 5},   // 
			 /*  9路盤      */{ 2, 4, 6},   // 
			 /* 10          */{ 3, 3, 6},   // 
			 /* 11          */{ 2, 5, 8},   // 
			 /* 12          */{ 3, 3, 8},   // 
			 /* 13路盤      */{ 3, 6, 9},   // 
			 /* 14          */{ 3, 3, 10},   // 
			 /* 15          */{ 3, 7, 11},   // 
			 /* 16          */{ 3, 3, 12},   // 
			 /* 17          */{ 3, 8, 13},   // 
			 /* 18          */{ 3, 3, 14},   // 
			 /* 19路盤      */{ 3, 9, 15}
};

// 置き碁の黒石位置
const int hcStonePos[10][9] = {
	{   // 置き石無し
		0,0,0,
		0,0,0,
		0,0,0
	},
	{   // 1子局：ダミー
		0,0,0,
		0,1,0,
		0,0,0
	},
	{   // 2子局
		0,0,1,
		0,0,0,
		1,0,0
	},
	{   // 3子局
		0,0,1,
		0,0,0,
		1,0,1
	},
	{   // 4子局
		1,0,1,
		0,0,0,
		1,0,1
	},
	{   // 5子局
		1,0,1,
		0,1,0,
		1,0,1
	},
	{   // 6子局
		1,0,1,
		1,0,1,
		1,0,1
	},
	{   // 7子局
		1,0,1,
		1,1,1,
		1,0,1
	},
	{   // 8子局
		1,1,1,
		1,0,1,
		1,1,1
	},
	{   // 9子局
		1,1,1,
		1,1,1,
		1,1,1
	}
};

// 対局設定項目の選択肢 と インデックスの初期値
const char* bdsize[] = { "4", "5", "6","7", "8", "9","10", "11", "12","13", "14", "15","16", "17", "18","19" };
int size_Idx = 3;
char* player_type[TYPE_NUM] = { "You","Com","Ham" };
int pBtype_Idx = HUMAN;
int pWtype_Idx = COMPUTER;
const char* handicap_stones[9] = { "NO", "2 stones", "3 stones", "4 stones", "5 stones", "6 stones", "7 stones", "8 stones", "9 stones" };
int handicap_Idx = 0;
int backup_hcIdx = 0;
const char* komi_list[3] = { "7.5","6.5","0.5" };
int komi_Idx = 0;
const char* move_speed[NUM_MOVE_SPEED] = { "SLOW","MIDL","FAST" };


// 盤面を ０で、盤外は３で初期化。ランダム座標配列を初期化
void init_board()
{
	for (int i = 0; i < gm.bd_max; i++) board[i] = 3;	// 盤外値3で埋める
	for (int y = 0; y < gm.size; y++) for (int x = 0; x < gm.size; x++) {
		board[get_z(x, y)] = 0;						// 盤面を空点値0でクリア
	}

	// board_random_z配列に座標zをランダム入れる
	int z_list[BOARD_MAX];	// 左上から順に並んだ座標リスト
	int index = 0;
	for (int y = 0; y < gm.size; y++) for (int x = 0; x < gm.size; x++) {
		z_list[index++] = get_z(x, y);
	}
	for (int i = 0; i < index; i++) {
		int r = GetRandomValue(0, index - 1);
		list_random_z[i] = z_list[r];	// z_listからランダムに選択
		z_list[r] = z_list[index - i - 1];	// 選択済みは末尾値を入れて削除
	}

}

// プレイヤーを初期化
void init_players() {
	// 黒プレイヤー
	playerB.type = HUMAN;
	playerB.name = "You";
	playerB.img = &typeImg[playerB.type];
	playerB.move = movesByType[HUMAN];

	// 白プレイヤー
	playerW.type = COMPUTER;
	playerW.name = "Com";
	playerW.img = &typeImg[playerW.type];
	playerW.move = movesByType[COMPUTER];
}

// ゲームを初期化
void init_game() {
	tesuu = 0;
	last_move_was_pass = EMPTy;
	game_ended_by_pass = false;
	game_resigned = EMPTy;

	gm.size = GetRandomValue(4, 8);	// タイトル画面用
	gm.handicap_stones = 0;
	gm.komi = 6.5f;
	gm.turn_Color = BLACk;
	gm.cur_move = -999;
	gm.winner = EMPTy;
	gm.curPlayer = playerB;
	gm.b_score = 0;
	gm.w_score = 0;
	gm.score = 0;

	cell_size = ((WINDOW_SIZE - 2 * TEMP_MARGIN) / gm.size);
	board_margin = ((WINDOW_SIZE - cell_size * gm.size) / 2);
}

// 置き碁の黒石を盤面にセット
void set_handicap_stones() {

	if (gm.handicap_stones == 0) return;
	
	// 4,5,6路の碁盤
	if (gm.size <= 6) {
		int pos = gm.size - 2;
		board[get_z(pos, 1)] = BLACk;
		board[get_z(1, pos)] = BLACk;
	}

	// 奇数サイズ碁盤
	else if (gm.size % 2) {
		int bdIdx = (gm.size - 4);
		for (int i = 0; i < 9; i++) {
			int x = starPos[bdIdx][i % 3];
			int y = starPos[bdIdx][i / 3];
			board[get_z(x, y)] = hcStonePos[gm.handicap_stones][i] ? BLACk : EMPTy;
		}
	}

	// 偶数サイズ碁盤
	else {
		int edge_pos = 0;
		if (gm.size < 9) edge_pos = 1;
		else if (gm.size < 13) edge_pos = 2;
		else edge_pos = 3;
		
		for (int i = 0; i < 9; i++) {
			int x = 0; int y = 0;
			int px = i % 3;
			int py = i / 3;

			if (px == 0) x = edge_pos;
			if (px == 1) x = gm.size / 2;
			if (px == 2) x = gm.size - edge_pos - 1;

			if (py == 0) y = edge_pos;
			if (py == 1) y = gm.size / 2;
			if (py == 2) y = gm.size - edge_pos - 1;

			if (i == 1) x--;
			if (i == 5) y--;

			board[get_z(x, y)] = hcStonePos[gm.handicap_stones][i] ? BLACk : EMPTy;
		}
	}
}

// 碁盤の描画
void draw_board(void) {
	// 碁盤インデックス
	//int bdIdx = (gm.size - 1) / 9;
	int bdIdx = (gm.size - 4);
	// 碁盤外枠
	int frame_pos = board_margin - (5 - bdIdx/9);
	int frame_size = gm.size * cell_size + (5 - bdIdx/9) * 2;
	DrawRectangleRounded((Rectangle) { (float)frame_pos + 4, (float)frame_pos + 4, (float)frame_size + 6, (float)frame_size + 6 }, 0.025f, 8, DARKGRAY2);
	DrawRectangleRounded((Rectangle) { (float)frame_pos, (float)frame_pos, (float)frame_size, (float)frame_size }, 0.02f, 8, GOLD);

	int lStart = board_margin + cell_size / 2;
	int lEnd = lStart + cell_size * (gm.size - 1);
	// 線の陰
	for (int i = 0; i < gm.size; i++) {
		DrawLine(lStart + 1, lStart + i * cell_size + 1, lEnd + 1, lStart + i * cell_size + 1, BLACK);
		DrawLine(lStart + i * cell_size + 1, lStart + 1, lStart + i * cell_size + 1, lEnd + 1, BLACK);
	}
	for (int i = 0; i < 3; i++) {    // 星
		DrawCircle(lStart + cell_size * starPos[bdIdx][0] + 1, lStart + cell_size * starPos[bdIdx][i] + 1, 3.0f, BLACK);
		DrawCircle(lStart + cell_size * starPos[bdIdx][1] + 1, lStart + cell_size * starPos[bdIdx][i] + 1, 3.0f, BLACK);
		DrawCircle(lStart + cell_size * starPos[bdIdx][2] + 1, lStart + cell_size * starPos[bdIdx][i] + 1, 3.0f, BLACK);
	}
	// 線の陽
	for (int i = 0; i < gm.size; i++) {
		DrawLine(lStart, lStart + i * cell_size, lEnd, lStart + i * cell_size, DARKBLUE);
		DrawLine(lStart + i * cell_size, lStart, lStart + i * cell_size, lEnd, DARKBLUE);
	}
	for (int i = 0; i < 3; i++) {    // 星
		DrawCircle(lStart + cell_size * starPos[bdIdx][0] - 1, lStart + cell_size * starPos[bdIdx][i] - 0, 2.8f, DARKBLUE);
		DrawCircle(lStart + cell_size * starPos[bdIdx][1] - 1, lStart + cell_size * starPos[bdIdx][i] - 0, 2.8f, DARKBLUE);
		DrawCircle(lStart + cell_size * starPos[bdIdx][2] - 1, lStart + cell_size * starPos[bdIdx][i] - 0, 2.8f, DARKBLUE);
	}
}

// 盤面の石を描画
void draw_stones() {
	//int lStart = board_margin + cell_size / 2;
	for (int x = 0; x < gm.size; x++) {
		for (int y = 0; y < gm.size; y++) {
			int z = get_z(x, y);
			int e = board[z];
			if (e == WHITe || e == BLACk || e == BDead || e == WDead) {
				int color = (e == BDead || e == WDead) ? e - 5 : e;
				//DrawCircle(lStart + x * cell_size + 5, lStart + y * cell_size + 5, (float)cell_size / 2 - 1, (Color) { 0, 0, 0, 24 });    // 陰
				DrawTextureEx(stoneImg[color], (Vector2) { (float)board_margin + x * cell_size + 5, (float)board_margin + y * cell_size + 5 },
					0.0f, (float)cell_size / (float)stoneImg[color].width, (Color) { 0, 0, 0, 32 });
				DrawTextureEx(stoneImg[color], (Vector2) { (float)board_margin + x * cell_size, (float)board_margin + y * cell_size },
					0.0f, (float)cell_size / (float)stoneImg[color].width, (Color) { 225, 225, 225, 255 });
			}
			if (e == BDead || e == WDead || e == BArea || e == WArea) {
				DrawPoly((Vector2) { (float)board_margin + x * cell_size + cell_size / 2, (float)board_margin + y * cell_size + cell_size / 2 },
					4, (float)cell_size / 5.0f, 0.0f, (e == BDead || e == WArea) ? SEESLWHITE : SEESLBLACK);
			}
		}
	}
}

// 対局情報を表示
void draw_game_info() {
	DrawText(TextFormat("handicap stones : %d", gm.handicap_stones), 870, 4, 20, LIGHTGRAY);
	DrawText(TextFormat("komi : %1.1f", gm.komi), 1100, 4, 20, LIGHTGRAY);
}

// プレイヤーを表示
void draw_players() {
	// black player
	DrawCircle(890, 70, 20, BLACK);
	DrawText(playerB.name, 920, 50, 40, GREEN);
	DrawTextureV(*playerB.img, (Vector2) { 884, 104 }, DARKYELLOW);
	DrawTextureV(*playerB.img, (Vector2) { 880, 100 }, WHITE);
	if ((last_move_was_pass == BLACk) || game_ended_by_pass)
		DrawText("PASS", 905, 240, 30, RED);
	//if (gm.turn_Color == WHITe && game_resigned)
	if (game_resigned == BLACk)
		DrawText("RESIGN", 885, 240, 30, RED);

	// white player
	DrawCircle(1070, 70, 20, WHITE);
	DrawText(playerW.name, 1100, 50, 40, GREEN);
	DrawTextureV(*playerW.img, (Vector2) { 1064, 104 }, DARKYELLOW);
	DrawTextureV(*playerW.img, (Vector2) { 1060, 100 }, WHITE);
	if ((last_move_was_pass == WHITe) || game_ended_by_pass)
		DrawText("PASS", 1085, 240, 30, RED);
	//if (gm.turn_Color == BLACk && game_resigned)
	if (game_resigned == WHITe)
		DrawText("RESIGN", 1065, 240, 30, RED);

}

// 碁盤座標(x,y)から 盤外を含む一次元座標 z を得る
int get_z(int x, int y) {
	return (y + 1) * gm.width + (x + 1);	// 0<= x <=8, 0<= y <=8
}
// 1次元座標から碁盤y座標を得る
int get_y(int z) {
	return z / gm.width;
}
// 1次元座標から碁盤x座標を得る
int get_x(int z) {
	return z - get_y(z) * gm.width;
}
//int get81(int tz)
// 1次元座標値z から 表示用の数値 x*100+y を返す
int get_xy_disp(int z) {
	if (z == 0) return 0;
	int y = z / gm.width;	 	// 座標をx*10+yに変換。表示用。
	int x = z - y * gm.width;	// 106 = 9*11 + 7 = (x,y)=(7,9) -> 709
	return x * 100 + y;			// 11路以上：x*100+y
}
// 石の色を反転させる
int flip_color(int color) {     // color=1(BLACk) => 2(WHITe)
	return 3 - color;           // color=2(WHITe) => 1(BLACk)
}

// 石を消す
void kesu(int tz, int color) {
	board[tz] = EMPTy;
	if (board[tz + toU] == color) kesu(tz + toU, color);
	if (board[tz + toD] == color) kesu(tz + toD, color);
	if (board[tz + toL] == color) kesu(tz + toL, color);
	if (board[tz + toR] == color) kesu(tz + toR, color);
}

// 座標zのグループ要素 elem1 を elem2 に変更する
void change_elements(int tz, int elem2) {
	int elem1 = board[tz];   // 連の要素
	board[tz] = elem2;   // 新しい
	if (board[tz + toU] == elem1) change_elements(tz + toU, elem2);
	if (board[tz + toD] == elem1) change_elements(tz + toD, elem2);
	if (board[tz + toL] == elem1) change_elements(tz + toL, elem2);
	if (board[tz + toR] == elem1) change_elements(tz + toR, elem2);
}

void switch_turn() {
	gm.turn_Color = flip_color(gm.turn_Color);
	gm.curPlayer = (gm.turn_Color == BLACk) ? playerB : playerW;

}

// マウス座標から碁盤座標を得る
int get_board_coord(int mouseX) {
	return (mouseX - board_margin + cell_size) / cell_size - 1;
}

// 碁盤座標からスクリーン座標を得る
int get_grid2screen_point(int board_coord) {
	return board_margin + cell_size / 2 + board_coord * cell_size;
}

// 碁盤上に石カーソルを表示
void DrawCursorStone() {
	int x = get_board_coord(GetMouseX());
	int y = get_board_coord(GetMouseY());
	//	if (is_on_board(x, y) && board[get_z(x, y)] == EMPTy) {
	int cx = get_grid2screen_point(x);
	int cy = get_grid2screen_point(y);
	DrawCircle(cx, cy, cell_size * 0.3f, gm.turn_Color == BLACk ? CURSOR_B : CURSOR_W);
	//	}
}

// 最新の着手マークを表示
void draw_move_mark(int z) {
	int x = get_x(z) - 1;
	int y = get_y(z) - 1;
	int cx = get_grid2screen_point(x);
	int cy = get_grid2screen_point(y);
	DrawCircle(cx, cy, cell_size * 0.15f, PINK);
	DrawText(TextFormat("%d", tesuu), cx - 5, cy - 8, 15, BLUE);

}

// 勝敗結果を表示
void draw_result() {
	DrawCircle(920, 470, 40, gm.winner == BLACk ? BLACK : WHITE);
	DrawText(gm.winner == BLACk ? playerB.name : playerW.name, 980, 430, 90, GOLD);
	DrawText("Winner", 890, 520, 90, YELLOW);

	DrawText("Counting stones and territory", 880, 285, 20, GRAY);
	DrawText(TextFormat("stone : %3d", gm.b_stones), 920, 310, 20, LIGHTGRAY);
	DrawText(TextFormat("stone : %3d", gm.w_stones), 1080, 310, 20, LIGHTGRAY);
	DrawText(TextFormat("space : %3d", gm.b_score), 920, 330, 20, LIGHTGRAY);
	DrawText(TextFormat("space : %3d", gm.w_score), 1080, 330, 20, LIGHTGRAY);
	DrawText(TextFormat("komi : %3.1f", gm.komi), 1100, 350, 20, LIGHTGRAY);
	DrawText("--------------------------------", 880, 365, 20, GRAY);
	DrawText("total", 865, 380, 20, GRAY);
	DrawText(TextFormat("Black : %3d", gm.b_stones + gm.b_score), 925, 380, 20, LIGHTGRAY);
	DrawText(TextFormat("White : %3.1f", gm.w_stones + gm.w_score + gm.komi), 1080, 380, 20, LIGHTGRAY);

}

// ボタン表示関数：押されたら true  - サイズは引数指定、テキスト長さに注意
bool BtnF(int x, int y, int w, int h, const char* label_left, const char* label_right, int font_size, Color fgColor, Color bgColor) {
	Vector2 mouse = GetMousePosition();
	int left_txtWidth = MeasureText(label_left, font_size);
	int left_text_margin_x = font_size / 2;
	int right_text_margin_x = font_size + left_txtWidth;
	int marginY = (h - font_size) / 2;
	Rectangle btn_box = { (float)x, (float)y, (float)w, (float)h };
	bool hovered = CheckCollisionPointRec(mouse, btn_box);

	DrawRectangleRounded(btn_box, 0.4f, 8, hovered ? bgColor : DARKGRAY2);
	DrawText(label_left, x + left_text_margin_x, y + marginY, font_size, hovered ? WHITE : LIGHTGRAY);
	DrawText(label_right, x + right_text_margin_x, y + marginY, font_size, fgColor);

	bool is_BtnF_Click = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
	return is_BtnF_Click;
}
/*
*/

// ボタン表示関数：押されたら true  - サイズはテキストにフィット
bool BtnV(int x, int y, const char* label, int font_size, Color fgColor, Color bgColor) {
	Vector2 mouse = GetMousePosition();
	int txtWidth = MeasureText(label, font_size);
	int marginX = font_size / 2;
	int marginY = font_size / 2;
	Rectangle btn_box = { (float)x, (float)y, (float)txtWidth + marginX * 2, (float)font_size + marginY * 2 };
	bool hovered = CheckCollisionPointRec(mouse, btn_box);

	DrawRectangleRounded(btn_box, 0.4f, 8, hovered ? bgColor : DARKGRAY2);
	DrawText(label, x + marginX, y + marginY, font_size, fgColor);

	bool is_BtnF_Click = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
	return is_BtnF_Click;
}

// ラベル表示関数
void Label(int x, int y, const char* label, int font_size, Color fgColor, Color bgColor) {
	int txtWidth = MeasureText(label, font_size);
	int marginX = font_size / 2;
	int marginY = font_size / 2;
	Rectangle label_box = { (float)x, (float)y, (float)txtWidth + marginX * 2, (float)font_size + marginY * 2 };
	DrawRectangleRounded(label_box, 0.4f, 8, bgColor);
	DrawText(label, x + marginX, y + marginY, font_size, fgColor);
}

// カウンタ配列をクリア
void clear_count() {
	for (int elem = 0; elem < NUM_ELEM; elem++) count[elem] = 0;
}

// チェック用ボード配列をクリア
void clear_check_board() {
	for (int z = 0; z < gm.bd_max; z++)	check_board[z] = 0;
}

// 指定座標tzの（連の要素数を数え 且つ 連周囲の要素数を数える） 再帰関数
// 4方向を調べて、他の要素なら他の要素をカウント、指定連の要素なら再帰でカウント。
void elem_counter(int tz, int elem, int* count, int num_elems)
{
	// 再帰しない条件
	if (check_board[tz]) return;				// チェック済み
	if (elem < 0 || elem >= num_elems) return;	// 未知の要素
	if (board[tz] != elem) return;				// 連の要素ではない

	// 指定連の要素をカウントup
	count[elem]++;
	check_board[tz] = elem + 1;	// この位置の要素はチェック済み(空点(0値)でもチェック済みになるように+1している）

	// 隣の 連ではない要素 をカウントして 再帰呼び出し
	for (int i = 0; i < 4; i++) {		// 4方向を調べる
		int z = tz + dir4[i];
		if (check_board[z]) continue;	// チェック済みなら次の方向へ
		if (board[z] != elem) {
			count[board[z]]++;			// (連ではない要素)をカウントup
			check_board[z] = -1;		// この位置はカウント済み(-1)に
		}
		elem_counter(z, elem, count, num_elems);  // 再帰で隣を調べる
	}
}

// 座標zは中央付近？
bool is_near_center(int z)
{
	int cx = gm.size / 2 - (get_x(z) - 1);
	int cy = gm.size / 2 - (get_y(z) - 1);
	int dc = gm.size >= 13 ? gm.size / 4 - 1 : 0;   // 中央付近とみなす天元からの距離
	return (cx * cx <= dc * dc) && (cy * cy <= dc * dc);
}

// 座標zは盤端か？
bool is_edge(int z) {
	return (board[z - 1] == WALL3 || board[z + 1] == WALL3 || board[z - gm.width] == WALL3 || board[z + gm.width] == WALL3);
}

// 座標zは二線か？
bool is_1_2_line(int z)
{
	int x = get_x(z) - 1;
	int y = get_y(z) - 1;
	return (x < 2 || x >= gm.size - 2 || y < 2 || y >= gm.size - 2);
}

// 座標zの周囲1に指定colorの石があるか
bool is_stone_1around(int z, int color) {

	if (board[z + toR] == color) return true;
	if (board[z + toL] == color) return true;
	if (board[z + toD] == color) return true;
	if (board[z + toU] == color) return true;

	if (board[z + toR_toD] == color) return true;
	if (board[z + toL_toU] == color) return true;
	if (board[z + toD_toL] == color) return true;
	if (board[z + toU_toR] == color) return true;

	return false;
}

// --- 座標の盤面範囲チェック
bool is_on_board(int x, int y) {
	return x >= 0 && x < gm.size && y >= 0 && y < gm.size;
}

// 範囲内の指定色の石数を得る
int get_num_stones(int x1, int y1, int x2, int y2, int color)
{
	int num = 0;
	for (int y = y1; y <= y2; y++) for (int x = x1; x <= x2; x++) {
		num += (board[get_z(x, y)] == color ? 1 : 0);
	}
	return num;
}

// ダメと石数を数える再帰関数
// 4方向を調べて、空白だったら+1、自分の石なら再帰で。相手の石、壁ならそのまま。
void count_dame_sub(int tz, int color, int* p_dame, int* p_ishi)
{
	check_board[tz] = 1;     // この位置(石)は検索済み
	(*p_ishi)++;             // 石の数
	for (int i = 0; i < 4; i++) {
		int z = tz + dir4[i];      // 4方向を調べる
		if (check_board[z]) continue;
		if (board[z] == 0) {
			check_board[z] = 1;  // この位置(空点)はカウント済みに
			(*p_dame)++;         // ダメの数
		}
		if (board[z] == color) count_dame_sub(z, color, p_dame, p_ishi);  // 未探索の自分の石
	}
}

// 位置 tz におけるダメの数と石の数を計算。
void count_dame(int tz, int* p_dame, int* p_ishi)
{
	*p_dame = *p_ishi = 0;
	for (int i = 0; i < gm.bd_max; i++) check_board[i] = 0;
	count_dame_sub(tz, board[tz], p_dame, p_ishi);
}

// 4方向を調べて、同じ要素なら+1、自分の石なら再帰で。相手の石、壁ならそのまま。
void count_elem_sub(int tz, int* elem, int* num)
{
	check_board[tz] = 1;			// この位置は検索済み
	(*num)++;						// 要素数をカウント
	for (int i = 0; i < 4; i++) {	// 4方向を調べる
		int z = tz + dir4[i];
		if (check_board[z] || board[z] != *elem) continue;	// チェック済み、連要素ではない・・・次の方向へ
		count_elem_sub(z, elem, num);  // 未探索の連要素
	}
}

// 位置 tz の連の要素数を計算。
int count_elem(int tz)
{
	int num = 0;
	int elem = board[tz];
	for (int i = 0; i < gm.bd_max; i++) check_board[i] = 0;
	count_elem_sub(tz, &elem, &num);
	return num;
}

// 地を数えて勝ちか負けかを返す
int count_score2()
{

	// 死に石があるか調べて 死に石をマークする
	int dame, ishi;
	for (int i = 0; i < gm.bd_max; i++) check_group[i] = 0;	// 連のチェックボードをクリア

	for (int y = 0; y < gm.size; y++) {
		for (int x = 0; x < gm.size; x++) {
			int z = get_z(x, y);
			if (board[z] == EMPTy || check_group[z]) continue;

			count_dame(z, &dame, &ishi);

			// count_dameでチェック済みを 連のチェックボードに追加
			for (int j = 0; j < gm.bd_max; j++) check_group[j] += check_board[j];

			// ダメが１未満は死に石
			if (dame <= 1) {
				if (board[z] == BLACk) gm.b_dead_stones += ishi;
				if (board[z] == WHITe) gm.w_dead_stones += ishi;
				int dead_mark = (board[z] == BLACk) ? BDead : WDead;
				change_elements(z, dead_mark);
			}
		}
	}

	/*
	int board_copy[BOARD_MAX];	// 現局面を保存
	memcpy(board_copy, board, sizeof(board));
	int ko_z_copy = ko_z;

	// 死に石を消してから・・・
	for (int y = 0; y < gm.size; y++) for (int x = 0; x < gm.size; x++) {
		int tz = get_z(x, y);
		if (board[tz] == BDead || board[tz] == WDead) board[tz] = EMPTy;
	}
	*/

	// 石と地をカウントする
	int num[NUM_ELEM] = { 0 };	// num[EMPTy]:空点数、num[BLACk]:黒石数、num[WHITe]:白石数、num[WALL3]:盤外数
	for (int i = 0; i < gm.bd_max; i++) check_group[i] = 0;	// 連のチェックボードをクリア
	for (int y = 0; y < gm.size; y++) {
		for (int x = 0; x < gm.size; x++) {
			int z = get_z(x, y);
			int elem = board[z];
			num[elem]++;
			if (elem != EMPTy) continue;			// 空点でなければスキップ
			if (check_group[z] > 0) continue;	// チェック済み空点はスキップ

			// 以下、地の空点数と その周囲の要素を要素別にカウントする
			clear_count();
			clear_check_board();
			elem_counter(z, elem, count, NUM_ELEM);
			// elem_counterでチェック済みを 連のチェックボードに追加
			for (int j = 0; j < gm.bd_max; j++) check_group[j] += check_board[j];

			if ((count[BLACk] || count[WDead]) && count[WHITe] == 0) {	// 黒石だけに囲まれている
				change_elements(z, BArea);	// 黒地マーク
				gm.b_score += count[EMPTy];
				gm.score += count[EMPTy];
			}
			if ((count[WHITe] || count[BDead]) && count[BLACk] == 0) {	// 白石だけに囲まれている
				change_elements(z, WArea);	// 白地マーク
				gm.w_score += count[EMPTy];
				gm.score -= count[EMPTy];
			}
		}
	}
	gm.b_score += num[WDead];
	gm.w_score += num[BDead];

	gm.b_stones = num[BLACk];
	gm.w_stones = num[WHITe];
	gm.score += (num[BLACk] - num[WHITe] + num[WDead] - num[BDead]);

	double final_score = gm.score - gm.komi;
	int win = final_score > 0 ? BLACk : WHITe;

	/*
	memcpy(board, board_copy, sizeof(board));  // 局面を戻す
	ko_z = ko_z_copy;
	*/

	return win;
}

// 石を置く。合法なら 0:false 、非合法なら 1以上の値:true が返る。playoutではfill_eye_err = 1
int move(int tz, int color, int fill_eye_err)
{
	if (tz == PASS) { ko_z = 0; return 0; }	// パスの場合
	if (board[tz] != EMPTy) { return 4; }		// 既に石がある

	// 四方が空点なら即置ける（高速化）
	if ((board[tz + toR] + board[tz+toL] + board[tz + toU] + board[tz + toD]) == EMPTy) {
		board[tz] = color;
		ko_z = 0;
		return 0;
	}

	if (tz == ko_z) return 2; // コウの位置には置けない

	int around[4][3];	// 4方向のダメ数、石数、色
	int un_col = flip_color(color);	// 相手の石の色

	// 4方向の石のダメと石数を調べる
	int space = 0;			// 4方向の空白の数
	int kabe = 0;			// 4方向の盤外の数
	int mikata_safe = 0;	// ダメ2以上で安全な味方の数
	int take_sum = 0;		// 取れる石の合計
	int ko_kamo = 0;		// コウになるかもしれない場所
	int diag_oppo = 0;		// 斜めにある相手の石数
	for (int i = 0; i < 4; i++) {
		around[i][0] = around[i][1] = around[i][2] = 0;
		int z = tz + dir4[i];
		int c = board[z];	// 石の色

		// 空点と壁はカウントして次の方向へ
		if (c == EMPTy) { space++; continue; }
		if (c == WALL3) { kabe++; continue;	}
		
		// ｚ位置の石数とダメ数を数える
		int dame;	// ダメの数
		int ishi;	// 石の数
		count_dame(z, &dame, &ishi);
		around[i][0] = dame;
		around[i][1] = ishi;
		around[i][2] = c;
		
		// 相手の石で ダメが１つ なら、取れる！取石数をカウント、取り１ならコウかもしれない（後の処理で判断） 
		if (c == un_col && dame == 1) { take_sum += ishi; ko_kamo = z; }
		// 自分の石で ダメが２つ以上なら、取られない！セーフカウント
		if (c == color && dame >= 2) mikata_safe++;
		// 斜めに相手の石があったら、カウント（後の処理で欠け目判定に使う）
		if (board[z + dirX[i]] == un_col) diag_oppo++;
	}
	if (take_sum == 0 && space == 0 && mikata_safe == 0) return 1;		// 自殺手
	bool kakeme = diag_oppo >= 2 || (diag_oppo && kabe);
	if (kabe + mikata_safe == 4 && !kakeme && fill_eye_err)	return 3;	// １目潰し(ルール違反ではないが)

	for (int i = 0; i < 4; i++) {
		int d = around[i][0];
		int n = around[i][1];
		int c = around[i][2];
		if (c == un_col && d == 1 && board[tz + dir4[i]]) {	// 石が取れる
			kesu(tz + dir4[i], un_col);
			hama[color - 1] += n;
		}
	}

	board[tz] = color;	// 石を置く

	int dame, ishi;
	count_dame(tz, &dame, &ishi);
	if (take_sum == 1 && ishi == 1 && dame == 1) ko_z = ko_kamo;	// コウになる
	else ko_z = 0;
	return 0;
}

// 全ての空点からランダムに一手を選ぶ
int get_random_z(int color) {

	// すべての空点を着手候補にする
	int kouho[BOARD_MAX] = { 0 };
	int kouho_num = 0;
	for (int z = gm.width; z < gm.bd_max - gm.width; z++) {
		if (board[z] == EMPTy) kouho[kouho_num++] = z;	// 空点を候補に追加
	}
	// 候補リストからランダムに選ぶ
	int z, r = 0;
	for (;;) {
		if (kouho_num == 0) { z = PASS; break; }					// pass
		else {
			r = GetRandomValue(0, kouho_num - 1);	// 乱数で1手選ぶ
			z = kouho[r];
		}
		int err = move(z, color, FILL_EYE_ERR);
		if (err == 0) break;

		kouho[r] = kouho[kouho_num - 1];	// エラーなので削除
		kouho_num--;
	}
	return z;
}

// ランダムに並んだ座標リスト list_random_z[] から一手を選ぶ
int get_z_from_random_list(int color)
{
	int i = 0;
	int z = 0;
	int eyes_max = gm.size * gm.size;
	for (i = 0; i < eyes_max; i++)
	{
		z = list_random_z[i];
		if (board[z] != EMPTy) continue;	// 空点ではない、次のランダムｚへ

		// 空点の場合、着手可能か？
		int err = move(z, color, FILL_EYE_ERR);
		if (err != EMPTy) continue;			// 着手禁止なので、次のランダムｚへ

		// 着手可能！forループを抜ける
		break;
	}
	if (i > eyes_max - 1) z = PASS;			// 着手できる座標ｚが無い、パス
	return z;
}

// HUMAN 人間の一手
int youMove() {
	int x = get_board_coord(GetMouseX());
	int y = get_board_coord(GetMouseY());
	if (!is_on_board(x, y)) return KEEP_THINKING;	// カーソルが盤上に無い

	int z = get_z(x, y);

	// 通常の打ち手処理
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		int err = move(z, gm.turn_Color, FILL_EYE_OK);
		return err ? ERR_KEEP_THINKING : z;
	}
	// パス処理（Pキー）
	else if (IsKeyPressed(KEY_P)) {
		return PASS;
	}
	// 投了処理（Rキー）
	else if (IsKeyPressed(KEY_R)) {
		game_resigned = true;
		return RESIGN;
	}
	/*
	*/
	return KEEP_THINKING;
}

// A move thought up randomly by a computer
int comMove() {
	int z;
	all_playouts = 0;	// playout回数を初期化

/*
#if 0	// 0 でUCT探索
	tz = get_random_z(gm.turn_Color);	// 乱数による一手
	// tz = select_best_move(gm.turn_Color);		// 原始モンテカルロ
#else
	if (gm.size < 9) tz = select_best_uct(gm.turn_Color);	// UCT
	else			  tz = get_random_z(gm.turn_Color);		// 乱数による一手
#endif
*/
	z = get_random_z(gm.turn_Color);		// 乱数による一手
	//z = get_z_from_random_list(gm.turn_Color);		// 乱数による一手
	
	return z;
}

// hamBotの一手
int hamMove() {
	// 空点から着手候補にする
	int kouho[BOARD_MAX] = { 0 };
	int kouho_num = 0;
	for (int z = gm.width; z < gm.bd_max - gm.width; z++) {
		// 除外
		if (board[z] > EMPTy) continue;							// 空点ではない
		if ((tesuu < gm.size) && (gm.size <= 8)) {
			if (is_edge(z)) continue;				// 7路以下で 最初の数手は1線に打たない
		}
		else if ((tesuu < gm.size) && (gm.size > 8)) {
			if (is_stone_1around(z, gm.turn_Color)) continue;	// 最初の数手は自分の石に付けない
			if (is_near_center(z)) continue;	// 最初の数手は中央付近に打たない
			if (is_1_2_line(z)) continue;		// 最初の数手は1,2線に打たない
		}
		else if (tesuu < gm.bd_max / 2 && (gm.size > 8)) {
			if (is_1_2_line(z) && !is_stone_1around(z, gm.turn_Color)) continue;	// 1,2線で周りに自分の石が無い座標z
		}

		// 候補に追加
		kouho[kouho_num++] = z;
	}
	// 候補リストからランダムに選ぶ
	int z, r = 0;
	for (;;) {
		if (kouho_num == 0) z = 0;					// pass
		else {
			r = GetRandomValue(0, kouho_num - 1);	// 乱数で1手選ぶ
			z = kouho[r];
		}
		int err = move(z, gm.turn_Color, FILL_EYE_ERR);
		if (err == 0) break;

		kouho[r] = kouho[kouho_num - 1];	// エラーなので削除
		kouho_num--;
	}
	return z;
}
