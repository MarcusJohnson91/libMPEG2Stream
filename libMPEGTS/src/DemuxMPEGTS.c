#include "../include/libMPEGTS.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void ParseTransportStreamAdaptionField(BitInput *BitI, MPEGTransportStream *TS) { // adaptation_field
        
    }
    
    void ParseTransportStreamPacket(BitInput *BitI, MPEGTransportStream *Stream) { // transport_packet
        Stream->Packet->SyncByte                   = ReadBits(BitI, 8);
        Stream->Packet->TransportErrorIndicator    = ReadBits(BitI, 1);
        Stream->Packet->StartOfPayloadIndicator    = ReadBits(BitI, 1);
        Stream->Packet->TransportPriorityIndicator = ReadBits(BitI, 1);
        Stream->Packet->PID                        = ReadBits(BitI, 13);
        Stream->Packet->TransportScramblingControl = ReadBits(BitI, 2);
        Stream->Packet->AdaptationFieldControl     = ReadBits(BitI, 2);
        Stream->Packet->ContinuityCounter          = ReadBits(BitI, 4);
        if (Stream->Packet->AdaptationFieldControl == 2 || Stream->Packet->AdaptationFieldControl == 3) {
            adaptation_field();
        }
        if (Stream->Packet->AdaptationFieldControl == 1 || Stream->Packet->AdaptationFieldControl == 3) {
            int N = 0; // FIXME: I just did this so it would compile what is N?
            for (int i = 0; i < N; i++) {
                SkipBits(BitI, 8); // data_byte
            }
        }
    }
    
    void InitMPEGTransportStream(MPEGTransportStream *Stream) {
        Stream->Packet     = calloc(sizeof(TransportStreamPacket), 1);
        Stream->Adaptation = calloc(sizeof(TSAdaptationField), 1);
    }
    
#ifdef __cplusplus
}
#endif
