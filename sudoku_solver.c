/* Do not make any changes in the skeleton. Your submission will be invalidated if the skeleton is changed */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>
#include <sys/sysinfo.h>

int orig_grid[36][36];
int done = 0;

pthread_mutex_t lock;

void read_grid_from_file(int size, char *ip_file, int grid[36][36]) {
	FILE *fp;
	int i, j;
	fp = fopen(ip_file, "r");
	for (i=0; i<size; i++) 
		for (j=0; j<size; j++) {
			fscanf(fp, "%d", &grid[i][j]);
	}
}

void print_grid(int size, int grid[36][36]) {
	int i, j;
	/* The segment below prints the grid in a standard format. Do not change */
	for (i=0; i<size; i++) {
		for (j=0; j<size; j++)
			printf("%d\t", grid[i][j]);
		printf("\n");
	}
}

int is_value_in_row(int grid[36][36], int size, int row, int value) {
    for (int i = 0; i < size; i++) {
        if (grid[row][i] == value) return 1;
    }
    return 0;
}

int is_value_in_column(int grid[36][36], int size, int col, int value) {
    for (int i = 0; i < size; i++) {
        if (grid[i][col] == value) return 1;
    }
    return 0;
}

int is_value_in_subgrid(int grid[36][36], int size, int row, int col,
                        int value) {
    int sub_size = (int)sqrt(size);
    int sg_top = row - row % sub_size;
    int sg_left = col - col % sub_size;
    for (int i = 0; i < sub_size; i++) {
        for (int j = 0; j < sub_size; j++) {
            if (grid[sg_top + i][sg_left + j] == value) return 1;
        }
    }
    return 0;
}

int is_valid(int grid[36][36], int size, int row, int col, int value) {
    if (is_value_in_row(grid, size, row, value)) return 0;
    if (is_value_in_column(grid, size, col, value)) return 0;
    if (is_value_in_subgrid(grid, size, row, col, value)) return 0;
    return 1;
}

typedef struct sudoku_in {
    int size;
    int start_row;
    int start_col;
    int start_value;
} sudoku_in;

int solve_sudoku(int grid[36][36], int size, int row, int col, int vcount) {
    if (vcount >= size * size) return 1;
    if (done) return 0;  // another thread solved it
    int next_col = col == size - 1 ? 0 : col + 1;
    int next_row = next_col == 0 ? (row == size - 1 ? 0 : row + 1) : row;

    if (orig_grid[row][col] != 0) {  // pre-filled
        return solve_sudoku(grid, size, next_row, next_col, ++vcount);
    }

    // try all possible values
    for (int v = 1; v <= size; v++) {
        if (is_valid(grid, size, row, col, v)) {
            grid[row][col] = v;
            int ok = solve_sudoku(grid, size, next_row, next_col, vcount + 1);
            if (ok) return 1;
        }
    }
    grid[row][col] = 0;
    return 0;
}

void *solve(void *input) {
    sudoku_in *p = input;
    int grid[36][36];
    for (int i = 0; i < p->size; i++) {
        for (int j = 0; j < p->size; j++) {
            grid[i][j] = orig_grid[i][j];
        }
    }
    if (solve_sudoku(grid, p->size, p->start_row, p->start_col, 0)) {
        pthread_mutex_lock(&lock);
        if (!done) {
            print_grid(p->size, grid);
            done = 1;
        }
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char *argv[]) {
	int grid[36][36], size, i, j;
	
	if (argc != 3) {
		printf("Usage: ./sudoku.out grid_size inputfile");
		exit(-1);
	}
	
	size = atoi(argv[1]);
	read_grid_from_file(size, argv[2], grid);

    /* Do your thing here */

    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("\n mutex init has failed\n");
        return 1;
    }
    
    // copy to orig_grid
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            orig_grid[i][j] = grid[i][j];
        }
    }

    int nthreads = get_nprocs();
    //const int nthreads = 4;
    pthread_t threads[nthreads];
    sudoku_in inputs[nthreads];
    for (int i = 0; i < nthreads; i++) {
        sudoku_in *inp = &inputs[i];
        inp->size = size;
        inp->start_row = rand() % size;
        inp->start_col = rand() % size;
        // inp->start_value = (rand() % size) + 1;
        inp->start_value = 1;
        pthread_create(&threads[i], NULL, solve, (void *)inp);
    }

    /* Wait for all threads to complete */
    for (int i = 0; i < nthreads; i++) {
        pthread_join(threads[i], NULL);
    }

    if (!done) printf("No solution\n");
    return 0;
}
