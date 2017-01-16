#include "../include/libMPEGTS.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void ParseTransportStreamAdaptionField(BitInput *BitI, MPEGTransportStream *TS) { // adaptation_field
        
    }
    
    void ParseTransportStreamPacket(BitInput *BitI, MPEGTransportStream *TS) { // transport_packet
        TS->SyncByte                   = ReadBits(BitI, 8);
        TS->TransportErrorIndicator    = ReadBits(BitI, 1);
        TS->StartOfPayloadIndicator    = ReadBits(BitI, 1);
        TS->TransportPriorityIndicator = ReadBits(BitI, 1);
        TS->PID                        = ReadBits(BitI, 13);
        TS->TransportScramblingControl = ReadBits(BitI, 2);
        TS->AdaptationFieldControl     = ReadBits(BitI, 2);
        TS->ContinuityCounter          = ReadBits(BitI, 4);
        if (TS->AdaptationFieldControl == 2 || TS->AdaptationFieldControl == 3) {
            adaptation_field();
        }
        if (TS->AdaptationFieldControl == 1 || TS->AdaptationFieldControl == 3) {
            int N = 0; // FIXME: I just did this so it would compile what is N?
            for (int i = 0; i < N; i++) {
                SkipBits(BitI, 8); // data_byte
            }
        }
    }
    
#ifdef __cplusplus
}
#endif
