#include <bits/stdc++.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#define TITLE "Bubble Shooter"
#define WINDOW "game"
#define MAX_N 20


#define inner_pd(x1,y1,x2,y2) ( (x1)*(x2) + (y1)*(y2) )

using namespace std;
using namespace cv;

void rounded_rectangle( Mat& src, Point topLeft, Point bottomRight, const Scalar lineColor, const int thickness, const int lineType , const int cornerRadius);


typedef struct Fort {
    double x,y;
    double r;
    Scalar color;
}Fort;

typedef struct Bullet {
    double x,y;
    double r;
    double speed;
    Scalar color;
}Bullet;

typedef struct Bubble {
    int id_n;
    string id;
    double x,y;
    double m;
    double r;
    double vx,vy;
    Scalar color;
    bool disappear;
}Bubble;

vector<Bubble> bubbles(0);
Fort fort_;
vector<Bullet> bullets(0);

double delta_t = 0.15;
double Bx_l, Bx_r, By;
double emptySpace = 100, Ex = 15;
int score;
int numCount;
int timer; // second
int difficulty=1;
bool isEnd;
bool retry=false;

int height=1080, width=960+50;
int tt_height=480, tt_width=720;
//Mat img(height, width, CV_8UC3, Scalar(0,95,175));
Mat img2(tt_height, tt_width, CV_8UC3, Scalar(255,235,210));

void setup(int num_of_bubbles, double radius, double mass, double speed, int totalTime, int diff)
{
    score = 0;
    numCount = 0;
    timer = totalTime;
    difficulty = diff;
    isEnd = false;

    delta_t = speed;
    Bx_l = Ex, Bx_r = width - Ex;
    By = height*2/3 + emptySpace -60;

    // fort
    fort_.x = width/2;
    fort_.y = height-40;
    fort_.r = 30;
    fort_.color = Scalar(0,0,0);

    // bubbles
    bubbles.clear();
    bullets.clear();
    for(int i=0; i<num_of_bubbles; i++) {
        Bubble new_bubble;
    RAND:
        stringstream ss;
        ss << i+1;
        new_bubble.id = ss.str(); // id
        new_bubble.id_n = i;

        new_bubble.disappear = false;

        new_bubble.m = mass;
        new_bubble.r = radius;
        new_bubble.x = new_bubble.r + rand() % (int)(Bx_r - 2*new_bubble.r - Bx_l) + Bx_l;
        new_bubble.y = new_bubble.r + rand() % (int)(By - 2*new_bubble.r - emptySpace) + emptySpace;
        for(int j=0; j<i; j++){
            if( pow(new_bubble.x-bubbles[j].x,2) + pow(new_bubble.y-bubbles[j].y,2)
              <= pow(new_bubble.r+bubbles[j].r,2) ) // bubble collides
                goto RAND;
        }
        int x_sign = rand()%2 ? 1 : -1, y_sign = rand()%2 ? 1 : -1;
        new_bubble.vx = x_sign * (8 + rand() % 12);
        new_bubble.vy = y_sign * (8 + rand() % 14);
        new_bubble.color = Scalar( rand()%255, rand()%255, rand()%255 );

        // push back
        bubbles.push_back(new_bubble);
    }
    /*
    for(int i=0;i<bubbles.size();i++)
        cout << bubbles[i].id << " ";
    cout << endl;
    */
}

