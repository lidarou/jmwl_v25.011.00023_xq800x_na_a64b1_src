#ifndef _H_AUDIOD2D_INTERFACE_H
#define _H_AUDIOD2D_INTERFACE_H

/*write d2d data to audio
p_ctx --- return value of d2d_test_open
buf -- pointer to data to be writed
size --- data size always 8 is ok
return value: the real writed size. when < 0 is failed*/

int32_t d2d_write_to_audio(void *p_ctx, uint8_t * buf, uint32_t size);

/*open d2d test interface
return NULL is failed, An valid pointer is ok
*/
void* d2d_test_open();

/*close d2d test interface
p_ctx --- return value of d2d_test_open
return 0 is ok, other value is failed*/
int d2d_test_close(void *p_ctx);

#endif

