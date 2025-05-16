#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <queue.h>

#define TEST_ASSERT(assert)                 \
	do                                      \
	{                                       \
		printf("ASSERT: " #assert " ... "); \
		if (assert)                         \
		{                                   \
			printf("PASS\n");               \
		}                                   \
		else                                \
		{                                   \
			printf("FAIL\n");               \
			exit(1);                        \
		}                                   \
	} while (0)

/* Create */
void test_create(void)
{
	fprintf(stderr, "*** TEST create ***\n");

	TEST_ASSERT(queue_create() != NULL);
}

/* Enqueue/Dequeue simple */
void test_queue_simple(void)
{
	int data = 3, *ptr;
	queue_t q;

	fprintf(stderr, "*** TEST queue_simple ***\n");

	q = queue_create();
	queue_enqueue(q, &data);
	queue_dequeue(q, (void **)&ptr);
	TEST_ASSERT(ptr == &data);
}

static void iterator_inc(queue_t q, void *data)
{
	int *a = (int *)data;

	if (*a == 42)
		queue_delete(q, data);
	else
		*a += 1;
}

void test_iterator(void)
{
	fprintf(stderr, "*** TEST iterator ***\n");
	queue_t q;
	int data[] = {1, 2, 3, 4, 5, 42, 6, 7, 8, 9};
	size_t i;

	/* Initialize the queue and enqueue items */
	q = queue_create();
	for (i = 0; i < sizeof(data) / sizeof(data[0]); i++)
		queue_enqueue(q, &data[i]);

	/* Increment every item of the queue, delete item '42' */
	queue_iterate(q, iterator_inc);
	assert(data[0] == 2);
	assert(queue_length(q) == 9);
}

void test_destroy_non_empty(void)
{
	fprintf(stderr, "*** TEST destroy non-empty ***\n");
	int data = 5;
	queue_t q = queue_create();
	queue_enqueue(q, &data);
	TEST_ASSERT(queue_destroy(q) == -1); // should fail
	// queue_dequeue(q, (void **)&data);	 // cleanup
	TEST_ASSERT(queue_destroy(q) == -1); // now succeed
}

void test_enqueue_dequeue(void)
{
	fprintf(stderr, "*** TEST enqueue + dequeue ***\n");
	int a = 1, b = 2, *p;
	queue_t q = queue_create();

	TEST_ASSERT(queue_enqueue(q, &a) == 0);
	TEST_ASSERT(queue_enqueue(q, &b) == 0);
	TEST_ASSERT(queue_length(q) == 2);

	TEST_ASSERT(queue_dequeue(q, (void **)&p) == 0);
	TEST_ASSERT(p == &a);
	TEST_ASSERT(queue_length(q) == 1);

	TEST_ASSERT(queue_dequeue(q, (void **)&p) == 0);
	TEST_ASSERT(p == &b);
	TEST_ASSERT(queue_length(q) == 0);

	TEST_ASSERT(queue_dequeue(q, (void **)&p) == -1); // empty
	queue_destroy(q);
}

void test_delete_middle(void)
{
	fprintf(stderr, "*** TEST delete middle ***\n");
	int a = 1, b = 2, c = 3;
	queue_t q = queue_create();

	queue_enqueue(q, &a);
	queue_enqueue(q, &b);
	queue_enqueue(q, &c);

	TEST_ASSERT(queue_length(q) == 3);
	TEST_ASSERT(queue_delete(q, &b) == 0); // remove middle
	TEST_ASSERT(queue_length(q) == 2);

	int *p;
	queue_dequeue(q, (void **)&p); // should be a
	TEST_ASSERT(p == &a);
	queue_dequeue(q, (void **)&p); // should be c
	TEST_ASSERT(p == &c);

	queue_destroy(q);
}

void test_delete_head_tail(void)
{
	fprintf(stderr, "*** TEST delete head/tail ***\n");
	int a = 1, b = 2, c = 3;
	queue_t q = queue_create();

	queue_enqueue(q, &a);
	queue_enqueue(q, &b);
	queue_enqueue(q, &c);

	TEST_ASSERT(queue_delete(q, &a) == 0); // delete head
	TEST_ASSERT(queue_delete(q, &c) == 0); // delete tail
	TEST_ASSERT(queue_length(q) == 1);

	int *p;
	queue_dequeue(q, (void **)&p);
	TEST_ASSERT(p == &b);

	queue_destroy(q);
}

void test_delete_nonexistent(void)
{
	fprintf(stderr, "*** TEST delete nonexistent ***\n");
	int a = 1, b = 2;
	queue_t q = queue_create();

	queue_enqueue(q, &a);
	TEST_ASSERT(queue_delete(q, &b) == -1); // b not present

	int *p;
	queue_dequeue(q, (void **)&p);
	TEST_ASSERT(p == &a);

	queue_destroy(q);
}

void test_null_parameters(void)
{
	fprintf(stderr, "*** TEST null parameters ***\n");
	int dummy = 0;
	int *p = NULL;
	queue_t q = queue_create();

	TEST_ASSERT(queue_enqueue(NULL, &dummy) == -1);
	TEST_ASSERT(queue_enqueue(q, NULL) == -1);
	TEST_ASSERT(queue_dequeue(NULL, (void **)&p) == -1);
	TEST_ASSERT(queue_dequeue(q, NULL) == -1);
	TEST_ASSERT(queue_delete(NULL, &dummy) == -1);
	TEST_ASSERT(queue_delete(q, NULL) == -1);
	TEST_ASSERT(queue_length(NULL) == -1);
	TEST_ASSERT(queue_iterate(NULL, NULL) == -1);
	TEST_ASSERT(queue_iterate(q, NULL) == -1);

	queue_destroy(q);
}

void test_queue_enqueue_and_dequeue()
{
	fprintf(stderr, "*** TEST enqueue and dequeue further ***\n");

	int a = 10, b = 20, *p;
	queue_t q = queue_create();

	TEST_ASSERT(queue_length(q) == 0);
	queue_enqueue(q, &a);
	queue_enqueue(q, &b);
	TEST_ASSERT(queue_length(q) == 2);

	queue_dequeue(q, (void **)&p); // saves the data in p
	TEST_ASSERT(p == &a);
	TEST_ASSERT(queue_length(q) == 1);

	queue_dequeue(q, (void **)&p);
	TEST_ASSERT(p == &b);
	TEST_ASSERT(queue_length(q) == 0);

	TEST_ASSERT(queue_destroy(q) == 0);
}

int main(void)
{
	test_destroy_non_empty();
	test_iterator();
	test_enqueue_dequeue();
	test_create();
	test_queue_simple();
	test_delete_middle();
	test_delete_head_tail();
	test_delete_nonexistent();
	test_null_parameters();
	test_queue_enqueue_and_dequeue();

	return 0;
}
