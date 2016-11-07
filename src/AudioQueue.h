/*
 * AudioQueue.h
 *
 *  Created on: 2016.11.7
 *      Author: leqi
 *  Brief： 一个可以指定读出大小的环形队列
 */

#ifndef AUDIOQUEUE_H_
#define AUDIOQUEUE_H_

#include <string.h>
#include <pthread.h>
#include <stdbool.h>


typedef struct audio_queue_t {
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	int capacity;
	int write;
	int read;
	int more;
} audio_queue_t;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * base 分配给队列的内存块首地址，内存长度为sizeof(audio_queue_t) + capacity
 * capacity 队列数据区大小，队列实际容量为capacity - 1
 */
audio_queue_t* queue_init(void* base, int capacity);

void queue_destroy(audio_queue_t* queue);

int queue_real_capacity(audio_queue_t* queue);

int queue_write_pos(audio_queue_t* queue);

int queue_read_pos(audio_queue_t* queue);

int queue_valid_len_sync(audio_queue_t* queue);

int queue_valid_len_asyn(audio_queue_t* queue);

int queue_left_space_snyc(audio_queue_t* queue);

int queue_left_space_asyn(audio_queue_t* queue);

int queue_empty(audio_queue_t* queue);

int queue_full(audio_queue_t* queue);

int queue_write(audio_queue_t* queue, char data[], int write_size);

int queue_read(audio_queue_t* queue, char data[],int read_size);

void queue_set_more(audio_queue_t* queue, int more);

int queue_get_more(audio_queue_t* queue);

#ifdef __cplusplus
}
#endif

#endif /* AUDIOQUEUE_H_ */
