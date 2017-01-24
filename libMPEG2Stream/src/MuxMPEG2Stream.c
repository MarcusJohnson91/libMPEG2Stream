#include "../include/libMPEG2Stream.h"

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
    
    void MuxMPEG2PESPacket(BitInput *BitO, Packet2Mux *Packet) {
        
    }
    
    void MuxMPEG2ProgramStream(BitInput *BitO, Packet2Mux *Packet) {
        
    }
    
    void MuxMPEG2TransportStream(BitInput *BitO, Packet2Mux *Packet) {
        
    }
    
    void SpliceMPEG2TransportStream(BitInput *BitO, Packet2Mux *Packet) {
        
    }
    
    
    
#ifdef __cplusplus
}
#endif
