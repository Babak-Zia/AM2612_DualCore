/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
* \addtogroup SSC-AM2612 SSC-AM2612
* @{
*/

/**
\file SSC-AM2612Objects
\author ET9300Utilities.ApplicationHandler (Version 1.6.4.0) | EthercatSSC@beckhoff.com

\brief SSC-AM2612 specific objects<br>
\brief NOTE : This file will be overwritten if a new object dictionary is generated!<br>
*/

#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
#define PROTO
#else
#define PROTO extern
#endif
/******************************************************************************
*                    Object 0x1600 : Output mapping 0
******************************************************************************/
/**
* \addtogroup 0x1600 0x1600 | Output mapping 0
* @{
* \brief Object 0x1600 (Output mapping 0) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x7000.0<br>
* SubIndex 2 - Reference to 0x7001.1<br>
* SubIndex 3 - Reference to 0x7001.2<br>
* SubIndex 4 - Reference to 0x7001.3<br>
* SubIndex 5 - Reference to 0x7002.0<br>
* SubIndex 6 - Reference to 0x7003.1<br>
* SubIndex 7 - Reference to 0x7003.2<br>
* SubIndex 8 - Reference to 0x7003.3<br>
* SubIndex 9 - Reference to 0x7003.4<br>
* SubIndex 10 - Reference to 0x7003.5<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1600[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x7000.0 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x7001.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x7001.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x7001.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x7002.0 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - Reference to 0x7003.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - Reference to 0x7003.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex8 - Reference to 0x7003.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex9 - Reference to 0x7003.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex10 - Reference to 0x7003.5 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1600[] = "Output mapping 0\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000"
"SubIndex 009\000"
"SubIndex 010\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x7000.0 */
UINT32 SI2; /* Subindex2 - Reference to 0x7001.1 */
UINT32 SI3; /* Subindex3 - Reference to 0x7001.2 */
UINT32 SI4; /* Subindex4 - Reference to 0x7001.3 */
UINT32 SI5; /* Subindex5 - Reference to 0x7002.0 */
UINT32 SI6; /* Subindex6 - Reference to 0x7003.1 */
UINT32 SI7; /* Subindex7 - Reference to 0x7003.2 */
UINT32 SI8; /* Subindex8 - Reference to 0x7003.3 */
UINT32 SI9; /* Subindex9 - Reference to 0x7003.4 */
UINT32 SI10; /* Subindex10 - Reference to 0x7003.5 */
} OBJ_STRUCT_PACKED_END
TOBJ1600;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1600 OutputMapping00x1600
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={10,0x70000008,0x70010108,0x70010208,0x70010308,0x70020010,0x70030108,0x70030210,0x70030340,0x70030420,0x70030540}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1610 : FSOE_Rx process data mapping
******************************************************************************/
/**
* \addtogroup 0x1610 0x1610 | FSOE_Rx process data mapping
* @{
* \brief Object 0x1610 (FSOE_Rx process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x7100.1<br>
* SubIndex 2 - Reference to 0x7100.2<br>
* SubIndex 3 - Reference to 0x7100.3<br>
* SubIndex 4 - Reference to 0x7100.4<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1610[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x7100.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x7100.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x7100.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex4 - Reference to 0x7100.4 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1610[] = "FSOE_Rx process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x7100.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x7100.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x7100.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x7100.4 */
} OBJ_STRUCT_PACKED_END
TOBJ1610;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1610 FSOE_RxProcessDataMapping0x1610
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={4,0x71000108,0x71000208,0x71000310,0x71000410}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A00 : Input mapping 0
******************************************************************************/
/**
* \addtogroup 0x1A00 0x1A00 | Input mapping 0
* @{
* \brief Object 0x1A00 (Input mapping 0) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x6000.0<br>
* SubIndex 2 - Reference to 0x6001.1<br>
* SubIndex 3 - Reference to 0x6001.2<br>
* SubIndex 4 - Reference to 0x6001.3<br>
* SubIndex 5 - Reference to 0x6002.0<br>
* SubIndex 6 - Reference to 0x6003.1<br>
* SubIndex 7 - Reference to 0x6003.2<br>
* SubIndex 8 - Reference to 0x6003.3<br>
* SubIndex 9 - Reference to 0x6003.4<br>
* SubIndex 10 - Reference to 0x6003.5<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1A00[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x6000.0 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x6001.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x6001.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x6001.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x6002.0 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex6 - Reference to 0x6003.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex7 - Reference to 0x6003.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex8 - Reference to 0x6003.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex9 - Reference to 0x6003.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex10 - Reference to 0x6003.5 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1A00[] = "Input mapping 0\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000"
"SubIndex 007\000"
"SubIndex 008\000"
"SubIndex 009\000"
"SubIndex 010\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x6000.0 */
UINT32 SI2; /* Subindex2 - Reference to 0x6001.1 */
UINT32 SI3; /* Subindex3 - Reference to 0x6001.2 */
UINT32 SI4; /* Subindex4 - Reference to 0x6001.3 */
UINT32 SI5; /* Subindex5 - Reference to 0x6002.0 */
UINT32 SI6; /* Subindex6 - Reference to 0x6003.1 */
UINT32 SI7; /* Subindex7 - Reference to 0x6003.2 */
UINT32 SI8; /* Subindex8 - Reference to 0x6003.3 */
UINT32 SI9; /* Subindex9 - Reference to 0x6003.4 */
UINT32 SI10; /* Subindex10 - Reference to 0x6003.5 */
} OBJ_STRUCT_PACKED_END
TOBJ1A00;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1A00 InputMapping00x1A00
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={10,0x60000008,0x60010108,0x60010208,0x60010308,0x60020010,0x60030108,0x60030210,0x60030340,0x60030420,0x60030540}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A10 : FSOE_Tx process data mapping
******************************************************************************/
/**
* \addtogroup 0x1A10 0x1A10 | FSOE_Tx process data mapping
* @{
* \brief Object 0x1A10 (FSOE_Tx process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x6100.1<br>
* SubIndex 2 - Reference to 0x6100.2<br>
* SubIndex 3 - Reference to 0x6100.3<br>
* SubIndex 4 - Reference to 0x6100.4<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1A10[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x6100.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x6100.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x6100.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex4 - Reference to 0x6100.4 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x1A10[] = "FSOE_Tx process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x6100.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x6100.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x6100.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x6100.4 */
} OBJ_STRUCT_PACKED_END
TOBJ1A10;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1A10 FSOE_TxProcessDataMapping0x1A10
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={4,0x61000108,0x61000208,0x61000310,0x61000410}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C12 : SyncManager 2 assignment
******************************************************************************/
/**
* \addtogroup 0x1C12 0x1C12 | SyncManager 2 assignment
* @{
* \brief Object 0x1C12 (SyncManager 2 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C12[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C12[] = "SyncManager 2 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[2];  /**< \brief Subindex 1 - 2 */
} OBJ_STRUCT_PACKED_END
TOBJ1C12;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C12 sRxPDOassign
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={2,{0x1600,0x1610}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1C13 : SyncManager 3 assignment
******************************************************************************/
/**
* \addtogroup 0x1C13 0x1C13 | SyncManager 3 assignment
* @{
* \brief Object 0x1C13 (SyncManager 3 assignment) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x1C13[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x1C13[] = "SyncManager 3 assignment\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
UINT16 aEntries[2];  /**< \brief Subindex 1 - 2 */
} OBJ_STRUCT_PACKED_END
TOBJ1C13;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ1C13 sTxPDOassign
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={2,{0x1A00,0x1A10}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6000 : Single_Byte_Tx
******************************************************************************/
/**
* \addtogroup 0x6000 0x6000 | Single_Byte_Tx
* @{
* \brief Object 0x6000 (Single_Byte_Tx) definition
*/
#ifdef _OBJD_
/**
* \brief Entry description
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM sEntryDesc0x6000 = { DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READ | OBJACCESS_TXPDOMAPPING };
/**
* \brief Object name
*/
OBJCONST UCHAR OBJMEM aName0x6000[] = "Single_Byte_Tx\000\377";
#endif //#ifdef _OBJD_

/**
* \brief Object variable
*/
PROTO UINT8 Single_Byte_Tx0x6000
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
= 0x00
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6001 : Array_Byte_TX
******************************************************************************/
/**
* \addtogroup 0x6001 0x6001 | Array_Byte_TX
* @{
* \brief Object 0x6001 (Array_Byte_TX) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6001[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER8 , 0x08 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x6001[] = "Array_Byte_TX\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
INT8 aEntries[3];  /**< \brief Subindex 1 - 3 */
} OBJ_STRUCT_PACKED_END
TOBJ6001;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6001 Array_Byte_TX0x6001
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={3,{0x00,0x00,0x00}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6002 : Single_Short_Tx
******************************************************************************/
/**
* \addtogroup 0x6002 0x6002 | Single_Short_Tx
* @{
* \brief Object 0x6002 (Single_Short_Tx) definition
*/
#ifdef _OBJD_
/**
* \brief Entry description
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM sEntryDesc0x6002 = { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING };
/**
* \brief Object name
*/
OBJCONST UCHAR OBJMEM aName0x6002[] = "Single_Short_Tx\000\377";
#endif //#ifdef _OBJD_

/**
* \brief Object variable
*/
PROTO INT16 Single_Short_Tx0x6002
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
= 0x0000
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6003 : Complex_Tx
******************************************************************************/
/**
* \addtogroup 0x6003 0x6003 | Complex_Tx
* @{
* \brief Object 0x6003 (Complex_Tx) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Complex_Byte<br>
* SubIndex 2 - Complex_Short<br>
* SubIndex 3 - Complex_Long<br>
* SubIndex 4 - Complex_Float<br>
* SubIndex 5 - Complex_Double<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6003[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER8 , 0x08 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - Complex_Byte */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - Complex_Short */
{ DEFTYPE_UNSIGNED64 , 0x40 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - Complex_Long */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex4 - Complex_Float */
{ DEFTYPE_REAL64 , 0x40 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex5 - Complex_Double */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6003[] = "Complex_Tx\000"
"Complex_Byte\000"
"Complex_Short\000"
"Complex_Long\000"
"Complex_Float\000"
"Complex_Double\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT8 Complex_Byte; /* Subindex1 - Complex_Byte */
INT16 Complex_Short; /* Subindex2 - Complex_Short */
UINT64 Complex_Long; /* Subindex3 - Complex_Long */
REAL32 Complex_Float; /* Subindex4 - Complex_Float */
REAL64 Complex_Double; /* Subindex5 - Complex_Double */
} OBJ_STRUCT_PACKED_END
TOBJ6003;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6003 Complex_Tx0x6003
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={5,0x00,0x0000,0x0000000000000000,0,0}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x6100 : FSOE_Tx
******************************************************************************/
/**
* \addtogroup 0x6100 0x6100 | FSOE_Tx
* @{
* \brief Object 0x6100 (FSOE_Tx) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - fsoeStatus<br>
* SubIndex 2 - safeInputs<br>
* SubIndex 3 - connectionId<br>
* SubIndex 4 - fsoeCRC<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x6100[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - fsoeStatus */
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - safeInputs */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - connectionId */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_TXPDOMAPPING }}; /* Subindex4 - fsoeCRC */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x6100[] = "FSOE_Tx\000"
"fsoeStatus\000"
"safeInputs\000"
"connectionId\000"
"fsoeCRC\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT8 FsoeStatus; /* Subindex1 - fsoeStatus */
UINT8 SafeInputs; /* Subindex2 - safeInputs */
UINT16 ConnectionId; /* Subindex3 - connectionId */
UINT16 FsoeCRC; /* Subindex4 - fsoeCRC */
} OBJ_STRUCT_PACKED_END
TOBJ6100;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ6100 FSOE_Tx0x6100
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={4,0x00,0x00,0x0000,0x0000}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7000 : Single_Byte_Rx
******************************************************************************/
/**
* \addtogroup 0x7000 0x7000 | Single_Byte_Rx
* @{
* \brief Object 0x7000 (Single_Byte_Rx) definition
*/
#ifdef _OBJD_
/**
* \brief Entry description
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM sEntryDesc0x7000 = { DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING };
/**
* \brief Object name
*/
OBJCONST UCHAR OBJMEM aName0x7000[] = "Single_Byte_Rx\000\377";
#endif //#ifdef _OBJD_

