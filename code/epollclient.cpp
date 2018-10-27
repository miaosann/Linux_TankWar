#include  <unistd.h>
#include  <sys/types.h>       /* basic system data types */
#include  <sys/socket.h>      /* basic socket definitions */
#include  <netinet/in.h>      /* sockaddr_in{} and other Internet defns */
#include  <arpa/inet.h>       /* inet(3) functions */
#include <netdb.h> /*gethostbyname function */
#include <curses.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <iostream>
using namespace std;


#define GRASS     ' '
#define EMPTY     '.'
#define WATER     '~'
#define MOUNTAIN  '^'
#define PLAYER    '&'
#define BANG    '@'
#define MAXLINE 1024

#define GRASS_PAIR     1
#define EMPTY_PAIR     1
#define WATER_PAIR     2
#define MOUNTAIN_PAIR  3
#define PLAYER_PAIR    4
#define BANG_PAIR    5
struct POSITION{
    int x;
    int y;
};

struct POS_SEND{
    int connfd;
    int type;//0是player,1是BNAG
    int x;
    int y;
    char direction;
};

int is_move_okay(int y, int x);
void draw_map(void);
void *fire_bangJ(void *);//向左开炮
void *fire_bangL(void *);//向右开炮
void *fire_bangI(void *);//向上开炮
void *fire_bangK(void *);//向下开炮
void startGame(int);
void *send_struct(void *);
void *receive_struct(void *);
void *send_struct_loop(void *);
void *receive_struct_loop(void *);
void *update_struct(void *);
void *pthread_write(void *sock_fd);
void *pthread_read(void *sock_fd);