void drawImg(Mat& img, int num_of_bubbles)
{
    // in boundary
    rectangle(img,Point(Bx_l,emptySpace),Point(Bx_r,By),Scalar(255,255,255),-1,LINE_AA);

    // bubbles
    for(int i=0; i<bubbles.size(); i++) {
        //circle(img,Point(bubbles[i].x,bubbles[i].y),bubbles[i].r,bubbles[i].color,2,LINE_AA);
        circle(img,Point(bubbles[i].x,bubbles[i].y),bubbles[i].r,bubbles[i].color,-1,LINE_AA);
        // id
        if(bubbles[i].id_n < 9) // 單位數
            //putText(img, bubbles[i].id, Point(bubbles[i].x-10,bubbles[i].y+10), 0, 1, bubbles[i].color, 2, LINE_AA);
            putText(img, bubbles[i].id, Point(bubbles[i].x-10,bubbles[i].y+10), 0, 1, Scalar(255,255,255), 2, LINE_AA);
        else // 雙位數
            //putText(img, bubbles[i].id, Point(bubbles[i].x-20,bubbles[i].y+10), 0, 1, bubbles[i].color, 2, LINE_AA);
            putText(img, bubbles[i].id, Point(bubbles[i].x-20,bubbles[i].y+10), 0, 1, Scalar(255,255,255), 2, LINE_AA);
    }

    // boundary
    int bound_thickness=2;
    rectangle(img,Point(Bx_l,emptySpace),Point(Bx_r,By),Scalar(255,0,175),bound_thickness,LINE_AA);

    // bullets
    for(int i=0; i<bullets.size(); i++) {
        circle(img, Point(bullets[i].x,bullets[i].y), bullets[i].r, fort_.color, -1, LINE_AA );
    }

    // fort
    int muzzle_w = 20;
    rectangle(img, Point(fort_.x-muzzle_w,fort_.y-50), Point(fort_.x+muzzle_w,fort_.y+20), Scalar(100,100,100), -1, LINE_AA);
    circle(img, Point(fort_.x,fort_.y), fort_.r, fort_.color, -1, LINE_AA );


    // score
    //rectangle(img, Point(width/2-50, 20), Point(width/2+50, 60), Scalar(255,255,255), -1, LINE_AA);
    char sc[12];
    sprintf(sc, "%d", score);
    putText(img, sc, Point(width/2-10-strlen(sc)*20, 65), 0, 1.7, Scalar(0,0,255), 3, LINE_AA);

    // time
    char t[12];
    int minute = timer / 60, second = timer % 60;
    sprintf(t, "%d:%d", minute, second);
    putText(img, t, Point(20, 45), 0, 1.1, Scalar(0,0,0), 2, LINE_AA); // second

    // next bubble to hit
    if(!isEnd) {
        char text[20] = "next: ";
        putText(img, text, Point(width-190, 50), 0, 1, Scalar(0,0,0), 2, LINE_AA);

        double _x = width-60, _y = 50;
        circle(img,Point(_x,_y),bubbles[0].r,bubbles[0].color,-1,LINE_AA);
        // id
        if(bubbles[0].id_n < 9) // 單位數
            //putText(img, bubbles[i].id, Point(bubbles[i].x-10,bubbles[i].y+10), 0, 1, bubbles[i].color, 2, LINE_AA);
            putText(img, bubbles[0].id, Point(_x-10, _y+10), 0, 1, Scalar(255,255,255), 2, LINE_AA);
        else // 雙位數
            //putText(img, bubbles[i].id, Point(bubbles[i].x-20,bubbles[i].y+10), 0, 1, bubbles[i].color, 2, LINE_AA);
            putText(img, bubbles[0].id, Point(_x-20, _y+10), 0, 1, Scalar(255,255,255), 2, LINE_AA);

        //
        //char text[20] = "next: ", nextid[5];
        //sprintf(nextid, "%d", numCount+1);
        //strcat(text, nextid);
        //putText(img, text, Point(width-150, 40), 0, 1, Scalar(255,255,255), 2, LINE_AA);
        //
    }
}

void showImg(Mat& img, int num_of_bubbles)
{
    drawImg(img, num_of_bubbles);
    imshow(WINDOW,img);
}

void pause()
{
    printf("press any key to resume\n");
    waitKey(0);
    return;
}


void pop_bullet(int i) // pop i-th bullet in vector
{
    bullets.erase( bullets.begin()+i );
}

void pop_bubble(int i) // pop i-th bubble in vector
{
    bubbles.erase( bubbles.begin()+i );
}


