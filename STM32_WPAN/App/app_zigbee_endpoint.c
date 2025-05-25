/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : app_zigbee_endpoint.c
  * Description        : Zigbee Application to manage endpoints and these clusters.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "app_common.h"
#include "app_conf.h"
#include "log_module.h"
#include "app_entry.h"
#include "app_zigbee.h"
#include "dbg_trace.h"
#include "ieee802154_enums.h"
#include "mcp_enums.h"

#include "stm32_lpm.h"
#include "stm32_rtos.h"
#include "stm32_timer.h"

#include "zigbee.h"
#include "zigbee.nwk.h"
#include "zigbee.security.h"
/* USER CODE BEGIN Includes_Zigbee_ZDO */
#include "zigbee.zdo.h"
/* USER CODE END Includes_Zigbee_ZDO */

/* Private includes -----------------------------------------------------------*/
#include "zcl/zcl.h"
/* USER CODE BEGIN PI */
#include "app_bsp.h"
#include "esl_cluster.h"
/* USER CODE END PI */

/* Private defines -----------------------------------------------------------*/
#define APP_ZIGBEE_CHANNEL                13u
#define APP_ZIGBEE_CHANNEL_MASK           ( 1u << APP_ZIGBEE_CHANNEL )
#define APP_ZIGBEE_TX_POWER               ((int8_t) 10)    /* TX-Power is at +10 dBm. */

#define APP_ZIGBEE_ENDPOINT               17u
#define APP_ZIGBEE_PROFILE_ID             ZCL_PROFILE_HOME_AUTOMATION
/* USER CODE BEGIN PD_DEVICE_ID */
#define APP_ZIGBEE_DEVICE_ID              0x010E // Custom ESL Device ID (Example - Needs official assignment if standardizing)
/* USER CODE END PD_DEVICE_ID */
#define APP_ZIGBEE_GROUP_ADDRESS          0x0001u

/* USER CODE BEGIN PD */
#define APP_ZIGBEE_STARTUP_FAIL_DELAY     500u

#define APP_ZIGBEE_APPLICATION_NAME       "ESL Server"
#define APP_ZIGBEE_APPLICATION_OS_NAME    "."

#define DEFAULT_PERIODIC_UPDATE_INTERVAL_MS (300 * 1000) // Default 5 minutes in ms
/* USER CODE END PD */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct {
    char product_name[32 + 1]; // +1 for null terminator
    char price[16 + 1];
    char discount[16 + 1];
    char producer[24 + 1];
    bool data_valid;
} EslData_t;
/* USER CODE END PTD */

/* Private constants ---------------------------------------------------------*/
/* USER CODE BEGIN PC */

/* USER CODE END PC */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */
static struct ZbZclClusterT *eslServerCluster = NULL;
static UTIL_TIMER_Object_t periodicUpdateTimer;
static EslData_t currentEslData = { .data_valid = false };
static uint16_t currentUpdateIntervalSec = 300; // Stores the interval in seconds
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
/* ESL Server Callbacks */
/* USER CODE MODIFIED: Changed srcInfo to dataInd based on build errors */
static enum ZclStatusCodeT APP_ZIGBEE_EslServerDisplayMessageCallback( struct ZbZclClusterT *cluster, void *arg, struct ZbZclEslDisplayMessageReqT *req, struct ZbApsdeDataIndT *dataInd );
static struct ZbZclEslServerCallbacksT eslServerCallbacks =
{
  .display_message = APP_ZIGBEE_EslServerDisplayMessageCallback,
};

static void PeriodicStatusUpdateTimerCallback(void *arg);
static void TriggerPeriodicStatusUpdate(void);
static void UpdateEpdDisplay(void);
static void PerformFactoryReset(void);
static void APP_ZIGBEE_HandleAttributeWrite(struct ZbZclClusterT *cluster, uint16_t attrId, void *value);
/* USER CODE MODIFIED: Changed callback prototype to use ZbNlmeLeaveConfT */
static void LeaveConfirmCallback(struct ZbNlmeLeaveConfT *conf, void *arg);
/* USER CODE END PFP */

/* Functions Definition ------------------------------------------------------*/

/**
 * @brief  Zigbee application initialization
 * @param  None
 * @retval None
 */
