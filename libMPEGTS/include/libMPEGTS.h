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
    
    typedef struct TransportStreamPacket {
        int8_t   SyncByte;                     // sync_byte
        bool     TransportErrorIndicator:1;    // transport_error_indicator
        bool     StartOfPayloadIndicator:1;    // payload_unit_start_indicator
        bool     TransportPriorityIndicator:1; // transport_priority
        uint16_t PID:13;                       // PID
        int8_t   TransportScramblingControl:2; // transport_scrambling_control
        int8_t   AdaptationFieldControl:2;     // adaptation_field_control
        uint8_t  ContinuityCounter:4;          // continuity_counter
    } TransportStreamPacket;
    
    typedef struct TSAdaptationField {
        uint8_t  AdaptationFieldSize;                 // adaptation_field_length
        bool     DiscontinuityIndicator:1;            // discontinuity_indicator
        bool     RandomAccessIndicator:1;             // random_access_indicator
        bool     ElementaryStreamPriorityIndicator:1; // elementary_stream_priority_indicator
        bool     PCRFlag:1;                           // PCR_flag
        bool     OPCRFlag:1;                          // OPCR_flag
        bool     SlicingPointFlag:1;                  // splicing_point_flag
        bool     TransportPrivateDataFlag:1;          // transport_private_data_flag
        bool     AdaptationFieldExtensionFlag:1;      // adaptation_field_extension_flag
    } TSAdaptationField;
    
    typedef struct MPEGTransportStream {
        TransportStreamPacket *Packet;
        TSAdaptationField     *Adaptation;
    } MPEGTransportStream;
    
#ifdef __cplusplus
}
#endif
