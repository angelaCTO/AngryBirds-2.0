/* Filename: test_main.cpp
 * Author(s): Angela To, 
 *            Dustin Mendoza, 
 *            Ali Khomadari
 * Date: 8/8/14
 */

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <errno.h>
#include <queue>
#include <deque>
#include <sys/stat.h>
#include <string>
#include <thread>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <pthread.h>
#include "BlackLib/BlackLib.h"

using namespace cv;
using namespace std;

/*-------------------------------------------------
             PREPROCESSOR CONSTANTS
  -------------------------------------------------*/
#define DEBUG	         true
#define ADC              true
#define PRETIME	         50    // 100
#define POSTTIME         100    // 1500
#define FPS 	         20
#define X_RESOLUTION     352
#define Y_RESOLUTION     288
#define WINDOW_SIZE      0
#define GROUND_THRESHOLD 0
#define TEST_THRESHOLD   700


/*-----------------------------------------------------
              FUNCTION PROTPOTYPES
  -----------------------------------------------------*/
void create_directory(const char *path, struct stat &st);
string create_id(const char *path, bool collision);
void *write_frames(void* qPtr);
string get_date();
queue<Mat>* createQ();
void writeWithThread(queue<Mat>* q);

// Path to save video files
const char *path = "/home/ubuntu/AngryBirds/SDCard/videos/";
// Struct required to check the status of video directory
struct  stat st;

char CODEC;

int main(){
  /*---------------------------------------------------
          VARIABLE DECLARATIONS / INITIALIZATION
    ---------------------------------------------------*/
    Mat         frame;
    queue       <Mat> frames;
    deque       <int> sensor_signal;
    pthread_t   write_thread;
    float       average_signal  = 0;
    float       normal_signal   = 0;
    int 	ex		= 0;
    int         thread_ret      = 0;
    int         frame_count     = 0;         // For Testing Purposes
    int         test_count      = 0;         // For Testing Purposes
    int         max_count       = 50;        // For Testing Purposes
    int		max_frames 	= 1024;
    int         limit           = PRETIME;
    bool        save            = false;
    bool        detected        = false;
    bool        collision       = false;
    ofstream signals;

    // Get the input from adc
    BlackADC *test_adc = new BlackADC(AIN4);

  /*---------------------------------------------------
       FRAME CAPTURE/STORAGE + COLLISION DETECTION
    ---------------------------------------------------*/
    VideoCapture input_cap(0);


    // Open the camera for capturing, if failure, terminate
    if (!input_cap.isOpened()){
        cout << "\nINPUT VIDEO COULD NOT BE OPENED\n" << endl;
        return -1;
    }

cout << "A" << endl;
    // Get the CODEC value for VideoWriter from input camera 
    ex = static_cast<int>(input_cap.get(CV_CAP_PROP_FOURCC));
cout << "B" << endl;
    char EXT[] = {ex & 0XFF , 
                 (ex & 0XFF00) >> 8,
                 (ex & 0XFF0000) >> 16,
                 (ex & 0XFF000000) >> 24, 
                 0};
cout << "C" << endl;
    // Convert integer CODEC value into 4 char value required
    // by VideoWriter
    union { int v; char c[5];} uEx ;
    uEx.v = ex;                      
    uEx.c[4]='\0';
    CODEC = ex;
cout << "D" << endl;

    // Set (lower) the resolution for the webcam
    input_cap.set(CV_CAP_PROP_FRAME_WIDTH, X_RESOLUTION);
    input_cap.set(CV_CAP_PROP_FRAME_HEIGHT, Y_RESOLUTION);

    queue<Mat>* currQ = createQ();

  /*---------------------------------------------------
       			MAIN LOOP
    ---------------------------------------------------*/

    // Read in each frame for storage and processing
    while(input_cap.read(frame) ){
		if(currQ->size() < max_frames){
			currQ->push(frame.clone());	// Add frames
		} else {
			writeWithThread(currQ);		// Pass off to thread
			currQ = createQ(); 		// Reset pointer
		}
    } 
    input_cap.release();
}



/*---------------------------------------------------------
                  FUNCTION DEFINITIONS
  ---------------------------------------------------------*/
/* Description: Creates a new directory to store footage if
 *              it doesn't exist already
 */
void create_directory(const char *path, struct stat &st){
    if(stat(path, &st) != 0){
        if(errno == ENOENT){
            cout << "Creating a new video directory" << endl;
            if(mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == 0){
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
string get_date(){
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%F %H_%M_%S", timeinfo);
    return(string(buffer));
}


/* Description: Creates a new ID to name output video file
 *              in format "Year-Month-Day Hour_Minute_Second"
 */
string create_id(const char *path, bool collision){
    if (collision){
        return (path + get_date() + ".avi");
    }
    // No Collision
    return( path + get_date() + "_NC" + ".avi");
}


/* Description: Writes full queue of frames into an output file 
 *              (.avi format) 
 */
void *write_frames(void *qPtr){
    queue<Mat>* q = (queue<Mat>*) qPtr;
    string vid_id;
    create_directory(path, st);
    vid_id = create_id(path, false);

    VideoWriter output_cap;
    output_cap.open(vid_id, 
		    CODEC,
		    FPS, 
                    Size(X_RESOLUTION, Y_RESOLUTION),
                    true);
    
/*
    VideoWriter output_cap(vid_id,
                           CV_FOURCC('M','J','P','G'),
                           FPS,
                           Size(X_RESOLUTION, Y_RESOLUTION),
                           true);
*/

    if(!output_cap.isOpened()){
    	cout << "\nOUTPUT VIDEO COULD NOT BE OPENED\n" << endl;
        return NULL;
    }

    // Write collision seqeuence to output file
    while(!q->empty()){
        output_cap.write(q->front());
		q->pop();
    }

    #ifdef DEBUG
    cout << "\nDONE WRITING\n" << endl;
    #endif

    output_cap.release();
    #ifdef DEBUG
    cout << "\nRELEASED OUTPUT CAP\n" << endl;
    #endif

    delete q;
    pthread_exit(NULL);

    #ifdef DEBUG
    cout  << "\nEXITED THREAD IN WRITE_FRAMES FUNCTION" << endl;
    #endif
}

/* Description: Creates a pointer to a new queue to capture new
 *              frames  
 */
queue<Mat>* createQ(){
	queue<Mat>* qPtr = new queue<Mat>;
	return qPtr;
}

/* Description: Spawns off a new thread for writing frames 
 *              from queue
 */
void writeWithThread(queue<Mat>* q){
	pthread_t thread;
	int retval = pthread_create(&thread,
				    NULL,
				    write_frames,
				    (void*)q);
	if(retval){
		cout << "ERROR CREATING THREAD" << endl;
		exit(EXIT_FAILURE);
	}
}


//-----EOF-----

