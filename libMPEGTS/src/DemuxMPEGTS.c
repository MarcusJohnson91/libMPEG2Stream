#include "../include/libMPEGTS.h"

#ifdef __cplusplus
extern "C" {
#endif
    
    void TSParsePESPacket(BitInput *BitI, MPEGTransportStream *Stream) { // PES_packet
        
    }
    
    void TSParseAdaptionField(BitInput *BitI, MPEGTransportStream *Stream) { // adaptation_field
        int N = 0; // FIXME: WHAT THE FUCK IS N?
        Stream->Adaptation->AdaptationFieldSize                = ReadBits(BitI, 8);
        Stream->Adaptation->DiscontinuityIndicator             = ReadBits(BitI, 1);
        Stream->Adaptation->RandomAccessIndicator              = ReadBits(BitI, 1);
        Stream->Adaptation->ElementaryStreamPriorityIndicator  = ReadBits(BitI, 1);
        Stream->Adaptation->PCRFlag                            = ReadBits(BitI, 1);
        Stream->Adaptation->OPCRFlag                           = ReadBits(BitI, 1);
        Stream->Adaptation->SlicingPointFlag                   = ReadBits(BitI, 1);
        Stream->Adaptation->TransportPrivateDataFlag           = ReadBits(BitI, 1);
        Stream->Adaptation->AdaptationFieldExtensionFlag       = ReadBits(BitI, 1);
        if (Stream->Adaptation->PCRFlag == true) {
            Stream->Adaptation->ProgramClockReferenceBase      = ReadBits(BitI, 33);
            SkipBits(BitI, 6);
            Stream->Adaptation->ProgramClockReferenceExtension = ReadBits(BitI, 9);
        }
        if (Stream->Adaptation->OPCRFlag == true) {
            Stream->Adaptation->OriginalProgramClockRefBase    = ReadBits(BitI, 33);
            SkipBits(BitI, 6);
            Stream->Adaptation->OriginalProgramClockRefExt     = ReadBits(BitI, 9);
        }
        if (Stream->Adaptation->SlicingPointFlag == true) {
            Stream->Adaptation->SpliceCountdown                = ReadBits(BitI, 8);
        }
        if (Stream->Adaptation->TransportPrivateDataFlag == true) {
            Stream->Adaptation->TransportPrivateDataSize       = ReadBits(BitI, 8);
            for (uint8_t PrivateByte = 0; PrivateByte < Stream->Adaptation->TransportPrivateDataSize; PrivateByte++) {
                Stream->Adaptation->TransportPrivateData[PrivateByte] = ReadBits(BitI, 8);
            }
        }
        if (Stream->Adaptation->AdaptationFieldExtensionFlag == true) {
            Stream->Adaptation->AdaptationFieldExtensionSize    = ReadBits(BitI, 8);
            Stream->Adaptation->LegalTimeWindowFlag             = ReadBits(BitI, 1);
            Stream->Adaptation->PiecewiseRateFlag               = ReadBits(BitI, 1);
            Stream->Adaptation->SeamlessSpliceFlag              = ReadBits(BitI, 1);
            AlignInput(BitI, 1);
            if (Stream->Adaptation->LegalTimeWindowFlag == true) {
                Stream->Adaptation->LegalTimeWindowValidFlag    = ReadBits(BitI, 1);
                if (Stream->Adaptation->LegalTimeWindowValidFlag == true) {
                    Stream->Adaptation->LegalTimeWindowOffset   = ReadBits(BitI, 15);
                }
            }
            if (Stream->Adaptation->PiecewiseRateFlag == true) {
                SkipBits(BitI, 2);
                Stream->Adaptation->PiecewiseRateFlag           = ReadBits(BitI, 22);
            }
            if (Stream->Adaptation->SeamlessSpliceFlag == true) {
                Stream->Adaptation->SpliceType                  = ReadBits(BitI, 8);
                uint8_t  DecodeTimeStamp3                       = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp2                       = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp1                       = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker_bit
                
                Stream->Adaptation->DecodeTimeStampNextAU  = DecodeTimeStamp1 << 18;
                Stream->Adaptation->DecodeTimeStampNextAU += DecodeTimeStamp2 << 3;
                Stream->Adaptation->DecodeTimeStampNextAU += DecodeTimeStamp1;
            }
            for (int i = 0; i < N; i++) {
                SkipBits(BitI, 8);
            }
        }
        for (int i = 0; i < N; i++) {
            SkipBits(BitI, 8);
        }
    }
    
    void TSParsePacket(BitInput *BitI, MPEGTransportStream *Stream) { // transport_packet
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
