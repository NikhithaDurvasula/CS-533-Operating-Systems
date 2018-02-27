#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include <aio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

struct aiocb cb;
ssize_t read_wrap(int fd, void * buf, size_t count)
{
  memset(&cb,0,sizeof(struct aiocb));
  cb.aio_nbytes = count;
  if(fd!=0)						// check whether the file descriptor is standard input
		cb.aio_offset=lseek(fd,0, SEEK_CUR);	// setting offset to current position on file read
	cb.aio_fildes = fd;				// setting file descriptor to aio_fildes
	cb.aio_buf = buf;				// setting buffer value
	cb.aio_sigevent.sigev_notify = SIGEV_NONE;	// setting sigv_notify to NONE
	if (aio_read(&cb) == -1)				// when aio_read gives an error
	   {
	    return errno;
     }
/* yield till the progress completes*/
	while (aio_error(&cb) == EINPROGRESS)
		yield();
	cb.aio_offset=lseek(fd,count, SEEK_CUR);		// increasing position of offset from current position to current position +number of positions read
	return  aio_return(&cb);
}