void *send_struct(void *sendstruct){
    struct POS_SEND send_struct = *(struct POS_SEND *)sendstruct; 
    int n;
    while(1){
        n = write(send_struct.connfd, &send_struct, sizeof(send_struct));
            if(n>0)
                break;
    }
    //printf("send!\n");
    printf("send : %d,%d\n",send_struct.x,send_struct.y);
}
void *send_struct_loop(void *sendstruct){
    struct POS_SEND send_struct = *(struct POS_SEND *)sendstruct; 
    int n;
    while(1){
        //connfd如果不满足自然不会发送
        n = write(send_struct.connfd, &send_struct, sizeof(send_struct));
    }
    //printf("send!\n");
     
}
void *receive_struct(void *receivefd){
    //struct POS_SEND receive_struct = *(struct POS_SEND *)receivestruct; 
    int connfd = *(int *)receivefd;
    int n;
     /*初始化对手玩家*/
    struct POS_SEND enemy;
    while(1){
        n = read(connfd, &enemy, sizeof(enemy));
        if(n>0)
            break;
    }
    //printf("ojbk!!!\n");
    mvaddch(enemy.y, enemy.x+10, PLAYER);
    refresh();
    printf("receive : %d,%d\n",enemy.x,enemy.y);
}
void *receive_struct_loop(void *receivefd){
    //struct POS_SEND receive_struct = *(struct POS_SEND *)receivestruct; 
    int connfd = *(int *)receivefd;
    int n;
     /*初始化对手玩家*/
    struct POS_SEND enemy;
    while(1){
        n = read(connfd, &enemy, sizeof(enemy));
        if(n>0){
            mvaddch(enemy.y, enemy.x+10, PLAYER);
            refresh();
        }
    }
    //printf("ojbk!!!\n");
}
void *update_struct(void *receivefd){
    //struct POS_SEND update_struct = *(struct POS_SEND *)updatestruct; 
    int connfd = *(int *)receivefd;
    int n;
     /*更改对手玩家*/
     struct POS_SEND enemy;
    while(1){
        n = read(connfd, &enemy, sizeof(enemy));
        if(n>0)
            break;
    }
    mvaddch(enemy.y, enemy.x+10, GRASS);
}
int is_move_okay(int y, int x)
{
    int testch;
    /* 当空间可以进入时返回true */
    testch = mvinch(y, x);
    return (((testch & A_CHARTEXT) == GRASS)
            || ((testch & A_CHARTEXT) == EMPTY));
}
int is_shoot(int y,int x){
     int testch;
    /* 当射中时返回true */
    testch = mvinch(y, x);
    return ((testch & A_CHARTEXT) == PLAYER);
}
void draw_map(void)
{
    int y, x;
    /* 绘制地图 */
    /* 背景 */
    attron(COLOR_PAIR(GRASS_PAIR));
    for (y = 0; y < LINES; y++) {
    mvhline(y, 0, GRASS, COLS);
    }
    attroff(COLOR_PAIR(GRASS_PAIR));
    /* 山脉和山道 */
    attron(COLOR_PAIR(MOUNTAIN_PAIR));
    for (x = COLS * 2 / 3; x < COLS * 3 / 4; x++) {
    mvvline(0, x, MOUNTAIN, LINES);
    }
    attroff(COLOR_PAIR(MOUNTAIN_PAIR));
    attron(COLOR_PAIR(GRASS_PAIR));
    mvhline(LINES / 4, 0, GRASS, COLS);
    mvhline(LINES * 3 / 4, 0, GRASS, COLS);
    attroff(COLOR_PAIR(GRASS_PAIR));
    /* 湖 */
    attron(COLOR_PAIR(WATER_PAIR));
    for (y = 5; y < LINES * 2 / 3 ; y++) {
    mvhline(y, 10, WATER, COLS / 3);
    }
    attroff(COLOR_PAIR(WATER_PAIR));
}
void *fire_bangJ(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x-1;
    int bangY = pos.y;
    
    while(bangX>=0){
        if(!is_shoot(bangY,bangX - 1)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangX--;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Vectory!!!\n");
            exit(0);
        }
    }
}
void *fire_bangJ1(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x-1;
    int bangY = pos.y;
    
    while(bangX>=0){
        if(!is_shoot(bangY,bangX - 1)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangX--;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Failure!!!\n");
            exit(0);
        }
    }
}
void *fire_bangL(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x+1;
    int bangY = pos.y;
    while(bangX<COLS){
        if(!is_shoot(bangY,bangX + 1)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangX++;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
             endwin();
            printf("Vectory!!!\n");
            exit(0);
        }
    }
}
void *fire_bangL1(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x+1;
    int bangY = pos.y;
    while(bangX<COLS){
        if(!is_shoot(bangY,bangX + 1)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangX++;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
             endwin();
            printf("Failure!!!\n");
            exit(0);
        }
    }
}
void *fire_bangI(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x;
    int bangY = pos.y-1;
    while(bangY>=0){
        if(!is_shoot(bangY - 1,bangX)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangY--;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Vectory!!!\n");
            exit(0);
        }
    }
}
void *fire_bangI1(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x;
    int bangY = pos.y-1;
    while(bangY>=0){
        if(!is_shoot(bangY - 1,bangX)){
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangY--;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Failure!!!\n");
            exit(0);
        }
    }
}
void *fire_bangK(void *position){
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x;
    int bangY = pos.y+1;
    while(bangY<LINES){
        if (!is_shoot(bangY + 1, bangX)) {  //判断是否打到人
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangY++;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Vectory!!!\n");
            exit(0);
        }
    }
}
void *fire_bangK1(void *position){  //另一个
    struct POSITION pos = *(struct POSITION *)position;
    int bangX = pos.x;
    int bangY = pos.y+1;
    while(bangY<LINES){
        if (!is_shoot(bangY + 1, bangX)) {  //判断是否打到人
            attron(COLOR_PAIR(GRASS_PAIR));
            mvaddch(bangY, bangX, GRASS);
            attroff(COLOR_PAIR(GRASS_PAIR));
            bangY++;
            attron(COLOR_PAIR(BANG_PAIR));
            mvaddch(bangY, bangX, BANG);
            attroff(COLOR_PAIR(BANG_PAIR));
            move(bangY,bangX);
            usleep(60000);
            refresh();
        }else{
            endwin();
            printf("Failure!!!\n");
            exit(0);
        }
    }
}
void startGame(int connfd){
    pthread_t send,receive;
    /* 初始化curses */
    initscr();
    keypad(stdscr, TRUE);
    cbreak();
    noecho();
     /* 初始化颜色 */
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    init_pair(GRASS_PAIR, COLOR_YELLOW, COLOR_GREEN);
    init_pair(WATER_PAIR, COLOR_CYAN, COLOR_BLUE);
    init_pair(MOUNTAIN_PAIR, COLOR_BLACK, COLOR_WHITE);
    init_pair(PLAYER_PAIR, COLOR_RED, COLOR_MAGENTA);
    init_pair(BANG_PAIR, COLOR_YELLOW, COLOR_CYAN);
    clear();
    /* 初始化探索地图 */
    draw_map();

    if(pthread_create(&send, NULL, pthread_write,(void *)&connfd) == -1){
        puts("fail to create pthread send");
        exit(1);
    }
    if(pthread_create(&receive, NULL, pthread_read,(void *)&connfd) == -1){
        puts("fail to create pthread receive");
        exit(1);
    }
    pthread_join(send,NULL);
    pthread_join(receive,NULL);
    endwin();
}


