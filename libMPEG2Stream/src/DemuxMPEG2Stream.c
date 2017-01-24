#include "../include/libMPEG2Stream.h"

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
    
    extern enum MPEGTSConstants {
        MPEGStartCode               = 0x000001, // Both Program Stream and Transport Stream
        MPEGTSMaxPrivateData        = 256,
        PESPacketCRCPolynomial      = 0x8811,
        MPEGTSSystemHeaderStartCode = 0x0, // system_header_start_code
    } MPEGTSConstants;
    
    typedef struct TransportStream { // Most basic level, consists of CRCs and metadata describing the Program Stream
                                     // There are Transport stream Ops and PES ops.
        
    } TransportStream;
    
    typedef struct ProgramStream { // Can be the most basic level as well as the TransportStream
        uint8_t  PackStartCode;                        // pack_start_code
        uint8_t  SystemClockRefBase1:3;                // system_clock_reference_base [32..30]
        uint16_t SystemClockRefBase2:15;               // system_clock_reference_base [29..15]
        uint16_t SystemClockRefBase3:15;               // system_clock_reference_base [14..0]
        uint64_t SystemClockRefBase:33;                // system_clock_reference_base
        uint16_t SystemClockRefExtension:9;            // system_clock_reference_extension
        uint32_t ProgramMuxRate:22;                    // program_mux_rate
        uint8_t  PackStuffingSize:3;                   // pack_stuffing_length
    } ProgramStream;
    
    typedef struct PacketizedElementaryStream { // This is contains the actual video and audio streams, it differs from them by offering a very thin layer on top to split the NALs and SliceGroups into packets.
        int32_t  PacketStartCodePrefix:24;            // packet_start_code_prefix
        uint8_t  StreamID;                            // stream_id
        uint16_t PESPacketSize;                       // PES_packet_length
        uint8_t  PESScramblingControl;                // PES_scrambling_control
        bool     PESPriority:1;                       // PES_priority
        bool     AlignmentIndicator:1;                // data_alignment_indicator
        bool     CopyrightIndicator:1;                // copyright
        bool     OriginalOrCopy:1;                    // original_or_copy
        uint8_t  PTSDTSFlags:2;                       // PTS_DTS_flags
        bool     ESCRFlag:1;                          // ESCR_flag
        bool     ESRateFlag:1;                        // ES_rate_flag
        bool     DSMTrickModeFlag:1;                  // DSM_trick_mode_flag
        bool     AdditionalCopyInfoFlag:1;            // additional_copy_info_flag
        bool     PESCRCFlag:1;                        // PES_CRC_flag
        bool     PESExtensionFlag:1;                  // PES_extension_flag
        uint8_t  PESHeaderSize;                       // PES_header_data_length
        uint64_t PTS:33;                              // PTS
        uint64_t DTS:33;                              // DTS
        uint64_t ESCR:33;                             // ESCR
        uint32_t ESRate:22;                           // ES_rate
        uint8_t  TrickModeControl:3;                  // trick_mode_control
        uint8_t  FieldID:2;                           // field_id
        bool     IntraSliceRefresh:1;                 // intra_slice_refresh
        uint8_t  FrequencyTruncation:2;               // frequency_truncation
        uint8_t  RepetitionControl:5;                 // rep_cntrl
        uint8_t  AdditionalCopyInfo:7;                // additional_copy_info
        uint16_t PreviousPESPacketCRC;                // previous_PES_packet_CRC
        bool     PESPrivateDataFlag:1;                // PES_private_data_flag
        bool     PackHeaderFieldFlag:1;               // pack_header_field_flag
        bool     ProgramPacketSeqCounterFlag:1;       // program_packet_sequence_counter_flag
        bool     PSTDBufferFlag:1;                    // P-STD_buffer_flag
        bool     PESExtensionFlag2:1;                 // PES_extension_flag_2
        uint8_t  PackFieldSize;                       // pack_field_length
        uint8_t  ProgramPacketSeqCounter:7;           // program_packet_sequence_counter
        bool     MPEGVersionIdentifier:1;             // MPEG1_MPEG2_identifier
        uint8_t  OriginalStuffSize:6;                 // original_stuff_length
        uint8_t  PSTDBufferScale:1;                   // P-STD_buffer_scale
        uint16_t PSTDBufferSize:13;                   // P-STD_buffer_size
        uint8_t  PESExtensionFieldSize:7;             // PES_extension_field_length
        bool     StreamIDExtensionFlag:1;             // stream_id_extension_flag
        uint8_t  StreamIDExtension:7;                 // stream_id_extension
        bool     TREFFieldPresentFlag:1;              // tref_extension_flag
        uint64_t TREF:33;                             // TREF
    } PacketizedElementaryStream;
    
    typedef struct ConditionalAccessSection {
        uint8_t  TableID;                             // table_id
        bool     SectionSyntaxIndicator:1;            // section_syntax_indicator
        uint16_t SectionSize:12;                      // section_length
        uint8_t  VersionNum:5;                        // version_number
        bool     CurrentNextIndicator:1;              // current_next_indicator
        uint8_t  SectionNumber;                       // section_number
        uint8_t  LastSectionNumber;                   // last_section_number
        uint32_t ConditionCRC32;                      // CRC_32
    } ConditionalAccessSection;
    
    typedef struct ProgramAssociatedSection {
        uint8_t  TableID;                             // table_id
        bool     SectionSyntaxIndicator:1;            // section_syntax_indicator
        uint16_t SectionSize:12;                      // section_length
        uint16_t TransportStreamID;                   // transport_stream_id
        uint8_t  VersionNum:5;                        // version_number
        bool     CurrentNextIndicator:1;              // current_next_indicator
        uint8_t  SectionNumber;                       // section_number
        uint8_t  LastSectionNumber;                   // last_section_number
        uint16_t ProgramNumber;                       // program_number
        uint16_t NetworkPID:13;                       // network_PID
        uint16_t ProgramMapPID:13;                    // program_map_PID
        uint32_t ProgramCRC32;                        // CRC_32
    } ProgramAssociatedSection;
    
    typedef struct TransportStreamPacket {
        int8_t   SyncByte;                            // sync_byte
        bool     TransportErrorIndicator:1;           // transport_error_indicator
        bool     StartOfPayloadIndicator:1;           // payload_unit_start_indicator
        bool     TransportPriorityIndicator:1;        // transport_priority
        uint16_t PID:13;                              // PID
        int8_t   TransportScramblingControl:2;        // transport_scrambling_control
        int8_t   AdaptationFieldControl:2;            // adaptation_field_control
        uint8_t  ContinuityCounter:4;                 // continuity_counter
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
        uint64_t ProgramClockReferenceBase:33;        // program_clock_reference_base
        uint16_t ProgramClockReferenceExtension:9;    // program_clock_reference_extension
        uint64_t OriginalProgramClockRefBase:33;      // original_program_clock_reference_base
        uint16_t OriginalProgramClockRefExt:9;        // original_program_clock_reference_extension
        int8_t   SpliceCountdown;                     // splice_countdown
        uint8_t  TransportPrivateDataSize;            // transport_private_data_length
        uint8_t  TransportPrivateData[MPEGTSMaxPrivateData];           // private_data_byte
        uint8_t  AdaptationFieldExtensionSize;        // adaptation_field_extension_length
        bool     LegalTimeWindowFlag:1;               // ltw_flag
        bool     PiecewiseRateFlag:1;                 // piecewise_rate_flag
        bool     SeamlessSpliceFlag:1;                // seamless_splice_flag
        bool     LegalTimeWindowValidFlag:1;          // ltw_valid_flag
        uint16_t LegalTimeWindowOffset:15;            // ltw_offset
        uint32_t PiecewiseRate:22;                    // piecewise_rate
        uint8_t  SpliceType:4;                        // Splice_type
        uint64_t DecodeTimeStampNextAU:33;            // DTS_next_AU
    } TSAdaptationField;
    
    MPEG2TransportStream *InitMPEGTransportStream(void) {
        MPEG2TransportStream *TransportStream = calloc(sizeof(MPEG2TransportStream), 1);
        TransportStream->Packet              = calloc(sizeof(TransportStreamPacket), 1);
        TransportStream->Adaptation          = calloc(sizeof(TSAdaptationField), 1);
        TransportStream->PES                 = calloc(sizeof(PacketizedElementaryStream), 1);
        TransportStream->Program             = calloc(sizeof(ProgramAssociatedSection), 1);
        TransportStream->Condition           = calloc(sizeof(ConditionalAccessSection), 1);
        return TransportStream;
    }
    
    MPEG2ProgramStream *InitMPEGProgramStream(void) {
        MPEG2ProgramStream *ProgramStream     = calloc(sizeof(MPEG2ProgramStream), 1);
        ProgramStream->PS                    = calloc(sizeof(ProgramStream), 1);
        ProgramStream->PES                   = calloc(sizeof(PacketizedElementaryStream), 1);
        return ProgramStream;
    }
    
    static void ParseConditionalAccessDescriptor(BitInput *BitI, MPEG2TransportStream *Stream) { // CA_descriptor
        int N = 0; // TODO: what is N?
        uint8_t  DescriptorTag         = ReadBits(BitI, 8);  // descriptor_tag
        uint8_t  DescriptorSize        = ReadBits(BitI, 8);  // descriptor_length
        uint16_t ConditionalAccessID   = ReadBits(BitI, 16); // CA_system_ID
        SkipBits(BitI, 3); // reserved
        uint16_t  ConditionalAccessPID = ReadBits(BitI, 13);
        for (int i = 0; i < N; i++) {
            SkipBits(BitI, 8); // private_data_byte
        }
    }
    
    static void ParseConditionalAccessSection(BitInput *BitI, MPEG2TransportStream *Stream) { // CA_section
        int N = 0; // TODO: find out what the hell N is
        Stream->Condition->TableID                = ReadBits(BitI, 8);
        Stream->Condition->SectionSyntaxIndicator = ReadBits(BitI, 1);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Stream->Condition->SectionSize            = ReadBits(BitI, 12);
        SkipBits(BitI, 18);
        Stream->Condition->VersionNum             = ReadBits(BitI, 5);
        Stream->Condition->CurrentNextIndicator   = ReadBits(BitI, 1);
        Stream->Condition->SectionNumber          = ReadBits(BitI, 8);
        Stream->Condition->LastSectionNumber      = ReadBits(BitI, 8);
        for (int i = 0; i < N; i++) {
            TSParseConditionalAccessDescriptor(BitI, Stream);
        }
        Stream->Condition->ConditionCRC32         = ReadBits(BitI, 32);
    }
    
    static void ParseProgramAssociationTable(BitInput *BitI, MPEG2TransportStream *Stream) { // program_association_section
        Stream->Program->TableID                = ReadBits(BitI, 8);
        Stream->Program->SectionSyntaxIndicator = ReadBits(BitI, 1);
        SkipBits(BitI, 3); // "0" + 2 bits reserved.
        Stream->Program->SectionSize            = ReadBits(BitI, 12);
        Stream->Program->TransportStreamID      = ReadBits(BitI, 16);
        SkipBits(BitI, 2); // Reserved
        Stream->Program->VersionNum             = ReadBits(BitI, 5);
        Stream->Program->CurrentNextIndicator   = ReadBits(BitI, 1);
        Stream->Program->SectionNumber          = ReadBits(BitI, 8);
        Stream->Program->LastSectionNumber      = ReadBits(BitI, 8);
        Stream->Program->ProgramNumber          = ReadBits(BitI, 16);
        Stream->Program->NetworkPID             = ReadBits(BitI, 13);
        Stream->Program->ProgramMapPID          = ReadBits(BitI, 13);
        Stream->Program->ProgramCRC32           = ReadBits(BitI, 32);
    }
    
    static void ParsePackHeader(BitInput *BitI, MPEG2ProgramStream *Stream) { // pack_header
        Stream->PS->PackStartCode = ReadBits(BitI, 32);
        SkipBits(BitI, 2); // 01
        Stream->PS->SystemClockRefBase1 = ReadBits(BitI, 3);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase2 = ReadBits(BitI, 15);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase3 = ReadBits(BitI, 15);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->SystemClockRefBase   = Stream->PS->SystemClockRefBase1 << 30;
        Stream->PS->SystemClockRefBase  += Stream->PS->SystemClockRefBase2 << 15;
        Stream->PS->SystemClockRefBase  += Stream->PS->SystemClockRefBase3;
        
        Stream->PS->SystemClockRefExtension = ReadBits(BitI, 9);
        SkipBits(BitI, 1); // marker_bit
        Stream->PS->ProgramMuxRate          = ReadBits(BitI, 22);
        SkipBits(BitI, 7); // marker_bit && reserved
        Stream->PS->PackStuffingSize        = ReadBits(BitI, 3);
        SkipBits(BitI, Bytes2Bits(Stream->PS->PackStuffingSize));
        if (PeekBits(BitI, 0) == MPEGTSSystemHeaderStartCode) {
            // system_header();
        }
    }
    
    static void ParsePESPacket(BitInput *BitI, PacketizedElementaryStream *Stream) { // PES_packet
        int N3 = 0, N2 = 0, N1 = 0; // FIXME: WTF IS N3, N2, and N1?
        Stream->PacketStartCodePrefix                     = ReadBits(BitI, 24);
        Stream->StreamID                                  = ReadBits(BitI, 8); // 13
        Stream->PESPacketSize                             = ReadBits(BitI, 16); //
        if (Stream->StreamID != ProgramStreamFolder &&
            Stream->StreamID != AnnexA_DSMCCStream &&
            Stream->StreamID != ProgramStreamMap &&
            Stream->StreamID != PrivateStream2 &&
            Stream->StreamID != PaddingStream &&
            Stream->StreamID != ECMStream &&
            Stream->StreamID != EMMStream &&
            Stream->StreamID != TypeEStream) {
            SkipBits(BitI, 2);
            Stream->PESScramblingControl   = ReadBits(BitI, 2);
            Stream->PESPriority            = ReadBits(BitI, 1);
            Stream->AlignmentIndicator     = ReadBits(BitI, 1);
            Stream->CopyrightIndicator     = ReadBits(BitI, 1);
            Stream->OriginalOrCopy         = ReadBits(BitI, 1);
            Stream->PTSDTSFlags            = ReadBits(BitI, 2);
            Stream->ESCRFlag               = ReadBits(BitI, 1);
            Stream->ESRateFlag             = ReadBits(BitI, 1);
            Stream->DSMTrickModeFlag       = ReadBits(BitI, 1);
            Stream->AdditionalCopyInfoFlag = ReadBits(BitI, 1);
            Stream->PESCRCFlag             = ReadBits(BitI, 1);
            Stream->PESExtensionFlag       = ReadBits(BitI, 1);
            Stream->PESHeaderSize          = ReadBits(BitI, 8);
            if (Stream->PTSDTSFlags == 2) {
                SkipBits(BitI, 4);
                uint8_t  PTS1 = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS2 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS3 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                
                Stream->PTS  = PTS1 << 30;
                Stream->PTS += PTS2 << 15;
                Stream->PTS += PTS3;
            }
            if (Stream->PTSDTSFlags == 3) {
                SkipBits(BitI, 4);
                uint8_t  PTS1 = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS2 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                uint16_t PTS3 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                
                Stream->PTS  = PTS1 << 30;
                Stream->PTS += PTS2 << 15;
                Stream->PTS += PTS3;
                
                SkipBits(BitI, 4);
                uint8_t  DTS1 = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker bit
                uint16_t DTS2 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                uint16_t DTS3 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                
                Stream->DTS  = DTS1 << 30;
                Stream->DTS += DTS2 << 15;
                Stream->DTS += DTS3;
            }
            if (Stream->ESCRFlag == true) {
                SkipBits(BitI, 2);
                uint8_t  ESCR1 = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker bit
                uint16_t ESCR2 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                uint16_t ESCR3 = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker bit
                
                Stream->ESCR  = ESCR1 << 30;
                Stream->ESCR += ESCR2 << 15;
                Stream->ESCR += ESCR3;
            }
            if (Stream->ESRateFlag == true) {
                SkipBits(BitI, 1);
                Stream->ESRate = ReadBits(BitI, 22);
                SkipBits(BitI, 1);
            }
            if (Stream->DSMTrickModeFlag == true) {
                Stream->TrickModeControl        = ReadBits(BitI, 3);
                if (Stream->TrickModeControl == FastForward) {
                    Stream->FieldID             = ReadBits(BitI, 2);
                    Stream->IntraSliceRefresh   = ReadBits(BitI, 1);
                    Stream->FrequencyTruncation = ReadBits(BitI, 2);
                } else if (Stream->TrickModeControl == SlowMotion) {
                    Stream->RepetitionControl   = ReadBits(BitI, 5);
                } else if (Stream->TrickModeControl == FreezeFrame) {
                    Stream->FieldID             = ReadBits(BitI, 2);
                    SkipBits(BitI, 3);
                } else if (Stream->TrickModeControl == FastRewind) {
                    Stream->FieldID             = ReadBits(BitI, 2);
                    Stream->IntraSliceRefresh   = ReadBits(BitI, 1);
                    Stream->FrequencyTruncation = ReadBits(BitI, 2);
                } else if (Stream->TrickModeControl == SlowRewind) {
                    Stream->RepetitionControl   = ReadBits(BitI, 5);
                } else {
                    SkipBits(BitI, 5);
                }
            }
            if (Stream->AdditionalCopyInfoFlag == true) {
                SkipBits(BitI, 1);
                Stream->AdditionalCopyInfo      = ReadBits(BitI, 7);
            }
            if (Stream->PESCRCFlag == true) {
                Stream->PreviousPESPacketCRC    = ReadBits(BitI, 16);
            }
            if (Stream->PESExtensionFlag == true) {
                Stream->PESPrivateDataFlag          = ReadBits(BitI, 1);
                Stream->PackHeaderFieldFlag         = ReadBits(BitI, 1);
                Stream->ProgramPacketSeqCounterFlag = ReadBits(BitI, 1);
                Stream->PSTDBufferFlag              = ReadBits(BitI, 1);
                Stream->PESExtensionFlag2           = ReadBits(BitI, 1);
                if (Stream->PESPrivateDataFlag == true) {
                    SkipBits(BitI, 128);
                }
                if (Stream->PackHeaderFieldFlag == true) {
                    Stream->PackFieldSize           = ReadBits(BitI, 8);
                    pack_header();
                }
                if (Stream->ProgramPacketSeqCounterFlag == true) {
                    SkipBits(BitI, 1);
                    Stream->ProgramPacketSeqCounter = ReadBits(BitI, 7);
                    SkipBits(BitI, 1);
                    Stream->MPEGVersionIdentifier   = ReadBits(BitI, 1);
                    Stream->OriginalStuffSize       = ReadBits(BitI, 6);
                }
                if (Stream->PSTDBufferFlag == true) {
                    SkipBits(BitI, 2);
                    Stream->PSTDBufferScale         = ReadBits(BitI, 1);
                    Stream->PSTDBufferSize          = ReadBits(BitI, 13);
                }
                if (Stream->PESExtensionFlag2 == true) {
                    SkipBits(BitI, 1);
                    Stream->PESExtensionFieldSize   = ReadBits(BitI, 7);
                    Stream->StreamIDExtensionFlag   = ReadBits(BitI, 1);
                    if (Stream->StreamIDExtensionFlag == false) {
                        Stream->StreamIDExtension   = ReadBits(BitI, 7);
                    } else {
                        SkipBits(BitI, 6);
                        // tref_extension_flag
                        Stream->TREFFieldPresentFlag = ReadBits(BitI, 1);
                        if (Stream->TREFFieldPresentFlag == false) {
                            SkipBits(BitI, 4);
                            uint8_t  TREF1 = ReadBits(BitI, 3);
                            SkipBits(BitI, 1); // marker bit
                            uint16_t TREF2 = ReadBits(BitI, 15);
                            SkipBits(BitI, 1); // marker bit
                            uint16_t TREF3 = ReadBits(BitI, 15);
                            SkipBits(BitI, 1); // marker bit
                            
                            Stream->TREF = TREF1 << 30;
                            Stream->TREF = TREF2 << 15;
                            Stream->TREF = TREF3;
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
                SkipBits(BitI, 8); // stuffing_byte
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
    
    static void TSParseAdaptionField(BitInput *BitI, MPEG2TransportStream *Stream) { // adaptation_field
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
                uint8_t  DecodeTimeStamp1                       = ReadBits(BitI, 3);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp2                       = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker_bit
                uint16_t DecodeTimeStamp3                       = ReadBits(BitI, 15);
                SkipBits(BitI, 1); // marker_bit
                
                Stream->Adaptation->DecodeTimeStampNextAU  = DecodeTimeStamp1 << 30;
                Stream->Adaptation->DecodeTimeStampNextAU += DecodeTimeStamp2 << 15;
                Stream->Adaptation->DecodeTimeStampNextAU += DecodeTimeStamp3;
            }
            for (int i = 0; i < N; i++) {
                SkipBits(BitI, 8);
            }
        }
        for (int i = 0; i < N; i++) {
            SkipBits(BitI, 8);
        }
    }
    
    static void ParseTransportStreamPacket(BitInput *BitI, MPEG2TransportStream *Stream) { // transport_packet
        Stream->Packet->SyncByte                   = ReadBits(BitI, 8);
        Stream->Packet->TransportErrorIndicator    = ReadBits(BitI, 1);
        Stream->Packet->StartOfPayloadIndicator    = ReadBits(BitI, 1);
        Stream->Packet->TransportPriorityIndicator = ReadBits(BitI, 1);
        Stream->Packet->PID                        = ReadBits(BitI, 13);
        Stream->Packet->TransportScramblingControl = ReadBits(BitI, 2);
        Stream->Packet->AdaptationFieldControl     = ReadBits(BitI, 2);
        Stream->Packet->ContinuityCounter          = ReadBits(BitI, 4);
        if (Stream->Packet->AdaptationFieldControl == 2 || Stream->Packet->AdaptationFieldControl == 3) {
            TSParseAdaptionField(BitI, Stream);
        }
        if (Stream->Packet->AdaptationFieldControl == 1 || Stream->Packet->AdaptationFieldControl == 3) {
            int N = 0; // FIXME: I just did this so it would compile what is N?
            for (int i = 0; i < N; i++) {
                SkipBits(BitI, 8); // data_byte
            }
        }
    }
    
    void DemuxMPEG2PESPackets(BitInput *BitI, MPEG2TransportStream *Stream) {
        
    }
    
    void DemuxMPEG2ProgramStream(BitInput *BitI, MPEG2TransportStream *Stream) {
        
    }
    
    void DemuxMPEG2TransportStream(BitInput *BitI, MPEG2TransportStream *Stream) {
        
    }
    
#ifdef __cplusplus
}
#endif
