#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <queue>
#include <deque>
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "serial/serialib.h"
#include "BlackLib/BlackLib.h"
#include "Server/ServerSocket.h"
#include "Socket/SocketException.h"
#include "ADXL/ADXL345.h"
#include "ADXL/BBB_I2C.h"

using namespace cv;
using namespace std;
using namespace cacaosd_bbb_i2c;
using namespace cacaosd_adxl345;

/*-------------------------------------------------
             PREPROCESSOR CONSTANTS
  -------------------------------------------------*/
#define PRETIME            64  //  64/16 = 4             sec before collision
#define POSTTIME           128 // 128/16 - 4 = 8 - 4 = 4 sec after  collision
#define SIG_PRETIME         40 // 1 sec before - if you want to change this, change ADXL_DELAY_US too
#define SIG_POSTTIME        120 // 2 sec after - if you want to change this, change ADXL_DELAY_US too
#define FPS                20
#define X_RESOLUTION       352
#define Y_RESOLUTION       288
#define WINDOW_SIZE        0
#define COMPRESSION_LEVEL  60
#define PORT_NUMBER        30000
#define BAUD_RATE          115200
#define LED_NORM	        500000
#define LED_DETECT	        100000

#define ADXL_DELAY_US       25000 // if you want to change this, change SIG_PRETIME and SIG_POSTTIME too
// #define ADXL_THRESH         15

/*-----------------------------------------------------
              FUNCTION PROTPOTYPES
  -----------------------------------------------------*/
void create_directory(const char *path, struct stat &st);
string create_vid_id(string path, bool collision);
string create_im_id(string path, int im_count, bool is_dated);
string create_dir_path(string path, string sub_dir_name);
string get_date();
void* listenForExit(void* param);
void* led_live(void* param);
void* ADXL_sig(void* param);
void flashLed(int numTimes, int sleep_period);
void record_log(const char *message);

int led_speed;

bool stopSig;
bool isActivity;
bool isActivity2;
ofstream ab_log;
string event_time_for_sig;
bool twoSensors;
bool debug;
// Set up the output pin for controlling the LED light
BlackGPIO *ledOut;



