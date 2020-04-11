
#include <pthread.h>		
#include <time.h>			
#include <unistd.h>			
#include <semaphore.h>		
#include <stdlib.h>			
#include <stdio.h>			

pthread_t *Students;		
pthread_t TA;				

int Chairs= 0;
int Current_Index = 0;


sem_t TA_Sleep;
sem_t Student_Sem;
sem_t ChairsSem[3];
pthread_mutex_t ChairAccess;


void *TA_Activity();
void *Student_Activity(void *threadID);

int main(int argc, char* argv[])
{
	int number_of_students;		
	int id;
	srand(time(NULL));


	sem_init(&TA_Sleep, 0, 0);
	sem_init(&Student_Sem, 0, 0);
	for(id = 0; id < 3; ++id)			
		sem_init(&ChairsSem[id], 0, 0);

	pthread_mutex_init(&ChairAccess, NULL);
	
	if(argc<2)
	{
		printf("Number of Students is  not specified. Using default (5) students.\n");
		number_of_students = 5;
	}
	else
	{
		printf("Number of Students specified. Creating %d threads.\n", number_of_students);
		number_of_students = atoi(argv[1]);
	}
		
	
	Students = (pthread_t*) malloc(sizeof(pthread_t)*number_of_students);

	pthread_create(&TA, NULL, TA_Activity, NULL);	
	for(id = 0; id < number_of_students; id++)
		pthread_create(&Students[id], NULL, Student_Activity,(void*) (long)id);

	
	pthread_join(TA, NULL);
	for(id = 0; id < number_of_students; id++)
		pthread_join(Students[id], NULL);

	
	free(Students); 
	return 0;
}

void *TA_Activity()
{
	while(1)
	{
		sem_wait(&TA_Sleep);		
		printf("Student awaked the TA\n");

		while(1)
		{
			
			pthread_mutex_lock(&ChairAccess);
			if(Chairs == 0) 
			{
			
				pthread_mutex_unlock(&ChairAccess);
				break;
			}
			
			sem_post(&ChairsSem[Current_Index]);
			Chairs--;
			printf("Student left his/her chair. Remaining Chairs %d\n", 3 - Chairs);
			Current_Index = (Current_Index + 1) % 3;
			pthread_mutex_unlock(&ChairAccess);
		

			printf("\t TA is currently clearing doubt of the student.\n");
			sleep(5);
			sem_post(&Student_Sem);
			usleep(1000);
		}
	}
}

void *Student_Activity(void *threadID) 
{
	int ProgrammingTime;

	while(1)
	{
		printf("Student %ld is doing programming assignment.\n", (long)threadID);
		ProgrammingTime = rand() % 10 + 1;
		sleep(ProgrammingTime);		

		printf("Student %ld needs help from the TA\n", (long)threadID);
		
		pthread_mutex_lock(&ChairAccess);
		int count = Chairs;
		pthread_mutex_unlock(&ChairAccess);

		if(count < 3)		
		{
			if(count == 0)	
				sem_post(&TA_Sleep);
			else
				printf("Student %ld sat on a chair waiting for the TA to finish. \n", (long)threadID);

			
			pthread_mutex_lock(&ChairAccess);
			int index = (Current_Index + Chairs) % 3;
			Chairs++;
			printf("Student sat on chair.Chairs Remaining: %d\n", 3 - Chairs);
			pthread_mutex_unlock(&ChairAccess);


			sem_wait(&ChairsSem[index]);		
			printf("\t Student %ld is getting help from the TA. \n", (long)threadID);
			sem_wait(&Student_Sem);		
			printf("Student %ld left TA room.\n",(long)threadID);
		}
		else 
			printf("Student %ld will return at another time. \n", (long)threadID);
		
	}
}

