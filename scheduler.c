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

// Global time variable
int sysTime = 0;

//define Job (before being processed)
struct Job{
	int number; //job number (identifier)
	int time; //time job arrives
	int memUnits; //mem units required
	int maxDemand; //max device demand
	int timeRun; //execution duration
	int remainingTime; // remaining service time; decr by quantum in Round Robin
	int priority; //priority (1 or 2)
	char status; //status of job (W = Waiting, H = Holding, R = Ready, J = Rejected, E = Executing, T = Terminated_
	struct Job* next_Job; //next job
	struct Job* prev_Job; //previous job
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
int availableDevices; //number of devices available
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
	availableDevices = numSerialDevices;
	timeSlice = quantum;

	//all lists are empty (no jobs)
	root_job_holding1 = NULL;
	root_job_holding2 = NULL;
	root_job_ready = NULL;
	root_job_waiting = NULL;
	root_job_terminated = NULL;

	printf("System started \n");
	printf("total memory: %d \n", mainMemSize);
	printf("memory available: %d \n", availableMainMem);
}

//create a new job from user input
void create_new_job(int jobNum, int time, int memUnits, int maxDemand, int timeRun, int priority){
	struct Job* new_job = malloc(sizeof(*new_job));
	new_job->number = jobNum;
	new_job->time = time;
	new_job->memUnits = memUnits;
	new_job->maxDemand = maxDemand;
	new_job->timeRun = timeRun;
	new_job->priority = priority;
	new_job->prev_Job = NULL;

	all_jobs[numJobs] = new_job; //add to all jobs array
	numJobs++; //increase the total number of jobs in the system

	printf("\nJob %d created \n", new_job->number);

	jobScheduler(new_job);
}

void first_in_first_out(struct Job *job, struct Job *queueRoot){
	struct Job *tmp = queueRoot;
	while(tmp->next_Job != NULL){
		tmp = tmp->next_Job;
	}
	tmp->next_Job = job;
	job->prev_Job = tmp;
	job->next_Job = NULL;
	printf("added to the end of the fifo queue \n");
}

void shortest_job_first(struct Job *job, struct Job *queueRoot){
	struct Job *tmp = queueRoot;
	if(job->timeRun < tmp->timeRun){
		tmp->prev_Job = job;
		job->next_Job = tmp;
		job->prev_Job = NULL;
		printf("new job added to top of holding queue 1\n");
	}else if(job->timeRun > tmp->timeRun){
		while(job->timeRun > tmp->timeRun){
			tmp = tmp->next_Job;
		}
		struct Job *tmp2 = tmp->next_Job;
		tmp->next_Job = job;
		job->prev_Job = tmp;
		job->next_Job = tmp2;
		tmp2->prev_Job = job;
		printf("moved to the right place in holding queue 1 \n");
		//move tmp until right timeRun
	}else{
		//timeRun and priorities are equal, FIFO
		first_in_first_out(job, queueRoot);
		printf("used fifo for holding queue 1 \n");
	}

}

void move_to_holding_q1(struct Job *job){
	if(root_job_holding1 == NULL){
		root_job_holding1 = job;
		root_job_holding1->next_Job = NULL;
		root_job_holding1->prev_Job = NULL;
		printf("first job in holding queue 1 \n");
	}else{
		shortest_job_first(job, root_job_holding1);
	}
	job->status = 'H';
	printf("successfully added to holding queue 1 \n");
}

void move_to_holding_q2(struct Job *job){
	if(root_job_holding2 == NULL){
		printf("first job in holding queue 2 \n");
		root_job_holding2 = job;
		root_job_holding2->next_Job = NULL;
		root_job_holding2->prev_Job = NULL;
	}else{
		first_in_first_out(job, root_job_holding2);
	}
	job->status = 'H';
	printf("added to holding queue 2 \n");
}

