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
#include <vector>
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
#define DEBUG	           true
#define PRETIME	           100	   
#define POSTTIME           1000000
#define FPS 	           20
#define X_RESOLUTION       352    	 
#define Y_RESOLUTION       288    
#define WINDOW_SIZE        0       
#define GROUND_THRESHOLD   0  
#define TEST_THRESHOLD     700 
#define COMPRESSION_LEVEL  40


/*-----------------------------------------------------
              FUNCTION PROTPOTYPES
  -----------------------------------------------------*/
void create_directory(const char *path, struct stat &st);
string create_vid_id(string path, bool collision);
string create_im_id(string path, int im_count); 
string create_dir_path(string path, string sub_dir_name);
string get_date();




int main()
{
  cout << "\nTEST2\n" << endl;
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
    float   average_signal  = 0;
    float   normal_signal   = 0;      
    int     dir_count       = 0;
    int     im_count        = 0;  
    int     frame_count     = 0;         // For Testing Purposes
    int     test_count      = 0;         // For Testing Purposes 
    int     max_count       = 50;        // For Testing Purposes
    int     limit           = PRETIME;  
    bool    save            = false;
    bool    detected        = false;
    bool    collision       = false;

    BlackADC* test_adc = new BlackADC(AIN4);
    string path = "/home/ubuntu/AngryBirds/SDCard/videos/";
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

/**********************************************************************/
/*-------------------------- VIDEO STORAGE ---------------------------*/
/**********************************************************************/
            // Create the (final) output destination - where all concatenated 
            // clips will be stored
            string path_name;
            const char* converted_path;
            path_name = create_dir_path(path, "NONE");
            converted_path = path_name.c_str(); 
            create_directory(converted_path, st);
            // NOTE: create this directory for use by another script that
            // will concatenate each subdir and put them in this dir

            // Create the sub directory that will store all the 
            // image files per collision event
            string subdir_name;
            string subdir_path;
            const char* converted_subpath;
            subdir_name = to_string(dir_count);
            subdir_path = create_dir_path(path, subdir_name);
            converted_subpath = subdir_path.c_str();
            create_directory(converted_subpath, st);

            // Write collision seqeuence to output file
            while(!frames.empty())
            {
                if (DEBUG) {cout << "WRITING FRAME: " << frame_count << endl;}

                // Write each frame into an (compressed) jpg img in 
                //designated subdir.  
                compress_params.push_back(CV_IMWRITE_JPEG_QUALITY);
                compress_params.push_back(COMPRESSION_LEVEL);
                im_count++;
                im_id = create_im_id(converted_subpath, im_count);        
                try {
                   imwrite(im_id, frames.front(), compress_params);
                }
		catch (runtime_error& e) {
                    cerr << "\nEXCEPTION CONVERTING IMAGES\n" << endl;
                    return 1;
                }
		frames.pop();
                frame_count += 1;
            }
            frame_count = 0;

            if (DEBUG) {cout << "\nDONE WRITING\n" << endl;}

            dir_count++;
            im_count = 0;
            test_count = 0;
            collision = false;
            save = false;
	    limit = PRETIME;

         }
        test_count++;
        signals.close();
    }
    input_cap.release();
}



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
    strftime(buffer, 80, "%F %H_%M_%S", timeinfo);
    return(string(buffer));
}

/* Description: Creates a new ID to name output image file
 *              in format "Year-Month-Day Hour_Minute_Second__#"
 */
string create_im_id(string path, int im_count) 
{
     string str_im_count;
     str_im_count = to_string(im_count);
     return (path + "/" + get_date() + "__" + str_im_count + ".png");
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

//-----EOF-----