/**
* \brief Object variable
*/
PROTO UINT8 Single_Byte_Rx0x7000
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
= 0x00
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7001 : Array_Byte_RX
******************************************************************************/
/**
* \addtogroup 0x7001 0x7001 | Array_Byte_RX
* @{
* \brief Object 0x7001 (Array_Byte_RX) definition
*/
#ifdef _OBJD_
/**
* \brief Entry descriptions<br>
* 
* Subindex 0<br>
* Subindex 1 - n (the same entry description is used)<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7001[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }};

/**
* \brief Object name definition<br>
* For Subindex 1 to n the syntax 'Subindex XXX' is used
*/
OBJCONST UCHAR OBJMEM aName0x7001[] = "Array_Byte_RX\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16   u16SubIndex0;  /**< \brief Subindex 0 */
INT8 aEntries[3];  /**< \brief Subindex 1 - 3 */
} OBJ_STRUCT_PACKED_END
TOBJ7001;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7001 Array_Byte_RX0x7001
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={3,{0x00,0x00,0x00}}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7002 : Single_Short_Rx
******************************************************************************/
/**
* \addtogroup 0x7002 0x7002 | Single_Short_Rx
* @{
* \brief Object 0x7002 (Single_Short_Rx) definition
*/
#ifdef _OBJD_
/**
* \brief Entry description
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM sEntryDesc0x7002 = { DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING };
/**
* \brief Object name
*/
OBJCONST UCHAR OBJMEM aName0x7002[] = "Single_Short_Rx\000\377";
#endif //#ifdef _OBJD_

