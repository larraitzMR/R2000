/*
 *****************************************************************************
 *                                                                           *
 *                 IMPINJ CONFIDENTIAL AND PROPRIETARY                       *
 *                                                                           *
 * This source code is the sole property of Impinj, Inc.  Reproduction or    *
 * utilization of this source code in whole or in part is forbidden without  *
 * the prior written consent of Impinj, Inc.                                 *
 *                                                                           *
 * (c) Copyright Impinj, Inc. 2009. All rights reserved.                     *
 *                                                                           *
 *****************************************************************************
 */

/*
 *****************************************************************************
 *
 * $Id: rfid_structs.h 67084 2011-03-24 21:50:15Z dshaheen $
 * 
 * Description:
 *     This is the RFID Library header file that contains the delcarations and
 *     definitions for the RFID library data types.
 *     
 *
 *****************************************************************************
 */

#ifndef RFID_STRUCTS_H_INCLUDED
#define RFID_STRUCTS_H_INCLUDED

#include "rfid_types.h"
#include "rfid_constants.h"

/******************************************************************************
 * Name:  RFID_VERSION - Represents the version information for components in
 *        the system.
 ******************************************************************************/
typedef struct {
    /* The major version (i.e, in 1.x.x.x, the 1)                               */
    INT32U  major;
    /* The minor version (i.e., in x.1.x.x, the 1)                              */
    INT32U  minor;
    /* The maintenance number (i.e., in x.x.1.x, the 1)                          */
    INT32U  maintenance;
    /* The release number (i.e., in x.x.x.1, the 1)                             */
    INT32U  release;
} RFID_VERSION;

/******************************************************************************
 * Name:  RFID_RADIO_INFO - Used to represent the information for a particular
 *        attached radio.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  This will be                    */
    /* sizeof(RFID_RADIO_INFO).                                               */
    INT32U          length;
     /* The version information for the radio's bus driver.                   */
    RFID_VERSION    driverVersion;
    /* The unique cookie for the radio.  This cookie is passed to             */
    /* RFID_RadioOpen() when the application wishes to take control of the    */
    /* radio.                                                                 */
    INT32U          cookie;
    /* The length, in bytes, of the uniqueId field(aka, serial number)        */
    /* including the null terminator                                          */
    INT32U          idLength;
    /* A pointer to an array of bytes that contain the radio module's unique  */
    /* ID (aka, serial number).                                               */
    INT8U*          pUniqueId;
} RFID_RADIO_INFO;

/******************************************************************************
 * Name:  RFID_RADIO_ENUM - The data that will be returned from a request to
 *        list the radios that are attached to the system.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  The application must set this to*/
    /* sizeof(RFID_RADIO_ENUM) before calling                                 */
    /* RFID_RetrieveAttachedRadiosList().                                     */
    INT32U              length;
    /* The total length, in bytes, of radio enumeration structure.  The       */
    /* application must fill in this field with the length of the radio       */
    /* enumeration buffer before calling RFID_RetrieveAttachedRadiosList().   */
    /* If the buffer is not large enough, on return this field contains the   */
    /* required size of the radio-enumeration buffer.  If the buffer is large */
    /* enough, on return this field contains the number of bytes returned     */
    INT32U              totalLength;    
    /* The number of radios that are attached to the system.  This is also the*/
    /* number of radio info structure pointers in the array pointed to by     */
    /* ppRadioInfo.                                                           */
    INT32U              countRadios;    
    /* A pointer to an array of RFID_RADIO_INFO structure pointers.  The array*/
    /* contains countRadios entries.                                          */
    RFID_RADIO_INFO**   ppRadioInfo;
} RFID_RADIO_ENUM;

