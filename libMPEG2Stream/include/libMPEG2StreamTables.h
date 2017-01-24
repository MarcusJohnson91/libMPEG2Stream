#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
    extern enum TSPIDTable {
        ProgramAssociationTable         =    0, // Program association table
        ConditionalAccesTable           =    1, // Conditional access table
        TransportStreamDescriptionTable =    2, // Transport stream description table
        IPMPControlInfoTable            =    3, // Intellectual Property Management and Protection
        LastReserved                    = 8191,
    } PIDTable;
    
    extern enum TSAdaptationFieldTable {
        Reserved                        =    0,
        PayloadOnly                     =    1,
        AdaptationFieldOnly             =    2,
        AdaptationThenPayload           =    3,
    } AdaptationFieldTable;
    
    extern enum TSStreamIDTypes {
        ProgramStreamMap                =  188,
        PrivateStream1                  =  189,
        PaddingStream                   =  190,
        PrivateStream2                  =  191,
        AudioStream                     =  192, // & 0xC0, the 5 least significant bits are the audio stream number.
        VideoStream                     =  224, // & 0xE0, the 4 least significant bits are the video stream number.
        ECMStream                       =  240,
        EMMStream                       =  241,
        AnnexA_DSMCCStream              =  242, // or 13818-1 Annex A
        Stream13522                     =  243,
        TypeAStream                     =  244,
        TypeBStream                     =  245,
        TypeCStream                     =  246,
        TypeDStream                     =  247,
        TypeEStream                     =  248,
        AncillaryStream                 =  249,
        SLPacketizedStream              =  250,
        FlexMuxStream                   =  251,
        MetadataStream                  =  252,
        ExtendedStreamID                =  253,
        ReservedStream                  =  254,
        ProgramStreamFolder             =  255,
    } TSStreamIDTypes;
    
    extern enum TSTrickModeTypes {
        FastForward = 0,
        SlowMotion  = 1,
        FreezeFrame = 2,
        FastRewind  = 3,
        SlowRewind  = 4,
    } TSTrickModeTypes;
    
    extern enum TSFieldID {
        TopFieldOnly    = 0,
        BottomFieldOnly = 1,
        DisplayFullPic  = 2,
    } TSFieldID;
    
    extern enum CoefficentSelection {
        OnlyDCCoeffsAreNonZero     = 0,
        OnlyFirst3CoeffsAreNonZero = 1,
        OnlyFirst6CoeffsAreNonZero = 2,
        AllCoeffsMayBeNonZero      = 3,
    } CoefficentSelection;
    
#ifdef __cplusplus
}
#endif
