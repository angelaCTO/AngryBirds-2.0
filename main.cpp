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
#define PRETIME            200 // 10 sec bf collision
#define POSTTIME           600 // 20 sec af collision
#define FPS                20
#define X_RESOLUTION       352
#define Y_RESOLUTION       288
#define WINDOW_SIZE        0
#define TEST_THRESHOLD     1000 // signal threshold  ... tbd from site tests
#define COMPRESSION_LEVEL  60
#define PORT_NUMBER        30000
#define DEVICE_PORT        "/dev/tty02"
#define BAUD_RATE          115200

//Uncomment below to let us know video is storing
#define DEBUG
//Uncomment below to let us analyze the sensor analog stream
#define ANALOG

/*-----------------------------------------------------
              FUNCTION PROTPOTYPES
  -----------------------------------------------------*/
void create_directory(const char *path, struct stat &st);
string create_vid_id(string path, bool collision);
string create_im_id(string path, int im_count, bool is_dated);
string create_dir_path(string path, string sub_dir_name);
string get_date();
void* listenForExit(void* i);
void flashLed(int numTimes);
bool isSignalRecieved(BlackGPIO *ir_in);

bool stopSig = false;

int main()
{
  /*---------------------------------------------------
          VARIABLE DECLARATIONS / INITIALIZATION
    ---------------------------------------------------*/
    Mat     frame;
    queue   <Mat> frames;
    vector  <int> compress_params;
    struct  stat st;
    string  vid_id;
    string  im_id;
    string  path_name;
    string  subdir_name;
    string  subdir_path;
    int     adc             = 0;
    int     dir_count       = 0;
    int     im_count        = 0;
    int     rc;
    int     frame_count;
    int     limit           = PRETIME;
    bool    save            = false;
    bool    detected        = false;
    bool    collision       = false;
    const char* converted_path;
    const char* converted_subpath;

    string img_path = "/home/ubuntu/AngryBirds/SDCard/images/";
    string vid_path = "/home/ubuntu/AngryBirds/SDCard/videos/";
    ofstream signals;

    // Gets analog readings from adc pin 4
    BlackADC *test_adc = new BlackADC(AIN4); 	       // P9_34/33
    BlackGPIO *ir_in = new BlackGPIO(GPIO_68, input);  // P8_10

// int tester = 0; for testing ...can delete
  /*---------------------------------------------------
         FRAME CAPTURE / STORAGE + COLLISION DETECTION
    ---------------------------------------------------*/
    // Start listening for signal to stop
    pthread_t exit_thread;
    rc = pthread_create(&exit_thread, NULL, listenForExit, (void*) NULL);
    if(rc){
        cout << "ERROR: unable to create thread" << endl;
    }

    // Open video no.0
    VideoCapture input_cap(0);

    // Set (lower) the resolution for the webcam
    input_cap.set(CV_CAP_PROP_FRAME_WIDTH, X_RESOLUTION);
    input_cap.set(CV_CAP_PROP_FRAME_HEIGHT, Y_RESOLUTION);

    cout << "\nCREATING MAIN IMAGES DIRECTORY\n" << endl;
    // Create the (final) output destination - where all concatenated
    // clips will be stored ("Images" directory)
    path_name = create_dir_path(img_path, "NONE");
    converted_path = path_name.c_str();
    create_directory(converted_path, st);
    cout << "\nDONE CREATING MAIN VIDEOS DIRECTORY\n" << endl;

    cout << "\nCREATING MAIN VIDEOS DIRECTORY\n" << endl;
    // Create the (final) output destination - where all concatenated
    // clips will be stored ("videos" directory)
    path_name = create_dir_path(vid_path, "NONE");
    converted_path = path_name.c_str();
    create_directory(converted_path, st);
    cout << "\nDONE CREATING MAIN VIDEOS DIRECTORY\n" << endl;

  /*---------------------------------------------------
                        MAIN LOOP
    ---------------------------------------------------*/
    while(true){
        // Check that the camera is open for capturing, 
        // if failure, terminate
        if (!input_cap.isOpened()) {
            cout << "\nINPUT VIDEO COULD NOT BE OPENED\n" << endl;
            return -1;
        }
	if(!isSignalRecieved(ir_in)) {
	        cout << "RECORDING" << endl;
		if(input_cap.read(frame) && !stopSig) {
		     // Open file to write signal data
		     signals.open("/home/ubuntu/AngryBirds/SDCard/signals.txt",
                          fstream::in  |
                          fstream::out |
                          fstream::app);

                    // Create the initial buffer of frames
                    if(frames.size() >= limit) {
	            frames.pop();
                    frames.push(frame.clone());
                    } else {
                        frames.push(frame.clone());
                    }

       		    // Output the analog readings to a signals.txt
        	    adc =  test_adc->getNumericValue();
        	    if (adc >= TEST_THRESHOLD) {
//                    if (tester == 500) { // For testing  ... can delete
//			tester = 0;
        	        cout << "\nEVENT DETECTED\n" << endl;
        	        signals << get_date() << ":    " << adc << endl;
        	        detected = true;
             	        limit = POSTTIME;
        	    }
//                    tester++;

        	    if (detected && frames.size() >= limit) {
            	        cout << "\nCREATING SUBDIR TO STORE THIS COLLISION\n" << endl;
            	        // Create the sub directory that will store all the 
            	        // image files per collision event
            	        subdir_name = to_string(dir_count);
                        subdir_path = create_dir_path(img_path, subdir_name);
                        converted_subpath = subdir_path.c_str();
            	        create_directory(converted_subpath, st);
            	        cout << "\nDONE CREATING THIS SUBDIR\n" << endl;

            	        // Write each frame into an (compressed) jpg img in
            	        //designated subdir.
            	        compress_params.push_back(CV_IMWRITE_JPEG_QUALITY);
            	        compress_params.push_back(COMPRESSION_LEVEL);

            	        // Write the collision sequence into the output sub dir
            	        cout << "\nWRITING THE COLLISION SEQUENCE\n" << endl;
            	        frame_count = 0;
            	        im_count = 0;
                        while (!frames.empty()) {
                    	    im_id = create_im_id(converted_subpath, im_count, false);
                	    try {
                    	        cout << "\nWRITING FRAME NUMBER: " << frame_count << endl;
		                imwrite(im_id, frames.front(), compress_params);
                	    } catch (runtime_error& e) {
                    	        cerr << "\nEXCEPTION CONVERTING IMAGES\n" << endl;
                    	        return 1;
                	    }
                            frames.pop();
                            im_count++;
                            frame_count++;
                        }
                        dir_count++;
                        detected = false;
                        signals.close();
                        usleep(100); //check server
                    }
                    signals.close();
	        } else {
		    break; //break out of infinite while and do cleanup
	        }
	    } else {
		flashLed(5); // Indicate that IR signal has been recieved
//                input_cap.release(); // Close the camera
	        sleep(120); // Sleep for 30 minutes // 2 Minutes Test
//                input_cap.open(0); // Re-open the camera
                flashLed(5); // Indicate that script will resume
	}
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
string create_im_id(string path, int im_count, bool is_dated) {
     string str_im_count;
     string abs_path;
     str_im_count = to_string(im_count);
     if (is_dated) {
         return(path + "/" + get_date() + "__" + str_im_count + ".jpg");
     } else {
         // Need to find a better way of doing this later ...
         // (For a 35 sec video - need 700 frames)
         if (im_count < 10) {
             str_im_count = string(1, '0').append(str_im_count);
         }
         if (im_count < 100) {
             str_im_count = string(1, '0').append(str_im_count);
         }
         return(path + "/" + str_im_count + ".jpg");
     }
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
        cout << "ERROR!" << endl;
    }
    cout << "GPIO pin value: " << ir_in->getValue() << endl;
    if (ir_in->isHigh()) {
        // Flash the LED to indicate signal has been recieved
        flashLed(5);
        cout << "SIGNAL HEARD! - STOP SCRIPT" << endl;
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


//-----EOF-----
