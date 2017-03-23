#include "../../../Dependencies/BitIO/libBitIO/include/BitIO.h"
#include "../../include/libMPEG2Stream.h"
#include "../../include/MPEG2StreamTypes.h"
#include "../../include/Demuxer/DemuxMPEG2Stream.h"

#ifdef __cplusplus
extern "C" {
#endif

    // This library (the needs to be renamed to CoreMPEG or MPEGSystem, etc) will not be real time, it just contains parsers and demuxers.
    // Users wishing to use this in an embedded real time app will need to write a wrapper around this library to handle the real time decoding and demuxing aspects that this library is concerned with.

    // Each elementary stream needs it's own PES packet.
    // A Program Stream can contain multiple elementary streams encapsulated into PES packets.
    // Transport Streams and Program streams are like brothers, they do the same things, but some are more capable than others.

    // Access Unit = A coded representation of a Presentation Unit. In audio, an AU = a frame. in video, an AU = a picture + all padding and metadata.
    // PacketID    = an int that identifies elementary streams (audio or video) in a program.
    // Program     = Elementary streams that are to be played synchronized with the same time base.

    // Transport Streams CAN CONTAIN PROGRAM STREAMS, OR ELEMENTARY STREAMS.

    // So, for Demuxing, the general idea is to accumulate PES packets until you've got a whole NAL or whateve?
    // Also, we need a way to identify the stream type

    enum MPEGTSConstants {
        MPEGStartCode                                         = 0x000001,// Both Program Stream and Transport Stream
        MPEGTSMaxPrivateData                                  = 256,
        PESPacketCRCPolynomial                                = 0x8811,
        MPEGTSSystemHeaderStartCode                           = 0x0,// system_header_start_code
    } MPEGTSConstants;



    MPEG2TransportStream *InitMPEGTransportStream(void) {
        MPEG2TransportStream *TransportStream                 = calloc(sizeof(MPEG2TransportStream), 1);
        TransportStream->Packet                               = calloc(sizeof(TransportStreamPacket), 1);
        TransportStream->Adaptation                           = calloc(sizeof(TSAdaptationField), 1);
        TransportStream->PES                                  = calloc(sizeof(PacketizedElementaryStream), 1);
        TransportStream->Program                              = calloc(sizeof(ProgramAssociatedSection), 1);
        TransportStream->Condition                            = calloc(sizeof(ConditionalAccessSection), 1);
        return TransportStream;
    }

    MPEG2ProgramStream *InitMPEGProgramStream(void) {
        MPEG2ProgramStream *ProgramStream                     = calloc(sizeof(MPEG2ProgramStream), 1);
        ProgramStream->PS                                     = calloc(sizeof(ProgramStream), 1);
        ProgramStream->PES                                    = calloc(sizeof(PacketizedElementaryStream), 1);
        return ProgramStream;
    }

    static void ParseConditionalAccessDescriptor(BitInput *BitI, MPEG2TransportStream *Stream) { // CA_descriptor
        int N                                                 = 0;// TODO: what is N?
        uint8_t  DescriptorTag                                = ReadBits(BitI, 8, true);// descriptor_tag
        uint8_t  DescriptorSize                               = ReadBits(BitI, 8, true);// descriptor_length
        uint16_t ConditionalAccessID                          = ReadBits(BitI, 16, true);// CA_system_ID
        SkipBits(BitI, 3); // reserved
        uint16_t  ConditionalAccessPID                        = ReadBits(BitI, 13, true);
        for (int i                                            = 0; i < N; i++) {
            SkipBits(BitI, 8); // private_data_byte
        }
    }

    static void ParseConditionalAccessSection(BitInput *BitI, MPEG2TransportStream *Stream) { // CA_section
        int N                                                 = 0;// TODO: find out what the hell N is
        Stream->Condition->TableID                            = ReadBits(BitI, 8, true);
        Stream->Condition->SectionSyntaxIndicator             = ReadBits(BitI, 1, true);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Stream->Condition->SectionSize                        = ReadBits(BitI, 12, true);
        SkipBits(BitI, 18);
        Stream->Condition->VersionNum                         = ReadBits(BitI, 5, true);
        Stream->Condition->CurrentNextIndicator               = ReadBits(BitI, 1, true);
        Stream->Condition->SectionNumber                      = ReadBits(BitI, 8, true);
        Stream->Condition->LastSectionNumber                  = ReadBits(BitI, 8, true);
        for (int i                                            = 0; i < N; i++) {
            TSParseConditionalAccessDescriptor(BitI, Stream);
        }
        Stream->Condition->ConditionCRC32                     = ReadBits(BitI, 32, true);
    }

    static void ParseProgramAssociationTable(BitInput *BitI, MPEG2TransportStream *Stream) { // program_association_section
        Stream->Program->TableID                              = ReadBits(BitI, 8, true);
        Stream->Program->SectionSyntaxIndicator               = ReadBits(BitI, 1, true);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Stream->Program->SectionSize                          = ReadBits(BitI, 12, true);
        Stream->Program->TransportStreamID                    = ReadBits(BitI, 16, true);
        SkipBits(BitI, 2); // Reserved
        Stream->Program->VersionNum                           = ReadBits(BitI, 5, true);
        Stream->Program->CurrentNextIndicator                 = ReadBits(BitI, 1, true);
        Stream->Program->SectionNumber                        = ReadBits(BitI, 8, true);
        Stream->Program->LastSectionNumber                    = ReadBits(BitI, 8, true);
        Stream->Program->ProgramNumber                        = ReadBits(BitI, 16, true);
        Stream->Program->NetworkPID                           = ReadBits(BitI, 13, true);
        Stream->Program->ProgramMapPID                        = ReadBits(BitI, 13, true);
        Stream->Program->ProgramCRC32                         = ReadBits(BitI, 32, true);
    }

    static void ParsePackHeader(BitInput *BitI, MPEG2ProgramStream *Stream) { // pack_header
        Stream->PS->PackStartCode                             = ReadBits(BitI, 32, true);
        SkipBits(BitI, 2); // 01
        Stream->PS->SystemClockRefBase1                       = ReadBits(BitI, 3, true);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase2                       = ReadBits(BitI, 15, true);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase3                       = ReadBits(BitI, 15, true);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase                        = Stream->PS->SystemClockRefBase1 << 30;
        Stream->PS->SystemClockRefBase                        += Stream->PS->SystemClockRefBase2 << 15;
        Stream->PS->SystemClockRefBase                        += Stream->PS->SystemClockRefBase3;

        Stream->PS->SystemClockRefExtension                   = ReadBits(BitI, 9, true);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->ProgramMuxRate                            = ReadBits(BitI, 22, true);
        SkipBits(BitI, 7); // marker_bit && reserved
        Stream->PS->PackStuffingSize                          = ReadBits(BitI, 3, true);
        SkipBits(BitI, Bytes2Bits(Stream->PS->PackStuffingSize));
        if (PeekBits(BitI, 0, true) == MPEGTSSystemHeaderStartCode) {
            // system_header();
        }
    }

    static void ParsePESPacket(BitInput *BitI, PacketizedElementaryStream *Stream) { // PES_packet
        int N3                                                = 0, N2 = 0, N1 = 0;// FIXME: WTF IS N3, N2, and N1?
        Stream->PacketStartCodePrefix                         = ReadBits(BitI, 24, true);
        Stream->StreamID                                      = ReadBits(BitI, 8, true);// 13
        Stream->PESPacketSize                                 = ReadBits(BitI, 16, true);//
        if (Stream->StreamID != ProgramStreamFolder &&
            Stream->StreamID != AnnexA_DSMCCStream &&
            Stream->StreamID != ProgramStreamMap &&
            Stream->StreamID != PrivateStream2 &&
            Stream->StreamID != PaddingStream &&
            Stream->StreamID != ECMStream &&
            Stream->StreamID != EMMStream &&
            Stream->StreamID != TypeEStream) {
            SkipBits(BitI, 2);
        Stream->PESScramblingControl                          = ReadBits(BitI, 2, true);
        Stream->PESPriority                                   = ReadBits(BitI, 1, true);
        Stream->AlignmentIndicator                            = ReadBits(BitI, 1, true);
        Stream->CopyrightIndicator                            = ReadBits(BitI, 1, true);
        Stream->OriginalOrCopy                                = ReadBits(BitI, 1, true);
        Stream->PTSDTSFlags                                   = ReadBits(BitI, 2, true);
        Stream->ESCRFlag                                      = ReadBits(BitI, 1, true);
        Stream->ESRateFlag                                    = ReadBits(BitI, 1, true);
        Stream->DSMTrickModeFlag                              = ReadBits(BitI, 1, true);
        Stream->AdditionalCopyInfoFlag                        = ReadBits(BitI, 1, true);
        Stream->PESCRCFlag                                    = ReadBits(BitI, 1, true);
        Stream->PESExtensionFlag                              = ReadBits(BitI, 1, true);
        Stream->PESHeaderSize                                 = ReadBits(BitI, 8, true);
            if (Stream->PTSDTSFlags == 2) {
                SkipBits(BitI, 4);
        uint8_t  PTS1                                         = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t PTS2                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t PTS3                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit

        Stream->PTS                                           = PTS1 << 30;
        Stream->PTS                                           += PTS2 << 15;
        Stream->PTS                                           += PTS3;
            }
            if (Stream->PTSDTSFlags == 3) {
                SkipBits(BitI, 4);
        uint8_t  PTS1                                         = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t PTS2                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t PTS3                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit

        Stream->PTS                                           = PTS1 << 30;
        Stream->PTS                                           += PTS2 << 15;
        Stream->PTS                                           += PTS3;

                SkipBits(BitI, 4);
        uint8_t  DTS1                                         = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t DTS2                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t DTS3                                         = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit

        Stream->DTS                                           = DTS1 << 30;
        Stream->DTS                                           += DTS2 << 15;
        Stream->DTS                                           += DTS3;
            }
            if (Stream->ESCRFlag == true) {
                SkipBits(BitI, 2);
        uint8_t  ESCR1                                        = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t ESCR2                                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit
        uint16_t ESCR3                                        = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker bit

        Stream->ESCR                                          = ESCR1 << 30;
        Stream->ESCR                                          += ESCR2 << 15;
        Stream->ESCR                                          += ESCR3;
            }
            if (Stream->ESRateFlag == true) {
                SkipBits(BitI, 1);
        Stream->ESRate                                        = ReadBits(BitI, 22, true);
                SkipBits(BitI, 1);
            }
            if (Stream->DSMTrickModeFlag == true) {
        Stream->TrickModeControl                              = ReadBits(BitI, 3, true);
                if (Stream->TrickModeControl == FastForward) {
        Stream->FieldID                                       = ReadBits(BitI, 2, true);
        Stream->IntraSliceRefresh                             = ReadBits(BitI, 1, true);
        Stream->FrequencyTruncation                           = ReadBits(BitI, 2, true);
                } else if (Stream->TrickModeControl == SlowMotion) {
        Stream->RepetitionControl                             = ReadBits(BitI, 5, true);
                } else if (Stream->TrickModeControl == FreezeFrame) {
        Stream->FieldID                                       = ReadBits(BitI, 2, true);
                    SkipBits(BitI, 3);
                } else if (Stream->TrickModeControl == FastRewind) {
        Stream->FieldID                                       = ReadBits(BitI, 2, true);
        Stream->IntraSliceRefresh                             = ReadBits(BitI, 1, true);
        Stream->FrequencyTruncation                           = ReadBits(BitI, 2, true);
                } else if (Stream->TrickModeControl == SlowRewind) {
        Stream->RepetitionControl                             = ReadBits(BitI, 5, true);
                } else {
                    SkipBits(BitI, 5);
                }
            }
            if (Stream->AdditionalCopyInfoFlag == true) {
                SkipBits(BitI, 1);
        Stream->AdditionalCopyInfo                            = ReadBits(BitI, 7, true);
            }
            if (Stream->PESCRCFlag == true) {
        Stream->PreviousPESPacketCRC                          = ReadBits(BitI, 16, true);
            }
            if (Stream->PESExtensionFlag == true) {
        Stream->PESPrivateDataFlag                            = ReadBits(BitI, 1, true);
        Stream->PackHeaderFieldFlag                           = ReadBits(BitI, 1, true);
        Stream->ProgramPacketSeqCounterFlag                   = ReadBits(BitI, 1, true);
        Stream->PSTDBufferFlag                                = ReadBits(BitI, 1, true);
        Stream->PESExtensionFlag2                             = ReadBits(BitI, 1, true);
                if (Stream->PESPrivateDataFlag == true) {
                    SkipBits(BitI, 128);
                }
                if (Stream->PackHeaderFieldFlag == true) {
        Stream->PackFieldSize                                 = ReadBits(BitI, 8, true);
                    pack_header();
                }
                if (Stream->ProgramPacketSeqCounterFlag == true) {
                    SkipBits(BitI, 1);
        Stream->ProgramPacketSeqCounter                       = ReadBits(BitI, 7, true);
                    SkipBits(BitI, 1);
        Stream->MPEGVersionIdentifier                         = ReadBits(BitI, 1, true);
        Stream->OriginalStuffSize                             = ReadBits(BitI, 6, true);
                }
                if (Stream->PSTDBufferFlag == true) {
                    SkipBits(BitI, 2);
        Stream->PSTDBufferScale                               = ReadBits(BitI, 1, true);
        Stream->PSTDBufferSize                                = ReadBits(BitI, 13, true);
                }
                if (Stream->PESExtensionFlag2 == true) {
                    SkipBits(BitI, 1);
        Stream->PESExtensionFieldSize                         = ReadBits(BitI, 7, true);
        Stream->StreamIDExtensionFlag                         = ReadBits(BitI, 1, true);
                    if (Stream->StreamIDExtensionFlag == false) {
        Stream->StreamIDExtension                             = ReadBits(BitI, 7, true);
                    } else {
                        SkipBits(BitI, 6);
                        // tref_extension_flag
        Stream->TREFFieldPresentFlag                          = ReadBits(BitI, 1, true);
                        if (Stream->TREFFieldPresentFlag == false) {
                            SkipBits(BitI, 4);
        uint8_t  TREF1                                        = ReadBits(BitI, 3, true);
                            SkipBits(BitI, 1); // marker bit
        uint16_t TREF2                                        = ReadBits(BitI, 15, true);
                            SkipBits(BitI, 1); // marker bit
        uint16_t TREF3                                        = ReadBits(BitI, 15, true);
                            SkipBits(BitI, 1); // marker bit

        Stream->TREF                                          = TREF1 << 30;
        Stream->TREF                                          = TREF2 << 15;
        Stream->TREF                                          = TREF3;
                        }
                    }
        for (int i                                            = 0; i < N3; i++) {
                        SkipBits(BitI, 8); // Reserved
                    }
                }
        for (int i                                            = 0; i < Stream->PESExtensionFieldSize; i++) {
                    SkipBits(BitI, 8);
                }
            }
        for (int i                                            = 0; i < N1; i++) {
                SkipBits(BitI, 8); // stuffing_byte
            }
        for (int i                                            = 0; i < N2; i++) {
                SkipBits(BitI, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == ProgramStreamFolder ||
                   Stream->StreamID == AnnexA_DSMCCStream ||
                   Stream->StreamID == ProgramStreamMap ||
                   Stream->StreamID == PrivateStream2 ||
                   Stream->StreamID == ECMStream ||
                   Stream->StreamID == EMMStream ||
                   Stream->StreamID == TypeEStream) {
        for (int i                                            = 0; i < Stream->PESPacketSize; i++) {
                SkipBits(BitI, 8); // PES_packet_data_byte
            }
        } else if (Stream->StreamID == PaddingStream) {
        for (int i                                            = 0; i < Stream->PESPacketSize; i++) {
                SkipBits(BitI, 8); // padding_byte
            }
        }
    }

    static void TSParseAdaptionField(BitInput *BitI, MPEG2TransportStream *Stream) { // adaptation_field
        int N                                                 = 0;// FIXME: WHAT THE FUCK IS N?
        Stream->Adaptation->AdaptationFieldSize               = ReadBits(BitI, 8, true);
        Stream->Adaptation->DiscontinuityIndicator            = ReadBits(BitI, 1, true);
        Stream->Adaptation->RandomAccessIndicator             = ReadBits(BitI, 1, true);
        Stream->Adaptation->ElementaryStreamPriorityIndicator = ReadBits(BitI, 1, true);
        Stream->Adaptation->PCRFlag                           = ReadBits(BitI, 1, true);
        Stream->Adaptation->OPCRFlag                          = ReadBits(BitI, 1, true);
        Stream->Adaptation->SlicingPointFlag                  = ReadBits(BitI, 1, true);
        Stream->Adaptation->TransportPrivateDataFlag          = ReadBits(BitI, 1, true);
        Stream->Adaptation->AdaptationFieldExtensionFlag      = ReadBits(BitI, 1, true);
        if (Stream->Adaptation->PCRFlag == true) {
        Stream->Adaptation->ProgramClockReferenceBase         = ReadBits(BitI, 33, true);
            SkipBits(BitI, 6);
        Stream->Adaptation->ProgramClockReferenceExtension    = ReadBits(BitI, 9, true);
        }
        if (Stream->Adaptation->OPCRFlag == true) {
        Stream->Adaptation->OriginalProgramClockRefBase       = ReadBits(BitI, 33, true);
            SkipBits(BitI, 6);
        Stream->Adaptation->OriginalProgramClockRefExt        = ReadBits(BitI, 9, true);
        }
        if (Stream->Adaptation->SlicingPointFlag == true) {
        Stream->Adaptation->SpliceCountdown                   = ReadBits(BitI, 8, true);
        }
        if (Stream->Adaptation->TransportPrivateDataFlag == true) {
        Stream->Adaptation->TransportPrivateDataSize          = ReadBits(BitI, 8, true);
        for (uint8_t PrivateByte                              = 0; PrivateByte < Stream->Adaptation->TransportPrivateDataSize; PrivateByte++) {
        Stream->Adaptation->TransportPrivateData[PrivateByte] = ReadBits(BitI, 8, true);
            }
        }
        if (Stream->Adaptation->AdaptationFieldExtensionFlag == true) {
        Stream->Adaptation->AdaptationFieldExtensionSize      = ReadBits(BitI, 8, true);
        Stream->Adaptation->LegalTimeWindowFlag               = ReadBits(BitI, 1, true);
        Stream->Adaptation->PiecewiseRateFlag                 = ReadBits(BitI, 1, true);
        Stream->Adaptation->SeamlessSpliceFlag                = ReadBits(BitI, 1, true);
            AlignInput(BitI, 1);
            if (Stream->Adaptation->LegalTimeWindowFlag == true) {
        Stream->Adaptation->LegalTimeWindowValidFlag          = ReadBits(BitI, 1, true);
                if (Stream->Adaptation->LegalTimeWindowValidFlag == true) {
        Stream->Adaptation->LegalTimeWindowOffset             = ReadBits(BitI, 15, true);
                }
            }
            if (Stream->Adaptation->PiecewiseRateFlag == true) {
                SkipBits(BitI, 2);
        Stream->Adaptation->PiecewiseRateFlag                 = ReadBits(BitI, 22, true);
            }
            if (Stream->Adaptation->SeamlessSpliceFlag == true) {
        Stream->Adaptation->SpliceType                        = ReadBits(BitI, 8, true);
        uint8_t  DecodeTimeStamp1                             = ReadBits(BitI, 3, true);
                SkipBits(BitI, 1); // marker_bit
        uint16_t DecodeTimeStamp2                             = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker_bit
        uint16_t DecodeTimeStamp3                             = ReadBits(BitI, 15, true);
                SkipBits(BitI, 1); // marker_bit

        Stream->Adaptation->DecodeTimeStampNextAU             = DecodeTimeStamp1 << 30;
        Stream->Adaptation->DecodeTimeStampNextAU             += DecodeTimeStamp2 << 15;
        Stream->Adaptation->DecodeTimeStampNextAU             += DecodeTimeStamp3;
            }
        for (int i                                            = 0; i < N; i++) {
                SkipBits(BitI, 8);
            }
        }
        for (int i                                            = 0; i < N; i++) {
            SkipBits(BitI, 8);
        }
    }

    static void ParseTransportStreamPacket(BitInput *BitI, MPEG2TransportStream *Stream) { // transport_packet
        Stream->Packet->SyncByte                              = ReadBits(BitI, 8, true);
        Stream->Packet->TransportErrorIndicator               = ReadBits(BitI, 1, true);
        Stream->Packet->StartOfPayloadIndicator               = ReadBits(BitI, 1, true);
        Stream->Packet->TransportPriorityIndicator            = ReadBits(BitI, 1, true);
        Stream->Packet->PID                                   = ReadBits(BitI, 13, true);
        Stream->Packet->TransportScramblingControl            = ReadBits(BitI, 2, true);
        Stream->Packet->AdaptationFieldControl                = ReadBits(BitI, 2, true);
        Stream->Packet->ContinuityCounter                     = ReadBits(BitI, 4, true);
        if (Stream->Packet->AdaptationFieldControl == 2 || Stream->Packet->AdaptationFieldControl == 3) {
            TSParseAdaptionField(BitI, Stream);
        }
        if (Stream->Packet->AdaptationFieldControl == 1 || Stream->Packet->AdaptationFieldControl == 3) {
        int N                                                 = 0;// FIXME: I just did this so it would compile what is N?
        for (int i                                            = 0; i < N; i++) {
                SkipBits(BitI, 8); // data_byte
            }
        }
    }

    void DemuxMPEG2PESPackets(BitInput *BitI, PacketizedElementaryStream *PESPacket) {

    }

    void DemuxMPEG2ProgramStream(BitInput *BitI, MPEG2ProgramStream *Stream) {

    }

    void DemuxMPEG2TransportStream(BitInput *BitI, MPEG2TransportStream *Stream) {

    }

#ifdef __cplusplus
}
#endif