// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: MIT

inline dma_info_t& dma_info_t::operator=(const dma_info_t &other)
{
    index = other.index;
    length = other.length;
    size = other.size;
    opts = other.opts;
    return *this;
}

inline bool dma_info_t::operator==(const dma_info_t &rhs) const
{
    return ((rhs.index == index)
            && (rhs.length == length)
            && (rhs.size == size)
            && (rhs.opts == opts));
}

inline ostream& operator<<(ostream& os, dma_info_t const &dma_info)
{
    os << "{" << dma_info.index  << ","
              << dma_info.length  << ","
              << dma_info.size << ","
              << "{" << dma_info.opts.dcs_en << "," 
                     << dma_info.opts.use_owner_pred << ","
                     << dma_info.opts.dcs << ","
                     << dma_info.opts.pred_cid << "}"
              << "}";
    return os;
}

inline void sc_trace(sc_trace_file *tf, const dma_info_t &v, const std::string &name)
{
    #define xstr(a) #a
    #define add_trace(sfx) \
        sstm_c << name << xstr(sfx); \
        sc_trace(tf, v sfx, sstm_c.str());

    std::stringstream sstm_c;
    add_trace(.index);
    add_trace(.length);
    add_trace(.size);
    add_trace(.opts.dcs_en);
    add_trace(.opts.use_owner_pred);
    add_trace(.opts.dcs);
    add_trace(.opts.pred_cid);

    #undef xstr
    #undef add_trace
}

