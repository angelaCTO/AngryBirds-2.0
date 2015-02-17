#include <fstream>
#include <iostream>
#include <unistd.h>
#include <stdio.h>
using namespace std;

int main()
{
    int count = 0;
    int c=4;
    // Create new file to test success of cron scheduler
    ofstream f;
    f.open("/home/ubuntu/AngryBirds/ok.txt", 
               std::fstream::in  | 
               std::fstream::out | 
               std::fstream::app);
    f << "\nSUCCESS1\n";
    f.close();
    while (c>=0) {
        f.open("/home/ubuntu/AngryBirds/ok.txt", 
                    std::fstream::in  | 
                    std::fstream::out |
                    std::fstream::app);
        cout << "opened ok" << endl;
        if(!f)
        {
            cerr<<"Cannot open the output file." << endl;
            return 1;
        }
        while (count < 10)
        {
            cout << "writing to file" << endl;
            f << "\nSUCCESS\n";
            count++;
            f << count;
            f << "ok";

        }
        count = 0;
        f.close();
        cout << "closed ok" << endl;
        c--;
    }
    f.open("/home/ubuntu/AngryBirds/ok.txt", 
                std::fstream::in  |
                std::fstream::out | 
                std::fstream::app);
    f << "\nSUCCESS2\n";
    f.close();

    // NOTE: output is diverted to /var/log/syslog during cron
    // Loop to test kill script
    while (1)
    {
        cout << "SUCCESS" << endl;
        sleep(1);
    }
}
