/*
 * scheduler.c
 *
 *  Created on: May 7, 2018
 *      Author: Monica and Brittany
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// this is just defined for our purposes (testing)
#define MAX_NUM_JOBS 25

//define Job (before being processed)
struct Job{
	int time; //time job arrives
	int memUnits; //mem units required
	int maxDemand; //max device demand
	int timeRun; //execution duration
	char status; //status of job
	/* W = Waiting, H = Holding, R = Ready, J = Rejected
	 * E = Executing, T = Terminated
	 */
	struct Job* next_Job; //next job
	struct Job* prev_Job; //previous job

	/* May not need those ^ pointers since each queue
	 * is a linked list, tbd
	 */
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

struct Job* root_job_waiting; //first job in waiting queue
struct Job* root_job_ready; //first job in ready queue
struct Job* root_job_terminated; //first job complete

//TODO if job added to another queue, delete it from original queue

//SORTED QUEUES
struct Job* root_job_holding1; //first job in holding queue1
struct Job* root_job_holding2; //first job in holding queue2

struct Job all_jobs[MAX_NUM_JOBS];
//array to keep track of every job + status in system

//initialize the system from user input
void start_system(int time, int memSize, int serialDevices, int quantum){

	//all initialized to 0 (no jobs)
	numJobs = 0;
	numWaiting = 0;
	numHolding = 0;
	numReady = 0;
	numTerminated = 0;
	numRejected = 0;

	//function parameters will be from user input
	simStartTime = time;
	mainMemSize = memSize;
	availableMainMem = mainMemSize;
	numSerialDevices = serialDevices;
	timeSlice = quantum;

	//all lists are empty (no jobs)
	root_job_waiting = NULL;
	root_job_holding1 = NULL;
	root_job_holding2 = NULL;
	root_job_ready = NULL;
	root_job_terminated = NULL;

	printf("System started");
}

//create a new job from user input
void create_new_job(int time, int memUnits, int maxDemand, int timeRun){
	struct Job* new_job = malloc(sizeof(*new_job));
	new_job->time = time;
	new_job->memUnits = memUnits;
	new_job->maxDemand = maxDemand;
	new_job->timeRun = timeRun;
	new_job->prev_Job = NULL;

	all_jobs[numJobs] = *new_job; //add to all jobs array
	numJobs++; //increase the total number of jobs in the system

	move_to_waiting_queue(new_job);

	printf("New job added");
}

//change the status of the job and remove from old queue
void change_status(struct Job *job, char new_status){
	struct Job *tmp;

	switch(new_status){

	case 'W':
		//no other action
		break;
	case 'H':
		//delete job from waiting queue
		tmp = root_job_waiting;
		while(tmp != job && tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->prev_Job = tmp->next_Job;
		tmp->next_Job = tmp->prev_Job;
		break;
	case 'R':
		//delete job from holding queue
		break;
	case 'E':
		tmp = root_job_ready;
		//delete job from ready queue
		while(tmp != job && tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->prev_Job = tmp->next_Job;
		tmp->next_Job = tmp->prev_Job;
		break;
	case 'T':
		//no other action
		break;
	}
	job->status = new_status;
}

void move_to_waiting_queue(struct Job *job){
	if(root_job_waiting == NULL){
		//no jobs waiting, add to top of queue
		root_job_waiting = job;
	}else{
		//jobs in waiting queue, add to the back
		struct Job *tmp = root_job_waiting;
		while(tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->next_Job = job;
	}
	numWaiting++;
	change_job_status(job, 'W');
}

void move_to_holding_q1(struct Job *job){

}

void move_to_holding_q2(struct Job *job){

}

void move_to_ready_queue(struct Job *job){

	//no jobs in the ready queue
	if(root_job_ready == NULL){
		root_job_ready = job;
	}else{
		struct Job *tmp = root_job_ready;
		while(tmp->next_Job != NULL){
			tmp = tmp->next_Job;
		}
		tmp->next_Job = job;
	}
	numReady++;
	change_job_status(job, 'R');
}


void request_devices(struct Job* job, int numDevices){


}

void release_devices(struct Job* job, int numDevices){

}

void jobScheduler(struct Job *job){
	if(job->memUnits > mainMemSize || job->maxDemand > numSerialDevices){
		//reject the job
		change_job_status(job, 'J');
		numWaiting--;
		numRejected++;
		//TODO what to do with the job
	}else if(job->memUnits > availableMainMem && job->memUnits < mainMemSize){
		//place job in RIGHT HOLDING QUEUE
		change_job_status(job, 'H');
		numWaiting--;
		numHolding++;
	}else if(job->memUnits <= availableMainMem){
		move_to_ready_queue(job);
		change_job_status(job, 'R');
	}
}

//Given a job from the ready queue, execute the job
//Terminate job after execution
void executeJob(struct Job *job){
	numReady--;
	change_job_status(job, 'E');
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

	//TODO this is makeshift holding queue handling
	//Directions say there is a more legit approach
	//for choosing which holding queue to use, tbd
	struct Job *tmp = root_job_holding1;
	while(tmp->next_Job != NULL){
		//if terminated job lets jobs move from holding to ready
		if(tmp->memUnits <= availableMainMem){
			numHolding--;

		}
	}
	tmp = root_job_holding2;
	while(tmp->next_Job != NULL){
		//see if terminating this job allows any other ready jobs to execute
		if(tmp->memUnits <= availableMainMem){
			executeJob(tmp);
		}
	}

}

int main(int argc, const char * argv[]){
	start_system(9, 45, 12, 1);
	create_new_job(10, 5, 4, 3);


	return 0;
}