/******************************************************************************
 * Name:  RFID_RADIO_LINK_PROFILE_ISO18K6C_CONFIG - ISO 18000-6C specific link
 *        profile information.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  The RFID Reader Library will    */
    /* fill this in with the appropriate value when RFID_RadioGetLinkProfile  */
    /* is called.                                                             */
    INT32U                          length;
    /* The modulation type used by the link profile.                          */
    RFID_18K6C_MODULATION_TYPE      modulationType;
    /* The duration, in nanoseconds, of the Tari.                             */
    INT32U                          tari;
    /* The difference, in Taris, between a data zero and a data one.          */
    RFID_18K6C_DATA_0_1_DIFFERENCE  data01Difference;
    /* The duration, in nanoseconds, of the low-going portion of the radio-to-*/
    /* tag PIE symbol.                                                        */
    INT32U                          pulseWidth;
    /* The width, in nanoseconds, of the radio-to-tag calibration.            */
    INT32U                          rtCalibration;
    /* The width, in nanoseconds, of the tag-to-radio calibration.            */
    INT32U                          trCalibration;
    /* The divide ratio used.                                                 */
    RFID_18K6C_DIVIDE_RATIO         divideRatio;
    /* The miller number (i.e., cycles per symbol).                           */
    RFID_18K6C_MILLER_NUMBER        millerNumber;
    /* The tag-to-radio link frequency in Hz.                                 */
    INT32U                          trLinkFrequency;
    /* The delay, in microseconds, inserted to ensure meeting the minimum T2  */
    /* timing.                                                                */
    INT32U                          varT2Delay;
    /* The amount of time, in 48MHz cycles, a radio module will wait between  */
    /* transmitting and then attempting to receive the backscattered signal   */
    /* from tags.                                                             */
    INT32U                          rxDelay;
    /* The minimum amount of ISO 18000-6C T2 time, in microseconds, after     */
    /* receiving a tag response before a radio may transmit again.            */
    INT32U                          minT2Delay;
    /* The number of microseconds it takes for a signal to propagate through  */
    /* the radio's transmit chain.                                            */
    INT32U                          txPropagationDelay;
} RFID_RADIO_LINK_PROFILE_ISO18K6C_CONFIG;

/******************************************************************************
 * Name:  RFID_RADIO_LINK_PROFILE - Information about a radio link profile.
 *        This is returned in repsonse to a request for link profile
 *        information.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  The application must set this to*/
    /* sizeof(RFID_RADIO_LINK_PROFILE) before calling                         */
    /* RFID_RadioGetLinkProfile().                                            */
    INT32U              length;
    /* Indicates if the profile is active.  A non-zero value indicates that   */
    /* the profile is active.  A zero value indicates that the profile is     */
    /* inactive.                                                              */
    BOOL32              enabled;
    /* The identifier for the profile.  This field, in combination with       */
    /* profileVersion, provides a unique identifier for the profile.          */
    INT64U              profileId;    
    /* The version for the profile.  This field, in combination with          */
    /* profileId, provides a unique identifier for the profile.               */
    INT32U              profileVersion;
    /* The tag protocol for which this profile was configured.  This value is */
    /* the discriminator for the profileConfig union.                         */
    RFID_RADIO_PROTOCOL profileProtocol;
    /* Indicates if the profile is a dense-reader-mode (DRM) profile.  A non- */
    /* zero value indicates a DRM profile.  A zero value indicates a non-DRM  */
    /* profile.                                                               */
    BOOL32              denseReaderMode;
    /* Number of samples over which the wide band Receive Signal Strength     */
    /* Indication (RSSI) will be averaged.                                    */
    INT32U              widebandRssiSamples;
    /* Number of samples over which the narrow band Receive Signal Strength   */
    /* Indication (RSSI) will be averaged.                                    */
    INT32U              narrowbandRssiSamples;
    /* Reserved for future use.                                               */
    BOOL32              realtimeRssiEnabled;
    /* Reserved for future use.                                               */
    INT32U              realtimeWidebandRssiSamples;
    /* Reserved for future use.                                               */
    INT32U              realtimeNarrowbandRssiSamples;
    /* The link profile configuration information.  profileProtocol           */
    /* determines  which part of the union to use.                            */
    union {
        RFID_RADIO_LINK_PROFILE_ISO18K6C_CONFIG iso18K6C;
    } profileConfig;
} RFID_RADIO_LINK_PROFILE;

/******************************************************************************
 * Name:  RFID_ANTENNA_PORT_STATUS - The status of an logical antenna port.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Must be set to by the           */
    /* application to sizeof(RFID_ANTENNA_PORT_STATUS)                        */
    INT32U                  length;
    /* The state (enabled/disabled) of the antenna port                       */
    RFID_ANTENNA_PORT_STATE state;
    /* The last measurement of the antenna-sense resistor (in ohms) for the   */
    /* logical antenna port’s physical transmit antenna port.                 */
    INT32U                  antennaSenseValue;
} RFID_ANTENNA_PORT_STATUS;

