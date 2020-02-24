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

const char * existBaseFileAddr = "/mnt/spif";
const char* existFileName = "/mnt/spif/test";
const char* existFileNameFix = "/mnt/spif/exist";
const char* CSVPath = "/mnt/vfat/test";
const char* manyFileSizeTest = "/mnt/spif/manyFileSizeTest.txt";
const size_t readCharBufferSize = 256;
const size_t maxCharbufferSize = 256;
const int maxGroup = 1000;
const char* manyFileSizeTestResCsvPath = "/mnt/vfat/manyFileSizeTestRes";

static double CalcTime(struct timespec diff)
{
	double ts = (double)diff.tv_sec
                      + (double)diff.tv_nsec / (1000000000.0);
	
	return ts;
}

static void diff_timespec(struct timespec *diff,
                          struct timespec *start,
                          struct timespec *stop)
{
  diff->tv_sec = stop->tv_sec - start->tv_sec;
  long nsec = stop->tv_nsec - start->tv_nsec;
  if (nsec < 0)
    {
      diff->tv_sec -= 1;
      nsec = 1000 * 1000 * 1000 - nsec;
    }
  diff->tv_nsec = nsec;
}

char* SetWriteBuffer(int ioBufferSize)
{
	if(ioBufferSize == 1){
		char *buffer0 = (char*)malloc(1);
		memset(buffer0, 'A', 1);
		return buffer0;
	} else if(ioBufferSize == 64){
		char *buffer1 = (char*)malloc(64);
		memset(buffer1, 'A', 64);
		return buffer1;
	} else {
		char *buffer2 = (char*)malloc(128);
		memset(buffer2, 'A', 128);
		return buffer2;
	}
}

void printTim(struct timespec diff)
{
	printf("\nTime:%f\n", (double)diff.tv_sec);
    printf("\nTime:%f\n", (double)diff.tv_nsec);           
}

bool ReadFiles(int fileSize, int ioBufferSize, int loopNum, double* ts)
{
	FILE* readFiles = fopen(existFileName, "r+");
	if(readFiles != NULL){
		char* buffer = SetWriteBuffer(ioBufferSize);
		int ret = 1;
		struct timespec start_ts;
		clock_gettime(CLOCK_REALTIME, &start_ts);
		while(ret){
			ret = fread(buffer, sizeof(char), ioBufferSize, readFiles);
		}
		fclose(readFiles);
		struct timespec stop_ts;
		clock_gettime(CLOCK_REALTIME, &stop_ts);
		struct timespec ts_d;
		diff_timespec(&ts_d, &start_ts, &stop_ts);
		*ts = CalcTime(ts_d);
	} else {
		printf("\nopen exist file error:%s!\n", strerror(errno));
		unlink(existFileName);
		return false;
	}
	unlink(existFileName);
	return true;
}
/***
	* @brief 写入单个文件,获取写入的时间
	* @param fileSize:写入文件的大小
	* @param ioBufferSize: 缓存区大小
	* @param loopNum: 暂时无作用
	* @param ts: 获取文件写入的时间
	*/
bool WriteFiles(int fileSize, int ioBufferSize, int loopNum, double* ts)
{
	FILE* writeFiles = fopen(existFileName, "w+");
	if(writeFiles != NULL){
		char* buffer = SetWriteBuffer(ioBufferSize);
		int surplusCharNum = fileSize;
		int writeCharNum = ioBufferSize;
		struct timespec start_ts;
		clock_gettime(CLOCK_REALTIME, &start_ts);
		
		while(surplusCharNum > 0){
			surplusCharNum -= ioBufferSize;
			if(surplusCharNum < 0){
				writeCharNum = surplusCharNum + ioBufferSize;
			}
			int ret = fwrite(buffer, sizeof(char), writeCharNum, writeFiles);\
			sleep(0.05);
			if (ret != writeCharNum){
				fclose(writeFiles);
				printf("file write error: %s\n", strerror(errno));
				return false;
			}
		}
		fflush(writeFiles);
		fclose(writeFiles);
		struct timespec stop_ts;
		clock_gettime(CLOCK_REALTIME, &stop_ts);

		struct timespec ts_d;
		diff_timespec(&ts_d, &start_ts, &stop_ts);
		*ts = CalcTime(ts_d);
		
	} else {
		printf("\nopen exist file error:%s!\n", strerror(errno));
		return false;
	}
	return true;
}

