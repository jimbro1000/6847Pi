//
// Created by Julian on 05/04/2023.
//
#include "pico/stdlib.h"
#include "6847pi.h"
#include "rowBuffer.h"

const uint32_t DEFAULT_ROW_BYTES = 32;
const uint32_t SLEEP = 5000;
const uint32_t BUFFER_SLEEP = 1;

// output buffer
output_row_t row_pipe[OUTPUT_BUFFER_SIZE];
// buffer insert point
output_row_t *row_pipe_ptr;
// buffer output point
output_row_t *read_pipe_ptr;
// expected buffer row length
uint32_t current_row_size = DEFAULT_ROW_BYTES;

/**
 * Initialise output buffering.
 */
void init_row_pipe() {
    // link output buffer rows
    row_pipe[OUTPUT_BUFFER_SIZE - 1].next = &row_pipe[0];
    for (int i = OUTPUT_BUFFER_SIZE - 2; i >= 0; --i) {
        row_pipe[i].next = &row_pipe[i + 1];
    }
    // set insert and output pointers
    row_pipe_ptr = &row_pipe[0];
    read_pipe_ptr = &row_pipe[0];
    // set screen line byte length
    for (int i = 0; i < OUTPUT_BUFFER_SIZE; ++i) {
        row_pipe_ptr->row_size = current_row_size;
        for (int j = 0; j < current_row_size; ++j) {
            row_pipe_ptr->row[j] = 0;
        }
        row_pipe_ptr = (output_row_t *) row_pipe_ptr->next;
    }
}

/**
 * Put formatted pixel output row to output buffer.
 *
 * accepts input only if space exists in buffer
 *
 * @param new_row output row
 * @return success
 */
bool push_to_output_buffer(output_row_t *new_row) {
    bool result = false;
    if (row_pipe_ptr->next != read_pipe_ptr) {
        row_pipe_ptr->row_size = new_row->row_size;
        for (int j=0; j<new_row->row_size; ++j) {
            row_pipe_ptr->row[j] = new_row->row[j];
        }
        result = true;
        row_pipe_ptr = (output_row_t *) row_pipe_ptr->next;
    }
    return result;
}

/**
 * Checked push of row data to output buffer.
 *
 * Blocks until row can be accepted
 *
 * @param row output row data
 */
void safe_push_row(output_row_t *row) {
    bool accepted = false;
    do {
        accepted = push_to_output_buffer(row);
        if (!accepted) {
            sleep_ms(BUFFER_SLEEP);
        }
    } while (!accepted);
}

output_row_t pop_from_output_buffer() {
    output_row_t result;
    result.row_size = read_pipe_ptr->row_size;
    for (int j=0; j<read_pipe_ptr->row_size; ++j) {
        result.row[j] = read_pipe_ptr->row[j];
    }
    return result;
}
