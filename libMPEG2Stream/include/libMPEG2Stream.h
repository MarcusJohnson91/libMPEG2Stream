/*!
 @header    libMPEGTS
 @author    Marcus Johnson aka BumbleBritches57
 @copyright 2017, released under the BSD 3 clause license
 @version   0.0
 @brief     This library contains a muxer and demuxer for MPEG2-TS aka H.222 streams
 */

#include "/usr/local/Packages/libBitIO/include/BitIO.h"
#include "libMPEG2StreamTables.h"

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
    // Lets say I'm got a series of PNG images I want to encode to lossless AVC, with a preexisting FLAC audio track.
    // I could use ModernFLAC to decode the audio to PCM via a "DecodeFLACFrame" function, but I think that's a bit too nitty gritty.
    // Could I instead take and have the user create a program that loops over these frames and passes them to a standalone muxer?
    // This way would allow a LOT more flexability...
    
    typedef struct Packet2Mux Packet2Mux;
    
    /*!
     @abstract     Main muxing function
     */
    void MuxMPEGTransportStream(BitInput *BitO, Packet2Mux *Packet);
    
    void MuxAVC2MPEGTransportStream(BitInput *BitO, Packet2Mux *Packet);
    
    typedef struct TSAdaptationField TSAdaptationField;
    
    typedef struct TransportStreamPacket TransportStreamPacket;
    
    typedef struct PacketizedElementaryStream PacketizedElementaryStream;
    
    typedef struct ProgramAssociatedSection ProgramAssociatedSection;
    
    typedef struct ConditionalAccessSection ConditionalAccessSection;
    
    typedef struct ProgramStream ProgramStream;
    
    typedef struct MPEG2TransportStream {
        TransportStreamPacket      *Packet;
        TSAdaptationField          *Adaptation;
        PacketizedElementaryStream *PES;
        ProgramAssociatedSection   *Program;
        ConditionalAccessSection   *Condition;
    } MPEG2TransportStream;
    
    typedef struct MPEG2ProgramStream {
        ProgramStream              *PS;
        PacketizedElementaryStream *PES;
    } MPEG2ProgramStream;
    
    /*!
     @abstract     Main Demuxing function
     */
    void DemuxMPEGTransportStream(BitInput *BitI, MPEG2TransportStream *Stream);
    
#ifdef __cplusplus
}
#endif
