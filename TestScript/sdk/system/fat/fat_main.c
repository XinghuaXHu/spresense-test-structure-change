#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <nuttx/clock.h>
#include <nuttx/config.h>
#include <dirent.h>
#include <sys/types.h>

#define SIZE_1K 1023
#define kRandMax 1024*1024*4
#define kRandMin 1024*100
#define TRUE  1
#define FALSE 0


//const char * existBaseFileAddr = "/mnt/spif";
const char * existBaseFileAddr = "/mnt/sd0";

pthread_mutex_t mutex;


static bool IsNumber(char ch)
{
	if(ch >= 48 && ch <= 57){
		return true;
	} else {
		return false;
	}
}

static bool StrIsInteger(const char* str)
{
	int len = strlen(str);
	int i = 0;
	for(i; i < len; ++i){
		if(!IsNumber(str[i])){
			return false;
		}
	}
	return true;
}
static int ctime()
{
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	return (int)ts.tv_sec;
}


static void UseAge()
{
	printf("\n 1.script as seek \n");
	printf("\n   option='seek' is seek test\n");
	printf("\n 1.script as repeat\n");
	printf("\n   option='-mt r' \n");
	printf("\n   repeat read, write, verify, and delete \n");
}

/****
	*@brief add many file
	*@param filesize: file size
	*@param filenum: quantity of document
	*/
static int add_file(char * filename,int filesize,int filenum)
{
	char fileaddr[50];
	char tempfilename[20];
	int i;
	int writetime;
	int residue;
	char tempbuf[SIZE_1K];
	memset(tempbuf,'a',SIZE_1K);
	writetime = filesize/SIZE_1K;
	residue = filesize%SIZE_1K;
	for(i=0;i<filenum;i++)
	{
		if(filename == NULL)
		{
			sprintf(fileaddr,"%s/%s_%d",existBaseFileAddr,"testfile",i);
		}
		else
		{
			sprintf(fileaddr,"%s/%s",existBaseFileAddr,filename);
		}
		printf("[%d] %s: Write Start! -- file size:%d\r\n",ctime(),fileaddr,filesize);
		FILE * fileWriteIO = fopen(fileaddr,"w+");
		if( fileWriteIO == NULL){
			printf("File open error\n");
      assert(FALSE); //Must Never Come
		}
		while(writetime--)
		{
			fwrite(tempbuf, sizeof(char), SIZE_1K, fileWriteIO);
		}
		fwrite(tempbuf, sizeof(char), residue, fileWriteIO);
		fclose(fileWriteIO);
		printf("[%d] %s: Write Done! -- file size:%d\r\n",ctime(),fileaddr,filesize);
	}
	return 0;
}
/****
	* @brief delete file
	* @param
	*/
static int delete_file(char * filename,int filenum)
{
	return 0;
}


static char tempbuf[SIZE_1K] = {'a'};
static char tempReadBuf[SIZE_1K];

static int create_write_data(char* buf, int datasize)
{
	int i;

	for(i=0;i<datasize;i++){
		buf[i] = (i&0xff);
	}
	return i;
}

/****
	* @brief read single file
	* @param
	*/
static int write_single_file(char * filename,int filesize)
{
	//int writetime;
	//int residue;
	//writetime = filesize/SIZE_1K;
	//residue = filesize%SIZE_1K;
	printf("[%d] %s: Write Start! -- file size:%d\r\n",ctime(),filename,filesize);
	FILE * fileWriteIO = fopen(filename,"w+");
	if( fileWriteIO == NULL){
		printf("File open error\n");
    assert(FALSE); //Must Never Come
	}
/*
	while(writetime--)
	{
		fwrite(tempbuf, sizeof(char), SIZE_1K, fileWriteIO);
	}
*/
	create_write_data(tempbuf, filesize);
	fwrite(tempbuf, sizeof(char), filesize, fileWriteIO);
	fclose(fileWriteIO);
	printf("[%d] %s: Write Done! -- file size:%d\r\n",ctime(),filename,filesize);
	return 0;
}
/****
	* @brief read single file
	* @param
	*/
