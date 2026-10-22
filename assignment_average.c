#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/*
  @brief to count the row and colums for input data frame
  @param fp file pointer
  @param rows pointer to store the number of rows
  @param cols pointer to store the number of columns
  @return 0
*/
int count_row_col(FILE *fp, int *rows, int *cols) {
  // always rewind: move the file pointer to the beginning of the file
  rewind(fp);

  *rows = 0;
  *cols = 0;

  char buffer[1024];

  // 1. read the first line to get the number of columns

  // important to accomadate different type of delimeters to pass the test:
  // space, tab, and carriage return
  if (fgets(buffer, sizeof(buffer), fp) != NULL) {
    char *item = strtok(buffer, " \t\r\n");
    while (item != NULL) {
      (*cols)++;
      item = strtok(NULL, " \t\r\n");
    }
  }

  // 2. read the file from the beginning and count the number of rows
  rewind(fp);
  while (fgets(buffer, sizeof(buffer), fp) != NULL) {
    char *item = strtok(buffer, " \t\r\n");
    if (item != NULL) {
      (*rows)++;
    }
  }

  rewind(fp);
  return 0;
}

/*
  @brief to read the data from the file and store it in a 2d array
  @param fp file pointer
  @param data pointer to the 2d array
  @param rows number of rows
  @param cols number of columns
  @return 0
*/
int read_to_2d_array(FILE *fp, int **data, int rows, int cols) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      fscanf(fp, "%d", &data[i][j]);
    }
  }
  return 0;
}

/*
  @brief TA for each assignment
  @param data 2d array
  @param rows number of rows
  @param col_idx column index
*/
void run_ta(int **data, int rows, int col_idx) {
  // assignment index is 1 based
  int assignment_num = col_idx + 1;
  double sum = 0.0;

  // calculate sum of the assignment scores
  for (int r = 0; r < rows; r++) {
    sum += data[r][col_idx];
  }

  // calculate average
  double avg = sum / rows;

  // print the average
  printf("Assignment %d - Average = %f\n", assignment_num, avg);

  exit(0);
}

/*
  @brief GTA for each chapter, fork one TA process for each assignment, run the
  TA process, and wait for it to finish
  @param data 2d array
  @param rows number of rows
  @param chapter chapter number
*/
void run_gta(int **data, int rows, int cols, int chapter) {
  for (int i = 0; i < 2; i++) {
    int col_idx = chapter * 2 + i;
    // important: check if the col_idx is valid
    // if not valid break out of the loop
    if (col_idx >= cols) {
      break;
    }
    pid_t pid = fork();

    if (pid < 0) { // pid < 0 means fork failed
      perror("Failed to fork TA");
      exit(1);
    } else if (pid == 0) {         // pid == 0 means child process
      run_ta(data, rows, col_idx); //
    }

    wait(NULL); // wait for the child process to finish
  }

  exit(0); // exit GTA
}

/**
@brief free the memory

@param data 2d array
@param rows number of rows
*/
void free_memory(int **data, int rows) {
  for (int i = 0; i < rows; i++) {
    free(data[i]);
  }
  free(data);
}

int main(int argc, char *argv[]) {
  // Prevent unused parameter warnings with -Werror
  (void)argc;
  (void)argv;

  // check the number of arguments
  // if less than 2, print correct usage and exit
  if (argc < 2) {
    printf("Usage: ./assignment_average [sample_in_grades.txt]\n");
    return 1;
  }
  // step 1. read the file path
  FILE *fp = fopen(argv[1], "r");

  // if failed to access the file, print error message and return 0
  if (fp == NULL) {
    printf("Failed to access %s\n", argv[1]);
    return 0;
  }

  // step 2. read the data and store into a two dimensional data frame
  int rows = 0;
  int cols = 0;
  count_row_col(fp, &rows, &cols);

  int **data = (int **)malloc(rows * sizeof(int *));
  for (int i = 0; i < rows; i++) {
    data[i] = (int *)malloc(cols * sizeof(int));
  }

  //   printf("DEBUG: rows = %d, cols = %d\n", rows, cols);

  read_to_2d_array(fp, data, rows, cols);
  fclose(fp);

  // step 4. create processes
  // important: do ceiling, so when cols = 3, num_chapters = 2 not 1.
  int num_chapters = (cols + 1) / 2;

  for (int c = 0; c < num_chapters; c++) {
    pid_t pid = fork();
    if (pid < 0) {
      perror("Failed to fork GTA");
      exit(1);
    } else if (pid == 0) {
      run_gta(data, rows, cols, c);
    }

    wait(NULL); // Teacher will wait this GTA finish
  }

  // free the memory
  free_memory(data, rows);

  return 0;
}