void updatePosition(int num_of_bubbles)
{
    // bubbles
    for(int i=0;i<num_of_bubbles;i++)
    {
        bubbles[i].x += bubbles[i].vx * delta_t;
        bubbles[i].y += bubbles[i].vy * delta_t;
    }

    // bullets
    for(int i=0;i<bullets.size();i++)
    {
        bullets[i].y -= bullets[i].speed;
        if( bullets[i].y < emptySpace + bullets[i].r )
            pop_bullet(i--);
    }
}

bool hitWall_LR(int i)  // left and right
{
    if( bubbles[i].x + bubbles[i].r >= Bx_r && bubbles[i].vx > 0 ||
        bubbles[i].x - bubbles[i].r <= Bx_l  && bubbles[i].vx < 0  )
            return true;
    return false;
}

bool hitWall_TB(int i)  // top and bottom
{
    if( bubbles[i].y + bubbles[i].r >= By && bubbles[i].vy > 0 ||
        bubbles[i].y - bubbles[i].r <= emptySpace  && bubbles[i].vy < 0 )
        return true;
    return false;
}

double distance(Bubble &bi, Bubble &bj)
{
    return sqrt((bi.x - bj.x)*(bi.x - bj.x) + (bi.y - bj.y)*(bi.y - bj.y))
            - bi.r - bj.r;
}

double distance(Bubble &bubble, Bullet &bullet)
{
    return sqrt((bubble.x - bullet.x)*(bubble.x - bullet.x) + (bubble.y - bullet.y)*(bubble.y - bullet.y))
            - bubble.r - bullet.r;
}

bool collision(int i, int j)
{
    if( i!=j && distance(bubbles[i], bubbles[j]) <= 0 )
        return true;

    return false;
}

void updateVi(int i, int j, Bubble tmp[])
{
    // normal(j->i)
    // n = Pi - Pj
    double Nx = bubbles[i].x - bubbles[j].x,
            Ny = bubbles[i].y - bubbles[j].y;

    // ( s, t ) = h( a, b )  + k( -b, a )
    // s = ha - kb , t = hb + ka
    // k = (t - sb/a) / (bb/a + a)
    // h = (s + kb) / a

    // i
    double si = bubbles[i].vx, ti = bubbles[i].vy;
    double ki = ( ti - si*Ny/Nx ) / ( Ny*Ny/Nx + Nx );
    double hi = ( si + ki*Ny ) / Nx;
    // j
    double sj = bubbles[j].vx, tj = bubbles[j].vy;
    double kj = ( tj - sj*Ny/Nx ) / ( Ny*Ny/Nx + Nx );
    double hj = ( sj + kj*Ny ) / Nx;


    // 平行切線的分量
    double Vix_paral = -ki * Ny, Viy_paral = ki * Nx;
    // 垂直切線的分量
    double Vix_vert = hi * Nx, Viy_vert = hi * Ny;
    // 平行切線的分量
    double Vjx_paral = -kj * Ny, Vjy_paral = kj * Nx;
    // 垂直切線的分量
    double Vjx_vert = hj * Nx, Vjy_vert = hj * Ny;

    double relative_Vx = Vix_vert - Vjx_vert, relative_Vy = Viy_vert - Vjy_vert;

    if( inner_pd(relative_Vx, relative_Vy, Nx, Ny) < 0 )
    {
        // 計算新的垂直切線分量
        double new_Vix_vert = ( (bubbles[i].m-bubbles[j].m)*Vix_vert + 2*bubbles[j].m*Vjx_vert ) / (bubbles[i].m+bubbles[j].m),
                new_Viy_vert = ( (bubbles[i].m-bubbles[j].m)*Viy_vert + 2*bubbles[j].m*Vjy_vert ) / (bubbles[i].m+bubbles[j].m);


        tmp[i].vx = new_Vix_vert + Vix_paral;
        tmp[i].vy = new_Viy_vert + Viy_paral;
    }
}