//bool WriteTable(const char* csvFilePathName, double data)
//{
//	FILE* WriteFiles = fopen(csvFilePathName, "a");
//	if(WriteFiles){
//		char *buffer = (char*)malloc(maxCharbufferSize);
//		sprintf(buffer, "%11.6lf", data);
//		int len = strlen(buffer);
//		buffer[len] = '\n';
//		buffer[len+1] = '\0';
//		fwrite(buffer, sizeof(char), strlen(buffer), WriteFiles);
//		free(buffer);
//		fflush(WriteFiles);
//		fclose(WriteFiles);
//		return true;
//	} else {
//		printf("\nWrite Table failed:%s!\n", strerror(errno));
//		return false;
//	}
//	return true;
//}

bool IsNumber(char ch)
{
	if(ch >= 48 && ch <= 57){
		return true;
	} else {
		return false;
	}
}

bool StrIsInteger(const char* str)
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

void UseAge()
{
	printf("\n1.If you want test with the same FileSize, you can input command followed:\n");
	printf("\n  flash_access_test -f -b -n\n");
	printf("\n   -f:FileSize\n");
	printf("\n   -b:Stdio_buffer_Size(must be 1, 64 or 128)\n");
	printf("\n   -n:WantTestNumber\n");
	printf("\n2.If you want test with various FileSizes, you can input command followed:\n");
	printf("\n  flash_access_test MFS -b -n -l\n");
	printf("\n   -b:Stdio_buffer_Size(must be 1, 64 or 128)\n");
	printf("\n   -n:WantTestNumber\n");
	printf("\n   -l:RandomFileSizeLength\n");
	printf("\n3.If you want write a default file whose size is fixed in SPI_Flash system, you can input command followed:\n");
	printf("\n  flash_access_test -f -n\n");
	printf("\n   -f:FileSize\n");
	printf("\n   -n:NumberOfFile\n");
}

bool WriteTestFile(const char* fileName, char* buffer, int ioBufferSize, int fileSize, double* ts)
{
	FILE* writeFile = fopen(fileName, "w+");
	if(writeFile == NULL){
		printf("\nopen manyFileSizeTest.txt error:%s!\n", strerror(errno));
		return false;
	} else {
		int surplusCharNum = fileSize;
		int writeCharNum = ioBufferSize;
		struct timespec start_ts;
		clock_gettime(CLOCK_REALTIME, &start_ts);
		while(surplusCharNum > 0){
			surplusCharNum -= ioBufferSize;
			if(surplusCharNum < 0){
				writeCharNum = surplusCharNum + ioBufferSize;
			}
			int ret = fwrite(buffer, sizeof(char), writeCharNum, writeFile);
			if (ret != writeCharNum){
				fclose(writeFile);
				printf("manyFileSizeTest.txt write error: %s\n", strerror(errno));
				return false;
			}
		}
		fflush(writeFile);
		fclose(writeFile);
		struct timespec stop_ts;
		clock_gettime(CLOCK_REALTIME, &stop_ts);
		struct timespec ts_d;
		diff_timespec(&ts_d, &start_ts, &stop_ts);
		*ts = CalcTime(ts_d);
		return true;
	}
}

bool ReadTestFile(const char* fileName, char* buffer, int ioBufferSize, double* ts)
{
	FILE* readFile = fopen(fileName, "r+");
	if(readFile != NULL){
		int ret = 1;
		struct timespec start_ts;
		clock_gettime(CLOCK_REALTIME, &start_ts);
		while(ret){
			ret = fread(buffer, sizeof(char), ioBufferSize, readFile);
		}
		fclose(readFile);
		struct timespec stop_ts;
		clock_gettime(CLOCK_REALTIME, &stop_ts);
		struct timespec ts_d;
		diff_timespec(&ts_d, &start_ts, &stop_ts);
		*ts = CalcTime(ts_d);
	} else {
		printf("\nopen exist file error:%s!\n", strerror(errno));
		return false;
	}
	/*remove(fileName);*/
	return true;
}

