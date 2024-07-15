// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: MIT

// Functions

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::reset_load_input()
{
    input_ready.req.reset_req();
    // this->reset_dma_read();
    start_load.ack.reset_ack();
    end_load.req.reset_req(); 
    load_req_dma_read.write(0);
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::reset_load_asi()
{
    this->reset_dma_read();
    start_load.req.reset_req();
    end_load.ack.reset_ack(); 
    load_asi_req_dma_read.write(0);
    load_asi_req_dma_write.write(0);
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::reset_compute_kernel()
{
    input_ready.ack.reset_ack();
    output_ready.req.reset_req();
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::reset_store_output()
{
    output_ready.ack.reset_ack();
    this->reset_accelerator_done();
    // this->reset_dma_write();
    start_store.ack.reset_ack();
    end_store.req.reset_req(); 
    store_req_dma_write.write(0);
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::reset_store_asi()
{
    this->reset_dma_write();
    start_store.req.reset_req();
    end_store.ack.reset_ack();
    store_asi_req_dma_read.write(0);
    store_asi_req_dma_write.write(0);
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::load_compute_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-compute-handshake");

        input_ready.req.req();
        // end_store.req.req(); //remove if internally tiled
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::compute_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("compute-load-handshake");

        input_ready.ack.ack();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::compute_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("compute-store-handshake");

        output_ready.req.req();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::store_compute_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-compute-handshake");

        output_ready.ack.ack();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::start_store_asi_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("start-store-handshake");

        start_store.ack.ack();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::end_store_asi_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("end-store-handshake");

        end_store.req.req();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::start_load_asi_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("start-load-handshake");

        start_load.ack.ack();
    }
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::end_load_asi_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("end-load-handshake");

        end_load.req.req();
    }
}

template <
    size_t _DMA_WIDTH_
    >
void esp_accelerator_asi<_DMA_WIDTH_>::asi_shim()
{
    {
        HLS_DEFINE_PROTOCOL("asi-shim-reset");
        dma_read_arbiter.write(NO_UNIT);
         dma_read_arbiter_prev.write(NO_UNIT);
        dma_write_arbiter.write(NO_UNIT);
        asi_dma_read_busy.write(0);
        asi_dma_write_busy.write(0);
        wait();
    }
    int _dma_read_arbiter, _dma_write_arbiter;
    int _dma_read_arbiter_prev;
    sc_int<1> _load_asi_req_dma_read, _load_asi_req_dma_write, 
    _store_asi_req_dma_read, _store_asi_req_dma_write,
    _load_req_dma_read, _store_req_dma_write, _dma_read_busy, _dma_write_busy;
    {
        HLS_DEFINE_PROTOCOL("asi-shim-config");
        cfg.wait_for_config(); // config process
    }
    while(true){
        {
            HLS_DEFINE_PROTOCOL("shim-read-signals");
            wait();
            _load_asi_req_dma_read = load_asi_req_dma_read.read();
            _store_asi_req_dma_read = store_asi_req_dma_read.read();
            _load_req_dma_read = load_req_dma_read.read();
            _dma_read_arbiter_prev = dma_read_arbiter_prev.read();
            _dma_read_arbiter = dma_read_arbiter.read();
            _dma_read_busy = asi_dma_read_busy.read();
            wait();

        switch(_dma_read_arbiter){
            case INPUT_ASI_UNIT: // Load ASI
            {
                if(_dma_read_busy&&_load_asi_req_dma_read){
                    break;
                }else{
                    if(_load_req_dma_read)
                        dma_read_arbiter.write(LOAD_UNIT); // load unit
                    else if(_store_asi_req_dma_read){
                        dma_read_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                        asi_dma_read_busy.write(1);
                    }
                    else if(_load_asi_req_dma_read){
                        dma_read_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                        asi_dma_read_busy.write(1);
                    }
                    else {
                        dma_read_arbiter.write(NO_UNIT); // No unit
                    }
                    break;
                }
            }
            case LOAD_UNIT: // Load Unit
            {
                if(_load_req_dma_read){ // load unit
                    break;
                }else{
                    if(_store_asi_req_dma_read){
                        dma_read_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                        asi_dma_read_busy.write(1);
                    }
                    else if(_load_asi_req_dma_read){
                        dma_read_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                        asi_dma_read_busy.write(1);
                    }
                    else dma_read_arbiter.write(NO_UNIT); // No unit
                    break;
                }
            }
            case OUTPUT_ASI_UNIT: //Store ASI
            {
                if(_dma_read_busy&&_store_asi_req_dma_read){
                    break;
                }else{
                    if(_load_asi_req_dma_read){
                        dma_read_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                        asi_dma_read_busy.write(1);
                    }
                    else if(_load_req_dma_read){
                        dma_read_arbiter.write(LOAD_UNIT); // load unit
                    }
                    else if(_store_asi_req_dma_read){
                        dma_read_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                        asi_dma_read_busy.write(1);
                    } else{
                        dma_read_arbiter.write(NO_UNIT); // No unit
                    }
                    break;
                }
            }
            case NO_UNIT: //Default
            default:
            {
                if(_dma_read_arbiter_prev==OUTPUT_ASI_UNIT && _load_asi_req_dma_read){
                    dma_read_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                    asi_dma_read_busy.write(1);
                }
                else if(_dma_read_arbiter_prev==INPUT_ASI_UNIT && _store_asi_req_dma_read){
                    dma_read_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                    asi_dma_read_busy.write(1);
                } 
                else if(_load_req_dma_read){
                    dma_read_arbiter.write(LOAD_UNIT); // load unit
                }
                else if(_store_asi_req_dma_read){
                    dma_read_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                    asi_dma_read_busy.write(1);
                } else if(_load_asi_req_dma_read){
                    dma_read_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                    asi_dma_read_busy.write(1);
                }
                else{
                    dma_read_arbiter.write(NO_UNIT); // Output ASI unit
                }
                break;
            }
        }
        wait();
    }
    {
        HLS_DEFINE_PROTOCOL("shim-write-signals");
        _dma_write_arbiter = dma_write_arbiter.read();
        _store_req_dma_write = store_req_dma_write.read();
        _load_asi_req_dma_write = load_asi_req_dma_write.read();
        _store_asi_req_dma_write = store_asi_req_dma_write.read();
        _dma_write_busy = asi_dma_write_busy.read();

        wait();
        switch(_dma_write_arbiter){

            case INPUT_ASI_UNIT:
            {
                if(_dma_write_busy&&_load_asi_req_dma_write){
                    break;
                }else{
                    if(_store_req_dma_write)
                        dma_write_arbiter.write(STORE_UNIT); // Store unit
                    else if(_store_asi_req_dma_write){
                        dma_write_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                        asi_dma_write_busy.write(1);
                    }
                    else if(_load_asi_req_dma_write){
                        dma_write_arbiter.write(INPUT_ASI_UNIT); // Input ASI unit
                        asi_dma_write_busy.write(1);
                    } else dma_write_arbiter.write(NO_UNIT); // No unit
                    break;
                }
            }
            case STORE_UNIT: 
            {
                if(_store_req_dma_write){// Store unit
                    break;
                }else{
                    if(_store_asi_req_dma_write){
                        dma_write_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI
                        asi_dma_write_busy.write(1);
                    }
                    else if(_load_asi_req_dma_write){
                        dma_write_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                        asi_dma_write_busy.write(1);
                    }
                    else dma_write_arbiter.write(NO_UNIT); // No unit
                    break;
                }
            }
            case OUTPUT_ASI_UNIT: 
            {
                if(_dma_write_busy&&_store_asi_req_dma_write){
                    break;
                }else{
                    if(_load_asi_req_dma_write){
                        dma_write_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                        asi_dma_write_busy.write(1);
                    }
                    else if(_store_req_dma_write)
                        dma_write_arbiter.write(STORE_UNIT); // Store unit
                    else if(_store_asi_req_dma_write){
                        dma_write_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI unit
                        asi_dma_write_busy.write(1);
                    } else dma_write_arbiter.write(NO_UNIT); // Nounit
                    break;
                }
            }
            case NO_UNIT: //Default
            default:
            {
		if(_load_asi_req_dma_write){
                    dma_write_arbiter.write(INPUT_ASI_UNIT); // Input ASI
                    asi_dma_write_busy.write(1);
                }
                else if(_store_req_dma_write)
                    dma_write_arbiter.write(STORE_UNIT); // Store unit
                else if(_store_asi_req_dma_write){
                    dma_write_arbiter.write(OUTPUT_ASI_UNIT); // Output ASI unit
                    asi_dma_write_busy.write(1);
                } else dma_write_arbiter.write(NO_UNIT); // NO unit
                break;
            }
        }
        wait();

        }
    }

}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::poll_flag(int sync_offset, sc_int<DMA_WIDTH> &var)
{
	{
		HLS_DEFINE_PROTOCOL("poll_flag");
        	wait();
		dma_info_t dma_info(sync_offset/ DMA_WORD_PER_BEAT, 1, SIZE_DWORD);//round_up(sync_len, WORDS_PER_DMA) >> WORDS_PER_DMA_LOG
		this->dma_read_ctrl.put(dma_info);
		wait();
        	sc_dt::sc_bv<DMA_WIDTH> dataBvin;
		dataBvin.range(DMA_WIDTH - 1, 0) = this->dma_read_chnl.get();
		wait();
		var = dataBvin.range(31, 0).to_uint();
		wait();
        	wait();
	}
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::update_flag(int32_t sync_offset, int8_t sync_flag)
{
	{
		HLS_DEFINE_PROTOCOL("update_flag");
//to resolve mesi hangs
       	//	enforce w-w ordering of flag
       	//	commented jul13
		//while (!(this->dma_write_chnl.ready)) wait();
		//wait();

		dma_info_t dma_info(sync_offset/ DMA_WORD_PER_BEAT , 1, SIZE_DWORD); //round_up(sync_len, WORDS_PER_DMA) >> WORDS_PER_DMA_LOG
		this->dma_write_ctrl.put(dma_info);
		wait();
        	sc_dt::sc_bv<DMA_WIDTH> dataBv;
        	dataBv.range(7, 0) = sync_flag;
        	wait();
        	this->dma_write_chnl.put(dataBv);
        	wait();
		
       		////enforce w-w ordering of flag
		//while (!(this->dma_write_chnl.ready)) wait();
		//wait();

		////FENCE
		//this->acc_fence.put(0x2);
		//wait();
		//while (!(this->acc_fence.ready)) wait();
		//wait();
	}
}

template <
    size_t _DMA_WIDTH_
    >
inline void esp_accelerator_asi<_DMA_WIDTH_>::fence()
{
    {
        HLS_DEFINE_PROTOCOL("fence");
	//enforce w-w ordering of flag
		while (!(this->dma_write_chnl.ready)) wait();
		wait();

		//FENCE
		this->acc_fence.put(0x2);
		wait();
		while (!(this->acc_fence.ready)) wait();
		wait();
    }
}

template <
    size_t _DMA_WIDTH_
    >
void esp_accelerator_asi<_DMA_WIDTH_>::input_asi_controller(){
    int64_t iter;
    bool last_task;
    {
        HLS_DEFINE_PROTOCOL("input-asi_controller-reset");
        this->reset_load_asi();
        input_asi_state_dbg.write(0);
        iter = 0;
        last_task = 0;
        last.write(0);
        wait();
    }
    conf_info_t config;
    int prod_ready_offset;
    int prod_valid_offset;

    sc_int<DMA_WIDTH> prod_valid;
    //int32_t num_tiles;
    {
        HLS_DEFINE_PROTOCOL("input-asi_controller-config");
        cfg.wait_for_config(); // config process
        config = this->conf_info.read();

        prod_ready_offset = config.prod_ready_offset;
        prod_valid_offset = config.prod_valid_offset;
        
        prod_valid = 0;
    
    }

    //while(true){ 
    while(!last_task){ 
        // Test producer's valid for new task
        {
            HLS_DEFINE_PROTOCOL("test-prod-valid");
            while(!prod_valid){
			    input_asi_state_dbg.write(0x11223344);
                wait();
                // printf("Inp ASI Ctrl set read_req_dma\n");
                load_asi_req_dma_read.write(1);
                wait();
                while(dma_read_arbiter.read() !=INPUT_ASI_UNIT ) wait();
                input_asi_state_dbg.write(POLL_PROD_VALID_REQ);
                poll_flag(prod_valid_offset, prod_valid);
                wait();
		// releasing resource while polling to avoid deadlocks
                dma_read_arbiter_prev.write(INPUT_ASI_UNIT);
                dma_read_arbiter.write(NO_UNIT);
                load_asi_req_dma_read.write(0); // relinquish dma read
                wait();
                asi_dma_read_busy.write(0);
                wait();
            }
                
            start_load.req.req();
            wait();

            //check last
            if(prod_valid ==2){
                last_task = 1;
                last.write(1);
                wait();
                last_iter.write(iter);
                wait();
            }

            //Reset the prod valid flag
	    input_asi_state_dbg.write(0x22334455);
            load_asi_req_dma_write.write(1);
            while(dma_write_arbiter.read() != INPUT_ASI_UNIT ) wait();
	    input_asi_state_dbg.write(RESET_PROD_VALID_REQ);
            update_flag(prod_valid_offset, 0);
	    fence();
            wait();
            load_asi_req_dma_write.write(0);
            wait();
            asi_dma_write_busy.write(0);
            wait();
            prod_valid = 0;
        //}

        //{
        //    HLS_DEFINE_PROTOCOL("set-prod-ready");
	    input_asi_state_dbg.write(0x33445566);
            wait();
            end_load.ack.ack();
            
            wait();
	    input_asi_state_dbg.write(last_task);
            //if(last_task) {
            //HLS_DEFINE_PROTOCOL("set-last");
	    //    input_asi_state_dbg.write(0xdeadbeef);
            //    wait();
            //    break; 
            //}
            //
            // Do not set producer ready if already read last task
	    if(!last_task){
            	wait();
	    	input_asi_state_dbg.write(0x44556677);

            	load_asi_req_dma_write.write(1);
            	while(dma_write_arbiter.read() != INPUT_ASI_UNIT) wait();
	    	input_asi_state_dbg.write(UPDATE_PROD_READY_REQ);
            	update_flag(prod_ready_offset, 1);
		fence();
            	wait();

            	//enforce w-w ordering of flag
            	load_asi_req_dma_write.write(0);
            	wait();
            	asi_dma_write_busy.write(0);
            	wait();
	    }
        }
        iter++;
    }
    //End process
    {
        HLS_DEFINE_PROTOCOL("input-asi_done");
	input_asi_state_dbg.write(0xdeadbeef);
	wait();
        this->process_done();
    }
}

template <
    size_t _DMA_WIDTH_
    >
void esp_accelerator_asi<_DMA_WIDTH_>::output_asi_controller(){
	//bool new_task;
    int64_t iter, st_last_iter;
    bool last_task, exit_clause;
    {
        HLS_DEFINE_PROTOCOL("output-asi_controller-reset");
        this->reset_store_asi();
        output_asi_state_dbg.write(0);
        iter = 0;
        st_last_iter = 0;
        exit_clause = 0;
        last_task = 0;
        wait();
    }
    conf_info_t config;
    int cons_ready_offset;
    int cons_valid_offset;

    sc_int<DMA_WIDTH> cons_ready;
    //int32_t num_tiles;
    {
        HLS_DEFINE_PROTOCOL("output-asi_controller-config");
        cfg.wait_for_config(); // config process
        config = this->conf_info.read();

        cons_ready_offset = config.cons_ready_offset;
        cons_valid_offset = config.cons_valid_offset;
        
        cons_ready = 0;
    }

    while(!exit_clause){ 
        // Test producer's valid for new task
        {
            HLS_DEFINE_PROTOCOL("test-cons-ready");

            while(!cons_ready){
			    output_asi_state_dbg.write(0x11223344);
                wait();
                store_asi_req_dma_read.write(1);
                wait();
                while(dma_read_arbiter.read() != OUTPUT_ASI_UNIT ) wait();

                output_asi_state_dbg.write(POLL_CONS_READY_REQ);
                poll_flag(cons_ready_offset, cons_ready);
                dma_read_arbiter_prev.write(OUTPUT_ASI_UNIT);
                dma_read_arbiter.write(NO_UNIT);
                store_asi_req_dma_read.write(0); //relinquish dma write
                wait();
                asi_dma_read_busy.write(0);
                wait();
            }
            start_store.req.req();
            wait();
        //}

        //{
        //    HLS_DEFINE_PROTOCOL("set-cons-valid");
	    output_asi_state_dbg.write(0x33445566);
            wait();
            end_store.ack.ack();
            wait();
	    output_asi_state_dbg.write(0x44556677);

            //Reset the prod valid flag
            store_asi_req_dma_write.write(1);
            wait();

            while(dma_write_arbiter.read() != OUTPUT_ASI_UNIT ) wait();

	    output_asi_state_dbg.write(UPDATE_CONS_READY_REQ);
            cons_ready = 0;
	    //fence();
            update_flag(cons_ready_offset, 0);
	    fence();
            //store_asi_req_dma_write.write(0);

            //wait();

            //store_asi_req_dma_write.write(1);
            //wait();
            //while(dma_write_arbiter.read() != OUTPUT_ASI_UNIT ) wait();

            //check last_task
            last_task = last.read();
            wait();
            if(last_task==1){ 
                st_last_iter = last_iter.read();
            	wait();
                exit_clause = last_task&&(st_last_iter == iter)?1:0;
            }
            
            wait();

            int flag_val = (exit_clause)? 2 : 1; // 2 indicates "last"
            update_flag(cons_valid_offset, flag_val);
 	    fence();
            wait();
	    output_asi_state_dbg.write(UPDATE_CONS_VALID_REQ);
            wait();
            store_asi_req_dma_write.write(0);
            wait();
            asi_dma_write_busy.write(0);

            //if(exit_clause) {
            //    output_asi_state_dbg.write(0xdeadbeef);
            //    wait();
            //	update_flag(cons_valid_offset, 2);
            //    wait();
	    //	output_asi_state_dbg.write(UPDATE_CONS_VALID_REQ);
            //	store_asi_req_dma_write.write(0);
            //	wait();
            //	asi_dma_write_busy.write(0);
            //	wait();
            //}
	    //else{
	    //	output_asi_state_dbg.write(UPDATE_CONS_VALID_REQ);
            //	update_flag(cons_valid_offset, 1);
            //	store_asi_req_dma_write.write(0);
            //	wait();
            //	asi_dma_write_busy.write(0);
            //	wait();
	    //}
        }
        iter ++;
    }
    // End accelerator
    {
        HLS_DEFINE_PROTOCOL("end-accel");
        output_asi_state_dbg.write(0xdeadc0de);
        wait();
        this->accelerator_done();
        this->process_done();
    }
}

