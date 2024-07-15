// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: MIT

#ifndef __ESP_ACCELERATOR_ASI_HPP__
#define __ESP_ACCELERATOR_ASI_HPP__

#include "esp_accelerator.hpp"

#include "utils/esp_handshake.hpp"
#include "utils/configs/esp_config.hpp"
#include "utils/configs/esp_config_proc.hpp"



#define INPUT_ASI 1
#define OUTPUT_ASI 2

#define INPUT_ASI_UNIT 0
#define OUTPUT_ASI_UNIT 2
#define LOAD_UNIT 1
#define STORE_UNIT 3
#define NO_UNIT 4

#define POLL_PROD_VALID_REQ 1
#define LOAD_DATA_REQ 2
#define POLL_CONS_READY_REQ 3
#define LOAD_DONE 5

#define UPDATE_CONS_VALID_REQ 6
#define UPDATE_PROD_VALID_REQ 7
#define RESET_PROD_VALID_REQ UPDATE_PROD_VALID_REQ
#define UPDATE_CONS_READY_REQ 8
#define UPDATE_PROD_READY_REQ 9
#define STORE_DATA_REQ 10
#define STORE_DONE 11
#define STORE_FENCE 12

//#define DMA_WORD_PER_BEAT 2
//
#if (DMA_WIDTH == 32)
#if (DATA_WIDTH == 8 || WORD_SIZE == 8)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 4
#elif (DATA_WIDTH == 16 || WORD_SIZE == 16)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 2
#elif (DATA_WIDTH == 32 || WORD_SIZE == 32)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 1
#elif (DATA_WIDTH == 64 || WORD_SIZE == 64)
#define DMA_BEAT_PER_WORD 2
#define DMA_WORD_PER_BEAT 0
#endif

#elif (DMA_WIDTH == 64)

#if (DATA_WIDTH == 8 || WORD_SIZE == 8)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 8
#elif (DATA_WIDTH == 16 || WORD_SIZE == 16)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 4
#elif (DATA_WIDTH == 32 || WORD_SIZE == 32)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 2
#elif (DATA_WIDTH == 64 || WORD_SIZE == 64)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 1
#endif
#endif


#define POLL_DONE 13
#define UPDATE_DONE 14

#define LOAD_CONFIG 15
#define UPDATE_CONFIG 16

#define COMPUTE 4

template <
    size_t _DMA_WIDTH_
    >
class esp_accelerator_asi : public esp_accelerator<_DMA_WIDTH_>
{
    public:

        // Input <-> Computation
        handshake_t input_ready;

        // Computation <-> Output
        handshake_t output_ready;

        handshake_t start_load;
        handshake_t end_load;
        handshake_t start_store;
        handshake_t end_store;

        // Constructor
        SC_HAS_PROCESS(esp_accelerator_asi);
        esp_accelerator_asi(const sc_module_name &name)
            : esp_accelerator<_DMA_WIDTH_>(name)
            , input_ready("input_ready")
            , output_ready("output_ready")
            , start_load("start_load")
            , end_load("end_load")
            , start_store("start_store")
            , end_store("end_store")
	        , cfg("config")
        {
            SC_CTHREAD(load_input, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);

            SC_CTHREAD(compute_kernel, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);

            SC_CTHREAD(store_output, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);


            SC_CTHREAD(asi_shim, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);


            SC_CTHREAD(input_asi_controller, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);


            SC_CTHREAD(output_asi_controller, this->clk.pos());
            this->reset_signal_is(this->rst, false);
            // set_stack_size(0x400000);


            HLS_PRESERVE_SIGNAL(dma_read_arbiter);
            HLS_PRESERVE_SIGNAL(dma_write_arbiter);
            HLS_PRESERVE_SIGNAL(dma_read_arbiter_prev);
            HLS_PRESERVE_SIGNAL(dma_write_arbiter_prev);
            HLS_PRESERVE_SIGNAL(load_asi_req_dma_read);
            HLS_PRESERVE_SIGNAL(load_asi_req_dma_write);
            HLS_PRESERVE_SIGNAL(store_asi_req_dma_read);
            HLS_PRESERVE_SIGNAL(store_asi_req_dma_write);
            HLS_PRESERVE_SIGNAL(load_req_dma_read);
            HLS_PRESERVE_SIGNAL(store_req_dma_write);
            HLS_PRESERVE_SIGNAL(input_asi_state_dbg);
            HLS_PRESERVE_SIGNAL(output_asi_state_dbg);
            HLS_PRESERVE_SIGNAL(last);
            HLS_PRESERVE_SIGNAL(last_iter);

            input_ready.bind_with(*this);
            output_ready.bind_with(*this);
            start_load.bind_with(*this);
            end_load.bind_with(*this);
            start_store.bind_with(*this);
            end_store.bind_with(*this);
            cfg.bind_with(*this);
        }

    // Configure gemm
    esp_config_proc cfg;

        // Processes

        // Load the input data
        virtual void load_input() = 0;

        // Realize the computation
        virtual void compute_kernel() = 0;

        // Store the output data
        virtual void store_output() = 0;

        // asi process
        void asi_shim();
        // void asi_shim();
        void input_asi_controller();
        void output_asi_controller();

        // Internal signals
        sc_signal< sc_int<8> > dma_read_arbiter; // 0: Load ASI, 1: Store ASI, 2: Load
        sc_signal< sc_int<8> > dma_write_arbiter;// 0: Load ASI, 1: Store ASI, 2: Store
        sc_signal< sc_int<8> > dma_read_arbiter_prev; // 0: Load ASI, 1: Store ASI, 2: Load
        sc_signal< sc_int<8> > dma_write_arbiter_prev;// 0: Load ASI, 1: Store ASI, 2: Store

        sc_signal< sc_int<1> > load_asi_req_dma_read;
        sc_signal< sc_int<1> > load_asi_req_dma_write;
        sc_signal< sc_int<1> > store_asi_req_dma_read;
        sc_signal< sc_int<1> > store_asi_req_dma_write;
        sc_signal< sc_int<1> > load_req_dma_read;
        sc_signal< sc_int<1> > store_req_dma_write;
        sc_signal< sc_int<1> > asi_dma_read_busy;
        sc_signal< sc_int<1> > asi_dma_write_busy;
        sc_signal< sc_int<1> > last;
        sc_signal< sc_int<64> > last_iter;// 0: Load ASI, 1: Store ASI, 2: Store

        // Debug Signals
        sc_signal< sc_int<32> > input_asi_state_dbg;
        sc_signal< sc_int<32> > output_asi_state_dbg;

        // Functions

        // Reset callable by load_input
        inline void reset_load_input();

        // Reset callable by compute_kernel
        inline void reset_compute_kernel();

        // Reset callable by store_output
        inline void reset_store_output();

        inline void reset_load_asi();
        inline void reset_store_asi();
        inline void fence();
        inline void poll_flag(int sync_offset, sc_int<DMA_WIDTH> &var);
        inline void update_flag(int sync_offset, int8_t var);


        // Handshake callable by load_input
        inline void load_compute_handshake();

        // Handshake callable by compute_kernel
        inline void compute_load_handshake();

        // Handshake callable by compute_kernel
        inline void compute_store_handshake();

        // Handshake callable by store_output
        inline void store_compute_handshake();

        inline void start_load_asi_handshake();
        inline void end_load_asi_handshake();
        inline void start_store_asi_handshake();
        inline void end_store_asi_handshake();
};

// Implementation
#include "esp_accelerator_asi.i.hpp"

#endif // __ESP_ACCELERATOR_3P_HPP__
