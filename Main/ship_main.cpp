/* Filename: ship_main.cpp
 * Author(s): Angela To
 * Description: Basic working code to test during 8/13/14 
 *              deployment
 * Date: 8/8/14
 */

#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <queue>
#include <deque>
#include <string>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include "BlackLib/BlackLib.h"

using namespace cv;
using namespace std;

/*-------------------------------------------------
             PREPROCESSOR CONSTANTS
  -------------------------------------------------*/
#define DEBUG	         true
#define PRETIME	         100	   
#define POSTTIME         1200   
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
string get_date();




int main()
{
  cout << "\nTEST1\n" << endl;
  /*---------------------------------------------------
          VARIABLE DECLARATIONS / INITIALIZATION
    ---------------------------------------------------*/
    Mat     frame;
    queue   <Mat> frames;
    deque   <int> sensor_signal;   
    deque   <int> averaged_signal;       // DUSTIN 
    struct  stat st;               
    string  vid_id;               
    float   average_signal  = 0;
    float   normal_signal   = 0;        
    int     frame_count     = 0;         // For Testing Purposes
    int     test_count      = 0;         // For Testing Purposes 
    int     max_count       = 50;        // For Testing Purposes
    int     limit           = PRETIME;  
    bool    save            = false;
    bool    detected        = false;
    bool    collision       = false;

    BlackADC* test_adc = new BlackADC(AIN4);
    const char * path = "/home/ubuntu/AngryBirds/SDCard/videos/";
    ofstream signals;

  /*---------------------------------------------------
       FRAME CAPTURE / STORAGE + COLLISION DETECTION
    ---------------------------------------------------*/
    // Open video no.0
    VideoCapture input_cap(0);
 
    // Set (lower) the resolution for the webcam 
    input_cap.set(CV_CAP_PROP_FRAME_WIDTH, X_RESOLUTION);
    input_cap.set(CV_CAP_PROP_FRAME_HEIGHT, Y_RESOLUTION);

    // Open the camera for capturing, if failure, terminate 
    if (!input_cap.isOpened())
    {
        cout << "\nINPUT VIDEO COULD NOT BE OPENED\n" << endl;
        return -1;
    }

    // Read in each frame for storage and processing 
    while(input_cap.read(frame))
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

        // Open file to write signal data
        signals.open("/home/ubuntu/AngryBirds/signals.txt", 
                      fstream::in  | 
                      fstream::out |
                      fstream::app); 

        // Basic test condition (for testing purposes - to be 
        // changed). Flag if particular signal exceeds test threshold
        // otherwise, proceed with continuous footage capture 
        if ((test_adc->getNumericValue() > TEST_THRESHOLD) ||
            (test_count >= max_count))
        {
            if (test_adc->getNumericValue() > TEST_THRESHOLD) 
            {
                 detected = true;
                 signals << test_adc->getNumericValue();
                 signals << "\n";
             }
            save = true;
	    limit = POSTTIME;
        }

        if (DEBUG) 
        {
            cout << "TEST_COUNT: " << test_count << endl;        
            cout << "FRAME SIZE: " << frames.size() << endl;
            cout << "SAVE: " << save << endl;   
        }
	
        // Event detected, save queue to write to output file 
        if((save) && (frames.size() >= limit))
        { 
            if (DEBUG)
            {
                if (detected)
                { 
                    cout << "\nEVENT TRIGGERED!\n" << endl;
                    //collision = true;
                }
                else 
                {
                    cout << "\nSAVING SCHEDULED NON-COLLISON CLIP\n" << endl;
                }
            }

            if (detected) {collision = true; }

            if (DEBUG) {cout << "\nCREATING VIDEO\n" << endl;}

            // Create the output destination 
            create_directory(path, st);
            vid_id = create_id(path, collision);
	    VideoWriter output_cap(vid_id, 
                                   CV_FOURCC('M','J','P','G'),
                                   FPS, 
                                   Size(X_RESOLUTION, Y_RESOLUTION), 
                                   true);

            if(!output_cap.isOpened())
            {
                cout << "\nOUTPUT VIDEO COULD NOT BE OPENED\n" << endl;
                return -1;
            }

            if (DEBUG) {cout << "\nPUSHING FRAMES\n" << endl;}

            // Write collision seqeuence to output file
            while(!frames.empty())
            {
                if (DEBUG) {cout << "WRITING FRAME: " << frame_count << endl;}
                output_cap.write(frames.front());
                frames.pop();
                frame_count += 1;
            }
            frame_count = 0;

            if (DEBUG) {cout << "\nDONE WRITING\n" << endl;}

            test_count = 0;
            collision = false;
            save = false;
	    limit = PRETIME;
            output_cap.release();
         }
        test_count++;
        signals.close();
    }
    input_cap.release();
}



/*---------------------------------------------------------
                  FUNCTION DEFINITIONS
  ---------------------------------------------------------*/
/* Description: Creates a new directory to store footage if
 *              it doesn't exist
 */
void create_directory(const char *path, struct stat &st) 
{
    if(stat(path, &st) != 0) 
    {
        if(errno == ENOENT) 
        {
            cout << "Creating a new video directory" << endl;
            if(mkdir(path, S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) == 0)
            { 
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
string get_date()
{
    time_t rawtime;
    struct tm *timeinfo;
    char buffer[80];

    time(&rawtime);  
    timeinfo = localtime(&rawtime);
    strftime(buffer, 80, "%F__%H_%M_%S", timeinfo);
    return(string(buffer));
}

/* Description: Creates a new ID to name output video file
 *              in format "Year-Month-Day Hour_Minute_Second"
 */
string create_id(const char *path, bool collision) 
{
    if (collision) 
    {
        return (path + get_date() + ".avi");
    }
    // No Collision
    return( path + get_date() + "_NC" + ".avi");
}

//-----EOF-----
