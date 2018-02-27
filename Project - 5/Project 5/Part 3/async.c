#include <stdio.h> 
#include <sys/stat.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <errno.h>
#include <aio.h>
#include<string.h>
#include <fcntl.h>
#include <stdlib.h>

#include "scheduler.h" 
//struct aiocb aiocb_new;			// creating global aiocb
ssize_t read_wrap(int fd, void * buf, size_t count)
{    
	struct aiocb aiocb_new; 	
	memset(&aiocb_new, 0, sizeof(struct aiocb));		// allocating memory
	aiocb_new.aio_nbytes = count;

	if(lseek(fd,0, SEEK_CUR)==-1 && lseek(fd,0, SEEK_CUR)==ESPIPE)	// check whether the file descriptor is standard input
		aiocb_new.aio_offset=0;		
	else 
		aiocb_new.aio_offset=lseek(fd,0, SEEK_CUR); 	// setting offset to current position on file read 
	aiocb_new.aio_fildes = fd;				// setting file descriptor to aio_fildes
	aiocb_new.aio_buf = buf;				// setting buffer value	
	aiocb_new.aio_sigevent.sigev_notify = SIGEV_NONE;	// setting sigv_notify to NONE
	if (aio_read(&aiocb_new) == -1)				// when aio_read gives an error
	{
		return errno;	
		//return -1;
	}	
	/* yield till the progress completes*/
	while (aio_error(&aiocb_new) == EINPROGRESS)
		yield();
	aiocb_new.aio_offset=lseek(fd,aio_return(&aiocb_new), SEEK_CUR);		// increasing position of offset from current position to current position +number of positions read
	int error= aio_error(&aiocb_new);			
	return  aio_return(&aiocb_new);				// return status

} 