/******************************************************************************
 * Name:  RFID_ANTENNA_PORT_CONFIG - The configuration parameters for a
 *        logical antenna port.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Must be set to by the           */
    /* application to sizeof(RFID_ANTENNA_PORT_CONFIG)                        */
    INT32U  length;
    /* The power level for the logical antenna port’s physical transmit       */
    /* antenna.  This value is specified in 0.1 (i.e., 1/10th) dBm.           */
    INT32U  powerLevel;
    /* The number of milliseconds to spend on this antenna port during a      */
    /* cycle.  Zero indicates that antenna usage will be controlled by the    */
    /* numberInventoryCycles field.                                           */
    INT32U  dwellTime;
    /* The number of inventory rounds to perform with this antenna port.  Zero*/
    /* indicates that the antenna usage will be controlled by the dwellTime   */
    /* field.                                                                 */
    INT32U  numberInventoryCycles;
    /* Deprecated as of v2.3.0, and will be removed in a future release.      */
    /* This attribute should be set to 0                                      */
    INT32U  physicalRxPort;
    /* The underlying physical transmit antenna port associated with the      */
    /* logical antenna port.  Must be between 0 and 3, inclusive.             */
    INT32U  physicalTxPort;
    /* The measured resistance, specified in ohms, above which the antenna-   */
    /* sense resistance should be considered to be an open circuit (i.e., a   */
    /* disconnected antenna).                                                 */
    INT32U  antennaSenseThreshold;
} RFID_ANTENNA_PORT_CONFIG;

/******************************************************************************
 * Name:  RFID_18K6C_SELECT_MASK - The select mask for partitioning a tag 
 * population
 ******************************************************************************/
enum {
    RFID_18K6C_SELECT_MASK_BYTE_LEN = 32
};
typedef struct {
    /* The memory bank to match against                                       */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The offset of the first bit to match                                   */
    INT32U                  offset;
    /* The number of bits in the mask                                         */
    INT32U                  count;
    /* The bit pattern to match.                                              */
    INT8U                   mask[RFID_18K6C_SELECT_MASK_BYTE_LEN];
} RFID_18K6C_SELECT_MASK;

/******************************************************************************
 * Name:  RFID_18K6C_SELECT_ACTION - The matching and non-matching action to
 *        take when a selection mask matches/doesn't match
 ******************************************************************************/
typedef struct {
    /* What will be affected by the action                                    */
    RFID_18K6C_TARGET   target;
    /* The actions to be performed on the tag populations (i.e., matching and */
    /* non-matching.                                                          */
    RFID_18K6C_ACTION   action;
    /* Should the EPC be truncated when the tag is singulated?  A non-zero    */
    /* value requestes that the EPC is truncated.  A zero value requests the  */
    /* entire EPC.                                                            */
    BOOL32              enableTruncate;
} RFID_18K6C_SELECT_ACTION;

/******************************************************************************
 * Name:  RFID_18K6C_SELECT_CRITERION - A single selection criterion - i.e.,
 *        combination of selction mask and action.
 ******************************************************************************/
typedef struct {
    /* The selection mask to test for                                         */
    RFID_18K6C_SELECT_MASK    mask;
    /* The actions to perform                                                 */
    RFID_18K6C_SELECT_ACTION  action;
} RFID_18K6C_SELECT_CRITERION;

/******************************************************************************
 * Name:  RFID_18K6C_SELECT_CRITERIA - A set of selction criteria for  
 *        partitioning a tag population.
 ******************************************************************************/
typedef struct {
    /* The number of criteria.  This value must be between 0 and 8, inclusive.*/
    INT32U                          countCriteria;
    /* A pointer to an array of countCriteria tag-selection criterion.  This  */
    /* must not be NULL, unless countCriteria is 0.                           */
    RFID_18K6C_SELECT_CRITERION*    pCriteria;
} RFID_18K6C_SELECT_CRITERIA;

/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_MASK - A single post-singulation match mask
 ******************************************************************************/
enum {
    RFID_18K6C_SINGULATION_MASK_BYTE_LEN = 62
};
typedef struct {
    /* Offset in bits, from the start of the EPC, of the first bit to match   */
    /* against the mask.                                                      */
    INT32U  offset;
    /* The number of bits in the mask.  A length of zero causes all tags to   */
    /* match.  If (offset + count) falls beyond the end of the mask, the tag  */
    /* is considered non-matching.                                            */
    INT32U  count;
    /* The bit pattern to match.                                              */
    INT8U   mask[RFID_18K6C_SINGULATION_MASK_BYTE_LEN];
} RFID_18K6C_SINGULATION_MASK;

/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_CRITERION - A single post-singulation match 
 *        criterion
 ******************************************************************************/
