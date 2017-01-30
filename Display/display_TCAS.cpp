#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <thread>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/uio.h>
#include <iomanip>

// Classes
class to_display
{
public:
    to_display();
    to_display(double,double,double,double,int);
    double bearing; // º
    double distance; // nm
    double d_altitude; // ft
    double v_speed; // ft/min
    int type; // 0 - RA, 1 - TA, 2 - OT, 3 - PT
};

using namespace cv;
using namespace std;

#define PI 3.14159265
#define NPLANES 10

/// Function headers
// threads headers
void UpdateVerticalSpeedThread();
void UpdatePlanesInformation();
void DisplayThread();

// data from simulator headers
void newSocks(int* sockback, struct sockaddr_in* sockadd_back, int portno, bool send, const char * specific_addr = "Nope" );
void receiveData(int sockBROAD, double * vU, int * climb);
void receiveDataList(int sockBROAD, std::vector< to_display >& display_l);

// sound header
void PlaySoundIfResolving();
void PlaySoundClearOfConflict();
void PlaySoundTraffic();

// draw headers
void writeDispMode();
void drawRange();
void writeTCASMode();
int drawPlane(int altitude);
void writeOnDisplay(Point *position, string text,
                    CvScalar color, CvScalar colorBackground,
                    double textScale, int dispNum);
void drawArrow( Mat img, double verticalSpeed );
void addPlaneToDisplay(int symbol, int bearing, double distance, int altitude, double verticalSpeed);
static void Circle( Mat& img, Point center, int radius, const void* color, int fill );
void DrawMarker(Mat& img, Point position, const void* color, int markerType, int markerSize, int thickness, int line_type);

// buttons headers
bool insideButton(int x, int y, Point topLeft, Point btmRight);
void CallBackFunc(int event, int x, int y, int flags, void* userdata);

/// Global Variables ///

/// Threads
int turnOff = 0;
int playOffSound = 0;

/// Frame Matrix

Mat frame;

/// Default

int previousResolution = 0;
int resolution = 0;
int nTraffic = 0;

// default selected buttons
int displayMode = 0; // 0 by default is NORM
int tcasMode = 2; // 2 by default is TA/RA

// range by default is 5 nmi
int range = 5;
int rangeIndex = 1;
// other possible range values
int rangeArray[6] = {3, 5, 10, 15, 20, 40};

int numMovPerSec = 20; // The larger the  value the smoother the arrow moves
int moveCounter = 0;
double verticalSpeed = 0.0;
double movingArrow = 0.0;
double dispVerticalSpeed = 0.0;

/// Buttons positions

// Left buttons
Point NORM_top_left(32, 226);
Point NORM_bot_right(162, 300);
Point ALL_top_left(32, 319);
Point ALL_bot_right(162, 392);
Point ABV_top_left(32, 407);
Point ABV_bot_right(162, 478);
Point BLW_top_left(32, 500);
Point BLW_bot_right(162, 572);

// Center down buttons
Point Minus_top_left(233, 676);
Point Minus_bot_right(356, 761);
Point Plus_top_left(686, 677);
Point Plus_bot_right(818, 762);

// Right buttons
Point Off_top_left(893, 228);
Point Off_bot_right(1007, 303);
Point TA_top_left(892, 318);
Point TA_bot_right(1011, 392);
Point TARA_top_left(893, 413);
Point TARA_bot_right(1006, 486);

/**
 * @function main
 * @brief Main function
 */
int main( void ){

    //Launch the UpdateVerticalSpeed thread
    std::thread t1(UpdateVerticalSpeedThread);
    //Launch the UpdatePlanesInformation thread
    std::thread t2(UpdatePlanesInformation);
    //Launch the Display thread
    std::thread t3(DisplayThread);

    //Join the thread with the main thread
    t1.join();
    t2.join();
    t3.join();

    return(0);
}

/// Threads

void UpdateVerticalSpeedThread(){

    int socket;
    struct sockaddr_in server;

    newSocks(&socket, &server, 8998, false, "Nope");

    for(;;){

        cout << "Updating Vertical Speed" << endl;
        previousResolution = resolution;
        receiveData(socket, &verticalSpeed, &resolution);
        verticalSpeed = (verticalSpeed / 1000);

        if(previousResolution == 0 && resolution != 0)
            PlaySoundIfResolving();
        if(previousResolution != 0 && resolution == 0)
            PlaySoundClearOfConflict();

        if(verticalSpeed != dispVerticalSpeed){
            if(verticalSpeed >= 0){
                movingArrow = ((verticalSpeed - dispVerticalSpeed) / numMovPerSec);
            }else{
                movingArrow = ((verticalSpeed - dispVerticalSpeed) / numMovPerSec);
            }
            moveCounter = 0;
        }

        if(turnOff == 1)
            return;
    }
}

struct plane {
    int symbol;
    int bearing;
    double distance;
    int altitude;
    double verticalSpeed;
} tmpPlane;

std::multimap<int,plane> plane_map;

