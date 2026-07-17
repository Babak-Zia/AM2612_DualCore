/*
* This source file is part of the EtherCAT Slave Stack Code licensed by Beckhoff Automation GmbH & Co KG, 33415 Verl, Germany.
* The corresponding license agreement applies. This hint shall not be removed.
*/

/**
* \addtogroup SSC-AM2612 SSC-AM2612
* @{
*/

/**
\file Module0x71000001_Objects
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
*                    Object 0x160F : FSOE_RX_RAW process data mapping
******************************************************************************/
/**
* \addtogroup 0x160F 0x160F | FSOE_RX_RAW process data mapping
* @{
* \brief Object 0x160F (FSOE_RX_RAW process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x70F0.1<br>
* SubIndex 2 - Reference to 0x70F0.2<br>
* SubIndex 3 - Reference to 0x70F0.3<br>
* SubIndex 4 - Reference to 0x70F0.4<br>
* SubIndex 5 - Reference to 0x70F0.5<br>
* SubIndex 6 - Reference to 0x70F0.6<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule71000001Obj0x160F[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x70F0.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x70F0.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x70F0.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x70F0.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x70F0.5 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex6 - Reference to 0x70F0.6 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule71000001Obj0x160F[] = "FSOE_RX_RAW process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x70F0.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x70F0.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x70F0.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x70F0.4 */
UINT32 SI5; /* Subindex5 - Reference to 0x70F0.5 */
UINT32 SI6; /* Subindex6 - Reference to 0x70F0.6 */
} OBJ_STRUCT_PACKED_END
TMODULE71000001OBJ160F;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE71000001OBJ160F Module71000001FSOE_RX_RAWProcessDataMapping0x160F
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={6,0x70F00110,0x70F00210,0x70F00310,0x70F00410,0x70F00510,0x70F00610}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x1A0F : FSOE_TX_RAW process data mapping
******************************************************************************/
/**
* \addtogroup 0x1A0F 0x1A0F | FSOE_TX_RAW process data mapping
* @{
* \brief Object 0x1A0F (FSOE_TX_RAW process data mapping) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Reference to 0x60F0.1<br>
* SubIndex 2 - Reference to 0x60F0.2<br>
* SubIndex 3 - Reference to 0x60F0.3<br>
* SubIndex 4 - Reference to 0x60F0.4<br>
* SubIndex 5 - Reference to 0x60F0.5<br>
* SubIndex 6 - Reference to 0x60F0.6<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule71000001Obj0x1A0F[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Reference to 0x60F0.1 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex2 - Reference to 0x60F0.2 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex3 - Reference to 0x60F0.3 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex4 - Reference to 0x60F0.4 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex5 - Reference to 0x60F0.5 */
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }}; /* Subindex6 - Reference to 0x60F0.6 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule71000001Obj0x1A0F[] = "FSOE_TX_RAW process data mapping\000"
"SubIndex 001\000"
"SubIndex 002\000"
"SubIndex 003\000"
"SubIndex 004\000"
"SubIndex 005\000"
"SubIndex 006\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 SI1; /* Subindex1 - Reference to 0x60F0.1 */
UINT32 SI2; /* Subindex2 - Reference to 0x60F0.2 */
UINT32 SI3; /* Subindex3 - Reference to 0x60F0.3 */
UINT32 SI4; /* Subindex4 - Reference to 0x60F0.4 */
UINT32 SI5; /* Subindex5 - Reference to 0x60F0.5 */
UINT32 SI6; /* Subindex6 - Reference to 0x60F0.6 */
} OBJ_STRUCT_PACKED_END
TMODULE71000001OBJ1A0F;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE71000001OBJ1A0F Module71000001FSOE_TX_RAWProcessDataMapping0x1A0F
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={6,0x60F00110,0x60F00210,0x60F00310,0x60F00410,0x60F00510,0x60F00610}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x60F0 : FSOE_TX_RAW
******************************************************************************/
/**
* \addtogroup 0x60F0 0x60F0 | FSOE_TX_RAW
* @{
* \brief Object 0x60F0 (FSOE_TX_RAW) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Word0<br>
* SubIndex 2 - Word1<br>
* SubIndex 3 - Word2<br>
* SubIndex 4 - Word3<br>
* SubIndex 5 - Word4<br>
* SubIndex 6 - Word5<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule71000001Obj0x60F0[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex1 - Word0 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex2 - Word1 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex3 - Word2 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex4 - Word3 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }, /* Subindex5 - Word4 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ | OBJACCESS_TXPDOMAPPING }}; /* Subindex6 - Word5 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule71000001Obj0x60F0[] = "FSOE_TX_RAW\000"
"Word0\000"
"Word1\000"
"Word2\000"
"Word3\000"
"Word4\000"
"Word5\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 Word0; /* Subindex1 - Word0 */
UINT16 Word1; /* Subindex2 - Word1 */
UINT16 Word2; /* Subindex3 - Word2 */
UINT16 Word3; /* Subindex4 - Word3 */
UINT16 Word4; /* Subindex5 - Word4 */
UINT16 Word5; /* Subindex6 - Word5 */
} OBJ_STRUCT_PACKED_END
TMODULE71000001OBJ60F0;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE71000001OBJ60F0 Module71000001FSOE_TX_RAW0x60F0
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={6,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x70F0 : FSOE_RX_RAW
******************************************************************************/
/**
* \addtogroup 0x70F0 0x70F0 | FSOE_RX_RAW
* @{
* \brief Object 0x70F0 (FSOE_RX_RAW) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Word0<br>
* SubIndex 2 - Word1<br>
* SubIndex 3 - Word2<br>
* SubIndex 4 - Word3<br>
* SubIndex 5 - Word4<br>
* SubIndex 6 - Word5<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule71000001Obj0x70F0[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex1 - Word0 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex2 - Word1 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex3 - Word2 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex4 - Word3 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }, /* Subindex5 - Word4 */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READWRITE | OBJACCESS_RXPDOMAPPING }}; /* Subindex6 - Word5 */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule71000001Obj0x70F0[] = "FSOE_RX_RAW\000"
"Word0\000"
"Word1\000"
"Word2\000"
"Word3\000"
"Word4\000"
"Word5\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT16 Word0; /* Subindex1 - Word0 */
UINT16 Word1; /* Subindex2 - Word1 */
UINT16 Word2; /* Subindex3 - Word2 */
UINT16 Word3; /* Subindex4 - Word3 */
UINT16 Word4; /* Subindex5 - Word4 */
UINT16 Word5; /* Subindex6 - Word5 */
} OBJ_STRUCT_PACKED_END
TMODULE71000001OBJ70F0;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE71000001OBJ70F0 Module71000001FSOE_RX_RAW0x70F0
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={6,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000}
#endif
;
/** @}*/