char* GetCsvFileName()
{
	char* csvFileName = (char*)malloc(maxCharbufferSize);
	int isFileExist = 0;
	int num = 1;
	char indexStr[maxCharbufferSize];
	
	FILE* file;
	while(file != NULL){
		strcpy(csvFileName, manyFileSizeTestResCsvPath);
		itoa(num, indexStr, 10);
		strcat(csvFileName, indexStr);
		strcat(csvFileName, ".csv");
		file = fopen(csvFileName, "r");
		fclose(file);
		++num;
	}
	return csvFileName;
}

bool TestOneFileSize(int ioBufferSize, const char* fileName, FILE* writeTable, int fileSize, double* totalWriteTime, double* totalReadTime)
{
	/*char* csvFileName = GetCsvFileName();
	FILE* writeTable = fopen(csvFileName, "w");
	if(writeTable == NULL){
	printf("\nwrite manyFileSizeTestCsv error:%s!\n", strerror(errno));
	return false;
	} else {
	char* buffer = SetWriteBuffer(ioBufferSize);
	double writeTime = 0.0;
	if(!WriteTestFile(fileName, buffer, ioBufferSize))
	}*/

	char fileSizeStr[maxCharbufferSize];
	itoa(fileSize, fileSizeStr, 10);
	printf("\nFileSize:");
	printf(fileSizeStr);
	printf("\n");
	char *timeBuffer = (char*)malloc(maxCharbufferSize);
	char* buffer = SetWriteBuffer(ioBufferSize);
	
		double writeTime = 0.0;
		if(!WriteTestFile(fileName, buffer, ioBufferSize, fileSize, &writeTime)){
			free(buffer);
			return false;
		} else {
			char* fixString = (char*)malloc(maxCharbufferSize);
			strcpy(fixString, "FileSize:");
			
			strcat(fixString, fileSizeStr);
			strcat(fixString, ",");
			*totalWriteTime = writeTime + *totalWriteTime;
			sprintf(timeBuffer, "%11.6lf", writeTime);
			int len = strlen(timeBuffer);
			timeBuffer[len] = ',';
			timeBuffer[len+1] = '\0';
			fwrite(fixString, sizeof(char), strlen(fixString), writeTable);
			fwrite(timeBuffer, sizeof(char), strlen(timeBuffer), writeTable);
			free(fixString);
			/*fflush(writeTable);*/
			printf("\nWriteTime:");
			timeBuffer[len] = '\0';
			printf(timeBuffer);
			printf(" sec\n");
			double readTime = 0.0;
			if(!ReadTestFile(fileName, buffer, ioBufferSize, &readTime)){
				free(buffer);
				return false;
			} else {
				*totalReadTime = readTime + *totalReadTime;
				sprintf(timeBuffer, "%11.6lf", readTime);
				len = strlen(timeBuffer);
				timeBuffer[len] = '\n';
				timeBuffer[len+1] = '\0';
				fwrite(timeBuffer, sizeof(char), strlen(timeBuffer), writeTable);
				printf("\nReadTime:");
				timeBuffer[len] = '\0';
				printf(timeBuffer);
				printf(" sec\n");
			}
		}
	
	free(buffer);
	free(timeBuffer);
	printf("\nFileSize:");
	printf(fileSizeStr);
	printf(" End\n");
	return true;
}

int GetRandFileSize(int size)
{
	char temp[size];
	int i = 0;
	for(i; i < size; ++i){
		temp[i] = rand()%10+48;
	}
	return atoi(temp);
}

