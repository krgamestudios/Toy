#pragma once

/* toy_console_colors.h - console utility

This file provides a number of macros that can set the color of text in a console
window. These are used for convenience only. They are supposed to be dropped into
a printf()'s first argument, like so:

    printf(TOY_CC_NOTICE "Hello world" TOY_CC_RESET);

NOTE: you need both font AND background for these to work

*/

//platform/compiler-specific instructions
#if defined(TOY_CC_ENABLED) && ( defined(__linux__) || defined(__MINGW32__) || defined(__GNUC__) )

//fonts color
#define TOY_CC_FONT_BLACK      "\033[30;"
#define TOY_CC_FONT_RED        "\033[31;"
#define TOY_CC_FONT_GREEN      "\033[32;"
#define TOY_CC_FONT_YELLOW     "\033[33;"
#define TOY_CC_FONT_BLUE       "\033[34;"
#define TOY_CC_FONT_PURPLE     "\033[35;"
#define TOY_CC_FONT_DGREEN     "\033[6;"
#define TOY_CC_FONT_WHITE      "\033[7;"
#define TOY_CC_FONT_CYAN       "\x1b[36m"

//background color
#define TOY_CC_BACK_BLACK      "40m"
#define TOY_CC_BACK_RED        "41m"
#define TOY_CC_BACK_GREEN      "42m"
#define TOY_CC_BACK_YELLOW     "43m"
#define TOY_CC_BACK_BLUE       "44m"
#define TOY_CC_BACK_PURPLE     "45m"
#define TOY_CC_BACK_DGREEN     "46m"
#define TOY_CC_BACK_WHITE      "47m"

//useful
#define TOY_CC_DEBUG TOY_CC_FONT_BLUE TOY_CC_BACK_BLACK
#define TOY_CC_NOTICE TOY_CC_FONT_GREEN TOY_CC_BACK_BLACK
#define TOY_CC_WARN TOY_CC_FONT_YELLOW TOY_CC_BACK_BLACK
#define TOY_CC_ERROR TOY_CC_FONT_RED TOY_CC_BACK_BLACK
#define TOY_CC_ASSERT TOY_CC_FONT_PURPLE TOY_CC_BACK_BLACK
#define TOY_CC_RESET "\033[0m"

//for unsupported platforms, these become no-ops
#else

//fonts color
#define TOY_CC_FONT_BLACK
#define TOY_CC_FONT_RED
#define TOY_CC_FONT_GREEN
#define TOY_CC_FONT_YELLOW
#define TOY_CC_FONT_BLUE
#define TOY_CC_FONT_PURPLE
#define TOY_CC_FONT_DGREEN
#define TOY_CC_FONT_WHITE
#define TOY_CC_FONT_CYAN

//background color
#define TOY_CC_BACK_BLACK
#define TOY_CC_BACK_RED
#define TOY_CC_BACK_GREEN
#define TOY_CC_BACK_YELLOW
#define TOY_CC_BACK_BLUE
#define TOY_CC_BACK_PURPLE
#define TOY_CC_BACK_DGREEN
#define TOY_CC_BACK_WHITE

//useful
#define TOY_CC_DEBUG TOY_CC_FONT_BLUE TOY_CC_BACK_BLACK
#define TOY_CC_NOTICE TOY_CC_FONT_GREEN TOY_CC_BACK_BLACK
#define TOY_CC_WARN TOY_CC_FONT_YELLOW TOY_CC_BACK_BLACK
#define TOY_CC_ERROR TOY_CC_FONT_RED TOY_CC_BACK_BLACK
#define TOY_CC_ASSERT TOY_CC_FONT_PURPLE TOY_CC_BACK_BLACK
#define TOY_CC_RESET

#endif