typedef struct {
    /* Indicates if the associated tag-protocol operation will be applied to  */
    /* matching or non-matching tags.  A non-zero value indicates that the    */
    /* tag-protocol operation is applied to matching tags.  A zero value of   */
    /* indicates that tag-protocol operation is applied to non- matching tags.*/
    BOOL32                      match;  
    /* The mask to be applied to the tag's EPC.                               */
    RFID_18K6C_SINGULATION_MASK mask;   
} RFID_18K6C_SINGULATION_CRITERION;

/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_CRITERIA - A set of selction criteria for
 *        post-singulation matching.
 ******************************************************************************/
typedef struct {
    /* The number of criteria.  This value must be between 0 and 1 inclusive. */
    INT32U                              countCriteria;
    /* An pointer to array of countCriteria post-singulation match mask       */
    /* criterion.  This value must not be NULL.                               */
    RFID_18K6C_SINGULATION_CRITERION*   pCriteria;
} RFID_18K6C_SINGULATION_CRITERIA;

/******************************************************************************
 * Name:  RFID_18K6C_TAG_GROUP - Specifies which tag population will be
 *        singulated
 ******************************************************************************/
typedef struct {
    /* The state of the SL flag                                               */
    RFID_18K6C_SELECTED                 selected;
    /* Which inventory session (S0, S1, etc.)                                 */
    RFID_18K6C_INVENTORY_SESSION        session;
    /* The state of the inventory session specified by the session field      */
    RFID_18K6C_INVENTORY_SESSION_TARGET target;
} RFID_18K6C_TAG_GROUP;

/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_FIXEDQ_PARMS - The parameters for the fixed Q
 *        (i.e., RFID_18K6C_SINGULATION_ALGORITHM_FIXEDQ) algorithm.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  When calling                    */
    /* RFID_18K6CSetSingulationAlgorithmParameters, or                        */
    /* RFID_18K6CGetSingulationAlgorithmParameters the application must set   */
    /* this to sizeof(RFID_18K6C_SINGULATION_FIXEDQ_PARMS).                   */
    INT32U  length;
    /* The Q value to use.  Valid values are 0 to 15, inclusive.              */
    INT32U  qValue;
    /* Specifies the number of times to try another execution of the          */
    /* singulation algorithm for the specified session/target before either   */
    /* toggling the target (if toggleTarget is non-zero) or terminating the   */
    /* inventory/tag access operation.  Valid values are 0-255, inclusive.    */
    INT32U  retryCount;
    /* A flag that indicates if, after performing the inventory cycle for the */
    /* specified target (i.e., A or B), if the target should be toggled (i.e.,*/
    /* A to B or B to A) and another inventory cycle run.  A non-zero value   */
    /* indicates that the target should be toggled.  A zero value indicates   */
    /* that the target should not be toggled.  Note that if the target is     */
    /* toggled, retryCount and repeatUntilNoTags will also apply to the new   */
    /* target.                                                                */
    BOOL32  toggleTarget;
    /* A flag that indicates whether or not the singulation algorithm should  */
    /* continue performing inventory rounds until no tags are singulated.  A  */
    /* non-zero value indicates that, for each execution of the singulation   */
    /* algorithm, inventory rounds should be performed until no tags are      */
    /* singulated.  A zero value indicates that a single inventory round      */
    /* should be performed for each execution of the singulation algorithm.   */
    BOOL32  repeatUntilNoTags;
} RFID_18K6C_SINGULATION_FIXEDQ_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_SINGULATION_DYNAMICQ_THRESH_PARMS - The parameters for
 *        the dynamic Q with Q-threshold adjustment (i.e., 
 *        RFID_18K6C_SINGULATION_ALGORITHM_DYNAMICQ_THRESH) algorithm.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  When calling                    */
    /* RFID_18K6CSetSingulationAlgorithmParameters, or                        */
    /* RFID_18K6CGetSingulationAlgorithmParameters the application must set   */
    /* this to sizeof(RFID_18K6C_SINGULATION_DYNAMICQ_PARMS).                 */
    INT32U  length;
    /* The starting Q value to use.  Valid values are 0 to 15, inclusive.     */
    /* minQValue <= startQValue <= maxQValue                                  */
    INT32U  startQValue;
    /* The minimum Q value to use.  Valid values are 0 to 15, inclusive.      */
    /* minQValue <= startQValue <= maxQValue                                  */
    INT32U  minQValue;
    /* The maximum Q value to use.  Valid values are 0 to 15, inclusive.      */
    /* minQValue <= startQValue <= maxQValue                                  */
    INT32U  maxQValue;
    /* Specifies the number of times to try another execution of the          */
    /* singulation algorithm for the specified session/target before either   */
    /* toggling the target (if toggleTarget is non-zero) or terminating the   */
    /* inventory/tag access operation.  Valid values are 0-255, inclusive.    */
    INT32U  retryCount;
    /* A flag that indicates if, after performing the inventory cycle for the */
    /* specified target (i.e., A or B), if the target should be toggled (i.e.,*/
    /* A to B or B to A) and another inventory cycle run.  A non-zero value   */
    /* indicates that the target should be toggled.  A zero value indicates   */
    /* that the target should not be toggled.  Note that if the target is     */
    /* toggled, retryCount  will also apply to the new target.                */
    BOOL32  toggleTarget;
    /* The multiplier, specified in units of fourths (i.e., 0.25), that will  */
    /* be applied to the Q-adjustment threshold as part of the dynamic-Q      */
    /* algorithm.  Valid values are 0-255, inclusive.                         */
    INT32U  thresholdMultiplier;
} RFID_18K6C_SINGULATION_DYNAMICQ_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_COMMON_PARMS - The ISO 18000-6C tag-protocol operation
 *        common parameters.
 ******************************************************************************/