void APP_ZIGBEE_ApplicationInit(void)
{
  LOG_INFO_APP( "ESL ZIGBEE Application Init" );

  /* Initialization of the Zigbee stack */
  APP_ZIGBEE_Init();

  /* Configure Application Form/Join parameters : Startup, Persistence and Start with/without Form/Join */
  stZigbeeAppInfo.eStartupControl = ZbStartTypeJoin; /* Always try to join */
  stZigbeeAppInfo.bPersistNotification = true; /* Persist network info */
  stZigbeeAppInfo.bNwkStartup = true;

  /* USER CODE BEGIN APP_ZIGBEE_ApplicationInit */
  /* Initialize status display */
  LOG_INFO_APP("Display System Initialized");
  /* USER CODE END APP_ZIGBEE_ApplicationInit */

  /* Initialize Zigbee stack layers */
  APP_ZIGBEE_StackLayersInit();
}

/**
 * @brief  Zigbee application start
 * @param  None
 * @retval None
 */
void APP_ZIGBEE_ApplicationStart( void )
{
  /* USER CODE BEGIN APP_ZIGBEE_ApplicationStart */

  /* Display Short Address */
  LOG_INFO_APP( "Use Short Address : 0x%04X", ZbShortAddress( stZigbeeAppInfo.pstZigbee ) );
  LOG_INFO_APP( "%s ready to work !", APP_ZIGBEE_APPLICATION_NAME );

  /* Start the periodic update timer */
  uint32_t timer_interval_ms = DEFAULT_PERIODIC_UPDATE_INTERVAL_MS;
  enum ZclDataTypeT type;
  enum ZclStatusCodeT status;
  // Read persisted or default attribute value for interval
  // Note: Need to ensure eslServerCluster is valid before reading attributes here.
  // It might be better to read this after ZbZclAddEndpoint and cluster allocation.
  if (eslServerCluster != NULL) {
      ZbZclAttrRead(eslServerCluster, ZCL_ESL_ATTR_PERIODIC_UPDATE_INTERVAL, &type, &currentUpdateIntervalSec, sizeof(currentUpdateIntervalSec), NULL);
      if (currentUpdateIntervalSec > 0) {
          timer_interval_ms = (uint32_t)currentUpdateIntervalSec * 1000;
      }
  }
  LOG_INFO_APP("Starting periodic update timer with interval: %lu ms", timer_interval_ms);
  UTIL_TIMER_Create(&periodicUpdateTimer, timer_interval_ms, UTIL_TIMER_PERIODIC, PeriodicStatusUpdateTimerCallback, NULL);
  UTIL_TIMER_Start(&periodicUpdateTimer);

  /* Start Operational LED Pattern */
  APP_BSP_LedPatternStart(LED_PATTERN_OPERATIONAL);

  /* USER CODE END APP_ZIGBEE_ApplicationStart */

  /* --- Always-On Implementation --- */
  /* Disable Stop Mode - Keep device in Run mode */
  LOG_INFO_APP("Disabling Stop Mode for Always-On operation.");
  UTIL_LPM_SetStopMode( 1 << CFG_LPM_APP, UTIL_LPM_DISABLE );
#if (CFG_LPM_STDBY_SUPPORTED > 0)
  /* Also disable Off Mode if supported/enabled */
  UTIL_LPM_SetOffMode( 1 << CFG_LPM_APP, UTIL_LPM_DISABLE );
#endif /* CFG_LPM_STDBY_SUPPORTED */
}

/**
 * @brief  Zigbee persistence startup
 * @param  None
 * @retval None
 */
void APP_ZIGBEE_PersistenceStartup(void)
{
  /* USER CODE BEGIN APP_ZIGBEE_PersistenceStartup */
  LOG_INFO_APP("Persistence Startup: Reading attributes...");
  // Read persisted attributes if necessary (e.g., interval)
  // Note: Attribute values are already read in APP_ZIGBEE_ApplicationStart for the timer
  /* USER CODE END APP_ZIGBEE_PersistenceStartup */
}

/**
 * @brief  Configure Zigbee application endpoints
 * @param  None
 * @retval None
 */
