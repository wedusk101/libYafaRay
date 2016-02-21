// Header file generated by CMake please don't change it
#ifndef Y_CONFIG_H
#define Y_CONFIG_H
#include "yafray_constants.h"
#include <core_api/color_console.h>
#include <core_api/console_verbosity.h>
#include <ctime>

#define MIN_RAYDIST @YAF_MIN_RAY_DIST@
#define YAF_SHADOW_BIAS @YAF_SHADOW_BIAS@

inline std::string printTime()
{
    std::time_t t = std::time(NULL);
    char mbstr[100];
    std::strftime( mbstr, sizeof(mbstr), "%H:%M:%S", std::localtime(&t) );
    return "[" + std::string(mbstr) + "] ";
}

#define Y_INFO yafout.info() << setColor(Green) << printTime() << "INFO: " << setColor()
#define Y_WARNING yafout.warning() << setColor(Yellow) << printTime() << "WARNING: " << setColor()
#define Y_ERROR yafout.error() << setColor(Red) << printTime() << "ERROR: " << setColor()
#define Y_LOG yafout.error() << setColor(Cyan) << printTime() << "LOG: " << setColor()
#define yendl std::endl

#cmakedefine HAVE_UNISTD_H 1

__BEGIN_YAFRAY
typedef float CFLOAT;
typedef float GFLOAT;
typedef float PFLOAT;
__END_YAFRAY
#endif

