/*
	Hashtable in C
		(c) 2023 Lucas Mendes <lucas AT lucasmendes.org>
	
	Compile with: gcc -O3 -Wall -DDEBUG -o hashtable hashtable.c
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/*
	debugf, __debugf
*/

#define MYSELF				__FUNCTION__
#define DEBUG_BUFFER_SIZE	1024
#ifdef DEBUG
#define debugf(...)			__debugf(MYSELF, __VA_ARGS__)
#else
#define debugf(...)
#endif

int __debugf(const char *myself, const char *fmt, ...) {
	char buffer[DEBUG_BUFFER_SIZE];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buffer, sizeof(buffer), fmt, vl);
	va_end(vl);
	return fprintf(stderr, "\33[1m\%32s\33[0m - %s\n", myself, buffer);
}

/*
	hashtable_node_t
*/

#define HASHTABLE_NODE_COUNT	100
#define HASHTABLE_KEY_SIZE		32

typedef struct hashtable_node_t {
	struct hashtable_node_t *next;
	char key[HASHTABLE_KEY_SIZE];
	int value;
} hashtable_node_t;

void hashtable_node_insert(hashtable_node_t *parent, hashtable_node_t *child) {
	if(parent->next == NULL) {
		parent->next = child;
		return;
	}
	hashtable_node_t *current = parent->next;
	for(; current->next != NULL; current = current->next);
	current->next = child;
	child->next = NULL;	// we never know! :)
}

hashtable_node_t *hashtable_node_find_by_key(hashtable_node_t *node, char *key) {
	if(strnlen(key, HASHTABLE_KEY_SIZE) >= HASHTABLE_KEY_SIZE) {
		debugf("key's length is greater than HASHTABLE_KEY_SIZE");
		return NULL;
	}
	for(hashtable_node_t *current = node; current != NULL; current = current->next) {
		if(!strncmp(current->key, key, HASHTABLE_KEY_SIZE)) {
			debugf("found value %d for key '%s'", current->value, key);
			return current;
		}
	}
	debugf("Cannot find node for key '%s'", key);
	return NULL;
}

void hashtable_node_free(hashtable_node_t *node) {
	free(node);
}

hashtable_node_t *hashtable_node_create(char *key, int value) {
	hashtable_node_t *aux = (hashtable_node_t *) malloc(sizeof(*aux));
	if(!aux) {
		debugf("Out of memory");
		return aux;
	}
	aux->next = NULL;
	aux->value = value;
	strncpy(aux->key, key, HASHTABLE_KEY_SIZE);
	return aux;
}

hashtable_node_t *hashtable_node_init(hashtable_node_t *n, char *key, int value) {
	strncpy(n->key, key, HASHTABLE_KEY_SIZE);
	n->value = value;
	n->next = NULL;
	return n;
}

/*
	hashtable_t
*/

typedef struct hashtable_t {
	hashtable_node_t *array[HASHTABLE_NODE_COUNT];
} hashtable_t;

static int calculate_hash(char *key) {
	int sum = 0;
	for(int i = 0; key[i] && i < HASHTABLE_KEY_SIZE; ++i) {
		sum += key[i];
	}
	return sum % HASHTABLE_NODE_COUNT;
}

void hashtable_destroy(hashtable_t *ht) {
	if(!ht) {
		debugf("Cannot free a NULL hashtable");
	}
	// FIX
	free(ht);
}

hashtable_t *hashtable_create() {
	hashtable_t *aux = (hashtable_t *) malloc(sizeof(*aux));
	if(!aux) {
		debugf("Out of memory");
		return aux;
	}
	memset(aux, 0, sizeof(*aux));
	return aux;
}

int hashtable_get(hashtable_t *ht, char *key) {
	int hash = calculate_hash(key);
	for(hashtable_node_t *current = ht->array[hash];
		current != NULL;
		current = current->next)
	{
		debugf("Trying for key '%s'", key);
		if(!strncmp(current->key, key, HASHTABLE_KEY_SIZE)) {
			return current->value;
		}
	}
	return 0;
}

int hashtable_set(hashtable_t *ht, char *key, int value) {
	int hash = calculate_hash(key);
	debugf("Hash for '%s' is %d", key, hash);
	hashtable_node_t *node = hashtable_node_create(key, value);
	if(!node) {
		debugf("Cannot allocate node for key '%s', value %d", key, value);
		return -1;
	}
	if(ht->array[hash] == NULL) {
		debugf("Empty hash slot for %d. Setting value %s for key '%d'",
			hash, key, value);
		ht->array[hash] = node;
		return 0;
	}
	hashtable_node_t *found = hashtable_node_find_by_key(ht->array[hash], key);
	if(!found) {
		debugf("Cannot find node for key '%s'. Inserting it to parent %p with value = %d",
			key, ht->array[hash], value);
		hashtable_node_insert(ht->array[hash], node);
	} else {
		debugf("Found node for key %s' at %p. Seting value %d to it", found, value);
		found->value = value;
	}
	return 0;
}

/*
	main
*/

int main() {
	hashtable_t *ht = hashtable_create();
	if(!ht) {
		debugf("Cannot allocate hashtable");
		return -1;
	}
	hashtable_set(ht, "eric", 111);
	hashtable_set(ht, "erhd", 222);
	hashtable_set(ht, "john", 333);
	debugf("eric = %d", hashtable_get(ht, "eric"));
	debugf("erhd = %d", hashtable_get(ht, "erhd"));
	debugf("john = %d", hashtable_get(ht, "john"));
	hashtable_destroy(ht);
	return 0;
}
