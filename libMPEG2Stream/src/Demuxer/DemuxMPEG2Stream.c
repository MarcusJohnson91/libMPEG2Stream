#include "../../../Dependencies/FoundationIO/libFoundationIO/include/BitIO.h"
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

    static void ParseConditionalAccessDescriptor(MPEG2TransportStream *Transport, BitBuffer *BitB) { // CA_descriptor
        int N                                                 = 0;// TODO: what is N?
        uint8_t  DescriptorTag                                = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);// descriptor_tag
        uint8_t  DescriptorSize                               = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);// descriptor_length
        uint16_t ConditionalAccessID                          = ReadBits(MSByteFirst, LSBitFirst, BitB, 16);// CA_system_ID
        BitBuffer_Skip(BitB, 3); // reserved
        uint16_t  ConditionalAccessPID                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        for (int i = 0; i < N; i++) {
            BitBuffer_Skip(BitB, 8); // private_data_byte
        }
    }

    static void ParseConditionalAccessSection(MPEG2TransportStream *Transport, BitBuffer *BitB) { // CA_section
        int N = 0; // TODO: find out what the hell N is
        Transport->Condition->TableID                = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Condition->SectionSyntaxIndicator = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        BitBuffer_Skip(BitB, 3); // "0" + 2 bits reserved.
        Transport->Condition->SectionSize            = ReadBits(MSByteFirst, LSBitFirst, BitB, 12);
        BitBuffer_Skip(BitB, 18);
        Transport->Condition->VersionNum             = ReadBits(MSByteFirst, LSBitFirst, BitB, 5);
        Transport->Condition->CurrentNextIndicator   = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Condition->SectionNumber          = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Condition->LastSectionNumber      = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        for (int i                                   = 0; i < N; i++) {
            TSParseConditionalAccessDescriptor(BitB, Transport);
        }
        Transport->Condition->ConditionCRC32         = ReadBits(MSByteFirst, LSBitFirst, BitB, 32);
    }

    static void ParseProgramAssociationTable(MPEG2TransportStream *Transport, BitBuffer *BitB) { // program_association_section
        Transport->Program->TableID                = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Program->SectionSyntaxIndicator = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        BitBuffer_Skip(BitB, 3); // "0" + 2 bits reserved.
        Transport->Program->SectionSize            = ReadBits(MSByteFirst, LSBitFirst, BitB, 12);
        Transport->Program->TransportStreamID      = ReadBits(MSByteFirst, LSBitFirst, BitB, 16);
        BitBuffer_Skip(BitB, 2); // Reserved
        Transport->Program->VersionNum             = ReadBits(MSByteFirst, LSBitFirst, BitB, 5);
        Transport->Program->CurrentNextIndicator   = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Program->SectionNumber          = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Program->LastSectionNumber      = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Program->ProgramNumber          = ReadBits(MSByteFirst, LSBitFirst, BitB, 16);
        Transport->Program->NetworkPID             = ReadBits(MSByteFirst, LSBitFirst, BitB, 13);
        Transport->Program->ProgramMapPID          = ReadBits(MSByteFirst, LSBitFirst, BitB, 13);
        Transport->Program->ProgramCRC32           = ReadBits(MSByteFirst, LSBitFirst, BitB, 32);
    }

    static void ParsePackHeader(MPEG2ProgramStream *Program, BitBuffer *BitB) { // pack_header
        Program->PSP->PackStartCode           = ReadBits(MSByteFirst, LSBitFirst, BitB, 32);
        BitBuffer_Skip(BitB, 2); // 01
        Program->PSP->SystemClockRefBase1     = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
        BitBuffer_Skip(BitB, 1); // marker_bit
        Program->PSP->SystemClockRefBase2     = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
        BitBuffer_Skip(BitB, 1); // marker_bit
        Program->PSP->SystemClockRefBase3     = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
        BitBuffer_Skip(BitB, 1); // marker_bit
        Program->PSP->SystemClockRefBase      = Program->PSP->SystemClockRefBase1 << 30;
        Program->PSP->SystemClockRefBase     += Program->PSP->SystemClockRefBase2 << 15;
        Program->PSP->SystemClockRefBase     += Program->PSP->SystemClockRefBase3;
        
        Program->PSP->SystemClockRefExtension = ReadBits(MSByteFirst, LSBitFirst, BitB, 9);
        BitBuffer_Skip(BitB, 1); // marker_bit
        Program->PSP->ProgramMuxRate          = ReadBits(MSByteFirst, LSBitFirst, BitB, 22);
        BitBuffer_Skip(BitB, 7); // marker_bit && reserved
        Program->PSP->PackStuffingSize        = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
        BitBuffer_Skip(BitB, Bytes2Bits(Program->PSP->PackStuffingSize));
        if (PeekBits(MSByteFirst, LSBitFirst, BitB, 0) == MPEG2TSSystemHeaderStartCode) {
            // system_header();
        }
    }

    static void ParsePESPacket(PacketizedElementaryStream *Stream, BitBuffer *BitB) { // PES_packet
        int N3                                       = 0, N2 = 0, N1 = 0;// FIXME : WTF IS N3, N2, and N1?
        Stream->PacketStartCodePrefix                = ReadBits(MSByteFirst, LSBitFirst, BitB, 24);
        Stream->StreamID                             = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);// 13
        Stream->PESPacketSize                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 16);//
        if (Stream->StreamID != ProgramStreamFolder &&
            Stream->StreamID != AnnexA_DSMCCStream &&
            Stream->StreamID != ProgramStreamMap &&
            Stream->StreamID != PrivateStream2 &&
            Stream->StreamID != PaddingStream &&
            Stream->StreamID != ECMStream &&
            Stream->StreamID != EMMStream &&
            Stream->StreamID != TypeEStream) {
            BitBuffer_Skip(BitB, 2);
            Stream->PESScramblingControl             = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
            Stream->PESPriority                      = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->AlignmentIndicator               = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->CopyrightIndicator               = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->OriginalOrCopy                   = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->PTSDTSFlags                      = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
            Stream->ESCRFlag                         = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->ESRateFlag                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->DSMTrickModeFlag                 = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->AdditionalCopyInfoFlag           = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->PESCRCFlag                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->PESExtensionFlag                 = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Stream->PESHeaderSize                    = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
            if (Stream->PTSDTSFlags == 2) {
                BitBuffer_Skip(BitB, 4);
                uint8_t  PTS1                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t PTS2                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t PTS3                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                
                Stream->PTS                          = PTS1 << 30;
                Stream->PTS                         += PTS2 << 15;
                Stream->PTS                         += PTS3;
            }
            if (Stream->PTSDTSFlags == 3) {
                BitBuffer_Skip(BitB, 4);
                uint8_t  PTS1                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t PTS2                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t PTS3                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                
                Stream->PTS                          = PTS1 << 30;
                Stream->PTS                         += PTS2 << 15;
                Stream->PTS                         += PTS3;
                
                BitBuffer_Skip(BitB, 4);
                uint8_t  DTS1                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t DTS2                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t DTS3                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                
                Stream->DTS                          = DTS1 << 30;
                Stream->DTS                         += DTS2 << 15;
                Stream->DTS                         += DTS3;
            }
            if (Stream->ESCRFlag == true) {
                BitBuffer_Skip(BitB, 2);
                uint8_t  ESCR1                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t ESCR2                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                uint16_t ESCR3                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker bit
                
                Stream->ESCR                         = ESCR1 << 30;
                Stream->ESCR                        += ESCR2 << 15;
                Stream->ESCR                        += ESCR3;
            }
            if (Stream->ESRateFlag == true) {
                BitBuffer_Skip(BitB, 1);
                Stream->ESRate                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 22);
                BitBuffer_Skip(BitB, 1);
            }
            if (Stream->DSMTrickModeFlag == true) {
                Stream->TrickModeControl             = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                if (Stream->TrickModeControl        == FastForward) {
                    Stream->FieldID                  = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
                    Stream->IntraSliceRefresh        = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                    Stream->FrequencyTruncation      = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
                } else if (Stream->TrickModeControl == SlowMotion) {
                    Stream->RepetitionControl        = ReadBits(MSByteFirst, LSBitFirst, BitB, 5);
                } else if (Stream->TrickModeControl == FreezeFrame) {
                    Stream->FieldID                  = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
                    BitBuffer_Skip(BitB, 3);
                } else if (Stream->TrickModeControl == FastRewind) {
                    Stream->FieldID                  = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
                    Stream->IntraSliceRefresh        = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                    Stream->FrequencyTruncation      = ReadBits(MSByteFirst, LSBitFirst, BitB, 2);
                } else if (Stream->TrickModeControl == SlowRewind) {
                    Stream->RepetitionControl        = ReadBits(MSByteFirst, LSBitFirst, BitB, 5);
                } else {
                    BitBuffer_Skip(BitB, 5);
                }
            }
            if (Stream->AdditionalCopyInfoFlag == true) {
                BitBuffer_Skip(BitB, 1);
                Stream->AdditionalCopyInfo           = ReadBits(MSByteFirst, LSBitFirst, BitB, 7);
            }
            if (Stream->PESCRCFlag == true) {
                Stream->PreviousPESPacketCRC         = ReadBits(MSByteFirst, LSBitFirst, BitB, 16);
            }
            if (Stream->PESExtensionFlag == true) {
                Stream->PESPrivateDataFlag           = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                Stream->PackHeaderFieldFlag          = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                Stream->ProgramPacketSeqCounterFlag  = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                Stream->PSTDBufferFlag               = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                Stream->PESExtensionFlag2            = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                if (Stream->PESPrivateDataFlag == true) {
                    BitBuffer_Skip(BitB, 128);
                }
                if (Stream->PackHeaderFieldFlag == true) {
                    Stream->PackFieldSize            = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
                    pack_header();
                }
                if (Stream->ProgramPacketSeqCounterFlag == true) {
                    BitBuffer_Skip(BitB, 1);
                    Stream->ProgramPacketSeqCounter  = ReadBits(MSByteFirst, LSBitFirst, BitB, 7);
                    BitBuffer_Skip(BitB, 1);
                    Stream->MPEGVersionIdentifier    = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                    Stream->OriginalStuffSize        = ReadBits(MSByteFirst, LSBitFirst, BitB, 6);
                }
                if (Stream->PSTDBufferFlag == true) {
                    BitBuffer_Skip(BitB, 2);
                    Stream->PSTDBufferScale         = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                    Stream->PSTDBufferSize          = ReadBits(MSByteFirst, LSBitFirst, BitB, 13);
                }
                if (Stream->PESExtensionFlag2 == true) {
                    BitBuffer_Skip(BitB, 1);
                    Stream->PESExtensionFieldSize    = ReadBits(MSByteFirst, LSBitFirst, BitB, 7);
                    Stream->StreamIDExtensionFlag    = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                    if (Stream->StreamIDExtensionFlag == false) {
                        Stream->StreamIDExtension    = ReadBits(MSByteFirst, LSBitFirst, BitB, 7);
                    } else {
                        BitBuffer_Skip(BitB, 6);
                        // tref_extension_flag
                        Stream->TREFFieldPresentFlag = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                        if (Stream->TREFFieldPresentFlag == false) {
                            BitBuffer_Skip(BitB, 4);
                            uint8_t  TREF1           = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                            BitBuffer_Skip(BitB, 1); // marker bit
                            uint16_t TREF2           = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                            BitBuffer_Skip(BitB, 1); // marker bit
                            uint16_t TREF3           = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                            BitBuffer_Skip(BitB, 1); // marker bit
                            
                            Stream->TREF             = TREF1 << 30;
                            Stream->TREF             = TREF2 << 15;
                            Stream->TREF             = TREF3;
                        }
                    }
                    for (int i = 0; i < N3; i++) {
                        BitBuffer_Skip(BitB, 8); // Reserved
                    }
                }
                for (int i = 0; i < Stream->PESExtensionFieldSize; i++) {
                    BitBuffer_Skip(BitB, 8);
                }
            }
            for (int i = 0; i < N1; i++) {
                BitBuffer_Skip(BitB, 8); // stuffing_byte so 0b11111111, throw it away.
            }
            for (int i = 0; i < N2; i++) {
                BitBuffer_Skip(BitB, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == ProgramStreamFolder ||
                   Stream->StreamID == AnnexA_DSMCCStream ||
                   Stream->StreamID == ProgramStreamMap ||
                   Stream->StreamID == PrivateStream2 ||
                   Stream->StreamID == ECMStream ||
                   Stream->StreamID == EMMStream ||
                   Stream->StreamID == TypeEStream) {
            for (int i = 0; i < Stream->PESPacketSize; i++) {
                BitBuffer_Skip(BitB, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == PaddingStream) {
            for (int i = 0; i < Stream->PESPacketSize; i++) {
                BitBuffer_Skip(BitB, 8); // padding_byte
            }
        }
    }

    static void ParseTransportStreamAdaptionField(MPEG2TransportStream *Transport, BitBuffer *BitB) { // adaptation_field
        Transport->Adaptation->AdaptationFieldSize                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Adaptation->DiscontinuityIndicator                    = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->RandomAccessIndicator                     = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->ElementaryStreamPriorityIndicator         = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->PCRFlag                                   = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->OPCRFlag                                  = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->SlicingPointFlag                          = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->TransportPrivateDataFlag                  = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
        Transport->Adaptation->AdaptationFieldExtensionFlag              = ReadBits(MSByteFirst, LSBitFirst, BitB, 1); // 16 bits read so far
        if (Transport->Adaptation->PCRFlag == true) { // Reads 48 bits
            Transport->Adaptation->ProgramClockReferenceBase             = ReadBits(MSByteFirst, LSBitFirst, BitB, 33);
            BitBuffer_Skip(BitB, 6);
            Transport->Adaptation->ProgramClockReferenceExtension        = ReadBits(MSByteFirst, LSBitFirst, BitB, 9);
        }
        if (Transport->Adaptation->OPCRFlag == true) { // Reads 48 bits
            Transport->Adaptation->OriginalProgramClockRefBase           = ReadBits(MSByteFirst, LSBitFirst, BitB, 33);
            BitBuffer_Skip(BitB, 6);
            Transport->Adaptation->OriginalProgramClockRefExt            = ReadBits(MSByteFirst, LSBitFirst, BitB, 9);
        }
        if (Transport->Adaptation->SlicingPointFlag == true) { // Reads 8 bits
            Transport->Adaptation->SpliceCountdown                       = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        }
        if (Transport->Adaptation->TransportPrivateDataFlag == true) { // Reads up to 2056 bits
            Transport->Adaptation->TransportPrivateDataSize              = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
            for (uint8_t PrivateByte = 0; PrivateByte < Transport->Adaptation->TransportPrivateDataSize; PrivateByte++) {
                Transport->Adaptation->TransportPrivateData[PrivateByte] = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
            }
        }
        if (Transport->Adaptation->AdaptationFieldExtensionFlag == true) { // Reads 12 bits
            Transport->Adaptation->AdaptationFieldExtensionSize          = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
            Transport->Adaptation->LegalTimeWindowFlag                   = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Transport->Adaptation->PiecewiseRateFlag                     = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            Transport->Adaptation->SeamlessSpliceFlag                    = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
            AlignInput(BitB, 1);
            if (Transport->Adaptation->LegalTimeWindowFlag == true) { // Reads 16 bits
                Transport->Adaptation->LegalTimeWindowValidFlag          = ReadBits(MSByteFirst, LSBitFirst, BitB, 1);
                if (Transport->Adaptation->LegalTimeWindowValidFlag == true) {
                    Transport->Adaptation->LegalTimeWindowOffset         = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                }
            }
            if (Transport->Adaptation->PiecewiseRateFlag == true) { // Reads 24 bits
                BitBuffer_Skip(BitB, 2);
                Transport->Adaptation->PiecewiseRateFlag                 = ReadBits(MSByteFirst, LSBitFirst, BitB, 22);
            }
            if (Transport->Adaptation->SeamlessSpliceFlag == true) { // Reads 44 bits
                Transport->Adaptation->SpliceType                        = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
                uint8_t  DecodeTimeStamp1                                = ReadBits(MSByteFirst, LSBitFirst, BitB, 3);
                BitBuffer_Skip(BitB, 1); // marker_bit
                uint16_t DecodeTimeStamp2                                = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker_bit
                uint16_t DecodeTimeStamp3                                = ReadBits(MSByteFirst, LSBitFirst, BitB, 15);
                BitBuffer_Skip(BitB, 1); // marker_bit
                
                Transport->Adaptation->DecodeTimeStampNextAU             = DecodeTimeStamp1 << 30;
                Transport->Adaptation->DecodeTimeStampNextAU            += DecodeTimeStamp2 << 15;
                Transport->Adaptation->DecodeTimeStampNextAU            += DecodeTimeStamp3;
            }
            for (int i = 0; i < 184 - Transport->Adaptation->AdaptationFieldSize; i++) { // So, the minimum number of bytes to read is 188 - 284, aka -96 lol
                BitBuffer_Skip(BitB, 8); // Reserved
            }
        }
        for (int i = 0; i < 184 - Transport->Adaptation->AdaptationFieldSize; i++) {
            BitBuffer_Skip(BitB, 8); // Stuffing so 0b11111111, throw it away.
        }
    }

    static void ParseTransportStreamPacket(MPEG2TransportStream *Transport, BitBuffer *BitB, uint8_t *TransportStream, size_t TransportStreamSize) { // transport_packet
        Transport->Packet->SyncByte                              = ReadBits(MSByteFirst, LSBitFirst, BitB, 8);
        Transport->Packet->TransportErrorIndicator               = ReadBits(MSByteFirst, LSBitFirst, BitB, 1); // false
        Transport->Packet->StartOfPayloadIndicator               = ReadBits(MSByteFirst, LSBitFirst, BitB, 1); // true
        Transport->Packet->TransportPriorityIndicator            = ReadBits(MSByteFirst, LSBitFirst, BitB, 1); // false
        Transport->Packet->PID                                   = ReadBits(MSByteFirst, LSBitFirst, BitB, 13); // 0
        Transport->Packet->TransportScramblingControl            = ReadBits(MSByteFirst, LSBitFirst, BitB, 2); // 0
        Transport->Packet->AdaptationFieldControl                = ReadBits(MSByteFirst, LSBitFirst, BitB, 2); // 1
        Transport->Packet->ContinuityCounter                     = ReadBits(MSByteFirst, LSBitFirst, BitB, 4); // 0
        if (Transport->Packet->AdaptationFieldControl == 2 || Transport->Packet->AdaptationFieldControl == 3) {
            ParseTransportStreamAdaptionField(BitB, Transport);
        }
        if (Transport->Packet->AdaptationFieldControl == 1 || Transport->Packet->AdaptationFieldControl == 3) {
            for (int i = 0; i < 184; i++) {
                TransportStream[i] = ReadBits(MSByteFirst, LSBitFirst, BitB, 8); // data_byte, start copying data to the transport stream.
            }
        }
    }

    void DemuxMPEG2PESPackets(PacketizedElementaryStream *PESPacket, BitBuffer *BitB) {
		
    }

    void DemuxMPEG2ProgramStreamPacket(MPEG2ProgramStream *Program, BitBuffer *BitB) {

    }

    void DemuxMPEG2TransportStreamPacket(MPEG2TransportStream *Transport, BitBuffer *BitB) {

    }

#ifdef __cplusplus
}
#endif
