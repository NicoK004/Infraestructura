/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/
# EJERCICIO 4 SEMAFOROS
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
sem_t ac;
sem_t bd;
sem_t bc;
void* a (void * x){
    
    printf("A\n");
    sem_post(&ac);
}
void* b (void * x){
    
    printf("B\n");
    sem_post(&bc);
    sem_post(&bd);
}
void* c (void * x){
    sem_wait(&ac);
    sem_wait(&bc);
    printf("C\n");
}
void* d (void * x){
    sem_wait(&bd);
    printf("D\n");
}

int main()
{
    pthread_t ta,tb,tc,td;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    
    sem_init(&ac,0,0);
    sem_init(&bd,0,0);
    sem_init(&bc,0,0);
    
    pthread_create(&ta, &attr, a, NULL);
    pthread_create(&tb, &attr, b, NULL);
    pthread_create(&tc, &attr, c, NULL);
    pthread_create(&td, &attr, d, NULL);
    
    pthread_join(ta, NULL);
    pthread_join(tb, NULL);
    pthread_join(tc, NULL);
    pthread_join(td, NULL);
    return 0;
}

#EJERCICIO 6

/******************************************************************************

                            Online C Compiler.
                Code, Compile, Run and Debug C program online.
Write your code in this editor and press "Run" button to compile and execute it.

*******************************************************************************/

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
sem_t m1am2;
sem_t m1af1;
sem_t m1aA1;

sem_t p1aa1;
sem_t p1ap2;



void* m1 (void * x){
    
    printf("M1\n");
    sem_post(&m1am2);
    sem_post(&m1af1);
    sem_post(&m1aA1);
}
void* m2 (void * x){
    sem_wait(&m1am2);
    printf("M2\n");
}
void* f1 (void * x){
    sem_wait(&m1af1);
    printf("F1\n");
}
void* p1 (void * x){
    printf("P1\n");
    sem_post(&p1aa1);
    sem_post(&p1ap2);
}
void* p2 (void * x){
    sem_wait(&p1ap2);
    printf("P2\n");
}
void* a1 (void * x){
    sem_wait(&p1aa1);
    sem_wait(&m1aA1);
    printf("A1\n");
}
void* in (void * x){
    printf("IN\n");
}
void* ig (void * x){
    printf("IG\n");
}

int main()
{
    pthread_t tm1,tm2,tf1,ta1,tp1,tp2,tin,tig;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
  

    sem_init(&m1am2,0,0);
    sem_init(&m1af1,0,0);
    sem_init(&m1aA1,0,0);
    sem_init(&p1aa1,0,0);
    sem_init(&p1ap2,0,0);
    
    pthread_create(&tm1, &attr, m1, NULL);
    pthread_create(&tm2, &attr, m2, NULL);
    pthread_create(&tf1, &attr, f1, NULL);
    pthread_create(&ta1, &attr, a1, NULL);
    
    pthread_create(&tp1, &attr, p1, NULL);
    pthread_create(&tp2, &attr, p2, NULL);
    pthread_create(&tin, &attr, in, NULL);
    pthread_create(&tig, &attr, ig, NULL);

    
    pthread_join(tm1, NULL);
    pthread_join(tm2, NULL);
    pthread_join(tf1, NULL);
    pthread_join(ta1, NULL);
    
    pthread_join(tp1, NULL);
    pthread_join(tp2, NULL);
    pthread_join(tin, NULL);
    pthread_join(tig, NULL);
    return 0;
}