bool TestMixFileSize(int ioBufferSize, const char* fileName, int loopNum, int size)
{
		char* csvFileName = GetCsvFileName();
		FILE* writeTable = fopen(csvFileName, "w");
		if(writeTable == NULL){
			printf("\nwrite ");
			printf(csvFileName);
			printf("\n error:%s!\n", strerror(errno));
			free(csvFileName);
			return false;
		} else {
			double totalReadTime = 0.0;
			double totalWriteTime = 0.0;
			int i = 0;
			for(i; i < loopNum; ++i){
				int fileSize = GetRandFileSize(size);
				/*int fileSize = rand();*/
				if(!TestOneFileSize(ioBufferSize, fileName, writeTable, fileSize, &totalWriteTime, &totalReadTime)){
					fclose(writeTable);
					free(csvFileName);
					return false;
				}
			}
			totalReadTime /= loopNum;
			totalWriteTime /= loopNum;
			char *timeBuffer = (char*)malloc(maxCharbufferSize);
			char fixString[maxCharbufferSize];
			strcpy(fixString, "Arevage:");
			strcat(fixString, ",");
			sprintf(timeBuffer, "%11.6lf", totalWriteTime);
			int len = strlen(timeBuffer);
			timeBuffer[len] = ',';
			timeBuffer[len+1] = '\0';
			fwrite(fixString, sizeof(char), strlen(fixString), writeTable);
			fwrite(timeBuffer, sizeof(char), strlen(timeBuffer), writeTable);
			printf("\nArevage Write Time:");
			timeBuffer[len] = '\0';
			printf(timeBuffer);
			printf(" sec\n");
			sprintf(timeBuffer, "%11.6lf", totalReadTime);
			len = strlen(timeBuffer);
			timeBuffer[len] = '\n';
			timeBuffer[len+1] = '\0';
			fwrite(timeBuffer, sizeof(char), strlen(timeBuffer), writeTable);
			printf("\nArevage Read Time:");
			timeBuffer[len] = '\0';
			printf(timeBuffer);
			printf(" sec\n");
			free(timeBuffer);
			fflush(writeTable);
			fclose(writeTable);
			printf("\nTest success and created result csv file:");
			printf(csvFileName);
			printf("\n");
			free(csvFileName);
			return true;
		}

}

bool WriteFixFiles(int fileSize, int ioBufferSize, int num)
{
	char fileName[maxCharbufferSize];
	sprintf(fileName, "%s%d", existFileNameFix, num+1);
	FILE* writeFiles = fopen(fileName, "w+");
	if(writeFiles != NULL){
		printf("\n%d. ", num);
		printf("Write fix exist file start\n");
		char* buffer = SetWriteBuffer(ioBufferSize);
		int surplusCharNum = fileSize;
		int writeCharNum = ioBufferSize;
		while(surplusCharNum > 0){
			surplusCharNum -= ioBufferSize;
			if(surplusCharNum < 0){
				writeCharNum = surplusCharNum + ioBufferSize;
			}
			/*sleep(0.01);*/
			int ret = fwrite(buffer, sizeof(char), writeCharNum, writeFiles);
			if (ret != writeCharNum){
				fclose(writeFiles);
				printf("file write error: %s\n", strerror(errno));
				return false;
			}
		}
		fflush(writeFiles);
		fclose(writeFiles);
	} else {
		printf("\nWrite fix exist file error:%s!\n", strerror(errno));
		return false;
	}
	printf("\n%d. ", num);
	printf("Write fix exist file success!\n");
	return true;
}

