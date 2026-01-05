#include "raylib.h"
#include "go_func.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static void UpdateDrawFrame(void);  // Update and Draw (one frame)

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{

    InitWindow(WINDOW_SIZE + 360, WINDOW_SIZE, "PonKotu GO");
    InitAudioDevice();
    seBtnClick = LoadSound("resources/putt.mp3");
    sePikon = LoadSound("resources/pikon.mp3");
    sePutStone = LoadSound("resources/putStone02.mp3");

    SetMasterVolume(1.0f);

    SetSoundVolume(seBtnClick, 0.5f);
    SetSoundVolume(sePikon   , 0.4f);
    SetSoundVolume(sePutStone, 0.3f);

    typeImg[HUMAN] = LoadTexture("resources/human128.png");
    typeImg[COMPUTER] = LoadTexture("resources/computer128.png");
    typeImg[HAMBOT] = LoadTexture("resources/hamBot128.png");
    stoneImg[BLACk] = LoadTexture("resources/StoneBlack.png");
    stoneImg[WHITe] = LoadTexture("resources/StoneWhite.png");

    current_interval = move_interval[move_interval_Idx];

    init_game();
    init_board();	// 盤面初期化

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    //--------------------------------------------------------------------------------------

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update and Draw
        //----------------------------------------------------------------------------------
        UpdateDrawFrame();
        //----------------------------------------------------------------------------------
    }
    // Unload loaded data (textures, sounds, models...)
    UnloadTexture(stoneImg[WHITe]);
    UnloadTexture(stoneImg[BLACk]);
    UnloadTexture(typeImg[HAMBOT]);
    UnloadTexture(typeImg[COMPUTER]);
    UnloadTexture(typeImg[HUMAN]);

    UnloadSound(sePutStone);
    UnloadSound(sePikon);
    UnloadSound(seBtnClick);

#endif

    CloseAudioDevice();
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}


// Update and Draw (one frame)
void UpdateDrawFrame(void)
{
    current_scene();
}

// /////////////////////////////////////////////////////////////////////////////////////////////
// 以下、web囲碁テスト用
// /////////////////////////////////////////////////////////////////////////////////////////////

// タイトル シーン
void title_scene()
{
    BeginDrawing();
    //ClearBackground(LIGHTBEIGE);
    ClearBackground(LIME);
    //DrawTexture(title_background, 0, -100, WHITE);

    draw_board();

    Label(15, -55, "Go", 550, (Color) { 255, 0, 0, 64 }, (Color) { 200, 200, 200, 16 });
    //Label(-130, 15, "Go", 550, WHITE, (Color) { 255, 255, 255, 0 });

    if (BtnV(-0, -70, "Go", 550, WHITE, (Color) { 255, 255, 255, 0 })) {
        current_scene = init_scene/**/;
        PlaySound(sePikon);
    }

    Label(205, 35, "Click to set up a game!", 70, DARKGRAY, (Color) { 200, 200, 200, 16 });
    Label(200, 30, "Click to set up a game!", 70, PINK, (Color) { 255, 255, 255, 0 });
    DrawText("made with c & raylib ", 550, 836, 20, RAYWHITE);
    DrawText("(C) 2026 hamuQonda on GitHub", 1060, 845, 10, RAYWHITE);

    EndDrawing();
}