void callBackFunc(int event, int x, int y, int flags, void*)
{
    if (x >= fort_.r && x <= width-fort_.r) {
        fort_.x = x;
        //fort_.y = y;
    } else if (x < fort_.r) {
        fort_.x = fort_.r;
    } else if (x > width-fort_.r) {
        fort_.x = width-fort_.r;
    }


    if (event == EVENT_RBUTTONDOWN) //right click
    {

    }
    if (event == EVENT_LBUTTONDBLCLK) //left double click
    {

    }
    if (event == EVENT_RBUTTONDBLCLK) //right double click
    {

    }
    if (event == EVENT_LBUTTONDOWN)  //left click to shoot
	{
        // generate a bullet object and shoot it
        Bullet new_bul;
        new_bul.x = fort_.x;
        new_bul.y = fort_.y;
        new_bul.r = 8;
        new_bul.color = Scalar(0,0,0);
        new_bul.speed = 15;
        bullets.push_back(new_bul);
	}
    //mouse move and flags is left button then write line
	else if (event == EVENT_MOUSEMOVE && (flags&EVENT_FLAG_LBUTTON))
	{

	}
}

void startGame(Mat& img, int num_of_bubbles, double r, double m, double speed, int totalTime, Scalar background_color, int difficulty)
{
    // setup
    setup(num_of_bubbles, r, m, speed, totalTime, difficulty);

    // wait for beginning
    namedWindow(WINDOW);
    showImg(img, num_of_bubbles);
    printf("Press any key to start\n");
    waitKey(0);
    //while( char c = waitKey(0) != ' ' ) ; // enter space to start
    setMouseCallback(WINDOW, callBackFunc);

    printf("START!\n");
    printf("Press Q to quit, press R to restart\n");
    clock_t begin_time = clock();

    while(1){
        if(numCount >= num_of_bubbles || timer <= 0)
            break;

        // time
        clock_t current_time = clock();
        if( totalTime - (current_time - begin_time)/1000 < timer && timer > 0 )
            timer--;

        Mat nextImg(height, width, CV_8UC3, background_color);
        showImg(nextImg, bubbles.size());

        // collision of each bubble
        Bubble tmp_bb[bubbles.size()];
        for(int i=0;i<bubbles.size();i++){
            tmp_bb[i].vx = bubbles[i].vx;
            tmp_bb[i].vy = bubbles[i].vy;
        }
        for(int i=0;i<bubbles.size();i++)
        {
            // hit the bound
            if( hitWall_LR(i) ){
                tmp_bb[i].vx *= -1;
            }
            else if( hitWall_TB(i) ){
                tmp_bb[i].vy *= -1;
            }

            // bubbles collide
            else {
                for(int j=0; j<bubbles.size(); j++){
                    if( j!=i && collision(i,j) ){
                        updateVi(i,j,tmp_bb);
                        break;
                    }
                }
            }

        }
        for(int i=0;i<bubbles.size();i++){
            bubbles[i].vx = tmp_bb[i].vx;
            bubbles[i].vy = tmp_bb[i].vy;
        }

        // bullet touches bubble
        for(int i=0; i<bubbles.size(); i++){
            for(int j=0; j<bullets.size(); j++){
                if( distance(bubbles[i],bullets[j]) < 0 )
                {
                    //printf("Before...\n");
                    //for(int i=0;i<bubbles.size();i++)
                        //printf("b[%d].v=( %lf, %lf ), pos=( %lf, %lf )\n", i, bubbles[i].vx, bubbles[i].vy, bubbles[i].x, bubbles[i].y);

                    pop_bullet(j--);
                    if( bubbles[i].id_n == numCount )
                    {
                        pop_bubble(i--);
                        numCount++;
                        score += 130 - ( 15 * ((totalTime-timer)%7) ) + 14 * difficulty;
                    } else {
                        score -= 20 + 10 * (num_of_bubbles - bubbles.size() + 1);
                    }


                    //printf("num_of_bubbles: %d\n",bubbles.size());
                    /*
                    for(int i=0;i<bubbles.size();i++)
                        cout << bubbles[i].id << " ";
                    cout << endl;
                    */

                    //printf("After...\n");
                    //for(int i=0;i<bubbles.size();i++)
                        //printf("b[%d].v=( %lf, %lf ), pos=( %lf, %lf )\n", i, bubbles[i].vx, bubbles[i].vy, bubbles[i].x, bubbles[i].y);
                }
            }
        }


        updatePosition(bubbles.size());

        char key = waitKey(1);
        if (key == 'q' || key == 'Q') {
            //printf("%d",retry);
            return;
        }
        else if (key == 'r' || key == 'R') {
            retry = true;
            return;
        }
    }

    // end
    isEnd = true;

    Mat nextImg(height, width, CV_8UC3, background_color);
    drawImg(nextImg, bubbles.size());
    // time's up
    if( timer <= 0 ){
        putText(nextImg, "Time's Up!", Point(width/2-350, 500), 4, 4, Scalar(0,0,255), 3, LINE_AA);
        putText(nextImg, "failed...", Point(width/2-120, 700), 4, 1.4, Scalar(100,100,100), 2, LINE_AA);
    } else {
        if( score > 0 )
            putText(nextImg, "YOU WIN!", Point(width/2-300, 500), 4, 4, Scalar(0,0,255), 3, LINE_AA);
        else
            putText(nextImg, "YOU LOSE", Point(width/2-320, 500), 4, 4, Scalar(0,0,255), 3, LINE_AA);

        // show score
        char str[30] = "Your score is ", sc[12];
        sprintf(sc, "%d", score);
        strcat(str, sc);
        putText(nextImg, str, Point(width/2-240, 700), 4, 1.4, Scalar(100,100,100), 2, LINE_AA);
        //
    }
    //
    imshow(WINDOW,nextImg);
    waitKey(0);

}

