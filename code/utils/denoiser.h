#ifndef DENOISER_H
#define DENOISER_H

#include <iostream>
using namespace std;

#include "../utils/os.h"
#include <QDebug>
#include <QString>

string denoiser( string input, string exe, int pr, int sr, float vr, int tr, int frame );

string getFrame( string path, int frame );

#endif