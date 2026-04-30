#ifndef PARSER_H
#define PARSER_H


#include "SDL3/SDL.h"

#include "renderer.h"


bool isnewline(const char c);
bool isempty(const char c);

model *parse_obj(const char *path);

#endif