void callBackFunc_title(int event, int x, int y, int flags, void*)
{
    if( event == EVENT_LBUTTONDOWN )
    {
        // startGame( img, int num_of_bubbles, double r, double m, double speed, int totalTime, color )
        if( tt_width*4/10-20 < x && x < tt_width*6/10+20 && 210 < y && y < 270 ) // EASY mode
        {
            destroyWindow(TITLE);
            do {
                retry = false;
                Mat clear_img(height, width, CV_8UC3, Scalar(255,220,240));
                startGame(clear_img, 12, 38, 50, 0.13, 50, Scalar(255,220,240), 2);
                destroyWindow(WINDOW);
            }while(retry);
        }
        else if( tt_width*4/10-20 < x && x < tt_width*6/10+20 && 300 < y && y < 360 ) // Normal mode
        {
            destroyWindow(TITLE);
            do {
                retry = false;
                Mat clear_img(height, width, CV_8UC3, Scalar(255,220,240));
                startGame(clear_img, 14, 36, 50, 0.155, 55, Scalar(255,220,240), 4);
                destroyWindow(WINDOW);
            }while(retry);
        }
        else if( tt_width*4/10-20 < x && x < tt_width*6/10+20 && 390 < y && y < 450 ) // HARD mode
        {
            destroyWindow(TITLE);
            do {
                retry = false;
                Mat clear_img(height, width, CV_8UC3, Scalar(255,220,240));
                startGame(clear_img, 17, 35, 50, 0.17, 60, Scalar(255,220,240), 6);
                destroyWindow(WINDOW);
            }while(retry);
        }
    }
}