void APP_ZIGBEE_ConfigEndpoints(void)
{
  struct ZbApsmeAddEndpointReqT   stRequest;
  struct ZbApsmeAddEndpointConfT  stConfig;
  /* USER CODE BEGIN APP_ZIGBEE_ConfigEndpoints1 */

  /* USER CODE END APP_ZIGBEE_ConfigEndpoints1 */

  /* Add EndPoint */
  memset( &stRequest, 0, sizeof( stRequest ) );
  memset( &stConfig, 0, sizeof( stConfig ) );

  stRequest.profileId = APP_ZIGBEE_PROFILE_ID;
  stRequest.deviceId = APP_ZIGBEE_DEVICE_ID;
  stRequest.endpoint = APP_ZIGBEE_ENDPOINT;
  ZbZclAddEndpoint( stZigbeeAppInfo.pstZigbee, &stRequest, &stConfig );
  assert( stConfig.status == ZB_STATUS_SUCCESS );

  /* USER CODE BEGIN APP_ZIGBEE_ConfigEndpoints_AddClusters */
  /* Add ESL Custom Server Cluster */
  eslServerCluster = ZbZclEslServerAlloc( stZigbeeAppInfo.pstZigbee, APP_ZIGBEE_ENDPOINT, &eslServerCallbacks, NULL );
  assert( eslServerCluster != NULL );
  ZbZclClusterEndpointRegister( eslServerCluster );
  /* USER CODE END APP_ZIGBEE_ConfigEndpoints_AddClusters */
}

/**
 * @brief  Set Group Addressing mode (if used)
 * @param  None
 * @retval Always returns false for this application as group addressing is not used.
 */
bool APP_ZIGBEE_ConfigGroupAddr( void )
{
  /* USER CODE BEGIN APP_ZIGBEE_ConfigGroupAddr */
  // Group addressing is not typically used for specific device updates like ESL
  /* USER CODE END APP_ZIGBEE_ConfigGroupAddr */
  return false;
}

/**
 * @brief  Return the Startup Configuration
 * @param  pstConfig  Configuration structure to fill
 * @retval None
 */
void APP_ZIGBEE_GetStartupConfig( struct ZbStartupT * pstConfig )
{
  /* Attempt to join a zigbee network */
  ZbStartupConfigGetProDefaults( pstConfig );

  /* Using the default HA preconfigured Link Key */
  memcpy( pstConfig->security.preconfiguredLinkKey, sec_key_ha, ZB_SEC_KEYSIZE );

  /* Setting up additional startup configuration parameters */
  pstConfig->startupControl = stZigbeeAppInfo.eStartupControl;
  pstConfig->channelList.count = 1;
  pstConfig->channelList.list[0].page = 0;
  pstConfig->channelList.list[0].channelMask = APP_ZIGBEE_CHANNEL_MASK;

  /* Set the TX-Power */
  if ( APP_ZIGBEE_SetTxPower( APP_ZIGBEE_TX_POWER ) == false )
  {
    LOG_ERROR_APP( "Switching to %d dB failed.", APP_ZIGBEE_TX_POWER );
    return;
  }

  /* USER CODE BEGIN APP_ZIGBEE_GetStartupConfig */
  /* USER CODE MODIFIED: Removed deviceType setting based on build error */
  /* Device type (Router) is typically set by the project configuration/CubeMX */
  // pstConfig->deviceType = ZB_NWK_DEVICE_TYPE_ROUTER; // Ensure it's a Router (Always On)
  /* USER CODE END APP_ZIGBEE_GetStartupConfig */
}

/**
 * @brief  Manage a New Device on Network (called only if Coord or Router).
 * @param  iShortAddress      Short Address of new Device
 * @param  dlExtendedAddress  Extended Address of new Device
 * @param  cCapability        Capability of new Device
 * @retval Group Address
 */
void APP_ZIGBEE_SetNewDevice( uint16_t iShortAddress, uint64_t dlExtendedAddress, uint8_t cCapability )
{
  LOG_INFO_APP( "New Device (%d) on Network : with Extended ( " LOG_DISPLAY64() " ) and Short ( 0x%04X ) Address.", cCapability, LOG_NUMBER64( dlExtendedAddress ), iShortAddress );

  /* USER CODE BEGIN APP_ZIGBEE_SetNewDevice */
  // Could potentially trigger binding here if needed
  /* USER CODE END APP_ZIGBEE_SetNewDevice */
}

/**
 * @brief  Print application information to the console
 * @param  None
 * @retval None
 */
