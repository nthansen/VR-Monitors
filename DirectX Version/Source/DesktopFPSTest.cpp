#include "desktop.h"
#include <iostream>

using namespace std;

int main(int argc, char **argv)
{
    cout << "STARTING DESKTOP CAPTURE SPEED TEST" << endl;
    Desktop * desktop = new Desktop();
    desktop->init();
    FRAME_DATA data; //frame data contains the image as well as metadata for dirty regions 
    bool timed;
    desktop->getFrame(&data, &timed);
}