void UpdatePlanesInformation(){

    int sockfd_disp;
    struct sockaddr_in serv_addr_disp;

    newSocks( &sockfd_disp, &serv_addr_disp, 8678, false, "Nope");

    std::vector<to_display> display;

    for(;;){

        cout << "Updating Planes Information" << endl;
        plane_map.clear();
        ///*
        receiveDataList(sockfd_disp, display);

        for(int i = 0; i < display.size(); i++){

            tmpPlane.symbol = display[i].type;
            tmpPlane.bearing = (int)display[i].bearing + 90;
            if(tmpPlane.bearing > 180)
                tmpPlane.bearing = 180 - tmpPlane.bearing;
            tmpPlane.distance = display[i].distance;
            tmpPlane.verticalSpeed = (display[i].v_speed/1000);

            plane_map.insert(std::pair<int,plane>(tmpPlane.symbol, tmpPlane));
        }
        //*/

        /*
        for(int i = 0; i < NPLANES; i++){

            // TODO : change this
            if(i == 0){
                tmpPlane.symbol = 0;
                tmpPlane.bearing = 16;
                tmpPlane.distance = 3.4;
                tmpPlane.altitude = 1200;
                tmpPlane.verticalSpeed = 505;
                plane_map.insert(std::pair<int,plane>(tmpPlane.symbol, tmpPlane));
            }

            if(i == 1){
                tmpPlane.symbol = 1;
                tmpPlane.bearing = 135;
                tmpPlane.distance = 10;
                tmpPlane.altitude = 800;
                tmpPlane.verticalSpeed = -10;
                plane_map.insert(std::pair<int,plane>(tmpPlane.symbol, tmpPlane));
            }
        }
         //*/

        int newTA = 0;

        if(nTraffic == 0 && newTA == 1)
            PlaySoundTraffic();

        ///*
        cout << "Added Planes:" << endl;
        std::multimap<int,plane>::iterator it;
        for(it = plane_map.begin(); it != plane_map.end(); it++){
            plane tmpP = it->second;

            cout << tmpP.symbol << "|" << tmpP.bearing << "|"
                 << tmpP.distance << "|" << tmpP.altitude
                 << "|" << tmpP.verticalSpeed << endl;
        }
        // */

        if(turnOff == 1)
            return;

        //this_thread::sleep_for( chrono::milliseconds( 1000 ) );
    }
}

void PlaySoundIfResolving() {

    if (resolution == 1){
        // play sound to climb
        system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/climb.ogg");
        this_thread::sleep_for( chrono::milliseconds( 200 ) );
        system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/climb.ogg");

    }else if(resolution == 2){
        // play sound to descend
        system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/descend.ogg");
        this_thread::sleep_for( chrono::milliseconds( 200 ) );
        system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/descend.ogg");
    }
}

void PlaySoundClearOfConflict(){
    system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/clear_of_conflict.ogg");
    nTraffic = 0;
}

void PlaySoundTraffic(){
    system("canberra-gtk-play -f /home/tribta/Downloads/traffic.ogg");
    this_thread::sleep_for( chrono::milliseconds( 100 ) );
    system("canberra-gtk-play -f /home/tribta/Downloads/traffic.ogg");
}

void DisplayThread() {

    cout << "Starting the Display" << endl;

    /// Window name
    char tcas_window[] = "TCAS";

    /// Create the window and set it to full screen
    cvNamedWindow(tcas_window, CV_WINDOW_NORMAL);
    cvSetWindowProperty(tcas_window, CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);

    for(;;) {

        /// Load background image
        if(resolution == 0 || tcasMode != 2)
            frame = imread("/home/tribta/Dropbox/Github/TCAS/Display/RaspberryPi_0.jpg", CV_LOAD_IMAGE_COLOR);
        else if(resolution == 1)
            frame = imread("/home/tribta/Dropbox/Github/TCAS/Display/RaspberryPi_1.jpg", CV_LOAD_IMAGE_COLOR);
        else if(resolution == 2)
            frame = imread("/home/tribta/Dropbox/Github/TCAS/Display/RaspberryPi_2.jpg", CV_LOAD_IMAGE_COLOR);

        /// Resize to IPAD resolution
        Size size(1024, 768);
        cv::resize(frame, frame, size);

        // If not OFF
        if(tcasMode != 0){

            /// Write the TCAS Mode
            writeTCASMode();

            /// Write the Disp Mode
            writeDispMode();

            /// Draw range
            drawRange();

            /// Draw arrow
            // move the arrow smoothly
            if(moveCounter < numMovPerSec){
                dispVerticalSpeed += movingArrow;
                moveCounter++;
            }
            drawArrow(frame, dispVerticalSpeed);


            /// Draw Planes

            std::multimap<int,plane>::iterator it;

            for(it = plane_map.begin(); it != plane_map.end(); it++){

                plane tmpP = it->second;
                int tmpAltitude = tmpP.altitude;

                if(drawPlane(tmpAltitude) == 1) {
                    addPlaneToDisplay(tmpP.symbol, tmpP.bearing,
                                      tmpP.distance, tmpAltitude,
                                      tmpP.verticalSpeed);
                }
            }

        }else{
            writeTCASMode();

            if(playOffSound > 0)
                playOffSound++;
        }

        /// Display the stuff!
        imshow(tcas_window, frame);

        /// Mouse Clicks
        cv::setMouseCallback(tcas_window, CallBackFunc, NULL);

        if(playOffSound == 3){
            system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/beep-09.wav");
            playOffSound = 0;
        }

        /// Press 'ESC' to stop the code
        if (waitKey(33)==27){
            turnOff = 1;
            return;
        }
    }
}

