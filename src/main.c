/*
 ============================================================================
 Name        : LoopQueue.c
 Author      : leqi
 Version     :
 Copyright   : Your copyright notice
 Description : Test loop queue function
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "AudioQueue.h"

#include <pthread.h>
#include <windows.h>
#define QUEUE_BUFF_MULTIPLE 80000



pthread_t tid_write;
void *thr_fun(void *param)
{
	audio_queue_t *queue = (audio_queue_t*)param;

	char a[] ={1,2,3,4,5,6,7,8,9};

	while(1)
	{

		queue_write(queue,a,9);

		Sleep(10);

	}

}

int main(void) {
	int i = 0;
	char read_buffer[255] = {0};

	int read_size;
	char *buf_temp = (char *) malloc(sizeof(audio_queue_t) +  QUEUE_BUFF_MULTIPLE + 1);
	if (NULL == buf_temp)
	{
		printf("malloc queue space error!\n");
		return 1;
	}

	audio_queue_t *queue = queue_init(buf_temp, QUEUE_BUFF_MULTIPLE + 1);

	pthread_create(&tid_write, NULL, thr_fun, (void*)queue);

	while(1)
	{
		//  ¶ÁÊý¾Ý
		read_size = queue_read(queue,read_buffer,20);
		for(i = 0; i < 20;i++)
		{
			printf("%-3d",read_buffer[i]&0xff);

		}
		printf("read size is %d\n\n",read_size);
	}


	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
	return EXIT_SUCCESS;
}