int main(int argc, char* argv[])
{
    /*---------------------------------------------------
            VARIABLE DECLARATIONS / INITIALIZATION
      ---------------------------------------------------*/
    Mat     frame;
    queue   <Mat> frames;
    vector  <int> compress_params;
    struct  stat st;
    string  im_id;
    string  path_name;
    string  subdir_name;
    string  subdir_path;
    int     dir_count       = 0;
    int     im_count        = 0;
    int     rc;
    int     limit           = PRETIME;
    bool    save            = false;
    bool    detected        = false;
    bool    collision       = false;
    const char* converted_path;
    const char* converted_subpath;
    struct timeval start, end;

    if(argc < 2){
        printf("Please enter -t as first arguement to activate 2 sensors, -d for debug. If not, simply pass any command line arguement");
        exit(0);
    }

    else{
    if(string(argv[1]) == "-t")
        twoSensors = true;
    else
        twoSensors = false;

    if(string(argv[2]) == "-d")
        debug = true;
    else
        debug = false;

    printf("%i\n",twoSensors);
    printf("%i\n",debug);

    record_log("CREATING MAIN IMAGES AND VIDEOS DIRECTORY.");

    string img_path = "/home/ubuntu/AngryBirds/SDCard/images";
    path_name = create_dir_path(img_path, "NONE");
    converted_path = path_name.c_str();
    create_directory(converted_path, st);

    record_log(converted_path);

    string vid_path = "/home/ubuntu/AngryBirds/SDCard/videos";
    path_name = create_dir_path(vid_path, "NONE");
    converted_path = path_name.c_str();
    create_directory(converted_path, st);

    record_log(converted_path);

    record_log("DONE CREATING MAIN IMAGES AND VIDEOS DIRECTORY: ");

    ofstream signals;
    record_log("Program started.");

    /*---------------------------------------------------
           FRAME CAPTURE / STORAGE + COLLISION DETECTION
      ---------------------------------------------------*/
    // Start listening for signal to stop
    pthread_t exit_thread;
    pthread_t led_thread;
    pthread_t ADXL_thread;

    stopSig = false;
    rc = pthread_create(&exit_thread, NULL, listenForExit, (void*) NULL);
    if(rc)
    {
        record_log("ERROR: unable to create exit thread.");
    }

    // gettimeofday(&start, NULL);
    isActivity = false;
    isActivity2 = false;
    rc = pthread_create(&ADXL_thread, NULL, ADXL_sig, (void*) NULL);
    // gettimeofday(&end, NULL);
    // long long time =   (end.tv_sec * (unsigned int)1e6 +   end.tv_usec) - 
    //              (start.tv_sec * (unsigned int)1e6 + start.tv_usec);
    // printf("time: %d\n",time);
    if(rc)
    {
        record_log("ERROR: unable to create ADXL thread.");
    }
    led_speed = LED_NORM;
    ledOut = new BlackGPIO(GPIO_30, output);// GPIO_30 = pin9-11
    rc = pthread_create(&led_thread, NULL, led_live, (void*) NULL);
    if(rc)
    {
        record_log("ERROR: unable to create led thread.");
    }

    // Open video no.0
    VideoCapture input_cap(0);
    if (!input_cap.isOpened())
        {
            record_log("INPUT VIDEO COULD NOT BE OPENED!");
            return -1;
        }

    // Set (lower) the resolution for the webcam
    input_cap.set(CV_CAP_PROP_FRAME_WIDTH, X_RESOLUTION);
    input_cap.set(CV_CAP_PROP_FRAME_HEIGHT, Y_RESOLUTION);

    // set compress values
    compress_params.push_back(CV_IMWRITE_JPEG_QUALITY);
    compress_params.push_back(COMPRESSION_LEVEL);

    /*---------------------------------------------------
                          MAIN LOOP
      ---------------------------------------------------*/
    record_log("starting the loop.");

    while(true)
    {
        // Check that the camera is open for capturing,
        // if failure, terminate

        if(stopSig)
        {
            record_log("Exit signal received, exiting ...");
            break;
        }

        if(!input_cap.read(frame))   //stop signal for post proc. maybe remove "read" since we checked camera before
            record_log("Cannot get frame from the cam.");
        else
        {
            if(frames.size() >= limit)
            {
                frames.pop();
                frames.push(frame.clone());
            }
            else
            {
                frames.push(frame.clone());
            }

            if( isActivity & !detected || isActivity2 & !detected)
            {
                record_log("Event detected.");

                record_log("CREATING SUBDIR TO STORE THE COLLISION.");
                // Create the sub directory that will store all the
                // image files per collision event
                subdir_name = "Cam_0_" + get_date();//device id
                event_time_for_sig = subdir_name;
                subdir_path = create_dir_path(img_path, subdir_name);
                converted_subpath = subdir_path.c_str();
                create_directory(converted_subpath, st);
                record_log("DONE CREATING SUBDIR");

                detected = true;
                led_speed = LED_DETECT;
                limit = POSTTIME;
            }

            if (detected && frames.size() >= limit)
            {
                isActivity = false;
                isActivity2 = false;
                // Write the collision sequence into the output sub dir
                record_log("WRITING THE COLLISION SEQUENCE");
                im_count = 0;
                while (!frames.empty())
                {
                    im_id = create_im_id(converted_subpath, im_count, false);
                    try
                    {
                        imwrite(im_id, frames.front(), compress_params);
                    }
                    catch (runtime_error& e)
                    {
                        record_log("EXCEPTION CONVERTING IMAGES");
                        return 1;
                    }
                    frames.pop();
                    im_count++;
                }
                record_log("DONE WRITING THE COLLISION SEQUENCE");
                limit = PRETIME;
                dir_count++;
                detected = false;
                led_speed = LED_NORM;
                usleep(100); //check server
            }
        }
    } // End While
    ledOut->setValue(low);
    input_cap.release();
    record_log("Exit successful.");
    usleep(500000);
}
} // End Main



/*---------------------------------------------------------
                  FUNCTION DEFINITIONS
  ---------------------------------------------------------*/
/* Description: Creates a new directory to store (temp)
 *              footage (for each event) for post processing
 */
 void record_log(const char *message)
 {
     ab_log.open("/home/ubuntu/AngryBirds/SDCard/log.txt",fstream::in | fstream::out | fstream::app);
     ab_log << get_date() << " - " << message << endl;
     ab_log.close();
 }

string create_dir_path(string path, string sub_dir_name)
{
    if (sub_dir_name.compare("NONE") == 0)
    {
        return path;
    }
    string dir_name;
    dir_name = path + "/" + sub_dir_name;
    return dir_name;
}


/* Description: Creates a new directory to store footage if
 *              it doesn't exist
 */
void create_directory(const char *path, struct stat &st)
{
    if(stat(path, &st) != 0)
    {
        if(errno == ENOENT)
        {
            record_log("CREATING A NEW SUBDIRECTORY");
            if(!mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == 0)
            {
                perror("mkdir");
            }
            else
                record_log(path);
                record_log("CREATED.");
        }
        else
            record_log("error = ENOENT");
    }
    else
        record_log("path and st = 0");
}


