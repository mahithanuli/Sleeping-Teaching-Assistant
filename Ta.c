#include <stdio.h>
#include <stdlib.h>
#include<wait.h>
#include<ctype.h>
#include<unistd.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

void* student_move( void* student_id );
void* ta_move();
int check_Number(char number[]);
int is_Waiting(int student_id);

#define WAITING_CHAIRS 3
#define DEFAULT_STUDENTS 5

sem_t sem_student;
sem_t sem_ta;
pthread_mutex_t mutex_lock;

int waiting_room_chairs[3];
int number_students_waiting = 0;
int next_seating_position = 0;
int next_teaching_position = 0;
int ta_sleep_flag = 0;

int main( int argc, char **argv ){

	int i;
	int student_num;

	if (argc > 1 ) {
		if ( check_Number(argv[1]) == 1) {
			student_num = atoi( argv[1] );
		}
		else {
			printf("Invalid input. Quitting program.");
			return 0;
		}
	}
	else {
		student_num = DEFAULT_STUDENTS;
	}

	int student_id[student_num];             // array of student number 
	pthread_t students[student_num];          // thread for that many no. of students
	pthread_t ta;                             // teaching assistant thread

	sem_init( &sem_student, 0, 0 );          //semaphore for students
	sem_init( &sem_ta, 0, 1 );                //semaphore for teaching assistant

	//Create threads.
	pthread_mutex_init( &mutex_lock, NULL ); //mutex thread intialisation
	pthread_create( &ta, NULL, ta_move, NULL ); //creating thread for ta
	for( i = 0; i < student_num; i++ )
	{
		student_id[i] = i + 1;
		pthread_create( &students[i], NULL, student_move, (void*) &student_id[i] );
	}

	//Join threads
	pthread_join(ta, NULL);
	for( i =0; i < student_num; i++ )
	{
		pthread_join( students[i],NULL );
	}

	return 0;
}

void* ta_move() {

	printf( "Checking for students.\n" );

	while( 1 ) {

		//if students are waiting
		if ( number_students_waiting > 0 ) {

			ta_sleep_flag = 0;
			sem_wait( &sem_student );
			pthread_mutex_lock( &mutex_lock );

			int help_time = rand() % 4+1 ;

			//TA helping student.
			printf( "Helping a student for %d seconds. Students waiting = %d.\t\t", help_time, (number_students_waiting - 1) );
			printf( "Student %d receiving help.\n",waiting_room_chairs[next_teaching_position] );
            //sleep( help_time );
			waiting_room_chairs[next_teaching_position]=0;
			number_students_waiting--;
			next_teaching_position = ( next_teaching_position + 1 ) % WAITING_CHAIRS;

			 sleep( help_time );

			pthread_mutex_unlock( &mutex_lock );
			sem_post( &sem_ta );

		}
		//if no students are waiting
		else {

			if ( ta_sleep_flag == 0 ) {

				printf( "No students waiting. Sleeping.\n" );
				ta_sleep_flag = 1;  // made one when the ta is sleeping

			}

		}

	}

}

void* student_move( void* student_id ) {

	int id_student = *(int*)student_id;

	while( 1 ) {

		//if student is waiting, continue waiting
		if ( is_Waiting( id_student ) == 1 ) { continue; }

		
        
		//student is programming.
		int time = rand() % 4;
		//printf( "\tStudent %d needs help for %d seconds.\n", id_student, time );
		pthread_mutex_lock( &mutex_lock );
		printf( "\tStudent %d needs help \n", id_student );
		sleep( time );

		// pthread_mutex_lock( &mutex_thread );

		if( number_students_waiting < WAITING_CHAIRS ) {

			waiting_room_chairs[next_seating_position] = id_student;
			number_students_waiting++;

			//student takes a seat in the hallway.
			printf( "\t\t--Student %d takes a seat. Students waiting = %d.\n", id_student, number_students_waiting );
			next_seating_position = ( next_seating_position + 1 ) % WAITING_CHAIRS;

			pthread_mutex_unlock( &mutex_lock );

			//wake TA if sleeping
			sem_post( &sem_student );
			sem_wait( &sem_ta );

		}
		else {

			pthread_mutex_unlock( &mutex_lock);
			//No chairs available. Student will try later.
			printf( "\t\t\tStudent %d will try later.\n",id_student );
		


		}

	}

}

int check_Number(char number[])
{
    int i;
		for ( i = 0 ; number[i] != 0; i++ )
    {

        if (!isdigit(number[i]))
            return 0;
    }
    return 1;
}

int is_Waiting( int student_id ) {
	int i;
	for ( i = 0; i < 3; i++ ) {
		if ( waiting_room_chairs[i] == student_id ) { return 1; }
	}
	return 0;
}