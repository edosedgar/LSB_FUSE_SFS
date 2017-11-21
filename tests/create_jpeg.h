#ifndef  __CREATE_JPEG__
#define __CREATE_JPEG__

#define MAX_WIDTH 1920
#define MIN_WIDTH 300

#define MAX_HEIGHT 1080
#define MIN_HEIGHT 300

void create_jpeg(const char* path);
void delete_jpeg(const char* path);

#endif