//
// Created by Julian on 05/04/2023.
//

#ifndef INC_6847PI_ROWBUFFER_H
#define INC_6847PI_ROWBUFFER_H

    extern const uint32_t DEFAULT_ROW_BYTES;
    extern const uint32_t SLEEP;
    extern const uint32_t BUFFER_SLEEP;

    void init_row_pipe();
    bool push_to_output_buffer(output_row_t *new_row);
    void safe_push_row(output_row_t *row);
    output_row_t pop_from_output_buffer();

#endif //INC_6847PI_ROWBUFFER_H