/******************************************************************************
*                    Object 0x9001 : FSOE_Communication_Parameter 
******************************************************************************/
/**
* \addtogroup 0x9001 0x9001 | FSOE_Communication_Parameter 
* @{
* \brief Object 0x9001 (FSOE_Communication_Parameter ) definition
*/
#ifdef _OBJD_
/**
* \brief Object entry descriptions<br>
* <br>
* SubIndex 0<br>
* SubIndex 1 - Version <br>
* SubIndex 2 - Safety_Address<br>
* SubIndex 3 - FSOE_Connection_ID<br>
* SubIndex 4 - Watchdog_Time<br>
* SubIndex 5 - Connection_Type<br>
* SubIndex 6 - Connection_Typee<br>
* SubIndex 7 - Communication_Parameter_Length<br>
* SubIndex 8 - Application_Parameter_Length<br>
*/
OBJCONST TSDOINFOENTRYDESC    OBJMEM asEntryDescModule71000001Obj0x9001[] = {
{ DEFTYPE_UNSIGNED8 , 0x8 , ACCESS_READ },
{ DEFTYPE_UNSIGNED32 , 0x20 , ACCESS_READ }, /* Subindex1 - Version  */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex2 - Safety_Address */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex3 - FSOE_Connection_ID */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex4 - Watchdog_Time */
{ DEFTYPE_UNSIGNED64 , 0x40 , ACCESS_READ }, /* Subindex5 - Connection_Type */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex6 - Connection_Typee */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }, /* Subindex7 - Communication_Parameter_Length */
{ DEFTYPE_UNSIGNED16 , 0x10 , ACCESS_READ }}; /* Subindex8 - Application_Parameter_Length */

