/* Filename: ir_compress_main.cpp  
 * Description: 
 * Date: 8/8/14
 */

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
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

using namespace cv;
using namespace std;

/*-------------------------------------------------
             PREPROCESSOR CONSTANTS
  -------------------------------------------------*/
#define PRETIME            100
#define POSTTIME           1000000
#define FPS                20
#define X_RESOLUTION       352
#define Y_RESOLUTION       288
#define WINDOW_SIZE        0
#define GROUND_THRESHOLD   0
#define TEST_THRESHOLD     50
#define COMPRESSION_LEVEL  60
#define PORT_NUMBER        30000
#define DEVICE_PORT        "/dev/tty02"
#define BAUD_RATE          115200

//Uncomment below to let us know video is storing
#define DEBUG

/*-----------------------------------------------------
              FUNCTION PROTPOTYPES
  -----------------------------------------------------*/
void create_directory(const char *path, struct stat &st);
string create_vid_id(string path, bool collision);
string create_im_id(string path, int im_count);
string create_dir_path(string path, string sub_dir_name);
string get_date();
void* listenForExit(void* i);
bool listenForSleepSignal(int Ret, char Buffer[128],
                          serialib LS, string sleepKey);
void processSignal();
void flashLed(int numTimes);
void openSerialPort(int Ret, serialib LS);
void closeSerialPort(serialib LS);
bool isSignalRecieved(BlackGPIO *ir_in);

bool stopSig = false;
serialib LS;
int Ret;
char Buffer[128];

int main()
{
  /*---------------------------------------------------
          VARIABLE DECLARATIONS / INITIALIZATION
    ---------------------------------------------------*/
    Mat     frame;
    queue   <Mat> frames;
    deque   <int> sensor_signal;
    deque   <int> averaged_signal;       // DUSTIN
    vector<int> compress_params;
    struct  stat st;
    string  vid_id;
    string  im_id;
    string  path_name;
    string  sleepKey;
    float   average_signal  = 0;
    float   normal_signal   = 0;
    int     adc             = 0;
    int     dir_count       = 0;
    int     im_count        = 0;
    int     frame_count     = 0;         // For Testing Purposes
    int     test_count      = 0;         // For Testing Purposes
    int     max_count       = 50;        // For Testing Purposes
    int     limit           = PRETIME;
    int     rc;
    bool    save            = false;
    bool    detected        = false;
    bool    collision       = false;
    bool    isSleepTime;
    const char* converted_path;

    string path = "/home/ubuntu/AngryBirds/SDCard/videos/";
    ofstream signals;

    // Gets analog readings from adc pin 4
    BlackADC *test_adc = new BlackADC(AIN4);
    BlackGPIO *ir_in = new BlackGPIO(GPIO_68, input);  // P8_10

  /*---------------------------------------------------
         FRAME CAPTURE / STORAGE + COLLISION DETECTION
    ---------------------------------------------------*/
    // Open video no.0
    VideoCapture input_cap(0);

    // Set (lower) the resolution for the webcam
    input_cap.set(CV_CAP_PROP_FRAME_WIDTH, X_RESOLUTION);
    input_cap.set(CV_CAP_PROP_FRAME_HEIGHT, Y_RESOLUTION);

    // Open the camera for capturing, if failure, terminate
    if (!input_cap.isOpened()) {
        cout << "\nINPUT VIDEO COULD NOT BE OPENED\n" << endl;
        return -1;
    }

    // Start listening for signal to stop
    pthread_t exit_thread;
    rc = pthread_create(&exit_thread, NULL, listenForExit, (void*) NULL);
    if(rc){
        cout << "ERROR: unable to create thread" << endl;
    }

  /*---------------------------------------------------
                        MAIN LOOP
    ---------------------------------------------------*/
    // Read in each frame for storage and processing
    while(input_cap.read(frame) &&
          !stopSig              &&
          !isSignalRecieved(ir_in)) {
/*
        // Listen for a signal to be sent from
        // recieving IR Arduino to indicate sleep time
        sleepKey = "sleep";
        isSleepTime = listenForSleepSignal(Ret, Buffer, LS, sleepKey);
        if (isSleepTime) {
            processSignal();
            break; // ?
        }
*/
        // Open file to write signal data
        signals.open("/home/ubuntu/AngryBirds/SDCard/signals.txt",
                      fstream::in  |
                      fstream::out |
                      fstream::app);

        // Output the analog readings to a signals.txt
                adc =  test_adc->getNumericValue();
        if (adc >= TEST_THRESHOLD) {
            signals << get_date() << ":    ";
            signals << test_adc->getNumericValue();
            signals << "\n";
        }

        // Create the (final) output destination - where all concatenated
        // clips will be stored
        path_name = create_dir_path(path, "NONE");
        converted_path = path_name.c_str();
        create_directory(converted_path, st);

        // Write each frame into an (compressed) jpg img in
        //designated subdir.
        compress_params.push_back(CV_IMWRITE_JPEG_QUALITY);
        compress_params.push_back(COMPRESSION_LEVEL);
        im_id = create_im_id(converted_path, im_count);
        try {
                imwrite(im_id, frame, compress_params);
        } catch (runtime_error& e) {
                cerr << "\nEXCEPTION CONVERTING IMAGES\n" << endl;
                return 1;
        }
        im_count++;
        signals.close();
        usleep(100); //check server
        cout << "img count: " << im_count << endl;
    } // End While
    input_cap.release();
    cout << "\nSTOPPED\n" << endl;
} // End Main



