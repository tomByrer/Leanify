#ifndef MAIN_H
#define MAIN_H



bool is_fast;
bool is_verbose;
#ifdef _WIN32
bool is_pause;
#endif // _WIN32

// iteration of zopfli
int iterations;

// a normal file: depth 1
// file inside zip that is inside another zip: depth 3
int depth;
int max_depth;




#endif