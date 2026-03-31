#include <stdio.h>
#include <string.h>

#define MAX_QUEUE 100

struct process {
	int id;
	char name[30];
	int type;
	int at;
	int bt;
	int remaining;
	int completed;
	int wt;
	int tat;
	int deadline;
	int abs_deadline;
	int io_req;
	int io_burst;
	int io_done;
	int io_start;
	int finish;
	int missed;
	int response_time;
	int first_run;
	char state[20];
};

int ready_queue[MAX_QUEUE];
int front = 0, rear = -1, queue_size = 0;

void enqueue(int pid) {
	rear = (rear + 1) % MAX_QUEUE;
	ready_queue[rear] = pid;
	queue_size++;
}

int dequeue() {
	if (queue_size == 0) return -1;
	int pid = ready_queue[front];
	front = (front + 1) % MAX_QUEUE;
	queue_size--;
	return pid;
}

int queue_empty() {
	return queue_size == 0;
}

void read_file(struct process p[], int *n);
void sort(struct process p[], int n);
void reset(struct process p[], int n);
void calculate(struct process p[], int n, int tq, int disp);
void print_results(struct process p[], int n, int disp, FILE *fp);

int main() {
	int n, tq;
	struct process p[50];

	read_file(p, &n);
	sort(p, n);

	printf("Enter the time quantum: ");
	scanf("%d", &tq);

	FILE *fp = fopen("results.csv", "w");
	fprintf(fp, "disp,avg_wt,avg_tat,cpu_util,hard_wt,hard_tat,soft_wt,soft_tat,hard_miss,soft_miss\n");

	int overheads[] = {0, 5, 10, 15, 20, 25};
	for (int i = 0; i < 6; i++) {
		reset(p, n);
		calculate(p, n, tq, overheads[i]);
		print_results(p, n, overheads[i], fp);
	}

	fclose(fp);
	printf("\nResults saved to results.csv\n");

	return 0;
}

void read_file(struct process p[], int *n) {
	FILE *f = fopen("tasks.csv", "r");
	if (!f) {
		printf("Error: tasks.csv not found\n");
		*n = 0;
		return;
	}

	char header[200];
	fgets(header, sizeof(header), f);

	*n = 0;
	while (fscanf(f, "%d,%[^,],%d,%d,%d,%d,%d,%d",
		&p[*n].id, p[*n].name, &p[*n].type,
		&p[*n].at, &p[*n].bt, &p[*n].deadline,
		&p[*n].io_req, &p[*n].io_burst) == 8) {
		(*n)++;
	}
	fclose(f);
	printf("Loaded %d processes from tasks.csv\n", *n);
}

void reset(struct process p[], int n) {
	for (int i = 0; i < n; i++) {
		p[i].remaining = p[i].bt;
		p[i].completed = 0;
		p[i].wt = 0;
		p[i].tat = 0;
		p[i].io_done = 0;
		p[i].io_start = -1;
		p[i].finish = 0;
		p[i].missed = 0;
		p[i].response_time = -1;
		p[i].first_run = 0;
		p[i].abs_deadline = p[i].at + p[i].deadline;
		strcpy(p[i].state, "NEW");
	}
	front = 0;
	rear = -1;
	queue_size = 0;
}

void sort(struct process p[], int n) {
	struct process temp;
	for (int i = 0; i < n - 1; i++) {
		for (int j = 0; j < n - i - 1; j++) {
			if (p[j].at > p[j + 1].at) {
				temp = p[j];
				p[j] = p[j + 1];
				p[j + 1] = temp;
			}
		}
	}
}

