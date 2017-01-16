#pragma once

#ifdef __cplusplus
extern "C" {
#endif
    
    extern enum MPEGTSConstants {
        MPEGTSMaxPrivateData = 256,
    } MPEGTSConstants;
    
    extern enum PIDTable {
        ProgramAssociationTable         =    0, // Program association table
        ConditionalAccesTable           =    1, // Conditional access table
        TransportStreamDescriptionTable =    2, // Transport stream description table
        IPMPControlInfoTable            =    3, // Intellectual Property Management and Protection
                                                // 4-15 are reserved
        LastReserved                    = 8191,
    } PIDTable;
    
    extern enum AdaptationFieldTable {
        Reserved                        =    0,
        PayloadOnly                     =    1,
        AdaptationFieldOnly             =    2,
        AdaptationThenPayload           =    3,
    } AdaptationFieldTable;
    
#ifdef __cplusplus
}
#endif
