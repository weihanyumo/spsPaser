#ifndef MYSENDH264TOTRMP
#define MYSENDH264TOTRMP



int My_SendH264To_Rtmp();
int read_buffer1(unsigned char *buf, int buf_size );
//���������Ļص�����
//we use this callback function to read data from buffer
int read_buffer2(unsigned char *buf, int buf_size );
#endif