void APP_ZIGBEE_PrintApplicationInfo(void)
{
  LOG_INFO_APP( "**********************************************************" );
  LOG_INFO_APP( "Network config : CENTRALIZED ROUTER" );

  /* USER CODE BEGIN APP_ZIGBEE_PrintApplicationInfo1 */
  LOG_INFO_APP( "Application Flashed : Zigbee %s %s", APP_ZIGBEE_APPLICATION_NAME, APP_ZIGBEE_APPLICATION_OS_NAME );

  /* USER CODE END APP_ZIGBEE_PrintApplicationInfo1 */
  LOG_INFO_APP( "Channel used: %d.", APP_ZIGBEE_CHANNEL );

  APP_ZIGBEE_PrintGenericInfo();

  LOG_INFO_APP( "Clusters allocated are:" );
  /* USER CODE BEGIN APP_ZIGBEE_PrintApplicationInfo_Clusters */
  LOG_INFO_APP( "ESL Custom Cluster (0x%04X) on Endpoint %d.", ZCL_CLUSTER_ESL_CUSTOM, APP_ZIGBEE_ENDPOINT );
  /* USER CODE END APP_ZIGBEE_PrintApplicationInfo_Clusters */

  LOG_INFO_APP( "**********************************************************" );
}

/* USER CODE BEGIN FD_LOCAL_FUNCTIONS */

/**
 * @brief Callback for the ESL Server displayMessage command.
 * USER CODE MODIFIED: Changed srcInfo to dataInd based on build errors
 */
static enum ZclStatusCodeT APP_ZIGBEE_EslServerDisplayMessageCallback( struct ZbZclClusterT *cluster, void *arg, struct ZbZclEslDisplayMessageReqT *req, struct ZbApsdeDataIndT *dataInd )
{
    LOG_INFO_APP("Received displayMessage command");
    APP_BSP_LedPatternStart(LED_PATTERN_MSG_RECEIVED); // Indicate message received

    // Copy data safely, ensuring null termination
    strncpy(currentEslData.product_name, req->product_name, sizeof(currentEslData.product_name) - 1);
    currentEslData.product_name[sizeof(currentEslData.product_name) - 1] = '\0';

    strncpy(currentEslData.price, req->price, sizeof(currentEslData.price) - 1);
    currentEslData.price[sizeof(currentEslData.price) - 1] = '\0';

    strncpy(currentEslData.discount, req->discount, sizeof(currentEslData.discount) - 1);
    currentEslData.discount[sizeof(currentEslData.discount) - 1] = '\0';

    strncpy(currentEslData.producer, req->producer, sizeof(currentEslData.producer) - 1);
    currentEslData.producer[sizeof(currentEslData.producer) - 1] = '\0';

    currentEslData.data_valid = true;

    LOG_INFO_APP("  Product: %s", currentEslData.product_name);
    LOG_INFO_APP("  Price: %s", currentEslData.price);
    LOG_INFO_APP("  Discount: %s", currentEslData.discount);
    LOG_INFO_APP("  Producer: %s", currentEslData.producer);

    // Update the EPD display
    UpdateEpdDisplay();

    // Update the status attribute (assuming EPD update is synchronous for now)
    // In a real scenario, check EPD status flags if available
    uint8_t update_status = ESL_UPDATE_STATUS_SUCCESS; // Assume success for placeholder
    /* USER CODE MODIFIED: Corrected ZbZclAttrWrite call based on user-provided prototype & build error */
    /* enum ZclStatusCodeT ZbZclAttrWrite(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *src, uint16_t attr_id, */
    /*                                     const uint8_t *attr_data, unsigned int max_len, ZclWriteModeT mode); */
    /* Assuming ZCL_WRITE_MODE_NORMAL is the correct mode. Check zcl.h for ZclWriteModeT definition. */
    /* Pass pointer to source address (&dataInd->src) */
    ZbZclAttrWrite(cluster, &dataInd->src, ZCL_ESL_ATTR_LAST_UPDATE_STATUS,
                   &update_status, sizeof(update_status), ZCL_WRITE_MODE_NORMAL);

    // Optionally, send default response if required by ZCL spec
    // ZbZclSendDefaultRsp(cluster, srcInfo, zclHdr->cmdId, ZCL_STATUS_SUCCESS);

    return ZCL_STATUS_SUCCESS;
}

/**
 * @brief Log display update and update status
 */
static void UpdateEpdDisplay(void)
{
    if (!currentEslData.data_valid) {
        LOG_WARNING_APP("UpdateEpdDisplay: No valid data to display");
        return;
    }

    LOG_INFO_APP("Display Update:");
    LOG_INFO_APP("  Product: %s", currentEslData.product_name);
    LOG_INFO_APP("  Price: %s", currentEslData.price);
    LOG_INFO_APP("  Discount: %s", currentEslData.discount);
    LOG_INFO_APP("  Producer: %s", currentEslData.producer);
    LOG_INFO_APP("Display Update Complete");

    // Update status
    uint8_t update_status = ESL_UPDATE_STATUS_SUCCESS;
    if (eslServerCluster != NULL) {
        ZbZclAttrWrite(eslServerCluster, NULL, ZCL_ESL_ATTR_LAST_UPDATE_STATUS,
                      &update_status, sizeof(update_status), ZCL_WRITE_MODE_NORMAL);
    }

    // Flash LED to indicate update
    APP_BSP_LedPatternStart(LED_PATTERN_STATUS_SENT);
}

