#include "../../../Dependencies/BitIO/libBitIO/include/BitIO.h"
#include "../../include/libMPEG2Stream.h"
#include "../../include/MPEG2StreamTypes.h"
#include "../../include/Demuxer/DemuxMPEG2Stream.h"

#ifdef __cplusplus
extern "C" {
#endif

    // Each elementary stream needs it's own PES packet.
    // A Program Stream can contain multiple elementary streams encapsulated into PES packets.
    // Transport Streams and Program streams are like brothers, they do the same things, but some are more capable than others.

    // Access Unit = A coded representation of a Presentation Unit. In audio, an AU = a frame. in video, an AU = a picture + all padding and metadata.
    // PacketID    = an int that identifies elementary streams (audio or video) in a program.
    // Program     = Elementary streams that are to be played synchronized with the same time base.

    // Transport Streams CAN CONTAIN PROGRAM STREAMS, OR ELEMENTARY STREAMS.

    // So, for Demuxing, the general idea is to accumulate PES packets until you've got a whole NAL or whateve?
    // Also, we need a way to identify the stream type
    
    
    
    
    /* REAL INFO */
    // Transport streams have no global header.
    // Packet size is 188 bytes, M2ts adds 4 bytes for copyright and timestamp.

    static void ParseConditionalAccessDescriptor(BitInput *BitI, MPEG2TransportStream *Transport) { // CA_descriptor
        int N                                                 = 0;// TODO: what is N?
        uint8_t  DescriptorTag                                = ReadBits(BitI, 8, true);// descriptor_tag
        uint8_t  DescriptorSize                               = ReadBits(BitI, 8, true);// descriptor_length
        uint16_t ConditionalAccessID                          = ReadBits(BitI, 16, true);// CA_system_ID
        SkipBits(BitI, 3); // reserved
        uint16_t  ConditionalAccessPID                        = ReadBits(BitI, 13, true);
        for (int i = 0; i < N; i++) {
            SkipBits(BitI, 8); // private_data_byte
        }
    }

    static void ParseConditionalAccessSection(BitInput *BitI, MPEG2TransportStream *Transport) { // CA_section
        int N = 0; // TODO: find out what the hell N is
        Transport->Condition->TableID                = ReadBits(BitI, 8, true);
        Transport->Condition->SectionSyntaxIndicator = ReadBits(BitI, 1, true);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Transport->Condition->SectionSize            = ReadBits(BitI, 12, true);
        SkipBits(BitI, 18);
        Transport->Condition->VersionNum             = ReadBits(BitI, 5, true);
        Transport->Condition->CurrentNextIndicator   = ReadBits(BitI, 1, true);
        Transport->Condition->SectionNumber          = ReadBits(BitI, 8, true);
        Transport->Condition->LastSectionNumber      = ReadBits(BitI, 8, true);
        for (int i                                   = 0; i < N; i++) {
            TSParseConditionalAccessDescriptor(BitI, Transport);
        }
        Transport->Condition->ConditionCRC32         = ReadBits(BitI, 32, true);
    }

    static void ParseProgramAssociationTable(BitInput *BitI, MPEG2TransportStream *Transport) { // program_association_section
        Transport->Program->TableID                = ReadBits(BitI, 8, true);
        Transport->Program->SectionSyntaxIndicator = ReadBits(BitI, 1, true);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Transport->Program->SectionSize            = ReadBits(BitI, 12, true);
        Transport->Program->TransportStreamID      = ReadBits(BitI, 16, true);
        SkipBits(BitI, 2); // Reserved
        Transport->Program->VersionNum             = ReadBits(BitI, 5, true);
        Transport->Program->CurrentNextIndicator   = ReadBits(BitI, 1, true);
        Transport->Program->SectionNumber          = ReadBits(BitI, 8, true);
        Transport->Program->LastSectionNumber      = ReadBits(BitI, 8, true);
        Transport->Program->ProgramNumber          = ReadBits(BitI, 16, true);
        Transport->Program->NetworkPID             = ReadBits(BitI, 13, true);
        Transport->Program->ProgramMapPID          = ReadBits(BitI, 13, true);
        Transport->Program->ProgramCRC32           = ReadBits(BitI, 32, true);
    }

    static void ParsePackHeader(BitInput *BitI, MPEG2ProgramStream *Program) { // pack_header
        Program->PSP->PackStartCode           = ReadBits(BitI, 32, true);
        SkipBits(BitI, 2); // 01
        Program->PSP->SystemClockRefBase1     = ReadBits(BitI, 3, true);
        SkipBits(BitI, 1); // marker_bit
        Program->PSP->SystemClockRefBase2     = ReadBits(BitI, 15, true);
        SkipBits(BitI, 1); // marker_bit
        Program->PSP->SystemClockRefBase3     = ReadBits(BitI, 15, true);
        SkipBits(BitI, 1); // marker_bit
        Program->PSP->SystemClockRefBase      = Program->PSP->SystemClockRefBase1 << 30;
        Program->PSP->SystemClockRefBase     += Program->PSP->SystemClockRefBase2 << 15;
        Program->PSP->SystemClockRefBase     += Program->PSP->SystemClockRefBase3;
        
        Program->PSP->SystemClockRefExtension = ReadBits(BitI, 9, true);
        SkipBits(BitI, 1); // marker_bit
        Program->PSP->ProgramMuxRate          = ReadBits(BitI, 22, true);
        SkipBits(BitI, 7); // marker_bit && reserved
        Program->PSP->PackStuffingSize        = ReadBits(BitI, 3, true);
        SkipBits(BitI, Bytes2Bits(Program->PSP->PackStuffingSize));
        if (PeekBits(BitI, 0, true) == MPEG2TSSystemHeaderStartCode) {
            // system_header();
        }
    }

    static void ParsePESPacket(BitInput *BitI, PacketizedElementaryStream *Stream) { // PES_packet
        int N3                                       = 0, N2 = 0, N1 = 0;// FIXME : WTF IS N3, N2, and N1?
        Stream->PacketStartCodePrefix                = ReadBits(BitI, 24, true);
        Stream->StreamID                             = ReadBits(BitI, 8, true);// 13
        Stream->PESPacketSize                        = ReadBits(BitI, 16, true);//
        if (Stream->StreamID != ProgramStreamFolder &&
            Stream->StreamID != AnnexA_DSMCCStream &&
            Stream->StreamID != ProgramStreamMap &&
            Stream->StreamID != PrivateStream2 &&
            Stream->StreamID != PaddingStream &&
            Stream->StreamID != ECMStream &&
            Stream->StreamID != EMMStream &&
            Stream->StreamID != TypeEStream) {
            SkipBits(BitI, 2);
            Stream->PESScramblingControl             = ReadBits(BitI, 2, true);
            Stream->PESPriority                      = ReadBits(BitI, 1, true);
            Stream->AlignmentIndicator               = ReadBits(BitI, 1, true);
            Stream->CopyrightIndicator               = ReadBits(BitI, 1, true);
            Stream->OriginalOrCopy                   = ReadBits(BitI, 1, true);
            Stream->PTSDTSFlags                      = ReadBits(BitI, 2, true);
            Stream->ESCRFlag                         = ReadBits(BitI, 1, true);
            Stream->ESRateFlag                       = ReadBits(BitI, 1, true);
            Stream->DSMTrickModeFlag                 = ReadBits(BitI, 1, true);
            Stream->AdditionalCopyInfoFlag           = ReadBits(BitI, 1, true);
            Stream->PESCRCFlag                       = ReadBits(BitI, 1, true);
            Stream->PESExtensionFlag                 = ReadBits(BitI, 1, true);
            Stream->PESHeaderSize                    = ReadBits(BitI, 8, true);
            if (Stream->PTSDTSFlags == 2) {
                SkipBits(BitI, 4);
                uint8_t  PTS1                        = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS2                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS3                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                
                Stream->PTS                          = PTS1 << 30;
                Stream->PTS                         += PTS2 << 15;
                Stream->PTS                         += PTS3;
            }
            if (Stream->PTSDTSFlags == 3) {
                SkipBits(BitI, 4);
                uint8_t  PTS1                        = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS2                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS3                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                
                Stream->PTS                          = PTS1 << 30;
                Stream->PTS                         += PTS2 << 15;
                Stream->PTS                         += PTS3;
                
                SkipBits(BitI, 4);
                uint8_t  DTS1                        = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t DTS2                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t DTS3                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                
                Stream->DTS                          = DTS1 << 30;
                Stream->DTS                         += DTS2 << 15;
                Stream->DTS                         += DTS3;
            }
            if (Stream->ESCRFlag == true) {
                SkipBits(BitI, 2);
                uint8_t  ESCR1                       = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t ESCR2                       = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                uint16_t ESCR3                       = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
                
                Stream->ESCR                         = ESCR1 << 30;
                Stream->ESCR                        += ESCR2 << 15;
                Stream->ESCR                        += ESCR3;
            }
            if (Stream->ESRateFlag == true) {
                SkipBits(BitI, 1);
                Stream->ESRate                       = ReadBits(BitI, 22, true);
                SkipBits(BitI, 1);
            }
            if (Stream->DSMTrickModeFlag == true) {
                Stream->TrickModeControl             = ReadBits(BitI, 3, true);
                if (Stream->TrickModeControl        == FastForward) {
                    Stream->FieldID                  = ReadBits(BitI, 2, true);
                    Stream->IntraSliceRefresh        = ReadBits(BitI, 1, true);
                    Stream->FrequencyTruncation      = ReadBits(BitI, 2, true);
                } else if (Stream->TrickModeControl == SlowMotion) {
                    Stream->RepetitionControl        = ReadBits(BitI, 5, true);
                } else if (Stream->TrickModeControl == FreezeFrame) {
                    Stream->FieldID                  = ReadBits(BitI, 2, true);
                    SkipBits(BitI, 3);
                } else if (Stream->TrickModeControl == FastRewind) {
                    Stream->FieldID                  = ReadBits(BitI, 2, true);
                    Stream->IntraSliceRefresh        = ReadBits(BitI, 1, true);
                    Stream->FrequencyTruncation      = ReadBits(BitI, 2, true);
                } else if (Stream->TrickModeControl == SlowRewind) {
                    Stream->RepetitionControl        = ReadBits(BitI, 5, true);
                } else {
                    SkipBits(BitI, 5);
                }
            }
            if (Stream->AdditionalCopyInfoFlag == true) {
                SkipBits(BitI, 1);
                Stream->AdditionalCopyInfo           = ReadBits(BitI, 7, true);
            }
            if (Stream->PESCRCFlag == true) {
                Stream->PreviousPESPacketCRC         = ReadBits(BitI, 16, true);
            }
            if (Stream->PESExtensionFlag == true) {
                Stream->PESPrivateDataFlag           = ReadBits(BitI, 1, true);
                Stream->PackHeaderFieldFlag          = ReadBits(BitI, 1, true);
                Stream->ProgramPacketSeqCounterFlag  = ReadBits(BitI, 1, true);
                Stream->PSTDBufferFlag               = ReadBits(BitI, 1, true);
                Stream->PESExtensionFlag2            = ReadBits(BitI, 1, true);
                if (Stream->PESPrivateDataFlag == true) {
                    SkipBits(BitI, 128);
                }
                if (Stream->PackHeaderFieldFlag == true) {
                    Stream->PackFieldSize            = ReadBits(BitI, 8, true);
                    pack_header();
                }
                if (Stream->ProgramPacketSeqCounterFlag == true) {
                    SkipBits(BitI, 1);
                    Stream->ProgramPacketSeqCounter  = ReadBits(BitI, 7, true);
                    SkipBits(BitI, 1);
                    Stream->MPEGVersionIdentifier    = ReadBits(BitI, 1, true);
                    Stream->OriginalStuffSize        = ReadBits(BitI, 6, true);
                }
                if (Stream->PSTDBufferFlag == true) {
                    SkipBits(BitI, 2);
                    Stream->PSTDBufferScale         = ReadBits(BitI, 1, true);
                    Stream->PSTDBufferSize          = ReadBits(BitI, 13, true);
                }
                if (Stream->PESExtensionFlag2 == true) {
                    SkipBits(BitI, 1);
                    Stream->PESExtensionFieldSize    = ReadBits(BitI, 7, true);
                    Stream->StreamIDExtensionFlag    = ReadBits(BitI, 1, true);
                    if (Stream->StreamIDExtensionFlag == false) {
                        Stream->StreamIDExtension    = ReadBits(BitI, 7, true);
                    } else {
                        SkipBits(BitI, 6);
                        // tref_extension_flag
                        Stream->TREFFieldPresentFlag = ReadBits(BitI, 1, true);
                        if (Stream->TREFFieldPresentFlag == false) {
                            SkipBits(BitI, 4);
                            uint8_t  TREF1           = ReadBits(BitI, 3, true);
                            SkipBits(BitI, 1); // marker bit
                            uint16_t TREF2           = ReadBits(BitI, 15, true);
                            SkipBits(BitI, 1); // marker bit
                            uint16_t TREF3           = ReadBits(BitI, 15, true);
                            SkipBits(BitI, 1); // marker bit
                            
                            Stream->TREF             = TREF1 << 30;
                            Stream->TREF             = TREF2 << 15;
                            Stream->TREF             = TREF3;
                        }
                    }
                    for (int i = 0; i < N3; i++) {
                        SkipBits(BitI, 8); // Reserved
                    }
                }
                for (int i = 0; i < Stream->PESExtensionFieldSize; i++) {
                    SkipBits(BitI, 8);
                }
            }
            for (int i = 0; i < N1; i++) {
                SkipBits(BitI, 8); // stuffing_byte so 0b11111111, throw it away.
            }
            for (int i = 0; i < N2; i++) {
                SkipBits(BitI, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == ProgramStreamFolder ||
                   Stream->StreamID == AnnexA_DSMCCStream ||
                   Stream->StreamID == ProgramStreamMap ||
                   Stream->StreamID == PrivateStream2 ||
                   Stream->StreamID == ECMStream ||
                   Stream->StreamID == EMMStream ||
                   Stream->StreamID == TypeEStream) {
            for (int i = 0; i < Stream->PESPacketSize; i++) {
                SkipBits(BitI, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == PaddingStream) {
            for (int i = 0; i < Stream->PESPacketSize; i++) {
                SkipBits(BitI, 8); // padding_byte
            }
        }
    }

    static void ParseTransportStreamAdaptionField(BitInput *BitI, MPEG2TransportStream *Transport) { // adaptation_field
        Transport->Adaptation->AdaptationFieldSize                       = ReadBits(BitI, 8, true);
        Transport->Adaptation->DiscontinuityIndicator                    = ReadBits(BitI, 1, true);
        Transport->Adaptation->RandomAccessIndicator                     = ReadBits(BitI, 1, true);
        Transport->Adaptation->ElementaryStreamPriorityIndicator         = ReadBits(BitI, 1, true);
        Transport->Adaptation->PCRFlag                                   = ReadBits(BitI, 1, true);
        Transport->Adaptation->OPCRFlag                                  = ReadBits(BitI, 1, true);
        Transport->Adaptation->SlicingPointFlag                          = ReadBits(BitI, 1, true);
        Transport->Adaptation->TransportPrivateDataFlag                  = ReadBits(BitI, 1, true);
        Transport->Adaptation->AdaptationFieldExtensionFlag              = ReadBits(BitI, 1, true); // 16 bits read so far
        if (Transport->Adaptation->PCRFlag == true) { // Reads 48 bits
            Transport->Adaptation->ProgramClockReferenceBase             = ReadBits(BitI, 33, true);
            SkipBits(BitI, 6);
            Transport->Adaptation->ProgramClockReferenceExtension        = ReadBits(BitI, 9, true);
        }
        if (Transport->Adaptation->OPCRFlag == true) { // Reads 48 bits
            Transport->Adaptation->OriginalProgramClockRefBase           = ReadBits(BitI, 33, true);
            SkipBits(BitI, 6);
            Transport->Adaptation->OriginalProgramClockRefExt            = ReadBits(BitI, 9, true);
        }
        if (Transport->Adaptation->SlicingPointFlag == true) { // Reads 8 bits
            Transport->Adaptation->SpliceCountdown                       = ReadBits(BitI, 8, true);
        }
        if (Transport->Adaptation->TransportPrivateDataFlag == true) { // Reads up to 2056 bits
            Transport->Adaptation->TransportPrivateDataSize              = ReadBits(BitI, 8, true);
            for (uint8_t PrivateByte = 0; PrivateByte < Transport->Adaptation->TransportPrivateDataSize; PrivateByte++) {
                Transport->Adaptation->TransportPrivateData[PrivateByte] = ReadBits(BitI, 8, true);
            }
        }
        if (Transport->Adaptation->AdaptationFieldExtensionFlag == true) { // Reads 12 bits
            Transport->Adaptation->AdaptationFieldExtensionSize          = ReadBits(BitI, 8, true);
            Transport->Adaptation->LegalTimeWindowFlag                   = ReadBits(BitI, 1, true);
            Transport->Adaptation->PiecewiseRateFlag                     = ReadBits(BitI, 1, true);
            Transport->Adaptation->SeamlessSpliceFlag                    = ReadBits(BitI, 1, true);
            AlignInput(BitI, 1);
            if (Transport->Adaptation->LegalTimeWindowFlag == true) { // Reads 16 bits
                Transport->Adaptation->LegalTimeWindowValidFlag          = ReadBits(BitI, 1, true);
                if (Transport->Adaptation->LegalTimeWindowValidFlag == true) {
                    Transport->Adaptation->LegalTimeWindowOffset         = ReadBits(BitI, 15, true);
                }
            }
            if (Transport->Adaptation->PiecewiseRateFlag == true) { // Reads 24 bits
                SkipBits(BitI, 2);
                Transport->Adaptation->PiecewiseRateFlag                 = ReadBits(BitI, 22, true);
            }
            if (Transport->Adaptation->SeamlessSpliceFlag == true) { // Reads 44 bits
                Transport->Adaptation->SpliceType                        = ReadBits(BitI, 8, true);
                uint8_t  DecodeTimeStamp1                                = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp2                                = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp3                                = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker_bit
                
                Transport->Adaptation->DecodeTimeStampNextAU             = DecodeTimeStamp1 << 30;
                Transport->Adaptation->DecodeTimeStampNextAU            += DecodeTimeStamp2 << 15;
                Transport->Adaptation->DecodeTimeStampNextAU            += DecodeTimeStamp3;
            }
            for (int i = 0; i < 184 - Transport->Adaptation->AdaptationFieldSize; i++) { // So, the minimum number of bytes to read is 188 - 284, aka -96 lol
                SkipBits(BitI, 8); // Reserved
            }
        }
        for (int i = 0; i < 184 - Transport->Adaptation->AdaptationFieldSize; i++) {
            SkipBits(BitI, 8); // Stuffing so 0b11111111, throw it away.
        }
    }

    static void ParseTransportStreamPacket(BitInput *BitI, MPEG2TransportStream *Transport, uint8_t *TransportStream, size_t TransportStreamSize) { // transport_packet
        Transport->Packet->SyncByte                              = ReadBits(BitI, 8, true);
        Transport->Packet->TransportErrorIndicator               = ReadBits(BitI, 1, true); // false
        Transport->Packet->StartOfPayloadIndicator               = ReadBits(BitI, 1, true); // true
        Transport->Packet->TransportPriorityIndicator            = ReadBits(BitI, 1, true); // false
        Transport->Packet->PID                                   = ReadBits(BitI, 13, true); // 0
        Transport->Packet->TransportScramblingControl            = ReadBits(BitI, 2, true); // 0
        Transport->Packet->AdaptationFieldControl                = ReadBits(BitI, 2, true); // 1
        Transport->Packet->ContinuityCounter                     = ReadBits(BitI, 4, true); // 0
        if (Transport->Packet->AdaptationFieldControl == 2 || Transport->Packet->AdaptationFieldControl == 3) {
            ParseTransportStreamAdaptionField(BitI, Transport);
        }
        if (Transport->Packet->AdaptationFieldControl == 1 || Transport->Packet->AdaptationFieldControl == 3) {
            for (int i = 0; i < 184; i++) {
                TransportStream[i] = ReadBits(BitI, 8, true); // data_byte, start copying data to the transport stream.
            }
        }
    }

    void DemuxMPEG2PESPackets(BitInput *BitI, PacketizedElementaryStream *PESPacket) {
		
    }

    void DemuxMPEG2ProgramStreamPacket(BitInput *BitI, MPEG2ProgramStream *Program) {

    }

    void DemuxMPEG2TransportStreamPacket(BitInput *BitI, MPEG2TransportStream *Transport) {

    }

#ifdef __cplusplus
}
#endif
