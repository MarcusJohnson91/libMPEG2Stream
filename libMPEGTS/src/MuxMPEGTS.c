#include "../include/libMPEGTS.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    enum PacketTypes {
        TrueHD = 0,
        DTSXLL = 1,
        AVC    = 2,
    };
    
    typedef struct Packet2Mux {
        uint8_t  PacketType;
        bool     IsSubstream;
        uint64_t DisplayTime;
        size_t   PacketSize;
        uint8_t *PacketData;
    } Packet2Mux;
    
    void MuxMPEGTransportStream(BitInput *BitO, Packet2Mux *Packet) { // Main, user facing function
        
    }
    
    void SpliceMPEGTransportStream(BitInput *BitO, Packet2Mux *Packet) {
        
    }
    
    
    
#ifdef __cplusplus
}
#endif
