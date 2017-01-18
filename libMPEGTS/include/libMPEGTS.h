/*!
 @header    libMPEGTS
 @author    Marcus Johnson aka BumbleBritches57
 @copyright 2017, released under the BSD 3 clause license
 @version   0.0
 @brief     This library contains a muxer and demuxer for MPEG2-TS aka H.222 streams
 */

#include "/usr/local/Packages/BitIO/include/BitIO.h"
#include "libMPEGTSTables.h"

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
    
    typedef struct MPEGTransportStream {
        TransportStreamPacket      *Packet;
        TSAdaptationField          *Adaptation;
        PacketizedElementaryStream *PES;
        ProgramAssociatedSection   *Program;
        ConditionalAccessSection   *Condition;
        ProgramStream              *PS;
    } MPEGTransportStream;
    
    /*!
     @abstract     Main Demuxing function
     */
    void DemuxMPEGTransportStream(BitInput *BitI, MPEGTransportStream *Stream);
    
#ifdef __cplusplus
}
#endif