void drawTitle(Mat &img)
{
    putText(img, "Bubble Shooter", Point(tt_width/2-220, 130), 0, 1.9, Scalar(230,135,45), 3, LINE_AA);

    for(int i=0;i<30;i++){   // fill in color
        rounded_rectangle(img, Point(tt_width*4/10-20+i,210+i), Point(tt_width*6/10+20-i,270-i), Scalar(95,235,135), 2, LINE_AA, 30); // EASY mode
        rounded_rectangle(img, Point(tt_width*4/10-20+i,300+i), Point(tt_width*6/10+20-i,360-i), Scalar(0,135,255), 2, LINE_AA, 30); // Normal mode
        rounded_rectangle(img, Point(tt_width*4/10-20+i,390+i), Point(tt_width*6/10+20-i,450-i), Scalar(45,45,255), 2, LINE_AA, 30); // HARD mode
    }
    rounded_rectangle(img, Point(tt_width*4/10-20,210), Point(tt_width*6/10+20,270), Scalar(65,205,105), 2, LINE_AA, 30);
    putText(img, "Easy", Point(tt_width*4/10+32,252), 2, 1.1, Scalar(255,255,255), 2, LINE_AA); // EASY mode
    rounded_rectangle(img, Point(tt_width*4/10-20,300), Point(tt_width*6/10+20,360), Scalar(0,105,225), 2, LINE_AA, 30);
    putText(img, "Normal", Point(tt_width*4/10+10,342), 2, 1.1, Scalar(255,255,255), 2, LINE_AA); // Normal mode
    rounded_rectangle(img, Point(tt_width*4/10-20,390), Point(tt_width*6/10+20,450), Scalar(15,15,225), 2, LINE_AA, 30);
    putText(img, "Hard", Point(tt_width*4/10+30,432), 2, 1.1, Scalar(255,255,255), 2, LINE_AA); // HARD mode
}

void title(Mat &img)
{
    namedWindow(TITLE);
    drawTitle(img);
    imshow(TITLE,img);
    waitKey(1);
    setMouseCallback(TITLE,callBackFunc_title);

    while(1){
        drawTitle(img);
        imshow(TITLE,img);
        waitKey(1);
        setMouseCallback(TITLE,callBackFunc_title);
    }
}

int main()
{
    int n = 9; // num_of_bubbles
    double r = 40; // radius
    double m = 50;

    while(1)
        title(img2);
    //startGame(img,n,r,m);


    return 0;
}

/*
int key = waitKey(0);
if(key == 'q' || key == 'Q')
    printf("press q\n");
else
    printf("else\n");
*/

/*
// draw colorful line
for(int i=20; ;i=(i+1)%255){
    line(img, Point(100,100), Point(200,200), Scalar(i,100,200-i), 2, LINE_AA);
    imshow(WINDOW,img);
    waitKey(5);
}
*/



// Reference
void rounded_rectangle( Mat& src, Point topLeft, Point bottomRight, const Scalar lineColor, const int thickness, const int lineType , const int cornerRadius)
{
    /* corners:
     * p1 - p2
     * |     |
     * p4 - p3
     */
    Point p1 = topLeft;
    Point p2 = Point (bottomRight.x, topLeft.y);
    Point p3 = bottomRight;
    Point p4 = Point (topLeft.x, bottomRight.y);

    // draw straight lines
    line(src, Point (p1.x+cornerRadius,p1.y), Point (p2.x-cornerRadius,p2.y), lineColor, thickness, lineType);
    line(src, Point (p2.x,p2.y+cornerRadius), Point (p3.x,p3.y-cornerRadius), lineColor, thickness, lineType);
    line(src, Point (p4.x+cornerRadius,p4.y), Point (p3.x-cornerRadius,p3.y), lineColor, thickness, lineType);
    line(src, Point (p1.x,p1.y+cornerRadius), Point (p4.x,p4.y-cornerRadius), lineColor, thickness, lineType);

    // draw arcs
    ellipse( src, p1+Point(cornerRadius, cornerRadius), Size( cornerRadius, cornerRadius ), 180.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p2+Point(-cornerRadius, cornerRadius), Size( cornerRadius, cornerRadius ), 270.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p3+Point(-cornerRadius, -cornerRadius), Size( cornerRadius, cornerRadius ), 0.0, 0, 90, lineColor, thickness, lineType );
    ellipse( src, p4+Point(cornerRadius, -cornerRadius), Size( cornerRadius, cornerRadius ), 90.0, 0, 90, lineColor, thickness, lineType );
}
