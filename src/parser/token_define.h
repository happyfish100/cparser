/*
 * This file is part of cparser.
 * Copyright (C) 2012 Matthias Braun <matze@braunis.de>
 */

#define PUNCTUATOR(name, tag)         T(_ALL, T_##name, tag, tag,     , false)
#define PUNCTUAVAL(name, tag, val)    T(_ALL, T_##name, tag, tag, =val, false)
#define KEY(mode, x)                  T(mode, T_##x,    #x,  #x,      , true)
#define ALT(mode, x, val)             T(mode, T_##x,    #x,  #x,  =val, true)

/* punctuators */
PUNCTUAVAL(LBRACK,          "[",       '[')
PUNCTUAVAL(RBRACK,          "]",       ']')
PUNCTUAVAL(LPAREN,          "(",       '(')
PUNCTUAVAL(RPAREN,          ")",       ')')
PUNCTUAVAL(LCURLY,          "{",       '{')
PUNCTUAVAL(RCURLY,          "}",       '}')
PUNCTUAVAL(DOT,             ".",       '.')
PUNCTUAVAL(AND,             "&",       '&')
PUNCTUAVAL(ASTERISK,        "*",       '*')
PUNCTUAVAL(PLUS,            "+",       '+')
PUNCTUAVAL(MINUS,           "-",       '-')
PUNCTUAVAL(TILDE,           "~",       '~')
PUNCTUAVAL(EXCLAMATIONMARK, "!",       '!')
PUNCTUAVAL(SLASH,           "/",       '/')
PUNCTUAVAL(PERCENT,         "%",       '%')
PUNCTUAVAL(LESS,            "<",       '<')
PUNCTUAVAL(GREATER,         ">",       '>')
PUNCTUAVAL(CARET,           "^",       '^')
PUNCTUAVAL(PIPE,            "|",       '|')
PUNCTUAVAL(QUESTIONMARK,    "?",       '?')
PUNCTUAVAL(COLON,           ":",       ':')
PUNCTUAVAL(SEMICOLON,       ";",       ';')
PUNCTUAVAL(EQUAL,           "=",       '=')
PUNCTUAVAL(COMMA,           ",",       ',')
PUNCTUAVAL(PREPROCESSOR,    "#",       '#')
PUNCTUAVAL(BACKSLASH,       "\\",      '\\')

PUNCTUAVAL(MINUSGREATER,         "->", 256)
PUNCTUATOR(PLUSPLUS,             "++")
PUNCTUATOR(MINUSMINUS,           "--")
PUNCTUATOR(LESSLESS,             "<<")
PUNCTUATOR(GREATERGREATER,       ">>")
PUNCTUATOR(LESSEQUAL,            "<=")
PUNCTUATOR(GREATEREQUAL,         ">=")
PUNCTUATOR(EQUALEQUAL,           "==")
PUNCTUATOR(EXCLAMATIONMARKEQUAL, "!=")
PUNCTUATOR(ANDAND,               "&&")
PUNCTUATOR(PIPEPIPE,             "||")
PUNCTUATOR(DOTDOTDOT,            "...")
PUNCTUATOR(ASTERISKEQUAL,        "*=")
PUNCTUATOR(SLASHEQUAL,           "/=")
PUNCTUATOR(PERCENTEQUAL,         "%=")
PUNCTUATOR(PLUSEQUAL,            "+=")
PUNCTUATOR(MINUSEQUAL,           "-=")
PUNCTUATOR(LESSLESSEQUAL,        "<<=")
PUNCTUATOR(GREATERGREATEREQUAL,  ">>=")
PUNCTUATOR(ANDEQUAL,             "&=")
PUNCTUATOR(CARETEQUAL,           "^=")
PUNCTUATOR(PIPEEQUAL,            "|=")
PUNCTUATOR(HASHHASH,             "##")

/* literals, misc */
T(_ALL, T_NEWLINE,            NULL, "new line",      ='\n', false)
T(_ALL, T_EOF,                NULL, "end of input",  = 512, false)
T(_ALL, T_IDENTIFIER,         NULL, "identifier",         , false)
T(_ALL, T_NUMBER,             NULL, "number constant",    , false)
T(_ALL, T_CHARACTER_CONSTANT, NULL, "character constant", , false)
T(_ALL, T_STRING_LITERAL,     NULL, "string literal",     , false)
T(_ALL, T_MACRO_PARAMETER,    NULL, "macro parameter",    , false)
T(_ALL, T_UNKNOWN_CHAR,       NULL, "unknown character",  , false)
T(_ALL, T_LINE_COMMENT,       NULL, "line comment as // ...",      , false)
T(_ALL, T_BLOCK_COMMENT,      NULL, "block comment as /* ... */",  , false)

