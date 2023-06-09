#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>
#include <stdbool.h>
#include <pthread.h>

#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20
#define DEAD 0
#define ALIVE 1


int life[BOARD_WIDTH][BOARD_HEIGHT][2];
int turn = 0;
bool gamePauseFlag = false;
// trueの間は一時停止
double interval = 500;

// 周囲の八方向の一覧
enum {
    DIRECTION_UP,
    DIRECTION_UP_LEFT,
    DIRECTION_LEFT,
    DIRECTION_DOWN_LEFT,
    DIRECTION_DOWN,
    DIRECTION_DOWN_RIGHT,
    DIRECTION_RIGHT,
    DIRECTION_UP_RIGHT,
    DIRECTION_MAX
};

// 周囲の八方向への移動
int directions[][2] = {
     { 0,-1}    // DIRECTION_UP,
    ,{-1,-1}    // DIRECTION_UP_LEFT,
    ,{-1, 0}    // DIRECTION_LEFT,
    ,{-1, 1}    // DIRECTION_DOWN_LEFT,
    ,{ 0, 1}    // DIRECTION_DOWN,
    ,{ 1, 1}    // DIRECTION_DOWN_RIGHT,
    ,{ 1, 0}    // DIRECTION_RIGHT,
    ,{ 1,-1}    // DIRECTION_UP_RIGHT,
};

// getch関数を使用可能にする
int getch(void) {
    struct termios oldattr, newattr;
    int ch;
    tcgetattr( STDIN_FILENO, &oldattr );
    newattr = oldattr;
    newattr.c_lflag &= ~( ICANON | ECHO );
    tcsetattr( STDIN_FILENO, TCSANOW, &newattr );
    ch = getchar();
    tcsetattr( STDIN_FILENO, TCSANOW, &oldattr );
    return ch;
}

// 各コマの生死の設定時の描画
void firstDrawBoard(int __cursorX, int __cursorY) {
    system("clear");
    
    for(int y = 1; y < BOARD_HEIGHT - 1; y++){
        for(int x = 1; x < BOARD_WIDTH - 1; x++){
            if(x==__cursorX && y==__cursorY){
                printf(" p");
            }else{
                switch(life[x][y][turn]) {
                    case DEAD: printf(" *"); break;
                    case ALIVE: printf(" ●"); break;
                }
            }
        }
    printf("\n");
    }
}

// ゲーム中の描画
void secondDrawBoard() {
    system("clear");
    for(int y = 1; y < BOARD_HEIGHT - 1; y++){
        for(int x = 1; x < BOARD_WIDTH - 1; x++){
            switch(life[x][y][turn]) {
                case DEAD: printf(" *"); break;
                case ALIVE: printf(" ●"); break;
            }
        }
        printf("\n");
    }
    printf("G:1Turn H:Start/Stop J:ChangeInterval K:ReSetting L:Quit\n");
}

// life[][]の各コマの生死の設定をし、cursorの座標を渡す
void setting(int *_cursorX, int *_cursorY) {
    
    while(1) {
        
        firstDrawBoard(*_cursorX, *_cursorY);
        
        switch(getch()) {
            case'w':
                if(*_cursorY > 1){
                    *_cursorY -= 1;
                }
                break;
            case's': 
                if(*_cursorY < BOARD_HEIGHT - 2){
                    *_cursorY += 1;
                }
                break;
            case'a':
                if(*_cursorX > 1){ 
                    *_cursorX -= 1;
                }
                break;
            case'd':
                if(*_cursorX < BOARD_WIDTH - 2){
                    *_cursorX += 1;
                }
                break;
            case'p':
                life[*_cursorX][*_cursorY][turn] ^= 1;
                break;
            case'q':
                return;
            default:
                break;
        }
    }
}

// 周辺のライフの数を数える
int aroundSum(int _x, int _y){
    int sum = 0;
    int x, y;
    
    for(int i = 0; i < DIRECTION_MAX; i++){
        x = _x, y = _y;
        x += directions[i][0];
        y += directions[i][1];
        sum += life[x][y][turn];
    }
    return sum;
}

// 1ターン進める
void advance() {
    int nextTurn = turn;
    nextTurn ^= 1;
    for(int y = 1; y < BOARD_HEIGHT - 1; y++){
        for(int x = 1; x < BOARD_WIDTH - 1; x++){
            int around = aroundSum(x, y);
            if(life[x][y][turn] == DEAD){
                // 死んでるマスの処理
                if(around == 3){
                    life[x][y][nextTurn] = ALIVE;
                }else{
                    life[x][y][nextTurn] = DEAD;
                }
            }else{
                // 生きてるマスの処理
                if(around == 2 || around == 3){
                    life[x][y][nextTurn] = ALIVE;
                }else{
                    life[x][y][nextTurn] = DEAD;
                }
            }
        }
    }
    turn = nextTurn;
}

// 自動再生時の描画の間隔を変える
void changeInterval() {
    system("clear");
    printf("Decide the Interval. [ms] (More than 100 is ideal)\n");
    scanf("%lf", &interval);
    gamePauseFlag = false;
}


// ゲームを実行する
void* game(void *arg) {
    
    while(gamePauseFlag == false){
        advance();
        secondDrawBoard();
        usleep(interval * 1000);
    }
}

int main() {
    // 初期化する
    for(int z = 0; z < 2; z++){
        for(int y = 0; y < BOARD_HEIGHT; y++){
            for(int x = 0; x < BOARD_WIDTH; x++){
                life[x][y][z] = DEAD;
            }
        }
    }
    printf("Use the WASD keys to move the cursor and use the P key to switch between DEAD and ALIVE.\n");
    printf("When you are done setting up, press the Q key.\n");
    printf("Now, press any key to start game.\n");
    getch();
    
    int cursorX = 1, cursorY = 1;
    setting(&cursorX, &cursorY);
    
    pthread_t game_thread;
    pthread_create(&game_thread, NULL, game, NULL);
    
    bool gameQuitFlag = false;
    // trueになったら終了
    
    while(gameQuitFlag == false){
        
        switch(getch()){
            case'g':
                gamePauseFlag = true;
                pthread_join(game_thread, NULL);
                advance();
                secondDrawBoard();
                break;
            case'h':
                if(gamePauseFlag == true){
                    pthread_create(&game_thread, NULL, game, NULL);
                    gamePauseFlag = false;
                }else{
                    gamePauseFlag = true;
                    pthread_join(game_thread, NULL);
                }
                break;
            case'j':
                gamePauseFlag = true;
                pthread_join(game_thread, NULL);
                
                changeInterval();
                
                gamePauseFlag = false;
                pthread_create(&game_thread, NULL, game, NULL);
                
                break;
            case'k':
                gamePauseFlag = true;
                pthread_join(game_thread, NULL);
                
                setting(&cursorX, &cursorY);
                
                gamePauseFlag = false;
                pthread_create(&game_thread, NULL, game, NULL);
                break;
            case'l':
                gamePauseFlag = true;
                gameQuitFlag = true;
                pthread_join(game_thread, NULL);
                break;
            default:
                break;
        }
    }
}