/* Description: Returns the current system time
 * Note: time portion of datetime stamp is formated as
 *       HOUR_MINUTE_SEC because colons are considered a invalid
  *       character in naming files.
 */
string get_date()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%F_%H_%M_%S", timeinfo);
    return(string(buffer));
}


/* Description: Creates a new ID to name output image file
 *              in format "Year-Month-Day Hour_Minute_Second__#"
 */
string create_im_id(string path, int im_count, bool is_dated)
{
    string str_im_count;
    string abs_path;
    str_im_count = to_string(im_count);
    if (is_dated)
    {
        return(path + "/" + get_date() + "__" + str_im_count + ".jpg");
    }
    else
    {
        // Need to find a better way of doing this later ...
        // (For a 35 sec video - need 700 frames)

        if (im_count < 10)
        {
            str_im_count = string(1, '0').append(str_im_count);
        }
        if (im_count < 100)
        {
            str_im_count = string(1, '0').append(str_im_count);
        }

        return(path + "/" + str_im_count + ".jpg");
    }
}


/* Description: Creates a new ID to name output video file
 *              in format "Year-Month-Day Hour_Minute_Second"
 */
string create_vid_id(string path, bool collision)
{
    if (collision)
    {
        return (path + get_date() + ".avi");
    }
    // No Collision
    return( path + get_date() + "_NC" + ".avi");
}


/*
 */
void *listenForExit(void* param)
{
    bool recievedData = false;
    try
    {
        ServerSocket server(PORT_NUMBER);

        ServerSocket new_sock;
        server.accept(new_sock);

        while(true && !recievedData)
        {
            try
            {
                string data;
                new_sock >> data; //storing data recieved from socket
                recievedData = true; //uncomment this for an infiite loop
            }
            catch(SocketException&)
            {
            }
        }
        stopSig = true;
    }
    catch(SocketException&)
    {
    }
    pthread_exit(NULL);
}