#ifdef CONFIG_BUILD_KERNEL
int main(int argc, FAR char *argv[])
#else
int gnss_main(int argc, char *argv[])
#endif
{
	printf("flash test version 1.0.2\r\n");
	if ((argc == 4) && StrIsInteger(argv[1]) && StrIsInteger(argv[2]) && StrIsInteger(argv[3])){  //查看结果
		int fileSize = atoi(argv[1]);
		int ioBufferSize = atoi(argv[2]);
		int loopNum = atoi(argv[3]);
//		clock_laptime_start(loopNum*4);
		if(ioBufferSize != 1 && ioBufferSize != 64 && ioBufferSize != 128){
			printf("\nFile_size must be 1 , 64 or 128!\n");
			return 0;
		}
		//write
		int i = 0;
//		char writeCSVName[maxCharbufferSize];//写文件结果表格
//		sprintf(writeCSVName, "%sWrite%sAnd%s.csv", CSVPath, argv[1], argv[2]);
//		FILE* WriteFilesFO = fopen(writeCSVName, "w");
//		if(!WriteFilesFO){
//			printf("\nWrite write table failed:%s!\n", strerror(errno));
//			return false;
//		}
//		char readCSVName[maxCharbufferSize];
//		sprintf(readCSVName, "%sRead%sAnd%s.csv", CSVPath, argv[1], argv[2]);
//		FILE* ReadFilesFo = fopen(readCSVName, "w");
//		if(!ReadFilesFo){
//			printf("\nWrite read table failed:%s!\n", strerror(errno));
//			return false;
//		}
		char *buffer = (char*)malloc(maxCharbufferSize);
		for(i; i < loopNum; ++i){
			//write
			double time = 0.0;
			if(!WriteFiles(fileSize, ioBufferSize, i, &time)){
				free(buffer);
//				fclose(WriteFilesFO);
//				fclose(ReadFilesFo);
				return 0;
			}
			if(time < 0.01){	//如果小于0.01 按照0.01计算
				time = 0.01;
			}
			printf("\n%d. Write:sec %11.6lf\n", i+1, time);
//			sprintf(buffer, "%11.6lf", time);
//			int len = strlen(buffer);
//			buffer[len] = '\n';
//			buffer[len+1] = '\0';
//			fwrite(buffer, sizeof(char), strlen(buffer), WriteFilesFO);
			
			//read
			if(!ReadFiles(fileSize, ioBufferSize, i, &time)){
				free(buffer);
//				fclose(WriteFilesFO);
//				fclose(ReadFilesFo);
				return 0;
			}
			if(time < 0.01){
				time = 0.01;
			}
			printf("\n%d. Read:sec %11.6lf\n", i+1, time);
//			sprintf(buffer, "%11.6lf", time);
//			len = strlen(buffer);
//			buffer[len] = '\n';
//			buffer[len+1] = '\0';
//			fwrite(buffer, sizeof(char), strlen(buffer), ReadFilesFo);
		}
		free(buffer);
//		fflush(WriteFilesFO);
//		fclose(WriteFilesFO);
//		fflush(ReadFilesFo);
//		fclose(ReadFilesFo);
		printf("\nProgame success!\n");
//		clock_laptime_stop();
	} else if(argc == 5 && strcmp("MFS", argv[1]) == 0 && StrIsInteger(argv[2]) && StrIsInteger(argv[3]) && StrIsInteger(argv[4])){
		int ioBufferSize = atoi(argv[2]);
		if(ioBufferSize != 1 && ioBufferSize != 64 && ioBufferSize != 128 ){
			printf("\nFile_size must be 1 , 64 or 128!\n");
			return 0;
		}
		if(!TestMixFileSize(ioBufferSize, manyFileSizeTest, atoi(argv[4]), atoi(argv[3]))){
			printf("\nTest failed!\n");
		} 
		return 0;
	} else if(argc == 3 && StrIsInteger(argv[1]) && StrIsInteger(argv[2])){
		int i = 0;
		for(i; i < atoi(argv[2]); ++i){
			if(!WriteFixFiles(atoi(argv[1]), 128, i)){
				return 0;
			}
		}
	} else if(argc == 4 && strcmp("af",argv[1]) == 0 && StrIsInteger(argv[2]) && StrIsInteger(argv[3])){
		//增加空闲文件,格式: af -m -n; 
		//-m 为文件的大小,-n 为文件的数量
		int fileSize = atoi(argv[2]);		//文件的大小
		int fileNum = atoi(argv[3]);		//文件的数量
		char * tempbuf;
		tempbuf = (char *)malloc(fileSize);
		memset(tempbuf,'a',fileSize);
		char filename[20];
		int i ;
		for(i=0;i<fileNum;i++)
		{
			memset(filename,0,20);
			sprintf(filename,"/mnt/spif/temp_%d",i);
			printf("add file name:%s\r\n",filename);
			FILE* tempFileIO = fopen(filename, "w");
			if(tempFileIO != NULL)
				fwrite(tempbuf, sizeof(char), fileSize, tempFileIO);
			else
				printf("file create failure!\r\n");
			fclose(tempFileIO);
		}
		free(tempbuf);
		printf("Add temp file finish!\r\n");
	} else if(argc == 2 && strcmp("df",argv[1])== 0){
		//delete file
		DIR *dir;
		struct dirent *pstr;
		char fileaddr[50];

		printf("delete file name!\r\n");

		if(( dir = opendir(existBaseFileAddr)) == NULL)
		{
			printf("The file dir open failure!");
			return -1;
		}

		printf("The file dir open success!\r\n");

		while((pstr=readdir(dir))  != NULL)
		{
			if(strcmp(pstr->d_name,".")==0 || strcmp(pstr->d_name,"..") == 0)
				continue;
			else if(pstr->d_type == 1)
			{
				sprintf(fileaddr,"%s/%s",existBaseFileAddr,pstr->d_name);
				unlink(fileaddr);
				//print  file the name
				printf("file name:%s,delete file success!\r\n",fileaddr);
			}
		}
		closedir(dir);
		printf("Delete file success");
	} else {
		UseAge();
		return 0;
	}
}