typedef struct {
    /* The maximum number of tags to which the tag-protocol operation will be */
    /* applied.  If this number is zero, then the operation is applied to all */
    /* tags that match the selection, and optionally post-singulation, match  */
    /* criteria.  If this number is non-zero, the antenna-port dwell-time and */
    /* inventory-round-count constraints still apply, however the operation   */
    /* will be prematurely terminated if the maximum number of tags have the  */
    /* tag-protocol operation applied to them.                                */
    INT32U                                  tagStopCount;
    /* A pointer to a callback function that the library will invoke with the */
    /* resulting operation response packets.                                  */
    RFID_PACKET_CALLBACK_FUNCTION           pCallback;
    /* An application-defined value that is passed through unmodified to the  */
    /* application-specified callback function.                               */
    void*                                   context;
    /* A pointer to a 32-bit integer that upon return will contain the return */
    /* code from the last call to the application-supplied callback function. */
    INT32S*                                 pCallbackCode;
} RFID_18K6C_COMMON_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_INVENTORY_PARMS - The ISO 18000-6C inventory operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_INVENTORY_PARMS) before calling                      */
    /* RFID_18K6CTagInventory().                                              */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
} RFID_18K6C_INVENTORY_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_READ_CMD_PARMS - The ISO 18000-6C tag-read operation
 *        read specific command parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_READ_CMD_PARMS) before calling RFID_18K6CTagRead().  */
    INT32U                  length;
    /* The memmory bank to read from.                                         */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The offset of the first 16-bit word to read.                           */
    INT16U                  offset;
    /* The nubmer of 16-bit words to read.  If this value is zero and bank is */
    /* RFID_18K6C_MEMORY_BANK_EPC, the read will return the contents of the   */
    /* tag’s EPC memory starting at the 16-bit word specified by offset       */
    /* through the end of the EPC.  If this value is zero and bank is not     */
    /* RFID_18K6C_MEMORY_BANK_EPC, the read will return, for the chosen memory*/
    /* bank, data starting from the 16-bit word specified by offset to the end*/
    /* of the memory bank.                                                    */
    INT16U                  count;
} RFID_18K6C_READ_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_READ_PARMS - The ISO 18000-6C tag-read operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_READ_PARMS) before calling RFID_18K6CTagRead().      */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The ISO 18000-6C tag-protocol read specific command parameters         */
    RFID_18K6C_READ_CMD_PARMS readCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_READ_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS - The parameters for specifying the
 *        data for a sequential tag write.
 ******************************************************************************/
typedef struct {
    /* The length of this structure.  The application must set this field to  */
    /* sizeof(RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS).                         */
    INT32U                  length;
    /* The memory bank to write to.                                           */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The number of 16-bit words that will be written.  This field must be   */
    /* between 1 and 32, inclusive.                                           */
    INT16U                  count;
    /* The offset, in the memory bank, of the first 16-bit word to write.     */
    INT16U                  offset;
    /* A pointer to the 16-bit values to write to the tag's memory bank.      */
    INT16U*                 pData;
} RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_WRITE_RANDOM_CMD_PARMS - The parameters for specifying the
 *        data for a random, single-memory-bank tag write.
 ******************************************************************************/