/// Other Functions Declaration

void newSocks(int* sockback, struct sockaddr_in* sockadd_back, int portno, bool send, const char * specific_addr)
{
    // Estruturas dos Sockets
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int sockfd;
    int so_broadcast;
    int so_reuseaddr;

    // Receive
    socklen_t clilen;
    struct sockaddr_in cli_addr;

    //Socket creation
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        cout << "ERROR opening socket" << endl;

    so_broadcast = 1;
    so_reuseaddr = 1;
    // Configure Socket (enable broadcasting)
    setsockopt( sockfd,
                SOL_SOCKET,
                SO_BROADCAST,
                &so_broadcast,
                sizeof(so_broadcast));
    // Configure Socket (enable adressreuse)
    setsockopt( sockfd,
                SOL_SOCKET,
                SO_REUSEADDR,
                &so_reuseaddr,
                sizeof(so_reuseaddr));

    // Configure socckaddr
    bzero((char *) &serv_addr, sizeof(serv_addr));
    // Family
    serv_addr.sin_family = AF_INET;
    // Adress
    if(send)
    {
        // Get Address
        server = gethostbyname("localhost");
        if (server == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
        }

        // Caso address especifico
        if(strcmp(specific_addr,"Nope") !=0 )
        {
            serv_addr.sin_addr.s_addr = inet_addr(specific_addr);
        }
        else
        {
            serv_addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
        }
    }
    else
    {
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }

    // Port
    serv_addr.sin_port = htons(portno);

    // Connect or Bind
    if(send)
    {
        if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            cout << "ERROR opening socket" << endl;
    }
    else
    {
        if (bind(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
            cout << "ERROR opening socket" << endl;
    }

    // Return
    *sockback = sockfd;
    *sockadd_back = serv_addr;
}


void receiveData(int sockBROAD, double * vU, int * climb)
{
    struct sockaddr_storage src_addr;

    /*Structured*/

    // Formato da mensagem
    struct iovec io[2];
    io[0].iov_base = vU;
    io[0].iov_len = sizeof(vU);
    io[1].iov_base = climb;
    io[1].iov_len = sizeof(climb);


    struct msghdr message;
    message.msg_name=&src_addr;
    message.msg_namelen=sizeof(src_addr);
    message.msg_iov=io;
    message.msg_iovlen=2;
    message.msg_control=0;
    message.msg_controllen=0;

    // receção de mensagens
    ssize_t count=recvmsg(sockBROAD,&message,0);
    if (count==-1) {
        printf("ERROR reading from socket");
    } else if (message.msg_flags&MSG_TRUNC) {
        printf("datagram too large for buffer: truncated\n");
    }
}

to_display::to_display()
{
    d_altitude = 0;
    bearing = 0;
    distance = 0;
    v_speed = 0;
    type = 0;
}

to_display::to_display(double d_alt, double bea, double dist,double v_sp,int t)
{
    d_altitude = d_alt;
    bearing = bea;
    distance = dist;
    v_speed = v_sp;
    type = t;
}

void receiveDataList(int sockBROAD, std::vector< to_display >& display_l)
{
    struct sockaddr_storage src_addr;

    int i, offset;
    // Limite de Aeronaves
    int list_size = 15;
    // Vector Buffer
    std::vector< to_display > airspace(list_size);
    std::vector< to_display > reducted_airspace;

    // Formata a mensagem a enviar
    struct iovec io[5*list_size];
    for(i=0;i<list_size;i++)
    {
        offset = i *5;

        io[0 + offset].iov_base = &display_l[i].type;
        io[0 + offset].iov_len = sizeof(display_l[i].type);
        io[1 + offset].iov_base = &display_l[i].bearing;
        io[1 + offset].iov_len = sizeof(display_l[i].bearing);
        io[2 + offset].iov_base = &display_l[i].distance;
        io[2 + offset].iov_len = sizeof(display_l[i].distance);
        io[3 + offset].iov_base = &display_l[i].d_altitude;
        io[3 + offset].iov_len = sizeof(display_l[i].d_altitude);
        io[4 + offset].iov_base = &display_l[i].v_speed;
        io[4 + offset].iov_len = sizeof(display_l[i].v_speed);

    }

    struct msghdr message;
    message.msg_name=&src_addr;
    message.msg_namelen=sizeof(src_addr);
    message.msg_iov=io;
    message.msg_iovlen=5*list_size;
    message.msg_control=0;
    message.msg_controllen=0;

    // receção de mensagens
    ssize_t count=recvmsg(sockBROAD,&message,0);
    if (count==-1) {
        perror("Error Planes Data:");
    } else if (message.msg_flags&MSG_TRUNC) {
        printf("datagram too large for buffer: truncated\n");
    } else {
        to_display aux;
        // Determina numero de avioes na lista
        int n_aeroplanes = count / 36;

        for(i=0;i<n_aeroplanes;i++)
        {
            aux = airspace[i];

            reducted_airspace.push_back(aux);
        }
    }
    display_l = reducted_airspace;
}


void writeTCASMode() {
    if(tcasMode == 0){
        writeOnDisplay( new Point(800, 152), "OFF",
                        cvScalar(40, 100, 255),
                        cvScalar(0, 0, 100), 1.6, 2);
    }else if(tcasMode == 1){
        writeOnDisplay( new Point(813, 152), "TA",
                        cvScalar(60, 200, 200),
                        cvScalar(0, 100, 100), 1.7, 2);
    }else{
        writeOnDisplay( new Point(790, 148), "TA|RA",
                        cvScalar(60, 200, 200),
                        cvScalar(0, 100, 100), 1.3, 2);
    }
}

void writeDispMode() {
    if(displayMode == 0){
        writeOnDisplay( new Point(132, 146), "NORM",
                        cvScalar(0, 170, 0),
                        cvScalar(0, 100, 0), 1.5, 1);
    }else if(displayMode == 1){
        writeOnDisplay( new Point(150, 150), "ALL",
                        cvScalar(0, 170, 0),
                        cvScalar(0, 100, 0), 1.7, 1);
    }else if(displayMode == 2){
        writeOnDisplay( new Point(147, 150), "ABV",
                        cvScalar(0, 170, 0),
                        cvScalar(0, 100, 0), 1.7, 1);
    }else{
        writeOnDisplay( new Point(145, 150), "BLW",
                        cvScalar(0, 170, 0),
                        cvScalar(0, 100, 0), 1.7, 1);
    }
}

void drawRange() {
    // Write Range, adjust position to write depending on the number of digits
    if(range < 10){
        writeOnDisplay( new Point(790, 650), std::to_string(range),
                        cvScalar(255, 90, 90),
                        cvScalar(150, 0, 0), 1.7, 3);
    }else{
        writeOnDisplay( new Point(775, 650), std::to_string(range),
                        cvScalar(255, 90, 90),
                        cvScalar(150, 0, 0), 1.5, 3);
    }
}

int drawPlane(int altitude) {

    if(displayMode == 0){ // NORM
        if(altitude > 2700 || altitude < -2700)
            return  0;
    }else if(displayMode == 2){ // ABV
        if(altitude < -2700 || altitude > 9000)
            return 0;
    }else if(displayMode == 3){ // BLW
        if(altitude > 2700 || altitude < -9000)
            return 0;
    }
    return 1;
}

void writeOnDisplay(Point *position, string text, CvScalar color, CvScalar colorBackground, double textScale, int dispNum) {

    cv::putText(frame, text, *position,
                FONT_HERSHEY_DUPLEX, textScale, colorBackground, 2, CV_AA);

    cv::Mat mask;

    if(dispNum == 1)
        mask = frame(cv::Rect(Point(118, 92), Point(278, 171)));
    else if(dispNum == 2)
        mask = frame(cv::Rect(Point(781, 92), Point(909, 174)));
    else
        mask = frame(cv::Rect(Point(774, 596), Point(843, 675)));

    GaussianBlur(mask, mask,  Size(55, 55), 0);

    cv::putText(frame, text, *position,
                FONT_HERSHEY_DUPLEX, textScale, color, 1, CV_AA);
}

void drawArrow( Mat img, double verticalSpeed ) {

    Point2f centre(525,394);
    int arrowLength;
    double angle;
    bool isClimbing = true;

    if(verticalSpeed < 0){
        isClimbing = false;
        verticalSpeed = -verticalSpeed;
    }

    if(verticalSpeed > 6)
        verticalSpeed = 6;

    if(verticalSpeed < 1)
        angle = verticalSpeed * 66;
    else
        angle = 60 * log(verticalSpeed) + 66;

    arrowLength = 265;

    Point d;
    double portionToDraw = 0.3;

    if(isClimbing == true){
        d = Point(centre.x - arrowLength * cos(angle*PI/180),centre.y - arrowLength * sin(angle*PI/180));
        centre.y -= portionToDraw * arrowLength * sin(angle*PI/180);
    }else{
        d = Point(centre.x - arrowLength * cos(angle*PI/180),centre.y + arrowLength * sin(angle*PI/180));
        centre.y += portionToDraw * arrowLength * sin(angle*PI/180);
    }

    centre.x -= portionToDraw * arrowLength * cos(angle*PI/180);

    CvScalar color = CV_RGB(255, 255, 255);
    int thickness = 3;
    int line_type = 8;
    double tipLength = 0.3;

    arrowedLine(img, centre, d, color, thickness, line_type, 0, tipLength);
}

void addPlaneToDisplay(int symbol, int bearing, double distance, int altitude, double verticalSpeed) {

    int textScale = 1;
    double rangeNmi;

    // using symmetry (make angle between -90 to 90)
    int tmpAng = bearing;
    if(tmpAng>90)
        tmpAng = 180 - tmpAng;
    else if(tmpAng<-90)
        tmpAng = -180 - tmpAng;

    // 120% of range to the front, 80% to the sides, 55% backwards (see report)
    rangeNmi = (9.259259259e-6*pow(tmpAng, 2) + 3.611111111e-3*tmpAng + 0.8) * range;

    // if it is an intruder (symbol = 0 || 1) draw partially the symbol
    // for the pilot to increase the range scale
    if(distance <= rangeNmi || symbol == 0 || symbol == 1){

        // Plane position in the display
        Point2f centre(526,504);

        double distFromCenter;

        // get the pixel distance depending on the angle
        if(tmpAng < 0){
            tmpAng = -tmpAng; //log must be >0
            if(tmpAng < 10)
                distFromCenter = -6*log(tmpAng) + 236;
            else
                distFromCenter = -34*log(tmpAng) + 305;
        }else{
            if(tmpAng >= 22)
                distFromCenter = 74*log(tmpAng) + 50;
            else if(tmpAng >= 12)
                distFromCenter = 35*log(tmpAng) + 170;
            else
                distFromCenter = 8*log(tmpAng) + 234;
        }

        // since log(0) doesn't exist it's a special case
        if(tmpAng == 0)
            distFromCenter = 240;

        // here we scale the distance to know where to draw the other plane
        if(distance < rangeNmi)
            distFromCenter = (distance/rangeNmi)*distFromCenter;
        // otherwise we write it in the biggest possible distance

        Point d;

        d = Point(centre.x - distFromCenter * cos(bearing*PI/180),centre.y - distFromCenter * sin(bearing*PI/180));

        Vec3b color;

        if(symbol == 0){
            /// Draw Square
            color = Vec3b(0, 0, 255);
            DrawMarker(frame, d, &color, MARKER_SQUARE, 25, 5, 8);
            DrawMarker(frame, d, &color, MARKER_SQUARE, 16, 5, 8);
            DrawMarker(frame, d, &color, MARKER_SQUARE, 6, 6, 8);
        }else if(symbol == 1){
            /// Draw circle
            color = Vec3b(70, 255, 255);
            int circleSize = 15;
            Circle(frame, d, circleSize, &color, 1);
        }else if(symbol == 2){
            /// Draw Full Diamond
            color = Vec3b(255, 250, 150);
            DrawMarker(frame, d, &color, MARKER_DIAMOND, 27, 4, 8);
            DrawMarker(frame, d, &color, MARKER_DIAMOND, 15, 5, 8);
            DrawMarker(frame, d, &color, MARKER_DIAMOND, 4, 5, 8);
        }else if(symbol == 3){
            /// Draw Opened Diamond
            color = Vec3b(255, 250, 150);
            DrawMarker(frame, d, &color, MARKER_DIAMOND, 29, 4, 8);
        }

        // Convert altitude to string
        string altitude_str;
        altitude = altitude / 100;
        if(altitude < 0){
            if(altitude > -10){
                altitude_str = "-0"+std::to_string(altitude);
            }else{
                altitude_str = "-"+std::to_string(altitude);
            }
        }else{
            if(altitude < 10){
                altitude_str = "0"+std::to_string(altitude);
            }else{
                altitude_str = std::to_string(altitude);
            }
        }

        // shift that the arrow needs to do for each number written
        int shift = 24*altitude_str.length();

        if (verticalSpeed <= 0.5 && verticalSpeed >= -0.5)
            shift -= 17;

        int distx = shift, disty = 55;

        if(bearing < 0)
            disty = 30;

        d.x = d.x - 14*altitude_str.length();

        Point textP(d.x + distx * cos(bearing*PI/180) , d.y + disty * sin(bearing*PI/180));

        cv::putText(frame, altitude_str, textP, FONT_HERSHEY_DUPLEX,
                    textScale, color, 1, CV_AA);

        if (verticalSpeed > 0.5){
            // up arrow
            arrowedLine(frame, cvPoint(textP.x + shift, textP.y),
                        cvPoint(textP.x + shift, textP.y - 20), color, 2, 8, 0, 0.35);
        } else if(verticalSpeed < -0.5){
            //down arrow
            arrowedLine(frame, cvPoint(textP.x + shift, textP.y - 20),
                        cvPoint(textP.x + shift, textP.y), color, 2, 8, 0, 0.35);
        }
    }
}

bool insideButton(int x, int y, Point topLeft, Point btmRight) {
    return (x > topLeft.x && y > topLeft.y &&
            x < btmRight.x && y < btmRight.y);
}

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
    if  ( event == EVENT_LBUTTONDOWN )
    {
        //cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;

        /// Check if any of the buttons was clicked

        if(insideButton(x, y, Off_top_left, Off_bot_right)){
            tcasMode = 0;
            playOffSound = 1;
        }else if(insideButton(x, y, TA_top_left, TA_bot_right)){
            tcasMode = 1;
            system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
        }else if(insideButton(x, y, TARA_top_left, TARA_bot_right)){
            tcasMode = 2;
            system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
        }

        // if not turned off
        if(tcasMode != 0){
            if(insideButton(x, y, Plus_top_left, Plus_bot_right)){
                if(rangeIndex < 5) {
                    rangeIndex++;
                    range = rangeArray[rangeIndex];
                }
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }else if(insideButton(x, y, Minus_top_left, Minus_bot_right)){
                if(rangeIndex > 0) {
                    rangeIndex--;
                    range = rangeArray[rangeIndex];
                }
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }else if(insideButton(x, y, NORM_top_left, NORM_bot_right)) {
                displayMode = 0;
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }else if(insideButton(x, y, ALL_top_left, ALL_bot_right)) {
                displayMode = 1;
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }else if(insideButton(x, y, ABV_top_left, ABV_bot_right)) {
                displayMode = 2;
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }else if(insideButton(x, y, BLW_top_left, BLW_bot_right)) {
                displayMode = 3;
                system("canberra-gtk-play -f /home/tribta/Dropbox/Github/TCAS/TCAS_sound/button-30.wav");
            }
        }

        //cout << frame.col(x).row(y) << endl;
        /*
        Point center(526,504);
        double distance = sqrt(pow(center.x-x, 2) + pow(center.y-y, 2));
        double angle =  (180/PI)*acos(((double)(center.x-x))/distance);
        if(y>center.y)
            angle = -angle;

        cout << angle << " " << distance << endl;
        //*/
    }
}

/// Changing OpenCV code to avoid drawing the symbols outside the main display
/// Using the color of the background (13, 5, 7) of the main display to draw
/// This only works if the main display has the RGB values between 5 and 15

#define ICV_HLINE( ptr, xl, xr, color, pix_size )            \
{                                                            \
    uchar* hline_ptr = (uchar*)(ptr) + (xl)*(pix_size);      \
    uchar* hline_max_ptr = (uchar*)(ptr) + (xr)*(pix_size);  \
                                                             \
    for( ; hline_ptr <= hline_max_ptr; hline_ptr += (pix_size))\
    {                                                        \
        int hline_j;                                         \
                                                              \
        if((int)hline_ptr[0] < 18 && (int)hline_ptr[0] > 10)      \
            if((int)hline_ptr[1] < 12 && (int)hline_ptr[1] > 5)    \
                if((int)hline_ptr[2] < 12 && (int)hline_ptr[2] > 5)\
                    for( hline_j = 0; hline_j < (pix_size); hline_j++ )  \
                    {                                                    \
                        hline_ptr[hline_j] = ((uchar*)color)[hline_j];\
                    }                                                    \
    }                                                        \
}

/* draws simple or filled circle */
static void
Circle( Mat& img, Point center, int radius, const void* color, int fill )
{
    Size size = img.size();
    size_t step = img.step;
    int pix_size = (int)img.elemSize();
    uchar* ptr = img.ptr();
    int err = 0, dx = radius, dy = 0, plus = 1, minus = (radius << 1) - 1;
    int inside = center.x >= radius && center.x < size.width - radius &&
                 center.y >= radius && center.y < size.height - radius;

#define ICV_PUT_POINT( ptr, x )     \
        memcpy( ptr + (x)*pix_size, color, pix_size );

    while( dx >= dy )
    {
        int mask;
        int y11 = center.y - dy, y12 = center.y + dy, y21 = center.y - dx, y22 = center.y + dx;
        int x11 = center.x - dx, x12 = center.x + dx, x21 = center.x - dy, x22 = center.x + dy;

        if( inside )
        {
            uchar *tptr0 = ptr + y11 * step;
            uchar *tptr1 = ptr + y12 * step;

            if( !fill )
            {
                ICV_PUT_POINT( tptr0, x11 );
                ICV_PUT_POINT( tptr1, x11 );
                ICV_PUT_POINT( tptr0, x12 );                //cout << (int)*tptr0 << endl;
                ICV_PUT_POINT( tptr1, x12 );
            }
            else
            {
                ICV_HLINE( tptr0, x11, x12, color, pix_size );
                ICV_HLINE( tptr1, x11, x12, color, pix_size );
            }

            tptr0 = ptr + y21 * step;
            tptr1 = ptr + y22 * step;

            if( !fill )
            {
                ICV_PUT_POINT( tptr0, x21 );
                ICV_PUT_POINT( tptr1, x21 );
                ICV_PUT_POINT( tptr0, x22 );
                ICV_PUT_POINT( tptr1, x22 );
            }
            else
            {
                ICV_HLINE( tptr0, x21, x22, color, pix_size );
                ICV_HLINE( tptr1, x21, x22, color, pix_size );
            }
        }
        else if( x11 < size.width && x12 >= 0 && y21 < size.height && y22 >= 0 )
        {
            if( fill )
            {
                x11 = std::max( x11, 0 );
                x12 = MIN( x12, size.width - 1 );
            }

            if( (unsigned)y11 < (unsigned)size.height )
            {
                uchar *tptr = ptr + y11 * step;

                if( !fill )
                {
                    if( x11 >= 0 )
                        ICV_PUT_POINT( tptr, x11 );
                    if( x12 < size.width )
                        ICV_PUT_POINT( tptr, x12 );
                }
                else
                ICV_HLINE( tptr, x11, x12, color, pix_size );
            }

            if( (unsigned)y12 < (unsigned)size.height )
            {
                uchar *tptr = ptr + y12 * step;

                if( !fill )
                {
                    if( x11 >= 0 )
                        ICV_PUT_POINT( tptr, x11 );
                    if( x12 < size.width )
                        ICV_PUT_POINT( tptr, x12 );
                }
                else
                ICV_HLINE( tptr, x11, x12, color, pix_size );
            }

            if( x21 < size.width && x22 >= 0 )
            {
                if( fill )
                {
                    x21 = std::max( x21, 0 );
                    x22 = MIN( x22, size.width - 1 );
                }

                if( (unsigned)y21 < (unsigned)size.height )
                {
                    uchar *tptr = ptr + y21 * step;

                    if( !fill )
                    {
                        if( x21 >= 0 )
                            ICV_PUT_POINT( tptr, x21 );
                        if( x22 < size.width )
                            ICV_PUT_POINT( tptr, x22 );
                    }
                    else
                    ICV_HLINE( tptr, x21, x22, color, pix_size );
                }

                if( (unsigned)y22 < (unsigned)size.height )
                {
                    uchar *tptr = ptr + y22 * step;

                    if( !fill )
                    {
                        if( x21 >= 0 )
                            ICV_PUT_POINT( tptr, x21 );
                        if( x22 < size.width )
                            ICV_PUT_POINT( tptr, x22 );
                    }
                    else
                    ICV_HLINE( tptr, x21, x22, color, pix_size );
                }
            }
        }
        dy++;
        err += plus;
        plus += 2;

        mask = (err <= 0) - 1;

        err -= minus & mask;
        dx += mask;
        minus -= mask & 2;
    }

#undef  ICV_PUT_POINT
}

enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT};