/**
* \brief Object variable
*/
PROTO INT16 Single_Short_Rx0x7002
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
= 0x0000
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7003 : Complex_Rx
******************************************************************************/
/**
* \addtogroup 0x7003 0x7003 | Complex_Rx
* @{
* \brief Object 0x7003 (Complex_Rx) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Complex_Byte<br>
* SubIndex 2 - Complex_Short<br>
* SubIndex 3 - Complex_Long<br>
* SubIndex 4 - Complex_Float<br>
* SubIndex 5 - Complex_Double<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7003[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_INTEGER8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - Complex_Byte */
{ DEFTYPE_INTEGER16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex2 - Complex_Short */
{ DEFTYPE_UNSIGNED64 , 0x40 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex3 - Complex_Long */
{ DEFTYPE_REAL32 , 0x20 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex4 - Complex_Float */
{ DEFTYPE_REAL64 , 0x40 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }}; /* Subindex5 - Complex_Double */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x7003[] = "Complex_Rx\000"
"Complex_Byte\000"
"Complex_Short\000"
"Complex_Long\000"
"Complex_Float\000"
"Complex_Double\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
INT8 Complex_Byte; /* Subindex1 - Complex_Byte */
INT16 Complex_Short; /* Subindex2 - Complex_Short */
UINT64 Complex_Long; /* Subindex3 - Complex_Long */
REAL32 Complex_Float; /* Subindex4 - Complex_Float */
REAL64 Complex_Double; /* Subindex5 - Complex_Double */
} OBJ_STRUCT_PACKED_END
TOBJ7003;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7003 Complex_Rx0x7003
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={5,0x00,0x0000,0x0000000000000000,0,0}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x7100 : FSOE_Rx
******************************************************************************/
/**
* \addtogroup 0x7100 0x7100 | FSOE_Rx
* @{
* \brief Object 0x7100 (FSOE_Rx) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - fsoeCommand<br>
* SubIndex 2 - safeOutputs<br>
* SubIndex 3 - connectionId<br>
* SubIndex 4 - fsoeCRC<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0x7100[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - fsoeCommand */
{ DEFTYPE_UNSIGNED8 , 0x08 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex2 - safeOutputs */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex3 - connectionId */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }}; /* Subindex4 - fsoeCRC */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0x7100[] = "FSOE_Rx\000"
"fsoeCommand\000"
"safeOutputs\000"
"connectionId\000"
"fsoeCRC\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT8 FsoeCommand; /* Subindex1 - fsoeCommand */
UINT8 SafeOutputs; /* Subindex2 - safeOutputs */
UINT16 ConnectionId; /* Subindex3 - connectionId */
UINT16 FsoeCRC; /* Subindex4 - fsoeCRC */
} OBJ_STRUCT_PACKED_END
TOBJ7100;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJ7100 FSOE_Rx0x7100
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={4,0x00,0x00,0x0000,0x0000}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0xF000 : Modular Device Profile
******************************************************************************/
/**
* \addtogroup 0xF000 0xF000 | Modular Device Profile
* @{
* \brief Object 0xF000 (Modular Device Profile) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Index distance <br>
* SubIndex 2 - Maximum number of modules <br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDesc0xF000[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex1 - Index distance  */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }}; /* Subindex2 - Maximum number of modules  */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aName0xF000[] = "Modular Device Profile\000"
"Index distance \000"
"Maximum number of modules \000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 IndexDistance; /* Subindex1 - Index distance  */
UINT16 MaximumNumberOfModules; /* Subindex2 - Maximum number of modules  */
} OBJ_STRUCT_PACKED_END
TOBJF000;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TOBJF000 ModularDeviceProfile0xF000
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={2,0x0010,0}
#endif
;
/** @}*/





