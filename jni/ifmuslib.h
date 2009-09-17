
#ifndef IFMUSLIB_H
#define IFMUSLIB_H

void muslib_init();
void muslib_start();
void muslib_stop();
void muslib_close();

int muslib_load_from_file(const char *filename);
int muslib_load_from_buffer(const unsigned char* source, int size);
int muslib_render(char* buffer, int size);

#endif //!IFMUSLIB_H