static void
Linee( Mat& img, Point pt1, Point pt2,
       const void* _color, int connectivity = 8 )
{
    if( connectivity == 0 )
        connectivity = 8;
    else if( connectivity == 1 )
        connectivity = 4;

    LineIterator iterator(img, pt1, pt2, connectivity, true);
    int i, count = iterator.count;
    int pix_size = (int)img.elemSize();
    const uchar* color = (const uchar*)_color;

    for( i = 0; i < count; i++, ++iterator )
    {
        uchar* ptr = *iterator;
        if( pix_size == 1 )
            ptr[0] = color[0];
        else if( pix_size == 3 )
        {
            ptr[0] = color[0];
            ptr[1] = color[1];
            ptr[2] = color[2];
        }
        else
            memcpy( *iterator, color, pix_size );
    }
}

/* filling convex polygon. v - array of vertices, ntps - number of points */
static void
FillConvexPoly( Mat& img, const Point* v, int npts, const void* color, int line_type, int shift )
{
    struct
    {
        int idx, di;
        int64 x, dx;
        int ye;
    }
            edge[2];

    int delta = 1 << shift >> 1;
    int i, y, imin = 0;
    int edges = npts;
    int64 xmin, xmax, ymin, ymax;
    uchar* ptr = img.ptr();
    Size size = img.size();
    int pix_size = (int)img.elemSize();
    Point p0;
    int delta1, delta2;

    if( line_type < CV_AA )
        delta1 = delta2 = XY_ONE >> 1;
    else
        delta1 = XY_ONE - 1, delta2 = 0;

    p0 = v[npts - 1];
    p0.x <<= XY_SHIFT - shift;
    p0.y <<= XY_SHIFT - shift;

    assert( 0 <= shift && shift <= XY_SHIFT );
    xmin = xmax = v[0].x;
    ymin = ymax = v[0].y;

    for( i = 0; i < npts; i++ )
    {
        Point p = v[i];
        if( p.y < ymin )
        {
            ymin = p.y;
            imin = i;
        }

        if(p.y > ymax)
            ymax = p.y;

        if(p.x > xmax)
            xmax = p.x;

        xmin = MIN( xmin, p.x );

        p.x <<= XY_SHIFT - shift;
        p.y <<= XY_SHIFT - shift;

        if( line_type <= 8 )
        {
            if( shift == 0 )
            {
                Point pt0, pt1;
                pt0.x = (int)(p0.x >> XY_SHIFT);
                pt0.y = (int)(p0.y >> XY_SHIFT);
                pt1.x = (int)(p.x >> XY_SHIFT);
                pt1.y = (int)(p.y >> XY_SHIFT);
                Linee( img, pt0, pt1, color, line_type );
            }
        }
        p0 = p;
    }

    xmin = (xmin + delta) >> shift;
    xmax = (xmax + delta) >> shift;
    ymin = (ymin + delta) >> shift;
    ymax = (ymax + delta) >> shift;

    if( npts < 3 || (int)xmax < 0 || (int)ymax < 0 || (int)xmin >= size.width || (int)ymin >= size.height )
        return;

    ymax = MIN( ymax, size.height - 1 );
    edge[0].idx = edge[1].idx = imin;

    edge[0].ye = edge[1].ye = y = (int)ymin;
    edge[0].di = 1;
    edge[1].di = npts - 1;

    edge[0].x = edge[1].x = -XY_ONE;
    edge[0].dx = edge[1].dx = 0;

    ptr += img.step*y;

    do
    {
        if( line_type < CV_AA || y < (int)ymax || y == (int)ymin )
        {
            for( i = 0; i < 2; i++ )
            {
                if( y >= edge[i].ye )
                {
                    int idx0 = edge[i].idx, di = edge[i].di;
                    int idx = idx0 + di;
                    if (idx >= npts) idx -= npts;
                    int ty = 0;

                    for (; edges-- > 0; )
                    {
                        ty = (int)((v[idx].y + delta) >> shift);
                        if (ty > y)
                        {
                            int64 xs = v[idx0].x;
                            int64 xe = v[idx].x;
                            if (shift != XY_SHIFT)
                            {
                                xs <<= XY_SHIFT - shift;
                                xe <<= XY_SHIFT - shift;
                            }

                            edge[i].ye = ty;
                            edge[i].dx = ((xe - xs)*2 + (ty - y)) / (2 * (ty - y));
                            edge[i].x = xs;
                            edge[i].idx = idx;
                            break;
                        }
                        idx0 = idx;
                        idx += di;
                        if (idx >= npts) idx -= npts;
                    }
                }
            }
        }

        if (edges < 0)
            break;

        if (y >= 0)
        {
            int left = 0, right = 1;
            if (edge[0].x > edge[1].x)
            {
                left = 1, right = 0;
            }

            int xx1 = (int)((edge[left].x + delta1) >> XY_SHIFT);
            int xx2 = (int)((edge[right].x + delta2) >> XY_SHIFT);

            if( xx2 >= 0 && xx1 < size.width )
            {
                if( xx1 < 0 )
                    xx1 = 0;
                if( xx2 >= size.width )
                    xx2 = size.width - 1;
                ICV_HLINE( ptr, xx1, xx2, color, pix_size );
            }
        }
        else
        {
            // TODO optimize scan for negative y
        }

        edge[0].x += edge[0].dx;
        edge[1].x += edge[1].dx;
        ptr += img.step;
    }
    while( ++y <= (int)ymax );
}

