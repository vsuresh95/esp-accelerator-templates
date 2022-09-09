// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: MIT

#ifndef __ESP_DMA_INFO_HPP__
#define __ESP_DMA_INFO_HPP__

#include <sstream>

#include "utils/esp_types.hpp"
#include "utils/esp_systemc.hpp"
#include "spandex_consts.hpp"

#define SIZE_BYTE   sc_dt::sc_bv<3>(0)
#define SIZE_HWORD  sc_dt::sc_bv<3>(1)
#define SIZE_WORD   sc_dt::sc_bv<3>(2)
#define SIZE_DWORD  sc_dt::sc_bv<3>(3)
#define SIZE_4WORD  sc_dt::sc_bv<3>(4)
#define SIZE_8WORD  sc_dt::sc_bv<3>(5)
#define SIZE_16WORD sc_dt::sc_bv<3>(6)
#define SIZE_32WORD sc_dt::sc_bv<3>(7)

typedef sc_uint<DCS_WIDTH>          dcs_t;
typedef sc_uint<CACHE_ID_WIDTH>     cache_id_t;

struct dma_spandex_options
{
    bool dcs_en;
    bool use_owner_pred;
    dcs_t dcs;
    cache_id_t pred_cid;

    inline bool operator==(const dma_spandex_options &rhs) const
    {
        return ((dcs_en == rhs.dcs_en)
                && (use_owner_pred == rhs.use_owner_pred)
                && (dcs == rhs.dcs)
                && (pred_cid == rhs.pred_cid));
    }
};

class dma_info_t
{

    public:

        // Index
        uint32_t index;

        // Length
        uint32_t length;

        // Length
        sc_dt::sc_bv<3> size;

        // Spandex options
        dma_spandex_options opts;

        // Constructors

        // TODO: consider making the dma_spandex_options optional in order to preserve existing behavior.
        dma_info_t()
            : index(0), length(0), size(SIZE_WORD), opts() { }

        dma_info_t(uint32_t i, uint32_t l, sc_dt::sc_bv<3> s)
            : index(i), length(l), size(s), opts() { }

        dma_info_t(uint32_t i, uint32_t l, sc_dt::sc_bv<3> s, dma_spandex_options o)
            : index(i), length(l), size(s), opts(o) { }

        dma_info_t(const dma_info_t &other)
            : index(other.index), length(other.length), size(other.size), opts(other.opts) { }

        // Operators

        // Assign operator
        inline dma_info_t &operator=(const dma_info_t &other);

        // Equals operator
        inline bool operator==(const dma_info_t &rhs) const;

        // Friend zone

        // Dump operator
        friend inline ostream &operator<<(ostream &os, dma_info_t const &dma_info);

        // Makes this type traceable by SystemC
        friend inline void sc_trace(sc_trace_file *tf, const dma_info_t &v,
                                    const std::string &name);

};

// Implementation
#include "esp_dma_info.i.hpp"

#endif // __ESP_DMA_INFO_HPP__