/**
* \brief Object/Entry names
*/
OBJCONST UCHAR OBJMEM aNameModule71000001Obj0x9001[] = "FSOE_Communication_Parameter \000"
"Version \000"
"Safety_Address\000"
"FSOE_Connection_ID\000"
"Watchdog_Time\000"
"Connection_Type\000"
"Connection_Typee\000"
"Communication_Parameter_Length\000"
"Application_Parameter_Length\000\377";
#endif //#ifdef _OBJD_

#ifndef _SSC_AM2612_OBJECTS_H_
/**
* \brief Object structure
*/
typedef struct OBJ_STRUCT_PACKED_START {
UINT16 u16SubIndex0;
UINT32 Version; /* Subindex1 - Version  */
UINT16 Safety_Address; /* Subindex2 - Safety_Address */
UINT16 FSOE_Connection_ID; /* Subindex3 - FSOE_Connection_ID */
UINT16 Watchdog_Time; /* Subindex4 - Watchdog_Time */
UINT64 Connection_Type; /* Subindex5 - Connection_Type */
UINT16 Connection_Typee; /* Subindex6 - Connection_Typee */
UINT16 Communication_Parameter_Length; /* Subindex7 - Communication_Parameter_Length */
UINT16 Application_Parameter_Length; /* Subindex8 - Application_Parameter_Length */
} OBJ_STRUCT_PACKED_END
TMODULE71000001OBJ9001;
#endif //#ifndef _SSC_AM2612_OBJECTS_H_

/**
* \brief Object variable
*/
PROTO TMODULE71000001OBJ9001 Module71000001FSOE_Communication_Parameter0x9001
#if defined(_SSC_AM2612_) && (_SSC_AM2612_ == 1)
={8,0,0,0,0,0,0,0,0}
#endif
;
/** @}*/







#if COE_SUPPORTED
#ifdef _OBJD_
TOBJECT    OBJMEM Module71000001ObjDic[] = {
/* Object 0x160F */
{NULL , NULL ,  0x160F , {DEFTYPE_PDOMAPPING , 6 | (OBJCODE_REC << 8)} , asEntryDescModule71000001Obj0x160F , aNameModule71000001Obj0x160F , &Module71000001FSOE_RX_RAWProcessDataMapping0x160F , NULL , NULL , 0x0000 },
/* Object 0x1A0F */
{NULL , NULL ,  0x1A0F , {DEFTYPE_PDOMAPPING , 6 | (OBJCODE_REC << 8)} , asEntryDescModule71000001Obj0x1A0F , aNameModule71000001Obj0x1A0F , &Module71000001FSOE_TX_RAWProcessDataMapping0x1A0F , NULL , NULL , 0x0000 },
/* Object 0x60F0 */
{NULL , NULL ,  0x60F0 , {DEFTYPE_RECORD , 6 | (OBJCODE_REC << 8)} , asEntryDescModule71000001Obj0x60F0 , aNameModule71000001Obj0x60F0 , &Module71000001FSOE_TX_RAW0x60F0 , NULL , NULL , 0x0000 },
/* Object 0x70F0 */
{NULL , NULL ,  0x70F0 , {DEFTYPE_RECORD , 6 | (OBJCODE_REC << 8)} , asEntryDescModule71000001Obj0x70F0 , aNameModule71000001Obj0x70F0 , &Module71000001FSOE_RX_RAW0x70F0 , NULL , NULL , 0x0000 },
/* Object 0x9001 */
{NULL , NULL ,  0x9001 , {DEFTYPE_RECORD , 8 | (OBJCODE_REC << 8)} , asEntryDescModule71000001Obj0x9001 , aNameModule71000001Obj0x9001 , &Module71000001FSOE_Communication_Parameter0x9001 , NULL , NULL , 0x0000 },
{NULL,NULL, 0xFFFF, {0, 0}, NULL, NULL, NULL, NULL}};
#endif    //#ifdef _OBJD_
#endif
#undef PROTO

/** @}*/