typedef struct {
    /* The length of this structure.  The application must set this field to  */
    /* sizeof(RFID_18K6C_WRITE_RANDOM_CMD_PARMS).                             */
    INT32U                  length;
    /* The memory bank to write to.                                           */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The number of 16-bit words that will be written.  This field must be   */
    /* between 1 and 32, inclusive.                                           */
    INT16U                  count;
    /* Reserved.  Set to zero.                                                */
    INT16U                  reserved;
    /* Pointer to an array of 16-bit offsets in the tag's memory bank where   */
    /* the corresponding array entry in pData will be written.                */
    INT16U*                 pOffset;
    /* Pointer to an array of 16-bit values that will be written to the tag-  */
    /* memory offset specified in the corresponding array entry in pOffset.   */
    INT16U*                 pData;
} RFID_18K6C_WRITE_RANDOM_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_WRITE_PARMS - The ISO 18000-6C tag-write operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_WRITE_PARMS).                                        */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The type of write (i.e., sequential, etc.) that will be performed.     */
    /* This field is the discriminator that determines what kind of write     */
    /* parameters structure pWriteParms points to.                            */
    RFID_18K6C_WRITE_TYPE   writeType;
    /* A discriminated union, with writeType as the discriminator, that       */
    /* contains the tag-write parameters.                                     */
    union {
        RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS   sequential;
        RFID_18K6C_WRITE_RANDOM_CMD_PARMS       random;
    } writeCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_WRITE_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_KILL_CMD_PARMS - The ISO 18000-6C tag-kill operation
 *        kill specific parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_KILL_CMD_PARMS) before calling RFID_18K6CTagKill().  */
    INT32U                  length;
    /* The kill password.  Must not be zero.                                  */
    INT32U                  killPassword;
} RFID_18K6C_KILL_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_KILL_PARMS - The ISO 18000-6C tag-kill operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_KILL_PARMS) before calling RFID_18K6CTagKill().      */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The ISO 18000-6C tag-kill specific command parameters                  */
    RFID_18K6C_KILL_CMD_PARMS killCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_KILL_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_TAG_PERM - The permission values for performing an ISO
 *        18000-6C tag lock operation
 ******************************************************************************/
typedef struct {
    /* Permissions for the tag's kill password                                */
    RFID_18K6C_TAG_PWD_PERM killPasswordPermissions;
    /* Permissions for the tag's access password                              */
    RFID_18K6C_TAG_PWD_PERM accessPasswordPermissions;
    /* Permissions for the tag's EPC memory bank                              */
    RFID_18K6C_TAG_MEM_PERM epcMemoryBankPermissions;
    /* Permissions for the tag's TID memory bank                              */
    RFID_18K6C_TAG_MEM_PERM tidMemoryBankPermissions;
    /* Permissions for the tag's user memory bank                             */
    RFID_18K6C_TAG_MEM_PERM userMemoryBankPermissions;
} RFID_18K6C_TAG_PERM;


/******************************************************************************
 * Name:  RFID_18K6C_LOCK_CMD_PARMS - The ISO 18000-6C tag-lock operation
 *        lock specific command parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_LOCK_CMD_PARMS) before calling RFID_18K6CTagLock().  */
    INT32U                  length;
    /* The access permissions for the tag.                                    */
    RFID_18K6C_TAG_PERM     permissions;
} RFID_18K6C_LOCK_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_LOCK_PARMS - The ISO 18000-6C tag-lock operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_LOCK_PARMS) before calling RFID_18K6CTagLock().      */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The ISO 18000-6C tag-lock operation lock specific command parameters.  */
    RFID_18K6C_LOCK_CMD_PARMS lockCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_LOCK_PARMS;



/******************************************************************************
 * Name:  RFID_18K6C_BLOCK_WRITE_CMD_PARMS - The ISO 18000-6C block-write 
 *        operation block-write specific command parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_BLOCK_WRITE_CMD_PARMS).                              */
    INT32U                  length;
	/* The memory bank to write to.                                           */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The offset of the first 16-bit word to write.                          */
    INT16U                  offset;
    /* The number of 16-bit words that will be written.  This field must be   */
    /* between 1 and 255, inclusive.                                          */
    INT16U                  count;
    /* A pointer to the 16-bit values to write to the tag's memory bank.      */
    INT16U*                 pData;
} RFID_18K6C_BLOCK_WRITE_CMD_PARMS;

