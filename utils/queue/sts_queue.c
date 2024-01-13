#include "sts_queue.h"
#include <stdlib.h>
#include <pthread.h>

typedef struct StsElement {
	void* next;
	void* value;
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

static void removeAll(StsHeader* header) {
	StsElement* el = header->head;
	while (el != NULL) {
		StsElement* next = el->next;
		free(el);
		el = next;
	}
}

static void freeAllValues(StsHeader* header) {
	pthread_mutex_lock(&header->mutex);

	StsElement* el = header->head;
	while (el != NULL) {
		StsElement* next = el->next;
		free(el->value);
		el = next;
	}

	pthread_mutex_unlock(&header->mutex);
}

static void destroy(StsHeader* header) {
	pthread_mutex_destroy(&header->mutex);
	removeAll(header);
	free(header);
	header = NULL;
}

static void push(StsHeader* header, void* elem, int priority) {
	// Create new element
	StsElement* element = malloc(sizeof(StsElement));
	element->value = elem;
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
		removeAll(header);
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
  freeAllValues,
  push,
  pop
};