void *handle(void *connfd);
void *pthread_read(void *sock_fd){
    int * aa = (int *) sock_fd;
    int connfd=*aa;
    struct POS_SEND enemy;
    struct POSITION pos;
    int x,y;
    pthread_t t0,t1,t2,t3;
    while(1){
        read(connfd, &enemy, sizeof(enemy));
        attron(COLOR_PAIR(GRASS_PAIR));
        mvaddch(y, x, GRASS);
        attroff(COLOR_PAIR(GRASS_PAIR));
        x = enemy.x;
        y = enemy.y;
        char direction = enemy.direction;

        pos.x = x;
        pos.y = y;
        
        if(enemy.type == 0){    //人
            attron(COLOR_PAIR(PLAYER_PAIR));
            mvaddch(y, x, PLAYER);
            attroff(COLOR_PAIR(PLAYER_PAIR));
        }else{
           // mvaddch(y, x, BANG);
            if(direction == 'j'){
                if(pthread_create(&t0, NULL, fire_bangJ1,(void *)&pos) == -1){
                    puts("fail to create pthread t0");
                    exit(1);
                }
                pthread_detach(t0);
                //pthread_join(t0,NULL);  
            } 
            else if(direction == 'k'){
                if(pthread_create(&t1, NULL, fire_bangK1,(void *)&pos) == -1){
                    puts("fail to create pthread t1");
                    exit(1);
                }
                pthread_detach(t1);
                //pthread_join(t1,NULL);    
            }
            else if(direction == 'l'){
                if(pthread_create(&t2, NULL, fire_bangL1,(void *)&pos) == -1){
                    puts("fail to create pthread t2");
                    exit(1);
                }
                pthread_detach(t2);
                //pthread_join(t2,NULL);  
            }
            else if(direction == 'i'){
                if(pthread_create(&t3, NULL, fire_bangI1,(void *)&pos) == -1){
                    puts("fail to create pthread t3");
                    exit(1);
                }
                pthread_detach(t3);
                //pthread_join(t3,NULL);  
            }
        }
        move(y, x);
        refresh();
    }
    
}
void *pthread_write(void *sock_fd){
    int * aa = (int *) sock_fd;
    int connfd=*aa;

    int y, x;
    int ch;
    struct POSITION position;
    pthread_t t0,t1,t2,t3;

    /* 在左下角初始化玩家 */
    y = LINES - 1;
    x = 0;
    attron(COLOR_PAIR(PLAYER_PAIR));
    mvaddch(y, x, PLAYER);
    attroff(COLOR_PAIR(PLAYER_PAIR));
    struct POS_SEND player;
    player.connfd = connfd;
    player.type = 0;
    player.x = x;
    player.y = y; 
    write(connfd,&player,sizeof(player));

    do {
    //在刷新更改的内容之前，发送
    struct POS_SEND player1;
    player1.connfd = connfd;
    player1.type = 0;
    player1.x = x;
    player1.y = y; 
    write(connfd,&player1,sizeof(player1));
    /* 默认获得一个闪烁的光标--表示玩家字符 */
    attron(COLOR_PAIR(PLAYER_PAIR));
    mvaddch(y, x, PLAYER);
    attroff(COLOR_PAIR(PLAYER_PAIR));
    move(y, x);
    refresh();

    ch = getch();
    /* 测试输入的键并获取方向 */
    switch (ch) {
    case KEY_UP:
    case 'w':
    case 'W':
        if ((y > 0) && is_move_okay(y - 1, x)) {
        attron(COLOR_PAIR(GRASS_PAIR));
        mvaddch(y, x, GRASS);
        attroff(COLOR_PAIR(GRASS_PAIR));
        y = y - 1;
        }
        break;
    case KEY_DOWN:
    case 's':
    case 'S':
        if ((y < LINES - 1) && is_move_okay(y + 1, x)) {
        attron(COLOR_PAIR(GRASS_PAIR));
        mvaddch(y, x, GRASS);
        attroff(COLOR_PAIR(GRASS_PAIR));
        y = y + 1;
        }
        break;
    case KEY_LEFT:
    case 'a':
    case 'A':
        if ((x > 0) && is_move_okay(y, x - 1)) {
        attron(COLOR_PAIR(GRASS_PAIR));
        mvaddch(y, x, GRASS);
        attroff(COLOR_PAIR(GRASS_PAIR));
        x = x - 1;
        }
        break;
    case KEY_RIGHT:
    case 'd':
    case 'D':
        if ((x < COLS - 1) && is_move_okay(y, x + 1)) {
        attron(COLOR_PAIR(GRASS_PAIR));
        mvaddch(y, x, GRASS);
        attroff(COLOR_PAIR(GRASS_PAIR));
        x = x + 1;
        }
        break;
    case 'j':
    case 'J':
        position.x = x;
        position.y = y;
        struct POS_SEND bangj;
        bangj.connfd = connfd;
        bangj.type = 1;
        bangj.x = x;
        bangj.y = y; 
        bangj.direction = 'j';
        write(connfd,&bangj,sizeof(bangj));
        if(pthread_create(&t0, NULL, fire_bangJ,(void *)&position) == -1){
            puts("fail to create pthread t0");
            exit(1);
        }
        pthread_detach(t0);    
        break;
    case 'l':
    case 'L':
        position.x = x;
        position.y = y;
        struct POS_SEND bangl;
        bangl.connfd = connfd;
        bangl.type = 1;
        bangl.x = x;
        bangl.y = y; 
        bangl.direction = 'l';
        write(connfd,&bangl,sizeof(bangl));
        if(pthread_create(&t1, NULL, fire_bangL,(void *)&position) == -1){
            puts("fail to create pthread t1");
            exit(1);
        }
        pthread_detach(t1);    
        break;
    case 'i':
    case 'I':
        position.x = x;
        position.y = y;
        struct POS_SEND bangi;
        bangi.connfd = connfd;
        bangi.type = 1;
        bangi.x = x;
        bangi.y = y; 
        bangi.direction = 'i';
        write(connfd,&bangi,sizeof(bangi));
        if(pthread_create(&t2, NULL, fire_bangI,(void *)&position) == -1){
            puts("fail to create pthread t2");
            exit(1);
        }
        pthread_detach(t2);    
        break;
    case 'k':
    case 'K':
        position.x = x;
        position.y = y;
        struct POS_SEND bangk;
        bangk.connfd = connfd;
        bangk.type = 1;
        bangk.x = x;
        bangk.y = y; 
        write(connfd,&bangk,sizeof(bangk));
        bangk.direction = 'k';
        if(pthread_create(&t3, NULL, fire_bangK,(void *)&position) == -1){
            puts("fail to create pthread t3");
            exit(1);
        }
        pthread_detach(t3);    
        break;
    }
    }
    while ((ch != 'q') && (ch != 'Q'));
    endwin();
    exit(0);
}
int main(int argc, char **argv)
{
    const char * servInetAddr = "127.0.0.1";
    int servPort = 8080;
    char buf[MAXLINE];
    int connfd;
    struct sockaddr_in servaddr;
    pthread_t t0;

    if (argc == 2) {
        servInetAddr = argv[1];
    }
    if (argc == 3) {
        servInetAddr = argv[1];
        servPort = atoi(argv[2]);
    }
    if (argc > 3) {
        printf("usage: echoclient <IPaddress> <Port>\n");
        return -1;
    }

    connfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(servPort);
    inet_pton(AF_INET, servInetAddr, &servaddr.sin_addr);
   
    if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        return -1;
    }
    printf("welcome to echoclient\n");
    printf("%d\n",connfd);
    // if(pthread_create(&t0, NULL, handle, (void *)&connfd) == -1){
    //      puts("fail to create pthread t0");
    //      exit(1);
    //  }
    // pthread_detach(t0);
    // int forkid = fork();
    // if(forkid==0){
    //     handle(connfd);     /* do it all */
    // }
    // else{
    //     startGame();
    // }
    startGame(connfd);
    close(connfd);
    printf("exit\n");
    exit(0);
}

void *handle(void *sock_fd)
{
    /*char sendline[MAXLINE], recvline[MAXLINE];
    struct Buffer pread;
    struct Buffer pwrite;*/
    int sockfd = *(int *)sock_fd;
    pthread_t t0,t1;
    if(pthread_create(&t0, NULL, pthread_write,(void *)&sockfd) == -1){
        puts("fail to create pthread t0");
        exit(1);
    }
    if(pthread_create(&t1, NULL, pthread_read,(void *)&sockfd) == -1){
        puts("fail to create pthread t1");
        exit(1);
    }
    pthread_join(t0,NULL);
    pthread_join(t1,NULL);
   
}