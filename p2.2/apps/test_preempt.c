/*
 * Preemption test
 * Testing that preemption works correctly by creating threads that would
 * normally not yield voluntarly.
 */

 #include <stdbool.h>
 #include <stdio.h>
 #include <stdlib.h>
 
 #include <uthread.h>
 
 static int counter = 0;
 
 void thread1(void *arg)
 {
     (void)arg;
     printf("Thread 1 starting\n");
     
     /* This loop would run forever without preemption */
     while (counter < 10) {
         counter++;
         printf("Thread 1: counter = %d\n", counter);
         /* Busy wait without yielding */
         for (volatile int i = 0; i < 1000000; i++) {}
     }
     
     printf("Thread 1 ending\n");
 }
 
 void thread2(void *arg)
 {
     (void)arg;
     printf("Thread 2 starting\n");
     
     /* This loop would run forever without preemption */
     while (counter < 20) {
         counter++;
         printf("Thread 2: counter = %d\n", counter);
         /* Busy wait without yielding */
         for (volatile int i = 0; i < 1000000; i++) {}
     }
     
     printf("Thread 2 ending\n");
 }
 
 void main_thread(void *arg)
 {
     (void)arg;
     
     printf("Main thread creating other threads\n");
     uthread_create(thread1, NULL);
     uthread_create(thread2, NULL);
     
     printf("Main thread ending\n");
 }
 
 int main(void)
 {
     printf("Testing preemption...\n");
     
     /* Run with preemption enabled */
     uthread_run(true, main_thread, NULL);
     
     printf("Final counter value: %d\n", counter);
     printf("Test complete\n");
     
     return 0;
 }