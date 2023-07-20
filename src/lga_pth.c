#include "lga_base.h"
#include "lga_pth.h"
#include <pthread.h>

typedef struct
{
    byte *grid_in;
    byte *grid_out;
    int grid_size;
    int start_row;
    int end_row;
} ThreadData;

static byte get_next_cell(int i, int j, byte *grid_in, int grid_size)
{
    byte next_cell = EMPTY;

    for (int dir = 0; dir < NUM_DIRECTIONS; dir++)
    {
        int rev_dir = (dir + NUM_DIRECTIONS / 2) % NUM_DIRECTIONS;
        byte rev_dir_mask = 0x01 << rev_dir;

        int di = directions[i % 2][dir][0];
        int dj = directions[i % 2][dir][1];
        int n_i = i + di;
        int n_j = j + dj;

        if (inbounds(n_i, n_j, grid_size))
        {
            if (grid_in[ind2d(n_i, n_j)] == WALL)
            {
                next_cell |= from_wall_collision(i, j, grid_in, grid_size, dir);
            }
            else if (grid_in[ind2d(n_i, n_j)] & rev_dir_mask)
            {
                next_cell |= rev_dir_mask;
            }
        }
    }

    return check_particles_collision(next_cell);
}

static void *update_thread(void *data)
{
    ThreadData *thread_data = (ThreadData *)data;
    byte *grid_in = thread_data->grid_in;
    byte *grid_out = thread_data->grid_out;
    int grid_size = thread_data->grid_size;

    for (int i = thread_data->start_row; i < thread_data->end_row; i++)
    {
        for (int j = 0; j < grid_size; j++)
        {
            if (grid_in[ind2d(i, j)] == WALL)
                grid_out[ind2d(i, j)] = WALL;
            else
                grid_out[ind2d(i, j)] = get_next_cell(i, j, grid_in, grid_size);
        }
    }

    pthread_exit(NULL);
}

void initialize_thread_data(
    ThreadData *thread_data,
    byte *grid_in,
    byte *grid_out,
    int grid_size,
    int start_row,
    int end_row)
{
    thread_data->grid_in = grid_in;
    thread_data->grid_out = grid_out;
    thread_data->grid_size = grid_size;
    thread_data->start_row = start_row;
    thread_data->end_row = end_row;
}

static void update(byte *grid_in, byte *grid_out, int grid_size, int num_threads)
{
    pthread_t threads[num_threads];
    ThreadData thread_data[num_threads];

    int rows_per_thread = grid_size / num_threads;
    int start_row = 0, end_row = 0;

    for (int i = 0; i < num_threads; i++)
    {
        end_row = start_row + rows_per_thread;
        initialize_thread_data(&thread_data[i], grid_in, grid_out, grid_size, start_row, end_row);
        pthread_create(&threads[i], NULL, update_thread, &thread_data[i]);
        start_row = end_row;
    }

    for (int i = 0; i < num_threads; i++)
        pthread_join(threads[i], NULL);
}

void simulate_pth(byte *grid_1, byte *grid_2, int grid_size, int num_threads)
{
    for (int i = 0; i < ITERATIONS / 2; i++)
    {
        update(grid_1, grid_2, grid_size, num_threads);
        update(grid_2, grid_1, grid_size, num_threads);
    }
}