static void
Line( Mat& img, Point p0, Point p1, const void* color,
      int thickness, int line_type)
{
    int flags = 3;
    int shift = 0;

    static const double INV_XY_ONE = 1./1;

    p0.x <<= XY_SHIFT - shift;
    p0.y <<= XY_SHIFT - shift;
    p1.x <<= XY_SHIFT - shift;
    p1.y <<= XY_SHIFT - shift;

    Point pt[4], dp = Point(0,0);
    double dx = (p0.x - p1.x)*INV_XY_ONE, dy = (p1.y - p0.y)*INV_XY_ONE;
    double r = dx * dx + dy * dy;
    int i, oddThickness = thickness & 1;
    thickness <<= XY_SHIFT - 1;

    if( fabs(r) > DBL_EPSILON )
    {
        r = (thickness + oddThickness*XY_ONE*0.5)/std::sqrt(r);
        dp.x = cvRound( dy * r );
        dp.y = cvRound( dx * r );

        pt[0].x = p0.x + dp.x;
        pt[0].y = p0.y + dp.y;
        pt[1].x = p0.x - dp.x;
        pt[1].y = p0.y - dp.y;
        pt[2].x = p1.x - dp.x;
        pt[2].y = p1.y - dp.y;
        pt[3].x = p1.x + dp.x;
        pt[3].y = p1.y + dp.y;

        FillConvexPoly( img, pt, 4, color, line_type, XY_SHIFT );
    }

    for( i = 0; i < 2; i++ )
    {
        if( flags & (i+1) )
        {
            if( line_type < CV_AA )
            {
                Point center;
                center.x = (int)((p0.x + (XY_ONE>>1)) >> XY_SHIFT);
                center.y = (int)((p0.y + (XY_ONE>>1)) >> XY_SHIFT);
                Circle( img, center, (thickness + (XY_ONE>>1)) >> XY_SHIFT, color, 1 );
            }
        }
        p0 = p1;
    }
}

void DrawMarker(Mat& img, Point position, const void* color, int markerType, int markerSize, int thickness, int line_type)
{
    switch(markerType)
    {
        // The diamond marker case
        case MARKER_DIAMOND:
            Line(img, Point(position.x, position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y), color, thickness, line_type);
            Line(img, Point(position.x+(markerSize/2), position.y), Point(position.x, position.y+(markerSize/2)), color, thickness, line_type);
            Line(img, Point(position.x, position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y), color, thickness, line_type);
            Line(img, Point(position.x-(markerSize/2), position.y), Point(position.x, position.y-(markerSize/2)), color, thickness, line_type);
            break;

            // The square marker case
        case MARKER_SQUARE:
            Line(img, Point(position.x-(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
            Line(img, Point(position.x+(markerSize/2), position.y-(markerSize/2)), Point(position.x+(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
            Line(img, Point(position.x+(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y+(markerSize/2)), color, thickness, line_type);
            Line(img, Point(position.x-(markerSize/2), position.y+(markerSize/2)), Point(position.x-(markerSize/2), position.y-(markerSize/2)), color, thickness, line_type);
            break;
    }
}