// 設定 シーン
void init_scene()
{
    BeginDrawing();
    ClearBackground(BLUE);

    // -- 対局設定 -- /////////////////////////////////////////////////////////////////
    //Label(845, -10, "<SETTINGS>", 50, RAYWHITE, DARKGRAY2);
    DrawText("<SETTINGS>", 880, 10, 50, RAYWHITE);
    DrawText("Click or Wheel on the button", 870, 70, 20, RAYWHITE);
    int msgPosY = 0;
    int rowSpace = 110;
    bool btn_click = false;
    bool collision_btn_mouse = false;

    // 黒プレイヤー ///////////////////////////////////////////////////////////////////
    msgPosY += rowSpace;
    btn_click = BtnF(850, msgPosY, 355, 60, "Player B =", player_type[pBtype_Idx], 30, YELLOW, GREEN);
    collision_btn_mouse = CheckCollisionPointRec(GetMousePosition(), (Rectangle) { 850.f, (float)msgPosY, 350.f, 60.f });

    if (btn_click) { pBtype_Idx++; pBtype_Idx %= TYPE_NUM; PlaySound(seBtnClick); } // ボタンクリックでの変更
    else if (collision_btn_mouse) {                                                 // マウスホイールでの変更
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            if (wheel > 0) pBtype_Idx++; else pBtype_Idx--;
            if (pBtype_Idx < 0) pBtype_Idx = TYPE_NUM - 1;
            if (pBtype_Idx > (TYPE_NUM - 1)) pBtype_Idx = 0;
            PlaySound(seBtnClick);
        }
    }
    playerB.type = pBtype_Idx;
    playerB.name = player_type[pBtype_Idx];
    playerB.img = &typeImg[pBtype_Idx];
    playerB.move = movesByType[pBtype_Idx];
    DrawCircle(988, msgPosY + 30, 18.0f, BLACK);
    DrawTextureEx(*playerB.img, (Vector2) { 1120.0f, (float)msgPosY }, 0.0f, 0.6f, WHITE);

    // 白プレイヤー ///////////////////////////////////////////////////////////////////
    msgPosY += rowSpace;
    btn_click = BtnF(850, msgPosY, 355, 60, "Player W =", player_type[pWtype_Idx], 30, YELLOW, GREEN);
    collision_btn_mouse = CheckCollisionPointRec(GetMousePosition(), (Rectangle) { 850.f, (float)msgPosY, 350.f, 60.f });
    
    if (btn_click) { pWtype_Idx++; pWtype_Idx %= TYPE_NUM; PlaySound(seBtnClick); } // ボタンクリックでの変更
    if (collision_btn_mouse) {                                                      // マウスホイールでの変更
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            if (wheel > 0) pWtype_Idx++; else pWtype_Idx--;
            if (pWtype_Idx < 0) pWtype_Idx = TYPE_NUM - 1;
            if (pWtype_Idx > (TYPE_NUM - 1)) pWtype_Idx = 0;
            PlaySound(seBtnClick);
        }
    }
    playerW.type = pWtype_Idx;
    playerW.name = player_type[pWtype_Idx];
    playerW.img = &typeImg[pWtype_Idx];
    playerW.move = movesByType[pWtype_Idx];
    DrawCircle(988, msgPosY + 30, 18.0f, WHITE);
    DrawTextureEx(*playerW.img, (Vector2) { 1120, (float)msgPosY }, 0.0f, 0.6f, WHITE);

    // 碁盤サイズ ///////////////////////////////////////////////////////////////////
    msgPosY += rowSpace;
    btn_click = BtnF(850, msgPosY, 350, 60, "Board SIZE =", bdsize[size_Idx], 30, YELLOW, GREEN);
    collision_btn_mouse = CheckCollisionPointRec(GetMousePosition(), (Rectangle) { 850.f, (float)msgPosY, 350.f, 60.f });

    if (btn_click) {                                        // ボタンクリックでのサイズ変更
        size_Idx++;
        if (size_Idx > NUM_BOARD_SIZE - 1) size_Idx = 0;
        init_board();
        PlaySound(seBtnClick);
    }
    else if (collision_btn_mouse) {                         // マウスホイールでの変更
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            if (wheel > 0) size_Idx++; else size_Idx--;
            if (size_Idx < 0) size_Idx = NUM_BOARD_SIZE - 1;
            if (size_Idx > NUM_BOARD_SIZE - 1) size_Idx = 0;
            PlaySound(seBtnClick);
        }
    }
    gm.size = TextToInteger(bdsize[size_Idx]);	// 碁盤の大きさ
    gm.width = gm.size + 2;						// 枠を含めた横幅
    gm.bd_max = gm.width * gm.width;			// 枠を含めた盤面配列サイズ
    gm.moves_limit = gm.size * gm.size * 2;		// 終了までの手数
    cell_size = ((WINDOW_SIZE - 2 * TEMP_MARGIN) / gm.size);
    board_margin = ((WINDOW_SIZE - cell_size * gm.size) / 2);

    // 置き碁 ///////////////////////////////////////////////////////////////////
    msgPosY += rowSpace;
    btn_click = BtnF(850, msgPosY, 350, 60, "Handicap =", handicap_stones[handicap_Idx], 30, YELLOW, GREEN);
    collision_btn_mouse = CheckCollisionPointRec(GetMousePosition(), (Rectangle) { 850.f, (float)msgPosY, 350.f, 60.f });

    int hc_Idx_max = (gm.size < 7) ? 1 : 8;
    if (gm.size < 7 && handicap_Idx > 1) {                      // 7路未満は 置き碁2個まで
        backup_hcIdx = handicap_Idx;                            // 設定中の置き碁数をバックアップ（set_handicap_stones()内で分岐処理する為）
        handicap_Idx = 1;
    }
    else if (gm.size >= 7 && backup_hcIdx > 0) {                // 7路以上は 置き碁２～９
        handicap_Idx = backup_hcIdx;                            // バックアップしてあった置き碁数をもどす
        backup_hcIdx = 0;
    }
    else if (btn_click) {                                       // ボタンクリックでの変更
        handicap_Idx++;
        if (handicap_Idx > hc_Idx_max) handicap_Idx = 0;
        init_board(); PlaySound(seBtnClick);
    }
    else if (collision_btn_mouse) {			                    // マウスホイールでの変更
        float wheel = GetMouseWheelMove();
        if (wheel != 0.0f) {
            if (wheel > 0) handicap_Idx++; else handicap_Idx--;
            if (handicap_Idx < 0) handicap_Idx = hc_Idx_max;
            if (handicap_Idx > hc_Idx_max) handicap_Idx = 0;
            PlaySound(seBtnClick);
        }
    }
    gm.handicap_stones = handicap_Idx == 0 ? 0 : handicap_Idx + 1;

    // 盤サイズと置き石を board[] に反映
    init_board();
    set_handicap_stones();

    // コミ ///////////////////////////////////////////////////////////////////
    msgPosY += rowSpace;
    if (gm.handicap_stones) {
        DrawText("  KOMI    =  0.5", 850, msgPosY, 30, GRAY);
    }
    else {
        //bool komi_toggle = BtnF(850, msgPosY, 350, 60, "  KOMI    =", komi_list[komi_Idx], 30, YELLOW, GREEN);
        //if (komi_toggle) { komi_Idx++; komi_Idx %= 3; PlaySound(seBtnClick); }						// ボタンクリックでのサイズ変更
        btn_click = BtnV(850, msgPosY, "  KOMI    =              ", 30, RAYWHITE, GREEN);
        if (btn_click) { komi_Idx++; komi_Idx %= 3; PlaySound(seBtnClick); }
        if (CheckCollisionPointRec(GetMousePosition(), (Rectangle) { 850.f, (float)msgPosY, 350.f, 60.f })) {	// マウスホイールでの変更
            float wheel = GetMouseWheelMove();
            if (wheel != 0.0f) {
                if (wheel > 0) komi_Idx++; else komi_Idx--;
                if (komi_Idx < 0) komi_Idx = 2;
                if (komi_Idx > 2) komi_Idx = 0;
                PlaySound(seBtnClick);
            }
        }
        gm.komi = gm.handicap_stones ? 0.5f : (float)atof(komi_list[komi_Idx]);
        DrawText(komi_list[komi_Idx], 1050, (float)msgPosY + 15, 30, YELLOW);
    }

    // 決定？ ///////////////////////////////////////////////////////////////////
    DrawText("Is this OK?", 900, 660, 35, PURPLE);
    if (BtnV(850, 700, "> START ", 60, RAYWHITE, GREEN)) {
        gm.turn_Color = gm.handicap_stones ? WHITe : BLACk;
        gm.curPlayer = gm.handicap_stones ? playerW : playerB;
        gm.cur_move = -999;
        gm.score = 0;
        gm.b_score = 0;
        gm.w_score = 0;
        gm.b_dead_stones = 0;
        gm.w_dead_stones = 0;
        gm.winner = -999;
        // 右、左、下、上への移動量 ※dir4,dirX配列はfor文で回す用
        toR = dir4[0] = +1       /*右*/;
        toL = dir4[1] = -1       /*左*/;
        toD = dir4[2] = +gm.width/*下*/;
        toU = dir4[3] = -gm.width/*上*/;
        // 斜め方向
        toR_toD = toR + toD;
        toL_toU = toL + toU;
        toD_toL = toD + toL;
        toU_toR = toU + toR;
        // dir4 に dirX を加算することで斜め方向への移動量
        dirX[0] = +gm.width;
        dirX[1] = -gm.width;
        dirX[2] = -1;
        dirX[3] = +1;
        PlaySound(sePikon);
        current_scene = play_scene; //// 対局シーンへ移行 /////////////////
    };

    draw_board();       // 碁盤の線の描画
    draw_stones();      // 盤面の石の描画

    /*
    DrawText(TextFormat("%dx%d", gm.size, gm.size), 290, 18, 160, (Color) { 255, 0, 255, 128 });
    DrawText(TextFormat("Handicap stones %d", gm.handicap_stones), 70, 500, 80, (Color) { 255, 0, 255, 128 });
    DrawText(TextFormat("Komi %1.1f", gm.komi), 320, 760, 80, (Color) { 255, 0, 255, 128 });
    */

    EndDrawing();
}

