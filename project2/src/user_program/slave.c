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
int main (int argc, char* argv[])
{
	char *page_dicrs[10000];
	int page_num = 0;
	char buf[BUF_SIZE];
	int i, dev_fd, file_fd;// the fd for the device and the fd for the input file
	size_t ret, offset, file_size, data_size = -1;
	//char file_name[50];
	char method[20];
	char ip[20];
	struct timeval start;	
	struct timeval end;
	double trans_time; //calulate the time between the device is opened and it is closed
	char *kernel_address, *file_address;
	long size_mmap = sysconf(_SC_PAGE_SIZE);


	int file_num = atoi(argv[1]);
	char file_list[file_num][50];
	for (int i=0; i<file_num; i++){
		strcpy(file_list[i], argv[i+2]);
	}	
	strcpy(method, argv[file_num + 2]);
	strcpy(ip, argv[file_num + 3]);

	gettimeofday(&start ,NULL);

	if( (dev_fd = open("/dev/slave_device", O_RDWR)) < 0)//should be O_RDWR for PROT_WRITE when mmap()
	{
		perror("failed to open /dev/slave_device\n");
		return 1;
	}

	if(ioctl(dev_fd, 0x12345677, ip) == -1)	//0x12345677 : connect to master in the device
	{
		perror("ioclt create slave socket error\n");
		return 1;
	}
	
	int totalfilesize=0;
	size_t set_size = 0;
	for (int i=0; i<file_num; i++){
		char file_name[50];
		strcpy(file_name, file_list[i]);
		file_size = 0;
		
		if( (file_fd = open (file_name, O_RDWR | O_CREAT | O_TRUNC)) < 0) 
		{
			perror("failed to open input file\n");
			return 1;
		}

    	//write(1, "ioctl success\n", 14);
		switch(method[0])
		{
			case 'f'://fcntl : read()/write()

				do
				{
					ret = read(dev_fd, buf, sizeof(buf)); // read from the the device
					write(file_fd, buf, ret); //write to the input file
					file_size += ret;
					if (i == (file_num - 1)){
						continue;
					}
					else if (file_size > (1 * PAGE_SIZE)){
						ret = 0;
					}
				}while(ret > 0);
				break;

			case 'm'://mmap
				kernel_address = mmap(NULL, PAGE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, 0);
				do
				{
					ret = ioctl(dev_fd, 0x12345678, sizeof(buf));
					//posix_fallocate(file_fd, file_size, ret);

					ftruncate(file_fd, file_size + ret);
					//ftruncate(file_fd, set_size + size_mmap);
					//kernel_address = mmap(NULL, ret, PROT_READ, MAP_SHARED, dev_fd, offset);
					offset = (set_size / PAGE_SIZE) * PAGE_SIZE;
					size_t offsize = set_size % PAGE_SIZE;
					file_address = mmap(NULL, ret, PROT_WRITE, MAP_SHARED, file_fd, offset);
					
					page_dicrs[page_num] = file_address;
					page_num += 1;

					memcpy(file_address + offsize, kernel_address, ret);
					//munmap(kernel_address, ret);
					ioctl(dev_fd, 0x12345676, file_address);
					munmap(file_address, ret);
					set_size += ret;
					file_size += ret;
					if (i == (file_num - 1)){
						continue;
					}
					else if (set_size > (1 * PAGE_SIZE)){
						ret = 0;
					}
				}while(ret > 0);	
				ftruncate(file_fd, file_size);
				munmap(kernel_address, PAGE_SIZE);
				break;
		}

		totalfilesize += file_size;
		close(file_fd);
	}

	for (int i = 0; i < page_num; i++) ioctl(dev_fd, 8787, page_dicrs[i]);

	if(ioctl(dev_fd, 0x12345679) == -1)// end receiving data, close the connection
	{
		perror("ioctl client exits error\n");
		return 1;
	}
	
	gettimeofday(&end, NULL);
	trans_time = (end.tv_sec - start.tv_sec)*1000 + (end.tv_usec - start.tv_usec)*0.0001;
	printf("Transmission time: %lf ms, File size: %d bytes\n", trans_time, totalfilesize / 8);

	close(dev_fd);
	return 0;
}