/******************************************************************************
 * Name:  RFID_18K6C_BLOCK_WRITE_PARMS - The ISO 18000-6C block-write operation
 *        parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_BLOCK_WRITE_PARMS).                                  */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The ISO 18000-6C block write specific command paramters                */
    RFID_18K6C_BLOCK_WRITE_CMD_PARMS blockWriteCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_BLOCK_WRITE_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_QT_CMD_PARMS - The ISO 18000-6C QT extension operation,
 *        QT specific command parameters
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_QT_CMD_PARMS).                                       */
    INT32U                          length;
    /* Indicates whether to read or wirte the tag QT control data             */
    RFID_18K6C_QT_CTRL_TYPE		    qtReadWrite;
    /* Indicates whether tag QT control data is to be written to temporary    */
    /* volatile memory or permanent non-volatile memory                       */
    RFID_18K6C_QT_PERSISTENCE_TYPE	qtPersistence;
    /* The QT payload short range control.  When enabled the tag range is     */
    /* reduced, when disabled the tag range is not reduced                    */
    RFID_18K6C_QT_SR_TYPE			qtShortRange;
    /* The QT payload memory map control.  Controls whether the tag will use  */
    /* the public memory map or the private memory map                        */
    RFID_18K6C_QT_MEMMAP_TYPE		qtMemoryMap;
} RFID_18K6C_QT_CMD_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_QT_PARMS - The ISO 18000-6C QT extension operation
 *        parameters
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_QT_PARMS).                                           */
    INT32U                                      length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS				        common;
    /* The type of optional access that will be performed along with the QT   */
    /* operation.                                                             */
    RFID_18K6C_QT_OPT_CMD_TYPE			        optCmdType;
    /* The access command parameters corresponding to the optCmdType, if it   */
    /* is not RFID_18K6C_QT_OPT_NONE                                          */
    union {
        RFID_18K6C_READ_CMD_PARMS			    readCmdParms;
        RFID_18K6C_WRITE_SEQUENTIAL_CMD_PARMS	seqWriteCmdParms;
        RFID_18K6C_WRITE_RANDOM_CMD_PARMS		randWriteCmdParms;
    } parameters;
    /* The QT specific command parameters to read or write the tag QT control */
    /*  data                                                                  */
    RFID_18K6C_QT_CMD_PARMS				        qtCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U							            accessPassword;
} RFID_18K6C_QT_PARMS;


/******************************************************************************
 * Name:  RFID_18K6C_BLOCK_ERASE_CMD_PARMS - The ISO 18000-6C tag-blockerase
 *        operation blockerase specific command parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_BLOCK_ERASE_CMD_PARMS) before calling                */
    /* RFID_18K6CTagBlockErase().                                             */
    INT32U                  length;
    /* The memmory bank to erase.                                             */
    RFID_18K6C_MEMORY_BANK  bank;
    /* The offset of the first 16-bit word to erase.                          */
    INT16U                  offset;
    /* The number of 16-bit words to erase.                                   */
    INT16U                  count;
} RFID_18K6C_BLOCK_ERASE_CMD_PARMS;



/******************************************************************************
 * Name:  RFID_18K6C_BLOCK_ERASE_PARMS - The ISO 18000-6C tag-blockerase
 *        operation parameters.
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_18K6C_BLOCK_ERASE_PARMS) before calling                    */
    /* RFID_18K6CTagBlockErase().                                             */
    INT32U                  length;
    /* The ISO 18000-6C tag-protocol operation common parameters.             */
    RFID_18K6C_COMMON_PARMS common;
    /* The ISO 18000-6C tag-protocol block erase specific command parameters  */
    RFID_18K6C_BLOCK_ERASE_CMD_PARMS blockEraseCmdParms;
    /* The access password.  A value of zero indicates no access password.    */
    INT32U                  accessPassword;
} RFID_18K6C_BLOCK_ERASE_PARMS;


/******************************************************************************
 * Name:  RFID_NONVOLATILE_BLOCK - Defines a nonvolatile memory block to use
 *        for update.
 ******************************************************************************/
typedef struct {
    /* The starting address of the radio module’s nonvolatile memory block.   */
    /* The address must be within the valid range of the radio module’s       */
    /* nonvolatile memory addresses.                                          */
    INT32U	address;
    /* The length, in bytes, of the radio module’s nonvolatile memory block.  */
    /* The last address (i.e., address+length-1) must be within the valid     */
    /* range of the radio module’s nonvolatile memory addresses.  This field  */
    /* must be greater than zero.                                             */
    INT32U	length;
    /* A pointer to a buffer that contains length bytes to write to the radio */
    /* module’s nonvolatile memory block at address.  This field must not be  */
    /* NULL.                                                                  */
    INT8U*	pData;
    /* Reserved for future use.  Set to zero.                                 */
    INT32U	flags;
} RFID_NONVOLATILE_MEMORY_BLOCK;