static int read_single_file(char * filename,int filesize)
{
	int readtime = filesize/SIZE_1K;
	int residue = filesize%SIZE_1K;

	memset(tempReadBuf,'a',SIZE_1K);

	printf("[%d] %s: Read Start! -- file size:%d\r\n",ctime(),filename,filesize);
	FILE * fileReadIO = fopen(filename,"r+");
	if( fileReadIO == NULL){
		printf("File open error\n");
    assert(FALSE); //Must Never Come
	}

	fread(tempReadBuf, sizeof(char), filesize, fileReadIO);

	fclose(fileReadIO);
	printf("[%d] %s: Read Done! -- file size:%d\r\n",ctime(),filename,filesize);
	return 0;
}

static int compare_write_read_data(filesize)
{
	return strncmp(tempbuf, tempReadBuf, filesize);
}

/****
	* @brief read single file
	* @param
	*/
static int test_many_thread_rw_file(void * filename)
{
	int ret;
	char tempfilename[50];
	//strcpy(tempfilename,filename);
	sprintf(tempfilename,"%s/%s",existBaseFileAddr,filename);
	int random;

	while(1)
	{
		//Get Random size
		random = (rand() % (kRandMax-kRandMin+1)) + kRandMin;
		random = random & 0x3ff;
		pthread_mutex_lock(&mutex);
		write_single_file(tempfilename,random);
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);
		read_single_file(tempfilename,random);
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);
		if(access(tempfilename,F_OK) != -1)
		{
			printf("[%d] %s: File create OK!\r\n",ctime(),tempfilename);
		}
		else
		{
			printf("[%d] %s: File create NG!\r\n",ctime(),tempfilename);
      assert(FALSE); //Must Never Come
		}
		pthread_mutex_unlock(&mutex);

		pthread_mutex_lock(&mutex);
		if(compare_write_read_data(random) == 0)
		{
			printf("[%d] %s: Verify OK!\r\n",ctime(),tempfilename);
		}
		else
		{
			printf("[%d] %s: Verify NG!\r\n",ctime(),tempfilename);
      assert(FALSE); //Must Never Come
		}
		pthread_mutex_unlock(&mutex);


		pthread_mutex_lock(&mutex);
		if(remove(tempfilename)==0)
		{
			printf("[%d] %s: Remove OK!\r\n",ctime(),tempfilename);
		}
		else
		{
			printf("[%d] %s: Remove NG!\r\n",ctime(),tempfilename);
      assert(FALSE); //Must Never Come
		}
		pthread_mutex_unlock(&mutex);

	}



}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int fat_main(int argc, char *argv[])
#endif
{
	//Get time
	time_t t;
	time(&t);

	printf("smartFS test version 1.0.1\r\n");
	if ((argc == 4) && (strcmp("af",argv[1])==0) && StrIsInteger(argv[2]) && StrIsInteger(argv[3]))
	{
		int filesize;
		int filenum;
		filesize = atoi(argv[2]);
		filenum = atoi(argv[3]);
		add_file(NULL,filesize,filenum);
		return 0;
	}
	else if((argc == 4) && (strcmp("afn",argv[1])==0) && !StrIsInteger(argv[2]) && StrIsInteger(argv[3]))
	{
		char filename[30];
		int filesize;
		strcpy(filename,argv[2]);
		filesize = atoi(argv[3]);
		add_file(filename,filesize,1);
		return 0;
	}
	else if((argc == 2) && (strcmp("seek",argv[1]))==0)
	{
		char filename[50];
		FILE * fp;
		int filesize = SIZE_1K;
		int i;
		int ret;

		printf("fseek test\n");

		sprintf(filename,"%s/%s",existBaseFileAddr,"seek_test");

		fp = fopen(filename,"w+");

		if( fp == NULL){
			printf("File open error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
		}

		create_write_data(tempbuf, filesize);
		if( fwrite(tempbuf, sizeof(char), filesize, fp) == 0 ){
			printf("Error fwrite\n");
      assert(FALSE); //Must Never Come
    }

		if( fclose(fp) != 0 ){
			printf("File close error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
      }

		for( i=1;i<filesize;i++){

			fp = fopen(filename,"r");

			if( fp == NULL){
				printf("File open error, filename=%s\n", filename);
        assert(FALSE); //Must Never Come
      }


			if( fseek(fp, i, SEEK_SET) != 0 ){
				printf("Error fseek\n");
        assert(FALSE); //Must Never Come
      }

			if( (ret=fread(tempReadBuf, sizeof(char), 1, fp)) == 0 ){
				printf("Error fread:Error No = %x\n",ret);
        assert(FALSE); //Must Never Come
			}

			if(tempbuf[i] == tempReadBuf[0]){
				printf("[%d]: Seek OK!\r\n",i);
			}
			else{
				printf("[%d]: Seek NG!\r\n",i);
        assert(FALSE); //Must Never Come
			}

			if( fclose(fp) != 0 ){
				printf("File close error, filename=%s\n", filename);
        assert(FALSE); //Must Never Come
			}

		}

		printf("Seek test[SEEK_CUR] OK\n");

		fp = fopen(filename,"r");

		if( fp == NULL){
			printf("File open error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
		}

		for( i=1;i<filesize;i++){

			if( fseek(fp, 1, SEEK_CUR) != 0 ){
				printf("Error fseek\n");
        assert(FALSE); //Must Never Come
			}

			if( (ret=fread(tempReadBuf, sizeof(char), 1, fp)) == 0 ){
				printf("Error fread:Error No = %x\n",ret);
        assert(FALSE); //Must Never Come
			}


			if(tempbuf[i] == tempReadBuf[0]){
				printf("[%d]: Seek OK!\r\n",i);
			}
			else{
				printf("[%d]: Seek NG!\r\n",i);
        assert(FALSE); //Must Never Come
			}

			if( fseek(fp, -1, SEEK_CUR) != 0 ){
				printf("Error fseek\n");
        assert(FALSE); //Must Never Come
			}


		}

		if( fclose(fp) != 0 ){
			printf("File close error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
		}

		printf("Seek test[SEEK_CUR] OK\n");


		fp = fopen(filename,"r");
		if( fp == NULL){
			printf("File open error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
		}


		if( fseek(fp, i, SEEK_END) != 0 ){
			printf("Error fseek\n");
      assert(FALSE); //Must Never Come
		}

		printf("Sile size = %ld\n",ftell(fp));

		if( fclose(fp) != 0 ){
			printf("File close error, filename=%s\n", filename);
      assert(FALSE); //Must Never Come
		}

		printf("Seek test[SEEK_END] OK\n");

	}
	else if((argc == 3 || argc == 4) && (!strcmp("-mt",argv[1])) && !StrIsInteger(argv[3]))
	{
		//argv[2]: ="r" is the file random size to test
		//			="k" is the file fixed size to test
		if(!strcmp("r",argv[2]))
		{
			pthread_t thread01;
			pthread_t thread02;
			pthread_t thread03;
			pthread_t thread04;
			pthread_t thread05;
			printf("[%d]many thread test,and the file random size to test\r\n",ctime());
			pthread_mutex_init(&mutex,NULL);
			int ret = pthread_create(&thread01,NULL,(void *)test_many_thread_rw_file,(void*)"thread01");
			ret = pthread_create(&thread02,NULL,(void *)test_many_thread_rw_file,(void*)"thread02");
			ret = pthread_create(&thread03,NULL,(void *)test_many_thread_rw_file,(void*)"thread03");
			ret = pthread_create(&thread04,NULL,(void *)test_many_thread_rw_file,(void*)"thread04");
			ret = pthread_create(&thread05,NULL,(void *)test_many_thread_rw_file,(void*)"thread05");

			pthread_join(thread01,NULL);
			pthread_join(thread02,NULL);
			pthread_join(thread03,NULL);
			pthread_join(thread04,NULL);
			pthread_join(thread05,NULL);
			printf("Main over!\r\n");
		}
		else if(!strcmp("k",argv[2]))
		{

		}
		else
		{
			UseAge();
			return -2;
		}
	}
	else
	{
		UseAge();
		return -1;
	}
}