void move_to_ready_queue(struct Job *job){
	//no jobs in the ready queue
	if(root_job_ready == NULL){
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
	availableMainMem -= job->memUnits;
	job->status = 'R';
	printf("successfully added to ready queue \n");
}

void move_to_waiting_queue(struct Job *job){

}

void request_devices(int jobNum, int numDevices){
	for(int i=0; i<numJobs; i++){
		if(all_jobs[i]->number == jobNum){
			if(all_jobs[i]->status == 'R' && all_jobs[i]->maxDemand < availableDevices){
				printf("\n%d devices successfully requested by job %d\n", numDevices, jobNum);
				availableDevices -= numDevices;
				printf("%d devices left \n", availableDevices);
				printf("job %d beginning execution \n", jobNum);
				executeJob(all_jobs[i]);
			}else{
				printf("\nrequest by job %d ignored \n", jobNum);
			}
		}
	}

}

void release_devices(int jobNum, int numDevices){
	for(int i=0; i<numJobs; i++){
		if(all_jobs[i]->number == jobNum){
			if(all_jobs[i]->status == 'T'){
				availableDevices += numDevices;
				printf("%d devices successfully release from job %d\n", numDevices, jobNum);
			}else{
				printf("release is invalid, release ignored \n");
			}
		}
	}
}

void jobScheduler(struct Job *job){
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

//TODO currently testing request devices from jobs that are READY (not executing)

//Given a job from the ready queue, execute the job
//Terminate job after execution
void executeJob(struct Job *job){
	job->status = 'E';
	availableMainMem = availableMainMem - job->memUnits;
	terminateJob(job);
}

void terminateJob(struct Job *job){
	job->status = 'T';
	if(root_job_terminated == NULL){
		root_job_terminated = job;
		job->next_Job = NULL;
	}else{
		job->next_Job = root_job_terminated;
		job->prev_Job = job;
	}
	availableMainMem = job->memUnits + availableMainMem;
	release_devices(job->number, job->maxDemand);
/*	//TODO release devices

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
	}*/
}


void procScheduler(struct Job *job){ //should get passed root of ready queue
	//if not enough devices, stick in wait queue
	//if enough, execute for timeSlice then move to next

	// TODO Deal with getting Device Request input
	while(job != NULL){
		/*
		 * If there's less time left to complete the process
		 * then it'll only decrement the remainingTime and increment the total time
		 * the right amount
		 * e.g. If there are 2 secs left and there's a Q = 4, it'll move to the next
		 * job after 2 secs instead of 4.
		 */
		for(int q = timeSlice; q > 0; q--){//decr remaining time by quantum; incr total time by quantum
			if(job->remainingTime > 0){
				job->remainingTime--;
				sysTime--;
			}
			else
				q = 0;
		}
		job = job->next_Job;
	}
}


void print_all_jobs(){
	for(int i=0; i<numJobs; i++){
		printf(all_jobs[i]->number);
		printf(all_jobs[i]->status);
	}
}

void print_queue(struct Job *job){
	while(job->next_Job != NULL){
		printf("%d\n", job->number);
		printf("%c\n",job->status);
		job = job->next_Job;
	}
}

int main(void){
	//start_system(9, 45, 12, 1);
	//create_new_job(10, 5, 4, 3, 1);
	start_system(1, 35, 12, 4);
	create_new_job(1, 3, 20, 5, 10, 1);
	create_new_job(2, 4, 30, 2, 12, 2);
	create_new_job(3, 9, 10, 8, 4, 1);
	request_devices(1, 5);
	create_new_job(4, 13, 20, 4, 11, 2);
	request_devices(3, 2);
	create_new_job(5, 24, 20, 10, 9, 1);
	create_new_job(6, 25, 20, 4, 12, 2);
	request_devices(4, 4);
	request_devices(5, 7);
	release_devices(3, 2);

	printf("\n ------------------ \n");
	printf("printing all jobs \n");
	printf("\n ------------------ \n");

	for(int i=0; i<numJobs; i++){
		printf("\nprinting job %d", all_jobs[i]->number);
		printf("\njob status: %c \n", all_jobs[i]->status);
	}
	return 0;
}