/* keywords */
KEY(_ALL,   auto)
KEY(_ALL,   break)
KEY(_ALL,   case)
KEY(_ALL,   char)
KEY(_ALL,   const)
ALT(_ALL, __const,      T_const)
ALT(_ALL, __const__,    T_const)
KEY(_ALL,   continue)
KEY(_ALL,   default)
KEY(_ALL,   do)
KEY(_ALL,   double)
KEY(_ALL,   else)
KEY(_ALL,   enum)
KEY(_ALL,   extern)
KEY(_ALL,   float)
KEY(_ALL,   for)
KEY(_ALL,   goto)
KEY(_ALL,   if)
KEY(_ALL,   int)
KEY(_ALL,   long)
KEY(_ALL,   register)
KEY(_ALL,   return)
KEY(_ALL,   short)
KEY(_ALL,   signed)
ALT(_ALL, __signed,     T_signed)
ALT(_ALL, __signed__,   T_signed)
KEY(_ALL,   sizeof)
KEY(_ALL,   static)
KEY(_ALL,   struct)
KEY(_ALL,   switch)
KEY(_ALL,   typedef)
KEY(_ALL,   union)
KEY(_ALL,   unsigned)
KEY(_ALL,   void)
KEY(_ALL,   volatile)
ALT(_ALL, __volatile,   T_volatile)
ALT(_ALL, __volatile__, T_volatile)
KEY(_ALL,   while)

/* C99 */
KEY(_C99|_GNUC,       _Bool)
KEY(_C99|_GNUC,       _Complex)
ALT(_ALL,            __complex__,  T__Complex)
ALT(_ALL,            __complex,    T__Complex)
KEY(_C99,            __func__)
ALT(_ALL,            __FUNCTION__, T___func__)
KEY(_C99|_GNUC,       _Imaginary)
KEY(_C99|_CXX|_GNUC,   inline)
ALT(_ALL,            __inline,     T_inline)
ALT(_ALL,            __inline__,   T_inline)
KEY(_C99,              restrict)
ALT(_ALL,            __restrict__, T_restrict)
ALT(_ALL,            __restrict,   T_restrict)

/* C11 */
KEY(_C11,  _Alignas)
KEY(_C11,  _Alignof)
ALT(_ALL, __alignof__, T__Alignof)
ALT(_ALL, __alignof,   T__Alignof)
KEY(_C11,  _Atomic)
KEY(_C11,  _Generic)
KEY(_C11,  _Noreturn)
KEY(_ALL,  _Static_assert)
KEY(_C11,  _Thread_local)
ALT(_ALL, __thread,    T__Thread_local)

/* gcc extensions */
KEY(_ALL,      __extension__)
KEY(_ALL,      __builtin_classify_type)
KEY(_ALL,      __builtin_va_list)
KEY(_ALL,      __builtin_va_arg)
KEY(_ALL,      __builtin_va_copy)
KEY(_ALL,      __builtin_offsetof)
KEY(_ALL,      __builtin_constant_p)
KEY(_ALL,      __builtin_types_compatible_p)
KEY(_ALL,      __builtin_isgreater)
KEY(_ALL,      __builtin_isgreaterequal)
KEY(_ALL,      __builtin_isless)
KEY(_ALL,      __builtin_islessequal)
KEY(_ALL,      __builtin_islessgreater)
KEY(_ALL,      __builtin_isunordered)
KEY(_ALL,      __PRETTY_FUNCTION__)
KEY(_ALL,      __label__)
KEY(_ALL,      __real__)
ALT(_ALL,      __real,                 T___real__)
KEY(_ALL,      __imag__)
ALT(_ALL,      __imag,                 T___imag__)
KEY(_GNUC,   asm)
ALT(_ALL,      __asm__,                T_asm)
ALT(_ALL,      __asm,                  T_asm)
KEY(_GNUC,       typeof)
ALT(_ALL,      __typeof,               T_typeof)
ALT(_ALL,      __typeof__,             T_typeof)
KEY(_ALL,      __attribute__)
ALT(_ALL,      __attribute,            T___attribute__)
KEY(_ALL,      __builtin_va_start)
ALT(_ALL,      __builtin_stdarg_start, T___builtin_va_start)
