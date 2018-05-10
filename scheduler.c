/*
 * scheduler.c
 *
 *  Created on: May 7, 2018
 *      Author: Monica and Brittany
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is just defined for testing purposes
#define MAX_NUM_JOBS 25

//define Job (before being processed)
struct Job{
	int time; //time job arrives
	int memUnits; //mem units required
	int maxDemand; //max device demand
	int timeRun; //execution duration
	int priority; //priority (1 or 2)
	char status; //status of job (W = Waiting, H = Holding, R = Ready, J = Rejected, E = Executing, T = Terminated_
	struct Job* next_Job; //next job
	struct Job* prev_Job; //previous job
	// May not need those ^ pointers since each queue is a linked list, tbd
};

//Global variables, used to initiate system requirements
int numJobs; //count of all jobs in system
int numWaiting; //number of jobs in waiting queue
int numHolding; //number of jobs in holding 1 and holding 2
int numReady; //number of jobs in ready queue
int numTerminated; //number of jobs terminated (complete)
int numRejected; //number of jobs rejected

int simStartTime; //system simulated start time
int mainMemSize; //size of system main memory
int availableMainMem; //size of available system main memory
int numSerialDevices; //number of serial devices on system
int timeSlice; //size of system time slice

struct Job* root_job_ready; //first job in ready queue
struct Job* root_job_waiting; //first job in waiting queue
struct Job* root_job_terminated; //first job complete

//TODO if job added to another queue, delete it from original queue

//SORTED QUEUES
struct Job* root_job_holding1; //first job in holding queue1
struct Job* root_job_holding2; //first job in holding queue2

struct Job* all_jobs[MAX_NUM_JOBS]; //array to keep track of every job + status in system

//initialize the system from user input
void start_system(int time, int memSize, int serialDevices, int quantum){

	//all initialized to 0 (no jobs)
	numJobs = 0;
	numHolding = 0;
	numReady = 0;

	//function parameters will be from user input
	simStartTime = time;
	mainMemSize = memSize;
	availableMainMem = mainMemSize;
	numSerialDevices = serialDevices;
	timeSlice = quantum;

	//all lists are empty (no jobs)
	root_job_holding1 = NULL;
	root_job_holding2 = NULL;
	root_job_ready = NULL;
	root_job_waiting = NULL;
	root_job_terminated = NULL;

	printf("System started \n");
}

//create a new job from user input
void create_new_job(int time, int memUnits, int maxDemand, int timeRun, int priority){
	struct Job* new_job = malloc(sizeof(*new_job));
	new_job->time = time;
	new_job->memUnits = memUnits;
	new_job->maxDemand = maxDemand;
	new_job->timeRun = timeRun;
	new_job->priority = priority;
	new_job->prev_Job = NULL;

	all_jobs[numJobs] = new_job; //add to all jobs array
	numJobs++; //increase the total number of jobs in the system

	printf("New job created \n");
}

void move_to_holding_q1(struct Job *job){
	if(root_job_holding1 == NULL){
		printf("root_job_holding1 is null \n");
		root_job_holding1 = job;
		root_job_holding1->next_Job = NULL;
		root_job_holding1->prev_Job = NULL;
	}else{
		struct Job *tmp = root_job_holding1;
		while(tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->next_Job = job;
	}
	printf("added to holding queue 1 \n");
}

void move_to_holding_q2(struct Job *job){
	if(root_job_holding2 == NULL){
		printf("root_job_holding2 is null \n");
		root_job_holding2 = job;
		root_job_holding2->next_Job = NULL;
		root_job_holding2->prev_Job = NULL;
	}else{
		struct Job *tmp = root_job_holding2;
		while(tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->next_Job = job;
	}
	printf("added to holding queue 2 \n");
}

void move_to_ready_queue(struct Job *job){
	//no jobs in the ready queue
	if(root_job_ready == NULL){
		printf("root job ready is null \n");
		root_job_ready = job;
		root_job_ready->next_Job = NULL;
		root_job_ready->prev_Job = NULL;
	}else{
		struct Job *tmp = root_job_ready;
		while(tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->next_Job = job;
		job->prev_Job = tmp;
	}
	printf("added to ready queue \n");
}

void move_to_waiting_queue(struct Job *job){

}

void request_devices(struct Job* job, int numDevices){

}

void release_devices(struct Job* job, int numDevices){

}

void jobScheduler(struct Job *job){
	printf("In job scheduler \n");
	if(job->memUnits > mainMemSize || job->maxDemand > numSerialDevices){//reject the job
		printf("Job needs too much memory or too many devices for the system, rejected");
		//pop off holding queue
		job->status = 'J'; //rejected
	}else if(job->memUnits > availableMainMem && job->memUnits < mainMemSize){//add to holding queue
		printf("currently not enough memory available \n");
		if(job->priority == 1){
			move_to_holding_q1(job);
		}else if(job->priority == 2){
			move_to_holding_q2(job);
		}else{
			printf("invalid priority \n");
		}
	}else if(job->memUnits <= availableMainMem){//add to ready queue
		move_to_ready_queue(job);
	}
}

//Given a job from the ready queue, execute the job
//Terminate job after execution
/*void executeJob(struct Job *job){
	availableMainMem = availableMainMem - job->memUnits;
	terminateJob(job);
}

void terminateJob(struct Job *job){
	change_job_status(job, 'T');
	if(root_job_terminated == NULL){
		root_job_terminated = job;
	}else{
		job->next_Job = root_job_terminated;
		job->prev_Job = job;
	}
	availableMainMem = job->memUnits + availableMainMem;

	//holding queue 1 has higher priority, check this queue first
	struct Job *tmp = root_job_holding1;
	while(tmp->next_Job != NULL){
		//if terminated job lets jobs move from holding to ready
		if(tmp->memUnits <= availableMainMem){
			numHolding--;

		}
	}
	//after checking holding queue 1, do the same to holding queue 2
	tmp = root_job_holding2;
	while(tmp->next_Job != NULL){
		//see if terminating this job allows any other ready jobs to execute
		if(tmp->memUnits <= availableMainMem){
			executeJob(tmp);
		}
	}
}*/

int main(void){
	//start_system(9, 45, 12, 1);
	//create_new_job(10, 5, 4, 3, 1);
	start_system(1, 200, 12, 4);
	create_new_job(3, 20, 5, 10, 1);
	create_new_job(4, 30, 2, 12, 2);
	create_new_job(9, 10, 8, 4, 1);

	for(int i=0; i<numJobs; i++){
		printf("printing job %d: ", i);
		jobScheduler(all_jobs[i]);
	}
	return 0;
}




