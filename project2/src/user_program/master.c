#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>

#define PAGE_SIZE 4096
#define BUF_SIZE 512
size_t get_filesize(const char* filename);//get the size of the input file


int main (int argc, char* argv[])
{
	char *page_dicrs[10000];
	int page_num = 0;
	char buf[BUF_SIZE];
	int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
	size_t ret, file_size, offset, tmp;
	char method[20];
	char *kernel_address = NULL, *file_address = NULL;
	struct timeval start;
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed

	int file_num = atoi(argv[1]);
	char file_list[file_num][50];
	for (int i=0; i<file_num; i++){
		strcpy(file_list[i], argv[i+2]);
	}	
	strcpy(method, argv[file_num + 2]);

	if( (dev_fd = open("/dev/master_device", O_RDWR)) < 0)
	{
		perror("failed to open /dev/master_device\n");
		return 1;
	}
	gettimeofday(&start ,NULL);

	if(ioctl(dev_fd, 0x12345677) == -1) //0x12345677 : create socket and accept the connection from the slave
	{
		perror("ioclt server create socket error\n");
		return 1;
	}
	int totalfilesize=0;	
	
	for (int i=0; i<file_num; i++){
		char file_name[50];
		strcpy(file_name, file_list[i]);
		offset = 0;
	
		if( (file_fd = open (file_name, O_RDWR)) < 0 )
		{
			perror("failed to open input file\n");
			return 1;
		}

		if( (file_size = get_filesize(file_name)) < 0)
		{
			perror("failed to get filesize\n");
			return 1;
		}
		totalfilesize += file_size;
		switch(method[0])
		{
			case 'f': //fcntl : read()/write()
				do
				{
					ret = read(file_fd, buf, sizeof(buf)); // read from the input file
					write(dev_fd, buf, ret);//write to the the device
				}while(ret > 0);
				break;

			case 'm'://mmap
				kernel_address = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, 0);
				while(offset < file_size){
					size_t tmp = PAGE_SIZE;
					if (tmp > (file_size - offset)) tmp = file_size - offset;
					file_address = mmap(NULL, tmp, PROT_READ, MAP_SHARED, file_fd, offset);
					
					page_dicrs[page_num] = file_address;
					page_num += 1;
					//do{
					//	int length = BUF_SIZE;
					//	if ((BUF_SIZE + offset) > file_size){
					//		length = file_size % BUF_SIZE;
					//	}
					//	memcpy(kernel_address, file_address, length);
					//	offset += length;
					//	ioctl(dev_fd, 0x12345678, length);
					//}while(offset < file_size && (offset % PAGE_SIZE) != 0);
					//kernel_address = mmap(NULL, tmp, PROT_WRITE, MAP_SHARED, dev_fd, offset);
					memcpy(kernel_address, file_address, tmp);
					ioctl(dev_fd, 0x12345678, tmp);
					
					offset += PAGE_SIZE;
					
				}
				ioctl(dev_fd, 0x12345676, kernel_address);
				munmap(kernel_address, PAGE_SIZE);
				break;
				
		}

		close(file_fd);
	}

	for (int i = 0; i < (page_num + 1); i++) ioctl(dev_fd, 8787, page_dicrs[i]);

	if(ioctl(dev_fd, 0x12345679) == -1) // end sending data, close the connection
	{
		perror("ioclt server exits error\n");
		return 1;
	}

	gettimeofday(&end, NULL);
	trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
	printf("Transmission time: %lf ms, File size: %d bytes\n", trans_time, totalfilesize / 8);
	
	close(dev_fd);

	return 0;
}

size_t get_filesize(const char* filename)
{
    struct stat st;
    stat(filename, &st);
    return st.st_size;
}
