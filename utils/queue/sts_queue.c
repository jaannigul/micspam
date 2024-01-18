#include "sts_queue.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct StsElement {
	void* next;
	void* value;
	_Bool isMalloced;
	int priority;
} StsElement;

struct StsHeader {
	StsElement* head;
	StsElement* tail;
	pthread_mutex_t mutex;
};

static StsHeader* create() {
	StsHeader* handle = malloc(sizeof(StsHeader));
	handle->head = NULL;
	handle->tail = NULL;

	pthread_mutex_init(&handle->mutex, NULL);

	return handle;
}

static void removeAllInternal(StsHeader* header) {
	StsElement* el = header->head;
	while (el != NULL) {
		StsElement* next = el->next;

		if (el->isMalloced)
			free(el->value);

		free(el);
		el = next;
	}

	header->head = NULL;
	header->tail = NULL;
}


static void removeAll(StsHeader* header) {
	pthread_mutex_lock(&header->mutex);
	removeAllInternal(header);
	pthread_mutex_unlock(&header->mutex);
}

static int getCurrentPriority(StsHeader* header) {
	pthread_mutex_lock(&header->mutex);
	int res = header->head == NULL ? 0 : header->head->priority;
	pthread_mutex_unlock(&header->mutex);

	return res;
}

static _Bool isEmpty(StsHeader* header) {
	pthread_mutex_lock(&header->mutex);
	_Bool res = header->head == NULL;
	pthread_mutex_unlock(&header->mutex);

	return res;
}

static void destroy(StsHeader* header) {
	pthread_mutex_destroy(&header->mutex);
	removeAll(header);
	free(header);
	header = NULL;
}

static void push(StsHeader* header, void* elem, int priority, _Bool isElemMalloced) {
	// Create new element
	StsElement* element = malloc(sizeof(StsElement));
	element->value = elem;
	element->isMalloced = isElemMalloced;
	element->priority = priority;
	element->next = NULL;

	pthread_mutex_lock(&header->mutex);
	// Is list empty
	if (header->head == NULL) {
		header->head = element;
		header->tail = element;
	}
	// only add items that have the same priority or greater priority
	else if(header->head->priority == priority) {
		// Rewire
		StsElement* oldTail = header->tail;
		oldTail->next = element;
		header->tail = element;
	}
	else if (header->head->priority < priority) {
		removeAllInternal(header);
		header->head = element;
		header->tail = element;
	}
	pthread_mutex_unlock(&header->mutex);
}

static void* pop(StsHeader* header) {
	pthread_mutex_lock(&header->mutex);
	StsElement* head = header->head;

	// Is empty?
	if (head == NULL) {
		pthread_mutex_unlock(&header->mutex);
		return NULL;
	}
	else {
		// Rewire
		header->head = head->next;

		// Get head and free element memory
		void* value = head->value;
		free(head);

		pthread_mutex_unlock(&header->mutex);
		return value;
	}
}

_StsQueue const StsQueue = {
  create,
  destroy,
  removeAll,
  isEmpty,
  getCurrentPriority,
  push,
  pop
};