/******************************************************************************
 * Name:  RFID_ISSUE_RADIO_COMMAND_PARMS - The parameter structure for the  
 *              RFID_RadioIssueCommand Interface
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_ISSUE_RADIO_COMMAND_PARMS) before calling                  */
    /* RFID_RadioIssueCommand().                                              */
    INT32U                  length;
    /* The radio command to be written to the HST_CMD register                */
    INT32U                  command;
    /* A pointer to a callback function that the library will invoke with the */
    /* resulting operation response packets.                                  */
    RFID_PACKET_CALLBACK_FUNCTION           pCallback;
    /* An application-defined value that is passed through unmodified to the  */
    /* application-specified callback function.                               */
    void*                                   context;
    /* A pointer to a 32-bit integer that upon return will contain the return */
    /* code from the last call to the application-supplied callback function. */
    INT32S*                                 pCallbackCode;
} RFID_ISSUE_RADIO_COMMAND_PARMS;


/******************************************************************************
 * Name:  RFID_RANDOM_CW_PARMS - The parameter structure for the  
 *              RFID_RadioTurnCarrierWaveOnRandom Interface
 ******************************************************************************/
typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_RANDOM_CW_PARMS) before calling                            */
    /* RFID_RadioTurnCarrierWaveOnRandom().                                   */
    INT32U                  length;
    /* The duration of the random CW data should be transmitted.  A value of  */
    /* 0 will run forever, and a cancel or abort command is required to stop.  */
    INT32U                  duration;
    /* A pointer to a callback function that the library will invoke with the */
    /* resulting operation response packets.                                  */
    RFID_PACKET_CALLBACK_FUNCTION           pCallback;
    /* An application-defined value that is passed through unmodified to the  */
    /* application-specified callback function.                               */
    void*                                   context;
    /* A pointer to a 32-bit integer that upon return will contain the return */
    /* code from the last call to the application-supplied callback function. */
    INT32S*                                 pCallbackCode;
} RFID_RANDOM_CW_PARMS;


/******************************************************************************
 * Name:  RFID_REGISTER_INFO - The parameter structure for the  
 *              RFID_MacReadRegisterInfo Interface
 ******************************************************************************/

typedef struct {
    /* The length of the structure in bytes.  Application must set this to    */
    /* sizeof(RFID_REGISTER_INFO) before calling                              */
    /* RFID_MacReadRegisterInfo().                                            */
    INT32U                     length;
    /* The type for the register requested.                                   */
    RFID_REGISTER_TYPE         type;   
    /* bankSize is only valid for RFID_REGISTER_SELECTOR and                  */
    /* RFID_REGISTER_BANKED types, and returns the size of the                */
    /* register bank                                                          */
    INT16U                     bankSize;
    /* selectorAddress is only valid for RFID_REGISTER_BANKED                 */
    /* type and returns the address of the register bank selector             */
    INT16U                     selectorAddress;
    /* currentSelector is only valid for RFID_REGISTER_BANKED                 */
    /* and RFID_REGISTER_SELECTOR types and returns the                       */
    /* current selector value for the register bank                           */
    INT16U                     currentSelector;
    /* reserved for future use, set to 0. Ensuring 32-bit alignment           */
    INT16U                     reserved;
    /* The access type (rw, ro, wo)for the register requested                 */
    RFID_REGISTER_ACCESS_TYPE  accessType;
} RFID_REGISTER_INFO;


/******************************************************************************
 * Name:  RFID_RADIO_IMPINJ_EXTENSIONS - Specifies which Impinj Extensions are
 *        enabled/disabled
 ******************************************************************************/
typedef struct {
    /* The block write mode determines the maximum number of words to write 
       per BlockWrite transaction                                             */
    RFID_BLOCKWRITE_MODE                blockWriteMode;
    /* The enable/disable for the tag focus feature                     */
    /* NOTE:  This will enable tag focus only if the additional              */
    /*        inventory criteria are satisfied.  Specifically, the tag query  */
    /*        group (RFID_18K6CSetQueryTagGroup) must be configured to set    */
    /*        session=S1 and target=A, and the singulation algorithm          */
    /*        parameters (RFID_18K6CSetSingulationAlgorithmParameters) must   */
    /*        be configured to set toggleTarget=0 (disabled).                 */ 
    RFID_TAG_FOCUS                tagFocus;
    /* Enables the FastID feature                                             */
    RFID_FAST_ID                  fastId;
} RFID_IMPINJ_EXTENSIONS;

#endif  /* #ifndef RFID_STRUCTS_H_INCLUDED */
