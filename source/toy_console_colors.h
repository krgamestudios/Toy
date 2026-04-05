#pragma once

/* toy_console_colors.h - console utility

This file provides a number of macros that can set the color of text in a console
window. These are used for convenience only. They are supposed to be dropped into
a printf()'s first argument, like so:

    printf(TOY_CC_NOTICE "Hello world" TOY_CC_RESET);

reference: https://stackoverflow.com/questions/4842424/list-of-ansi-color-escape-sequences

*/

//platform/compiler-specific instructions
#if defined(__linux__) || defined(__MINGW32__) || defined(__GNUC__)

//fonts color
#define TOY_CC_FONT_BLACK      "30"
#define TOY_CC_FONT_RED        "31"
#define TOY_CC_FONT_GREEN      "32"
#define TOY_CC_FONT_YELLOW     "33"
#define TOY_CC_FONT_BLUE       "34"
#define TOY_CC_FONT_MAGENTA    "35"
#define TOY_CC_FONT_CYAN       "36"
#define TOY_CC_FONT_WHITE      "37"
#define TOY_CC_FONT_DEFAULT    "39"

//background color
#define TOY_CC_BACK_BLACK      "40"
#define TOY_CC_BACK_RED        "41"
#define TOY_CC_BACK_GREEN      "42"
#define TOY_CC_BACK_YELLOW     "43"
#define TOY_CC_BACK_BLUE       "44"
#define TOY_CC_BACK_MAGENTA    "45"
#define TOY_CC_BACK_CYAN       "46"
#define TOY_CC_BACK_WHITE      "47"
#define TOY_CC_BACK_DEFAULT    "49"

//useful macros
#define TOY_CC_DEBUG    "\033[" TOY_CC_FONT_BLUE     ";" TOY_CC_BACK_DEFAULT "m"
#define TOY_CC_NOTICE   "\033[" TOY_CC_FONT_GREEN    ";" TOY_CC_BACK_DEFAULT "m"
#define TOY_CC_WARN     "\033[" TOY_CC_FONT_YELLOW   ";" TOY_CC_BACK_DEFAULT "m"
#define TOY_CC_ERROR    "\033[" TOY_CC_FONT_RED      ";" TOY_CC_BACK_DEFAULT "m"
#define TOY_CC_ASSERT   "\033[" TOY_CC_FONT_BLACK    ";" TOY_CC_BACK_MAGENTA "m"
#define TOY_CC_RESET    "\033[" "0" "m"

//for unsupported platforms, these become no-ops
#else

//fonts color
#define TOY_CC_FONT_BLACK
#define TOY_CC_FONT_RED
#define TOY_CC_FONT_GREEN
#define TOY_CC_FONT_YELLOW
#define TOY_CC_FONT_BLUE
#define TOY_CC_FONT_MAGENTA
#define TOY_CC_FONT_CYAN
#define TOY_CC_FONT_WHITE
#define TOY_CC_FONT_DEFAULT

//background color
#define TOY_CC_BACK_BLACK
#define TOY_CC_BACK_RED
#define TOY_CC_BACK_GREEN
#define TOY_CC_BACK_YELLOW
#define TOY_CC_BACK_BLUE
#define TOY_CC_BACK_MAGENTA
#define TOY_CC_BACK_CYAN
#define TOY_CC_BACK_WHITE
#define TOY_CC_BACK_DEFAULT

//useful
#define TOY_CC_DEBUG
#define TOY_CC_NOTICE
#define TOY_CC_WARN
#define TOY_CC_ERROR
#define TOY_CC_ASSERT
#define TOY_CC_RESET

#endif