/**
 * @brief Timer callback for sending periodic status updates.
 */
static void PeriodicStatusUpdateTimerCallback(void *arg)
{
    UNUSED(arg);
    LOG_INFO_APP("Periodic Status Update Timer Elapsed");

    // Ensure we're in a valid state to send updates
    if (!currentEslData.data_valid) {
        LOG_WARNING_APP("No valid data to send in periodic update");
        return;
    }

    // Process the periodic update
    TriggerPeriodicStatusUpdate();
}

/**
 * @brief Triggers the sending of the periodic status update command.
 */
static void TriggerPeriodicStatusUpdate(void)
{
    LOG_INFO_APP("Triggering periodic status update");

    // First check if the cluster is valid
    if (eslServerCluster == NULL) {
        LOG_ERROR_APP("ESL Server Cluster not initialized");
        return;
    }

    // Then check network status
    if (!APP_ZIGBEE_IsAppliJoinNetwork()) {
        LOG_WARNING_APP("Cannot send Periodic Status Update: Not joined to network");
        return;
    }

    struct ZbApsAddrT dstAddr;
    struct ZbZclEslPeriodicStatusUpdateReqT statusReq;
    enum ZclStatusCodeT zclStatus;
    uint8_t last_status = ESL_UPDATE_STATUS_SUCCESS;

    // Read current status attribute
    enum ZclDataTypeT type;
    if (ZbZclAttrRead(eslServerCluster, ZCL_ESL_ATTR_LAST_UPDATE_STATUS, &type, &last_status, sizeof(last_status), false) != ZCL_STATUS_SUCCESS) {
        LOG_WARNING_APP("Failed to read last update status, using default");
    }

    // Setup destination address
    memset(&dstAddr, 0, sizeof(dstAddr));
    dstAddr.mode = ZB_APSDE_ADDRMODE_SHORT;
    dstAddr.endpoint = APP_ZIGBEE_ENDPOINT;
    dstAddr.nwkAddr = 0x0000; // Coordinator address

    // Setup status request
    statusReq.label_id = ZbExtendedAddress(stZigbeeAppInfo.pstZigbee);
    statusReq.update_status = last_status;

    LOG_INFO_APP("Sending Periodic Status Update (Status: %d) to Addr: 0x%04X, EP: %d",
                 statusReq.update_status, dstAddr.nwkAddr, dstAddr.endpoint);

    // Send the command with error handling
    zclStatus = ZbZclEslServerSendPeriodicStatusUpdate(eslServerCluster, &dstAddr, &statusReq, NULL, NULL);

    if (zclStatus == ZCL_STATUS_SUCCESS) {
        LOG_INFO_APP("Periodic Status Update sent successfully");
        APP_BSP_LedPatternStart(LED_PATTERN_STATUS_SENT);
    } else {
        LOG_ERROR_APP("Failed to send Periodic Status Update: 0x%02X", zclStatus);
        // Write error status to attribute
        uint8_t error_status = ESL_UPDATE_STATUS_OTHER_ERROR;
        ZbZclAttrWrite(eslServerCluster, NULL, ZCL_ESL_ATTR_LAST_UPDATE_STATUS,
                      &error_status, sizeof(error_status), ZCL_WRITE_MODE_NORMAL);
    }
}

/**
 * @brief Handles the short press action for Button 1.
 */
void APP_BSP_Button1ShortPressAction(void)
{
    LOG_INFO_APP("Button 1 Short Press: Forcing Status Update and Display Refresh.");

    // 1. Force immediate status update
    TriggerPeriodicStatusUpdate();

    // 2. Force display refresh (redraw current data)
    UpdateEpdDisplay();
}

/**
 * @brief Handles the long press action for Button 1 (Factory Reset).
 */
void APP_BSP_Button1LongPressAction(void)
{
    /* USER CODE MODIFIED: Corrected LOG_WARN_APP typo */
    LOG_WARNING_APP("Button 1 Long Press: Initiating Factory Reset...");
    PerformFactoryReset();
}

/**
 * @brief Performs factory reset: leaves network, clears data, rejoins.
 */