void* ADXL_sig(void* param)
{   

    int threshTime = 30;
    //setup ADXL
    BBB_I2C i2c;
	ADXL345 adxl(i2c);
	adxl.initialize();
	if(adxl.getLinkEnabled())
        record_log("ADXL initialized.");
     adxl.setRate(0x0A);
    int16_t last_x, x;
    int16_t last_y, y;
    int16_t last_z, z;
    queue <int16_t> sig_x, sig_y, sig_z;
    bool save = false;
    int sig_limit = SIG_PRETIME;

    ofstream sig_log;
    const char* file_name;
    string file_path;
    usleep(ADXL_DELAY_US);
    adxl.getAcceleration(&last_x, &last_y, &last_z);
    usleep(ADXL_DELAY_US);


    ADXL345 adxl2(ADXL345_ALTERNATE_ADDRESS);
    int16_t last2_x, x2;
    int16_t last2_y, y2;
    int16_t last2_z, z2;
     queue <int16_t> sig2_x, sig2_y, sig2_z;
    bool save2 = false;
    int sig2_limit = SIG_PRETIME;  
    ofstream sig2_log;
    const char* file2_name;
    string file2_path;

    
if(twoSensors == true){
    adxl2.initialize();
    if(adxl2.getLinkEnabled())
        record_log("ADXL2 initialized.");
    adxl2.setRate(0x0A);
    usleep(ADXL_DELAY_US);
    adxl2.getAcceleration(&last2_x, &last2_y, &last2_z);
    usleep(ADXL_DELAY_US);
}


    int threshx[30];
    int threshy[30];
    int threshz[30];


    printf("collecting data and setting threshold....\n");

    for(int i = 0; i < 30; i++){
        adxl.getAcceleration(&x,&y,&z);
        threshx[i] = x;
        threshy[i] = y;
        threshz[i] = z;
    }

    int sumx = 0;
    int sumy = 0;
    int sumz = 0;


    int averagex = 0;
    int averagey = 0;
    int averagez = 0;

    for(int i  = 0; i  < 30; i++){
        sumx = sumx += threshx[i];
        sumy = sumy += threshy[i];
        sumz = sumz += threshz[i];
    }

    averagex = sumx / 30;
    averagey = sumy / 30;
    averagez = sumz / 30;

    int ADXL_THRESH = (averagex + averagey + averagez)/3;


    printf("current threshold set to: %i\n", ADXL_THRESH);



	while(!stopSig)
	{
	    adxl.getAcceleration(&x, &y, &z);
        if( ((last_x > x+ADXL_THRESH) | (last_y > y+ADXL_THRESH) | (last_z > z+ADXL_THRESH) | (last_x < x-ADXL_THRESH) | (last_y < y-ADXL_THRESH) | (last_z < z-ADXL_THRESH)) & !isActivity )
        {
            isActivity = true;
            save = true;
            sig_limit = SIG_POSTTIME;
            if(debug == true){
                printf("activity in ADXL1!!\n");
                printf("dx = %d, dy = %d, dz = %d\n", x-last_x, y-last_y, z-last_z);
            }
        }
        

        if(debug == true){
            printf("x1 = %d, y1 = %d, z1 = %d\n", x, y, z);
            printf("data rate: %d\n",adxl.getRate());
        }

        last_x = x;
        last_y = y;
        last_z = z;

        if(sig_x.size() >= sig_limit)
        {
            sig_x.pop();
            sig_x.push(x);
            sig_y.pop();
            sig_y.push(y);
            sig_z.pop();
            sig_z.push(z);
        }
        else
        {
            sig_x.push(x);
            sig_y.push(y);
            sig_z.push(z);
        }

        if(save & (sig_x.size() >= sig_limit) )
        {
            file_path = "/home/ubuntu/AngryBirds/SDCard/videos/" + event_time_for_sig + ".txt";
            file_name = file_path.c_str();
            sig_log.open(file_name, fstream::in | fstream::out | fstream::app);
            while(sig_x.size())
            {
                sig_log << sig_x.front() << ", " << sig_y.front() << ", " << sig_z.front() << endl;
                sig_x.pop();
                sig_y.pop();
                sig_z.pop();
            }
            sig_log.close();
            save = false;
            sig_limit = SIG_PRETIME;
        }

        usleep(ADXL_DELAY_US);

    if(twoSensors == true){
        adxl2.getAcceleration(&x2, &y2, &z2);
        // printf("x2 = %d, y2 = %d, z3 = %d\n", x2, y2, z2);

        if( ((last2_x > x2+ADXL_THRESH) | (last2_y > y2+ADXL_THRESH) | (last2_z > z2+ADXL_THRESH) | (last2_x < x2-ADXL_THRESH) | (last2_y < y2-ADXL_THRESH) | (last2_z < z2-ADXL_THRESH)) & !isActivity )
        {
            isActivity2 = true;
            save2 = true;
            sig2_limit = SIG_POSTTIME;
            if(debug == true){
                printf("activity in ADXL2!!\n");
                printf("dx = %d, dy = %d, dz = %d\n", x-last_x, y-last_y, z-last_z);
            }
        }

        if(debug == true){
            printf("x2 = %d, y2 = %d, z2 = %d\n", x2, y2, z2);
            printf("data rate: %d\n",adxl2.getRate());
        }

        last2_x = x2;
        last2_y = y2;
        last2_z = z2;

        if(sig2_x.size() >= sig2_limit)
        {
            sig2_x.pop();
            sig2_x.push(x2);
            sig2_y.pop();
            sig2_y.push(y2);
            sig2_z.pop();
            sig2_z.push(z2);
        }
        else
        {
            sig2_x.push(x2);
            sig2_y.push(y2);
            sig2_z.push(z2);
        }

        if(save2 & (sig2_x.size() >= sig2_limit) )
        {
            file2_path = "/home/ubuntu/AngryBirds/SDCard/videos/" + event_time_for_sig + ".txt";
            file2_name = file2_path.c_str();
            sig2_log.open(file2_name, fstream::in | fstream::out | fstream::app);
            while(sig2_x.size())
            {
                sig2_log << sig2_x.front() << ", " << sig2_y.front() << ", " << sig2_z.front() << endl;
                sig2_x.pop();
                sig2_y.pop();
                sig2_z.pop();
            }
            sig2_log.close();
            save2 = false;
            sig2_limit = SIG_PRETIME;
        }
        usleep(ADXL_DELAY_US);
    }
	}//closing bracket for while loop
	pthread_exit(NULL);
}

void* led_live(void* param)
{
    while(!stopSig)
    {
        flashLed(5, led_speed); // Indicate that IR signal has been recieved
//                input_cap.release(); // Close the camera
        sleep(1); // Sleep for 30 minutes // 2 Minutes Test
//                input_cap.open(0); // Re-open the camera
        flashLed(5, led_speed); // Indicate that script will resume
        sleep(1);
    }
    pthread_exit(NULL);
}

/* Flash the LED (P9_11 - GPIO 30)
 */
void flashLed(int numTimes, int sleep_period)
{
    int i = 0;
    while (i < numTimes)
    {
        ledOut->setValue(high);
        usleep(sleep_period);
        ledOut->setValue(low);
        usleep(sleep_period);
        i++;
    }
}


//-----EOF-----
