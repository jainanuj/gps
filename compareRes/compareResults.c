//
//  compareResults.c
//  compareResults
//
//  Created by Anuj Jain on 6/25/17.
//  Copyright © 2017 Anuj Jain. All rights reserved.
//

#include "compareResults.h"

void parse_results(char *res_file, float **file_results_ptr, int num_states)
{
    float   *f_results = *file_results_ptr;
    FILE    *fp;
    int     state_id, throwaway, i;
    float   state_val;

    fp = fopen( res_file, "rb" );
    f_results = (float *)malloc(sizeof(float) * num_states);
    for (i = 0; i < num_states; i++)
    {
        fscanf(fp, "%d", &state_id);
        fscanf(fp, "%d", &throwaway);
        fscanf(fp, "%f", &state_val);
        
        f_results[state_id] = state_val;
    }
    *file_results_ptr = f_results;
    return;
}

void compare_results(float *file1_results,float *file2_results, int num_states)
{
    int i, max_diff_state;
    float *diff = (float *)malloc(sizeof(float) * num_states);
    float max_diff = 0;
    float mean_diff = 0;
    float sum_diff = 0;
    
    for (i = 0; i< num_states; i++)
    {
        if (file1_results[i] > file2_results[i])
            diff[i] = file1_results[i] - file2_results[i];
        else
            diff[i] = file2_results[i] - file1_results[i];
        
        if (diff[i] > max_diff)
	{
            max_diff = diff[i];
	    max_diff_state = i;
	}
        sum_diff += diff[i];
    }
    mean_diff = sum_diff/num_states;
    
    printf("Max Diff = %f; Max_Diff_state = %d; Mean Diff = %f; \n",max_diff, max_diff_state, mean_diff);
    return;
}

int main( int argc, char *argv[] )
{
    char    *opt;
    char    *file1 = NULL, *file2 = NULL;
    float   *file1_results = NULL;
    float   *file2_results = NULL;
    int     num_states = 0;
 
//    sleep(10);
    if (argc > 1)
    {
        opt = argv[1];
        num_states = atoi(opt);
        if (argc > 2)
        {
            file1 = strdup(argv[2]);
            file2 = strdup(argv[3]);
        }
    }
    
    parse_results(file1, &file1_results, num_states);
    parse_results(file2, &file2_results, num_states);
    
    compare_results(file1_results, file2_results, num_states);

    return 0;
}
