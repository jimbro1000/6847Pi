//
// Created by Julian on 05/04/2023.
//

#ifndef INC_6847PI_ROWBUFFER_H
#define INC_6847PI_ROWBUFFER_H

    const uint32_t DEFAULT_ROW_BYTES = 32;
    const uint32_t SLEEP = 5000;
    const uint32_t BUFFER_SLEEP = 1;

    void init_row_pipe();
    bool push_to_output_buffer(struct OutputRow *new_row);
    void safe_push_row(struct OutputRow *row);
    struct OutputRow pop_from_output_buffer();

#endif //INC_6847PI_ROWBUFFER_H