void calculate(struct process p[], int n, int tq, int disp) {
	int clk = 0;
	int done = 0;
	int disp_left = 0;
	int current = -1;
	int slice_left = 0;

	printf("\n=== SIMULATION (TQ=%d, Overhead=%d) ===\n", tq, disp);

	while (done < n) {
		// Check I/O completions FIRST
		for (int i = 0; i < n; i++) {
			if (strcmp(p[i].state, "WAITING") == 0 && p[i].io_start != -1) {
				if (clk == p[i].io_start + p[i].io_burst) {
					p[i].io_done = 1;
					strcpy(p[i].state, "READY");
					enqueue(i);
					printf("[T=%d] P%d: I/O completed -> READY (waiting for CPU)\n", clk, p[i].id);
				}
			}
		}

		// Check arrivals SECOND
		for (int i = 0; i < n; i++) {
			if (strcmp(p[i].state, "NEW") == 0 && p[i].at == clk) {
				strcpy(p[i].state, "READY");
				enqueue(i);
				printf("[T=%d] P%d: Arrived -> READY\n", clk, p[i].id);
			}
		}

		// Dispatcher overhead
		if (disp_left > 0) {
			disp_left--;
			clk++;
			continue;
		}

		// Get next from queue
		if (current == -1 && !queue_empty()) {
			current = dequeue();
			strcpy(p[current].state, "RUNNING");
			slice_left = tq;
			if (!p[current].first_run) {
				p[current].first_run = 1;
				p[current].response_time = clk - p[current].at;
			}
			printf("[T=%d] P%d: READY -> RUNNING\n", clk, p[current].id);
		}

		// Execute 1ms
		if (current != -1) {
			p[current].remaining--;
			p[current].completed++;
			slice_left--;
			clk++;

			// I/O check
			if (p[current].io_req > 0 &&
				!p[current].io_done &&
				p[current].io_start == -1 &&
				p[current].completed == p[current].io_req) {

				p[current].io_start = clk;
				strcpy(p[current].state, "WAITING");
				printf("[T=%d] P%d: RUNNING -> WAITING (I/O for %dms, until T=%d)\n",
					clk, p[current].id, p[current].io_burst, clk + p[current].io_burst);
				current = -1;
				disp_left = disp;
				continue;
			}

			// Completed check
			if (p[current].remaining == 0) {
				p[current].finish = clk;
				strcpy(p[current].state, "COMPLETED");
				p[current].tat = p[current].finish - p[current].at;
				p[current].wt = p[current].tat - p[current].bt -
								(p[current].io_burst > 0 ? p[current].io_burst : 0);
				if (p[current].wt < 0) p[current].wt = 0;

				if (p[current].finish > p[current].abs_deadline) {
					p[current].missed = 1;
					int margin = p[current].finish - p[current].abs_deadline;
					printf("[T=%d] P%d: COMPLETED *** MISSED deadline %d by %dms ***\n",
						clk, p[current].id, p[current].abs_deadline, margin);
				} else {
					int margin = p[current].abs_deadline - p[current].finish;
					printf("[T=%d] P%d: COMPLETED (met deadline %d with %dms spare)\n",
						clk, p[current].id, p[current].abs_deadline, margin);
				}
				done++;
				current = -1;
				disp_left = disp;
				continue;
			}

			// Quantum expired
			if (slice_left == 0) {
				strcpy(p[current].state, "READY");
				enqueue(current);
				printf("[T=%d] P%d: Quantum expired -> READY (rem=%dms, waiting for next turn)\n",
					clk, p[current].id, p[current].remaining);
				current = -1;
				disp_left = disp;
				continue;
			}
		} else {
			clk++;
		}
	}
}

void print_results(struct process p[], int n, int disp, FILE *fp) {
	float wt = 0, tat = 0;
	float hard_wt = 0, hard_tat = 0;
	float soft_wt = 0, soft_tat = 0;
	int hard_n = 0, soft_n = 0;
	int hard_miss = 0, soft_miss = 0;
	int total_cpu = 0, max_time = 0;

	printf("\n=== RESULTS (Overhead=%d) ===\n", disp);

	for (int i = 0; i < n; i++) {
		wt += p[i].wt;
		tat += p[i].tat;
		total_cpu += p[i].bt;
		if (p[i].finish > max_time) max_time = p[i].finish;

		if (p[i].type == 0) {
			hard_wt += p[i].wt;
			hard_tat += p[i].tat;
			hard_n++;
			if (p[i].missed) hard_miss++;
		} else if (p[i].type == 1) {
			soft_wt += p[i].wt;
			soft_tat += p[i].tat;
			soft_n++;
			if (p[i].missed) soft_miss++;
		}
	}

	float cpu_util = max_time > 0 ? (float)total_cpu / max_time * 100 : 0;

	printf("avg wt=%.2f tat=%.2f cpu=%.2f%%\n", wt/n, tat/n, cpu_util);
	if (hard_n > 0)
		printf("HARD: wt=%.2f tat=%.2f miss=%d/%d\n",
			hard_wt/hard_n, hard_tat/hard_n, hard_miss, hard_n);
	if (soft_n > 0)
		printf("SOFT: wt=%.2f tat=%.2f miss=%d/%d\n",
			soft_wt/soft_n, soft_tat/soft_n, soft_miss, soft_n);

	fprintf(fp, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d,%d\n",
		disp, wt/n, tat/n, cpu_util,
		hard_n > 0 ? hard_wt/hard_n : 0,
		hard_n > 0 ? hard_tat/hard_n : 0,
		soft_n > 0 ? soft_wt/soft_n : 0,
		soft_n > 0 ? soft_tat/soft_n : 0,
		hard_miss, soft_miss);
}