static void PerformFactoryReset(void)
{
    /* USER CODE MODIFIED: Corrected ZbLeaveReq call based on user-provided prototype */
    /* enum ZbStatusCodeT ZbLeaveReq(struct ZigBeeT *zb, void (*callback)(struct ZbNlmeLeaveConfT *conf, void *arg), void *cbarg); */
    enum ZbStatusCodeT status;

    LOG_INFO_APP("Leaving Zigbee network...");
    APP_BSP_LedPatternStart(LED_PATTERN_SEARCHING); // Indicate searching state

    // Stop periodic timer
    UTIL_TIMER_Stop(&periodicUpdateTimer);

    // Clear local data
    memset(&currentEslData, 0, sizeof(currentEslData));
    currentEslData.data_valid = false;

    // Clear relevant persisted data (Zigbee stack handles network data clearing on leave)
    // If custom attributes were persisted, clear them here.
    // Example: ZbPersistDelete(persist_id_for_esl_data);

    /* Initiate Zigbee Leave Request - Note: Request parameters (deviceAddr, rejoin, removeChild) */
    /* are handled internally by the stack or need separate configuration before calling ZbLeaveReq. */
    /* Check stack documentation for how to configure leave parameters if needed. */

    /* Call ZbLeaveReq with callback */
    status = ZbLeaveReq(stZigbeeAppInfo.pstZigbee, LeaveConfirmCallback, NULL);

    if (status == ZB_STATUS_SUCCESS)
    {
        LOG_INFO_APP("Leave request sent successfully.");
        // Callback LeaveConfirmCallback will be called upon completion.
    }
    else
    {
        LOG_ERROR_APP("Failed to send Leave request: 0x%02X", status);
        // Handle error - maybe try again or indicate error state
        APP_BSP_LedPatternStart(LED_PATTERN_ERROR);
    }
}

/**
 * @brief Callback function for ZbLeaveReq confirmation.
 * USER CODE MODIFIED: Changed argument type to ZbNlmeLeaveConfT*
 */
static void LeaveConfirmCallback(struct ZbNlmeLeaveConfT *conf, void *arg)
{
    /* USER CODE MODIFIED: Access status from ZbNlmeLeaveConfT */
    if (conf->status == ZB_STATUS_SUCCESS)
    {
        LOG_INFO_APP("Successfully left the network.");
        // Stack should attempt rejoin automatically based on startup config.
        // If not, might need manual trigger:
        // APP_ZIGBEE_NwkFormOrJoinTaskInit();
    }
    else
    {
        LOG_ERROR_APP("Leave confirmation failed: 0x%02X", conf->status);
        // Handle failure - maybe retry leave or indicate persistent error
        APP_BSP_LedPatternStart(LED_PATTERN_ERROR);
    }
}

/**
 * @brief Callback for attribute write operations (e.g., changing update interval).
 *        NOTE: This needs to be integrated into the ZCL attribute handling, potentially
 *        by modifying ZbZclAttrWrite or adding a specific callback in esl_cluster.c.
 *        This is a placeholder showing the logic.
 */
static void APP_ZIGBEE_HandleAttributeWrite(struct ZbZclClusterT *cluster, uint16_t attrId, void *value)
{
    if (cluster == eslServerCluster && attrId == ZCL_ESL_ATTR_PERIODIC_UPDATE_INTERVAL)
    {
        uint16_t newIntervalSec = *(uint16_t*)value;
        LOG_INFO_APP("Periodic Update Interval attribute written: %d seconds", newIntervalSec);

        if (newIntervalSec > 0 && newIntervalSec != currentUpdateIntervalSec)
        {
            currentUpdateIntervalSec = newIntervalSec;
            uint32_t newIntervalMs = (uint32_t)newIntervalSec * 1000;

            LOG_INFO_APP("Restarting periodic update timer with interval: %lu ms", newIntervalMs);
            UTIL_TIMER_Stop(&periodicUpdateTimer);
            UTIL_TIMER_SetPeriod(&periodicUpdateTimer, newIntervalMs);
            UTIL_TIMER_Start(&periodicUpdateTimer);
        }
        else if (newIntervalSec == 0)
        {              LOG_WARNING_APP("Disabling periodic updates (interval set to 0).");
             UTIL_TIMER_Stop(&periodicUpdateTimer);
             currentUpdateIntervalSec = 0;
        }
    }
}

/* USER CODE END FD_LOCAL_FUNCTIONS */

