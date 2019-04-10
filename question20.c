#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
sem_t resource_ready;
sem_t student_semaphors[3];
char* student_types[3] = { "question_paper & pen", "question_paper & paper", "pen & paper" };
bool items_on_table[3] = { false, false, false };
sem_t teacher_semaphores[3];
void* student(void* arg)
{
int student_id = *(int*) arg;
int type_id = student_id % 3;
for (int i = 0; i < 3; ++i)
{
printf("student %d Waiting for %s\n",student_id, student_types[type_id]);
sem_wait(&student_semaphors[type_id]);
printf("\student %d Now making the a assignment\n", student_id);
usleep(rand() % 50000);
sem_post(&resource_ready);
printf("student %d Now assignment\n", student_id);
usleep(rand() % 50000);
}
return NULL;
}
sem_t teacher_lock;
void* teacher(void* arg)
{
int teacher_id = *(int*) arg;
for (int i = 0; i < 3; ++i)
{
sem_wait(&teacher_semaphores[teacher_id]);
sem_wait(&teacher_lock);
if (items_on_table[(teacher_id + 1) % 3])
{
items_on_table[(teacher_id + 1) % 3] = false;
sem_post(&student_semaphors[(teacher_id + 2) % 3]);
}
else if (items_on_table[(teacher_id + 2) % 3])
{
items_on_table[(teacher_id + 2) % 3] = false;
sem_post(&student_semaphors[(teacher_id + 1) % 3]);
}
else
{
items_on_table[teacher_id] = true;
}
sem_post(&teacher_lock);
}
return NULL;
}
void* resource(void* arg)
{
int resource_id = *(int*) arg;
for (int i = 0; i < 3; ++i)
{
usleep(rand() % 200000);
sem_wait(&resource_ready);
sem_post(&teacher_semaphores[resource_id]);
sem_post(&teacher_semaphores[(resource_id + 1) % 3]);
printf("resource %d giving out %s\n",resource_id, student_types[(resource_id + 2) % 3]);
}
return NULL;
}
int main(int argc, char* arvg[])
{
srand(time(NULL));
sem_init(&resource_ready, 0, 1);
sem_init(&teacher_lock, 0, 1);
for (int i = 0; i < 3; ++i)
{
sem_init(&student_semaphors[i], 0, 0);
sem_init(&teacher_semaphores[i], 0, 0);
}
int student_ids[3];
pthread_t student_threads[3];
for (int i = 0; i < 3; ++i)
{
student_ids[i] = i;
if (pthread_create(&student_threads[i], NULL, student, &student_ids[i]) == EAGAIN)
{
perror("Insufficient resources to create thread");
return 0;
}
}
int teacher_ids[3];
pthread_t teacher_threads[3];
for (int i = 0; i < 3; ++i)
{
teacher_ids[i] = i;
if (pthread_create(&teacher_threads[i], NULL, teacher, &teacher_ids[i]) == EAGAIN)
{
perror("Insufficient resources to create thread");
return 0;
}
}
int resource_ids[3];
pthread_t resource_threads[3];
for (int i = 0; i < 3; ++i)
{
resource_ids[i] =i;
if (pthread_create(&resource_threads[i], NULL, resource, &resource_ids[i]) == EAGAIN)
{
perror("Insufficient resources to create thread");
return 0;
}
}
for (int i = 0; i < 3; ++i)
{
pthread_join(student_threads[i], NULL);
}

return 0;
}	