void play_scene() {

    // Update game

    // 音オンオフ
    if (IsKeyPressed(KEY_V)) {
        is_mVolume = !is_mVolume;
        float v = (is_mVolume) ? 1.0f : 0.0f;
        SetMasterVolume(v);
    }

    // コンピュータの一手の速さ：スペースキーで（低・中・速）切替
    if (IsKeyPressed(KEY_SPACE)) {
        move_interval_Idx += move_speed_switch;
        current_interval = move_interval[move_interval_Idx];
        move_speed_switch = (move_interval_Idx == MIDL_MOVE) ? move_speed_switch : -move_speed_switch;
    }


    if (!game_resigned || !game_ended_by_pass || !ended_by_moves_lim) {

        move_timer += GetFrameTime();
        if (gm.curPlayer.type==HUMAN || move_timer >= current_interval) {

            int z = gm.curPlayer.move();
            if (z == ERR_KEEP_THINKING || z == KEEP_THINKING) {
                // 禁じ手or思考中は何もしない
            }
            else {
                /*
                print_board();
                printf("play_z = %1.1f,手数=%d,色=%d,all_playouts=%d\n", get_xy_disp(z) / 10.0f, ++tesuu, gm.turn_Color, all_playouts);
                printf("last_move_pass = %s\n", (last_move_was_pass ? "true" : "false"));
                */

                if (z == PASS) {
                    ko_z = 0;		// コウ解消
                    if (!last_move_was_pass) {
                        last_move_was_pass = gm.turn_Color;	// 現在の手はパス
                        PlaySound(seBtnClick);
                    }
                    else {
                        game_ended_by_pass = true;	// パスが連続したら終局
                        current_scene = end_scene;
                        //*debug*/printf("LINE:1407/n");
                        PlaySound(sePikon);
                    }
                }
                else if (z == RESIGN) {
                    game_resigned = gm.turn_Color;
                    current_scene = end_scene;
                    //*debug*/printf("LINE:1414/n");
                    PlaySound(sePikon);
                }
                else {
                    gm.cur_move = z;
                    last_move_was_pass = EMPTy;
                    PlaySound(sePutStone);
                }
                kifu[tesuu] = z;
                tesuu++;
                switch_turn();	// 手番交代
                move_timer = 0;

            }
        }
        //double t = (double)(clock() + 1 - bt) / CLOCKS_PER_SEC;

        //print_board();
        // printf("%.1f 秒, %.0f playout/秒\n", t, all_playouts / t);

        //printf("tesuu = %d / %d\n", tesuu, gm.moves_limit);

        if (tesuu > gm.moves_limit && playerB.type != HUMAN && playerW.type != HUMAN) {
            ended_by_moves_lim = true;// 3コウでのループ対策
            current_scene = end_scene;
            /*debug*/printf("LINE:1438/n");
            PlaySound(sePikon);
        }
    }

    // Draw
    BeginDrawing();
    ClearBackground(DDARKGREEN);

    draw_board();
    draw_stones();
    draw_move_mark(gm.cur_move);
    draw_game_info();

    // pass button & resign button
    if (gm.curPlayer.type == HUMAN) {
        DrawCursorStone();
        if (BtnV(1100, 740, "pass", 30, LIGHTGRAY, LIGHTGREEN)) {
            ko_z = 0;		// コウ解消
            if (!last_move_was_pass) {
                last_move_was_pass = gm.turn_Color;	// 現在の手はパス
                PlaySound(seBtnClick);
            }
            else {
                game_ended_by_pass = true;	// パスが連続したら終局
                current_scene = end_scene;
                /*debug*/printf("LINE:1464n");
                PlaySound(sePikon);
            }
            switch_turn();	// 手番交代
        }
        if (BtnV(890, 740, "resign", 30, LIGHTGRAY, LIGHTGREEN)) {
            game_resigned = gm.turn_Color;
            switch_turn();	// 手番交代
            current_scene = end_scene;
            /*debug*/printf("LINE:1473/n");
            PlaySound(sePikon);
        }
    }
    draw_players();

    // 手番を示す枠を表示
    if (gm.turn_Color == BLACk)	DrawRectangleLines(865, 45, 158, 190, LIGHTGREEN);
    else						DrawRectangleLines(1045, 45, 158, 190, LIGHTGREEN);

    DrawText(TextFormat("move : %d", tesuu), 700, 4, 20, GRAY);

    char s__on[] = "Sound ON";
    char s_off[] = "Sound OFF";

    DrawText(TextFormat("key     V : %s", (is_mVolume ? s__on : s_off)), 860, WINDOW_SIZE - 45, 20, GRAY);
    DrawText(           "key SPACE : Move SPEED ->", 860, WINDOW_SIZE - 20, 20, GRAY);
    DrawText(TextFormat("%s", move_speed[move_interval_Idx]), 1155, WINDOW_SIZE - 20, 20, GRAY);

    EndDrawing();

}

// 終局シーン（結果表示）
void end_scene() {
    // Update
    if (game_resigned) {
        gm.winner = game_resigned == BLACk ? WHITe : BLACk;
    }
    else if (gm.winner < 0) {
        gm.winner = count_score2();
    }

    SetMasterVolume(1.0f);
    is_mVolume = true;

    // Draw
    BeginDrawing();
    ClearBackground(DARKPURPLE);
    DrawText(TextFormat("move : %d", tesuu), 700, 4, 20, LIGHTGRAY);

    // 勝者を示す枠を表示
    if (gm.winner == BLACk)	DrawRectangle(865, 45, 158, 190, GOLD);
    else					DrawRectangle(1045, 45, 158, 190, GOLD);

    draw_players();

    draw_board();
    draw_stones();
    draw_move_mark(gm.cur_move);
    draw_game_info();
    draw_result();
  //if (BtnV(850, 690, "Settings",
    if (BtnV(850, 650, "> TOP ", 80, RAYWHITE, GREEN)) {
        init_players();
        init_game();
        init_board();	// 盤面初期化
        set_handicap_stones();
        PlaySound(sePikon);
        current_scene = title_scene;
    }
    Label(880, 620, "Back to", 20, RED, BLANK);

    EndDrawing();
}
