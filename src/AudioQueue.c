/*
 * AudioQueue.cpp
 *
 *  Created on: 2016.11.7
 *      Author: leqi
 */
#include<stdlib.h>
#include "AudioQueue.h"

/**
 * base 分配给队列的内存块首地址，内存长度为sizeof(audio_queue_t) + capacity
 * capacity 队列数据区大小，队列实际容量为capacity - 1
 */
audio_queue_t* queue_init(void* base, int capacity)
{
	if (NULL == base) {
		return NULL;
	}
	audio_queue_t* queue = (audio_queue_t*) base;
	pthread_mutex_init(&(queue->mutex),NULL);
	pthread_cond_init(&queue->cond,NULL); //创建条件变量

	queue->capacity = capacity;
	queue->read = 0;
	queue->write = 0;
	queue->more = true;

	return queue;
}

void queue_destroy(audio_queue_t* queue)
{
	if (NULL != queue) {
		pthread_mutex_destroy(&(queue->mutex));
		pthread_cond_destroy(&queue->cond);
	}
}
// 队列总共大小
int queue_real_capacity(audio_queue_t* queue)
{
	return queue->capacity - 1;
}

//  write 位置
int queue_write_pos(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int write = (queue->write);
	pthread_mutex_unlock(&(queue->mutex));

	return write;
}
// read 位置
int queue_read_pos(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int read = (queue->read);
	pthread_mutex_unlock(&(queue->mutex));

	return read;
}


// 同步：当前队列中有效数据大小
int queue_valid_len_sync(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int len = (queue->write - queue->read + queue->capacity) % queue->capacity;
	pthread_mutex_unlock(&(queue->mutex));

	return len;
}

// 异步：当前队列中有效数据大小
int queue_valid_len_asyn(audio_queue_t* queue)
{
	return (queue->write - queue->read + queue->capacity) % queue->capacity;
}


/*  同步：当前队列剩余空间大小 */
int queue_left_space_snyc(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int left = queue->capacity - 1 - queue_valid_len_asyn(queue);
	pthread_mutex_unlock(&(queue->mutex));

	return left;
}


/*  异步：当前队列剩余空间大小 */
int queue_left_space_asyn(audio_queue_t* queue)
{
	return queue->capacity - 1 - queue_valid_len_asyn(queue);
}

int queue_empty(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int empty = queue->write == queue->read;
	pthread_mutex_unlock(&(queue->mutex));

	return empty;
}

int queue_full(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int full = ((queue->write + 1 + queue->capacity) % queue->capacity == queue->read);
	pthread_mutex_unlock(&(queue->mutex));

	return full;
}

int queue_write(audio_queue_t* queue, char data[], int write_size)
{
	if (queue == NULL || data == NULL || write_size <= 0) {
		return false;
	}
	pthread_mutex_lock(&(queue->mutex));
	if (queue_left_space_asyn(queue) < write_size) {  // 如果队列剩余空间小于待写入的大小，报错
		pthread_mutex_unlock(&(queue->mutex));
		printf("queue_left_asyn queue < dataLen \n");
		return false;
	}

	char* queueBase = (char*)(queue + 1);
	char* begin = &((queueBase)[queue->write]);
	if (queue->write + write_size <= queue->capacity - 1) { // 当前读的位置 + read_size < 队列边界
		memcpy(begin, data, write_size);
	} else {

		int dataLen1 = queue->capacity - 1 - queue->write + 1;
		int dataLen2 = write_size - dataLen1;

		memcpy(begin, data, dataLen1);
		memcpy(queueBase, data + dataLen1, dataLen2);
	}

	queue->write = (queue->write + write_size) % queue->capacity;  // 更新读标志的位置
	pthread_mutex_unlock(&(queue->mutex));

	// 唤醒等待pthread_cond_t条件的所有线程
	pthread_cond_broadcast(&queue->cond);
	return true;
}

int queue_read(audio_queue_t* queue, char data[],int read_size)
{
	char* queueBase = NULL;
	char* begin = NULL;
//	int queueLen = 0;
	char *temp_buff = NULL;
	if (queue == NULL || data == NULL) {
		return 0;
	}

	pthread_mutex_lock(&(queue->mutex));

	while(queue_valid_len_asyn(queue) < read_size)
	{
		pthread_cond_wait(&(queue->cond),&(queue->mutex));
	}
	pthread_mutex_unlock(&(queue->mutex));
	queueBase = (char*)(queue + 1);
	begin = &((queueBase)[queue->read]);

	if (NULL == data){
		printf("queue_read malloc error %d\n", read_size);
		return 0;
	}
	
	if (queue->read + read_size <= queue->capacity - 1) {
		memcpy(data, begin, read_size);
	} else {

		int readLen1 = queue->capacity - 1 - queue->read + 1;
		int readLen2 = read_size - readLen1;

		memcpy(data, begin, readLen1);
		memcpy(&(data[readLen1]), queueBase, readLen2);
	}
	queue->read = (queue->read + read_size) % queue->capacity;

	return read_size;
}

void queue_set_more(audio_queue_t* queue, int more)
{
	pthread_mutex_lock(&(queue->mutex));
	queue->more = more;
	pthread_mutex_unlock(&(queue->mutex));
}

int queue_get_more(audio_queue_t* queue)
{
	pthread_mutex_lock(&(queue->mutex));
	int end = queue->more;
	pthread_mutex_unlock(&(queue->mutex));

	return end;
}
