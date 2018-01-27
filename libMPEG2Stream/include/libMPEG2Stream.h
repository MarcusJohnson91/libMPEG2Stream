/*!
 @header    libMPEGTS
 @author    Marcus Johnson aka BumbleBritches57
 @copyright 2017, released under the BSD 3 clause license
 @version   0.1.0
 @brief     This library contains a muxer and demuxer for MPEG2-TS aka H.222 streams
 */

#include "../../Dependencies/BitIO/libBitIO/include/BitIO.h"
#include "libMPEG2StreamTables.h"

#pragma once

#ifndef LIBMPEG2STREAM_LIBMPEG2STREAM_H
#define LIBMPEG2STREAM_LIBMPEG2STREAM_H

#ifdef __cplusplus
extern "C" {
#endif
    
    enum MPEG2StreamConstants {
        PESPacketSize                      =    188, // bytes
        MPEG2TransportStreamMagic          =   0x47,
        MPEG2TransportStreamMaxPrivateData =    256, // bytes
        MPEG2TSSystemHeaderStartCode       =    0x0,
        PESPacketCRCPolynomial             = 0x8811,
    };
    
    enum MPEG2StreamSourcePacketSize {
        MPEG2TransportStreamPacketSize       = 188,
        MPEG2TransportStreamBlurayPacketSize = 192,
        MPEG2TransportStreamISDBPacketSize   = 204,
        MPEG2TransportStreamDVBPacketSize    = 204,
        MPEG2TransportStreamATSCPacketSize   = 208,
    };
    
    enum TSPIDTable {
        ProgramAssociationTable            =    0,// Program association table
        ConditionalAccesTable              =    1,// Conditional access table
        TransportStreamDescriptionTable    =    2,// Transport stream description table
        IPMPControlInfoTable               =    3,// Intellectual Property Management and Protection
        LastReserved                       = 8191,
    };

    enum TSAdaptationFieldTable {
        Reserved                           = 0,
        PayloadOnly                        = 1,
        AdaptationFieldOnly                = 2,
        AdaptationThenPayload              = 3,
    };

    enum TSStreamIDTypes {
        ProgramStreamMap                   = 188,
        PrivateStream1                     = 189,
        PaddingStream                      = 190,
        PrivateStream2                     = 191,
        AudioStream                        = 192,// & 0xC0, the 5 least significant bits are the audio stream number.
        VideoStream                        = 224,// & 0xE0, the 4 least significant bits are the video stream number.
        ECMStream                          = 240,
        EMMStream                          = 241,
        AnnexA_DSMCCStream                 = 242,// or 13818-1 Annex A
        Stream13522                        = 243,
        TypeAStream                        = 244,
        TypeBStream                        = 245,
        TypeCStream                        = 246,
        TypeDStream                        = 247,
        TypeEStream                        = 248,
        AncillaryStream                    = 249,
        SLPacketizedStream                 = 250,
        FlexMuxStream                      = 251,
        MetadataStream                     = 252,
        ExtendedStreamID                   = 253,
        ReservedStream                     = 254,
        ProgramStreamFolder                = 255,
    };

    enum TSTrickModeTypes {
        FastForward                        = 0,
        SlowMotion                         = 1,
        FreezeFrame                        = 2,
        FastRewind                         = 3,
        SlowRewind                         = 4,
    };

    enum TSFieldID {
        TopFieldOnly                       = 0,
        BottomFieldOnly                    = 1,
        DisplayFullPic                     = 2,
    };

    enum CoefficentSelection {
        OnlyDCCoeffsAreNonZero             = 0,
        OnlyFirst3CoeffsAreNonZero         = 1,
        OnlyFirst6CoeffsAreNonZero         = 2,
        AllCoeffsMayBeNonZero              = 3,
    };

    enum PacketTypes {
        TrueHD                             = 0,
        DTSXLL                             = 1,
        AVC                                = 2,
    };
    
    // Lets say I'm got a series of PNG images I want to encode to lossless AVC, with a preexisting FLAC audio track.
    // I could use ModernFLAC to decode the audio to PCM via a "DecodeFLACFrame" function, but I think that's a bit too nitty gritty.
    // Could I instead take and have the user create a program that loops over these frames and passes them to a standalone muxer?
    // This way would allow a LOT more flexability...
    
    typedef struct Packet2Mux Packet2Mux;
    
    typedef struct MPEG2TransportStream MPEG2TransportStream;
    
    typedef struct MPEG2ProgramStream MPEG2ProgramStream;
    
    MPEG2TransportStream *InitMPEGTransportStream(void);
    
    /*!
     @abstract     Main Demuxing function
     */
    void DemuxMPEGTransportStream(MPEG2TransportStream *Stream, BitBuffer *BitB);
    
    /*!
     @abstract     Main muxing function
     */
    void MuxMPEGTransportStream(BitBuffer *BitB, Packet2Mux *Packet, uint64_t PacketType, uint64_t PacketLocale, uint64_t MainLocale);
    
    void MuxAVC2MPEGTransportStream(BitBuffer *BitB, Packet2Mux *Packet);
    
#ifdef __cplusplus
}
#endif

#endif /* LIBMPEG2STREAM_LIBMPEG2STREAM_H */