/*---------------------------------------------------------
                  FUNCTION DEFINITIONS
  ---------------------------------------------------------*/
/* Description: Creates a new directory to store (temp)
 *              footage (for each event) for post processing
 */
string create_dir_path(string path, string sub_dir_name) {
   if (sub_dir_name.compare("NONE") == 0) {
      return path;
   }
   string dir_name;
   dir_name = path + "/" + sub_dir_name;
   return dir_name;
}


/* Description: Creates a new directory to store footage if
 *              it doesn't exist
 */
void create_directory(const char *path, struct stat &st) {
    if(stat(path, &st) != 0) {
        if(errno == ENOENT) {
            cout << "Creating a new video directory" << endl;
            if(mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == 0) {
                perror("mkdir");
            }
        }
    }
}


/* Description: Returns the current system time
 * Note: time portion of datetime stamp is formated as
 *       HOUR_MINUTE_SEC because colons are considered a invalid
  *       character in naming files.
 */
string get_date() {
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%F__%H_%M_%S", timeinfo);
    return(string(buffer));
}


/* Description: Creates a new ID to name output image file
 *              in format "Year-Month-Day Hour_Minute_Second__#"
 */
string create_im_id(string path, int im_count) {
     string str_im_count;
     str_im_count = to_string(im_count);
     return (path + "/" + get_date() + "__" + str_im_count + ".jpg");
}


/* Description: Creates a new ID to name output video file
 *              in format "Year-Month-Day Hour_Minute_Second"
 */
string create_vid_id(string path, bool collision) {
    if (collision)
    {
        return (path + get_date() + ".avi");
    }
    // No Collision
    return( path + get_date() + "_NC" + ".avi");
}


/*
 */
void *listenForExit(void* i){
    bool recievedData = false;
    try {
        ServerSocket server(PORT_NUMBER);

        ServerSocket new_sock;
        server.accept(new_sock);

        while(true && !recievedData){
            try {
                string data;
                new_sock >> data; //storing data recieved from socket
                recievedData = true; //uncomment this for an infiite loop
            } catch(SocketException&){
            }
        }
        stopSig = true;
    } catch(SocketException&){
    }
    pthread_exit(NULL);
}


/* Returns true if an IR signal has been recieved
 */
bool isSignalRecieved(BlackGPIO *ir_in) {
    if (ir_in->fail()) {
        cout << "ERROR" << endl;
    }
    cout << "GPIO pin value: " << ir_in->getValue() << endl;
    if (ir_in->isHigh()) {
        // Flash the LED to indicate signal has been recieved
        flashLed(5);
        cout << "SIGNAL HEARD!" << endl;
        return true;
    } else {
        return false;
    }
}


/* Flash the LED (P9_11 - GPIO 30)
 */
void flashLed(int numTimes) {
    // Set up the output pin for controlling the LED light
    BlackGPIO *ledOut = new BlackGPIO(GPIO_30, output);  // P9_11

    int i = 0;
    while (i < numTimes) {
        ledOut->setValue(high);
        sleep(1);
        ledOut->setValue(low);
        sleep(1);
        i++;
    }
}


/* Listens for a flag to be sent from the Arduino IR reciever to
 * BBB to indicate time to sleep. Compare incoming signals against
 * a preset sleep key to determine validity of signal
 */
bool listenForSleepSignal(int Ret, char Buffer[128],
                          serialib LS, string sleepKey) {
    // Open the port to begin listening
    openSerialPort(Ret, LS);

    // Listen for a signal to be recieved from the Arduino
    Ret =  LS.ReadString(Buffer, '\n', 128, 5000);

    // The final character of the string must be a line feed ('\n')
    if (Ret > 0) {
        // Check if recieved string matches our sleep key.
        // If a match is found, break and return true;
        string bufStr(Buffer);
        if (bufStr.compare(sleepKey) == 0) {
            return true;
        }
    }
    // We have not recieved the flag to sleep
    else {
        return false;
    }
    // Close the port
    closeSerialPort(LS);
}


/* If a sleep signal has been recieved, sleep the main (video capturing)
 * proces for 30 minutes
 */
void processSignal() {
   // Flash LED to indicate that signal has been detected
   flashLed(5);

   // DO SOMETHING HERE

   // Flash LED again to indicate that system is now asleep
   flashLed(5);
}


/*
 * Opens the serial port between the Beaglebone Black and the Arduino
 * Pro Mini at the given BAUD_RATE
 */
void openSerialPort(int Ret, serialib LS){
    // Open serial link at 115200 bauds
    Ret = LS.Open(DEVICE_PORT,BAUD_RATE);
    // Check if an error has occured
    if (Ret != 1) {
        printf ("Error while opening port. Permission problem ?\n");
        return;
    }
    printf ("Serial port opened successfully !\n");
}


/*
 * Closes the serial port between the Beaglebone Black and the Arduino
 * Pro Mini
 */
void closeSerialPort(serialib LS){
    LS.Close();
}

//-----EOF-----