PROTO UINT8 yes(UINT16 index, UINT8 subindex, UINT32 dataSize, UINT16 MBXMEM * pData, UINT8 bCompleteAccess);


#ifdef _OBJD_
TOBJECT    OBJMEM ApplicationObjDic[] = {
/* Object 0x1600 */
{NULL , NULL ,  0x1600 , {DEFTYPE_PDOMAPPING , 10 | (OBJCODE_REC << 8)} , asEntryDesc0x1600 , aName0x1600 , &OutputMapping00x1600 , NULL , NULL , 0x0000 },
/* Object 0x1610 */
{NULL , NULL ,  0x1610 , {DEFTYPE_PDOMAPPING , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x1610 , aName0x1610 , &FSOE_RxProcessDataMapping0x1610 , NULL , NULL , 0x0000 },
/* Object 0x1A00 */
{NULL , NULL ,  0x1A00 , {DEFTYPE_PDOMAPPING , 10 | (OBJCODE_REC << 8)} , asEntryDesc0x1A00 , aName0x1A00 , &InputMapping00x1A00 , NULL , NULL , 0x0000 },
/* Object 0x1A10 */
{NULL , NULL ,  0x1A10 , {DEFTYPE_PDOMAPPING , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x1A10 , aName0x1A10 , &FSOE_TxProcessDataMapping0x1A10 , NULL , NULL , 0x0000 },
/* Object 0x1C12 */
{NULL , NULL ,  0x1C12 , {DEFTYPE_UNSIGNED16 , 2 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C12 , aName0x1C12 , &sRxPDOassign , NULL , NULL , 0x0000 },
/* Object 0x1C13 */
{NULL , NULL ,  0x1C13 , {DEFTYPE_UNSIGNED16 , 2 | (OBJCODE_ARR << 8)} , asEntryDesc0x1C13 , aName0x1C13 , &sTxPDOassign , NULL , NULL , 0x0000 },
/* Object 0x6000 */
{NULL , NULL ,  0x6000 , {DEFTYPE_UNSIGNED8 , 0 | (OBJCODE_VAR << 8)} , &sEntryDesc0x6000 , aName0x6000 , &Single_Byte_Tx0x6000 , NULL , NULL , 0x0000 },
/* Object 0x6001 */
{NULL , NULL ,  0x6001 , {DEFTYPE_INTEGER8 , 3 | (OBJCODE_ARR << 8)} , asEntryDesc0x6001 , aName0x6001 , &Array_Byte_TX0x6001 , NULL , NULL , 0x0000 },
/* Object 0x6002 */
{NULL , NULL ,  0x6002 , {DEFTYPE_INTEGER16 , 0 | (OBJCODE_VAR << 8)} , &sEntryDesc0x6002 , aName0x6002 , &Single_Short_Tx0x6002 , NULL , NULL , 0x0000 },
/* Object 0x6003 */
{NULL , NULL ,  0x6003 , {DEFTYPE_RECORD , 5 | (OBJCODE_REC << 8)} , asEntryDesc0x6003 , aName0x6003 , &Complex_Tx0x6003 , yes , NULL , 0x0000 },
/* Object 0x6100 */
{NULL , NULL ,  0x6100 , {DEFTYPE_RECORD , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x6100 , aName0x6100 , &FSOE_Tx0x6100 , yes , NULL , 0x0000 },
/* Object 0x7000 */
{NULL , NULL ,  0x7000 , {DEFTYPE_UNSIGNED8 , 0 | (OBJCODE_VAR << 8)} , &sEntryDesc0x7000 , aName0x7000 , &Single_Byte_Rx0x7000 , NULL , NULL , 0x0000 },
/* Object 0x7001 */
{NULL , NULL ,  0x7001 , {DEFTYPE_INTEGER8 , 3 | (OBJCODE_ARR << 8)} , asEntryDesc0x7001 , aName0x7001 , &Array_Byte_RX0x7001 , NULL , NULL , 0x0000 },
/* Object 0x7002 */
{NULL , NULL ,  0x7002 , {DEFTYPE_INTEGER16 , 0 | (OBJCODE_VAR << 8)} , &sEntryDesc0x7002 , aName0x7002 , &Single_Short_Rx0x7002 , NULL , NULL , 0x0000 },
/* Object 0x7003 */
{NULL , NULL ,  0x7003 , {DEFTYPE_RECORD , 5 | (OBJCODE_REC << 8)} , asEntryDesc0x7003 , aName0x7003 , &Complex_Rx0x7003 , yes , NULL , 0x0000 },
/* Object 0x7100 */
{NULL , NULL ,  0x7100 , {DEFTYPE_RECORD , 4 | (OBJCODE_REC << 8)} , asEntryDesc0x7100 , aName0x7100 , &FSOE_Rx0x7100 , yes , NULL , 0x0000 },
/* Object 0xF000 */
{NULL , NULL ,  0xF000 , {DEFTYPE_RECORD , 2 | (OBJCODE_REC << 8)} , asEntryDesc0xF000 , aName0xF000 , &ModularDeviceProfile0xF000 , NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_

#undef PROTO

/** @}*/
#define _SSC_AM2612_OBJECTS_H_
