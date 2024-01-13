/*
Jordan Dehmel
jdehmel@outlook.com
github.com/jorbDehmel
2022 - present
MIT licence via mit-license.org held by author
*/

#ifndef COLORTAGS_H
#define COLORTAGS_H

namespace tags
{
const char reset[] = "\033[0;0m";

const char red[] = "\033[0;31m";
const char yellow[] = "\033[0;33m";
const char green[] = "\033[0;32m";
const char blue[] = "\033[0;34m";
const char violet[] = "\033[0;35m";

const char red_thin[] = "\033[2;31m";
const char yellow_thin[] = "\033[2;33m";
const char green_thin[] = "\033[2;32m";
const char blue_thin[] = "\033[2;34m";
const char violet_thin[] = "\033[2;35m";

const char red_bold[] = "\033[1;31m";
const char yellow_bold[] = "\033[1;33m";
const char green_bold[] = "\033[1;32m";
const char blue_bold[] = "\033[1;34m";
const char violet_bold[] = "\033[1;35m";

const char italics[] = "\033[0;3m";
const char bold[] = "\033[0;1m";
const char underscore[] = "\033[0;4m";
const char strikeout[] = "\033[0;9m";

const char bf_black[] = "\033[0;41m";
const char bf_white[] = "\033[0;48m";
const char bf_red[] = "\033[0;42m";
const char bf_yellow[] = "\033[0;44m";
const char bf_green[] = "\033[0;43m";
const char bf_blue[] = "\033[0;45m";
const char bf_violet[] = "\033[0;46m";

const char charfill[] = "\033[2;8m";
} // namespace tags

#endif
