/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "app_common.h"
#include "app_conf.h"
#include "log_module.h"
#include "app_entry.h"
#include "app_zigbee.h"
#include "dbg_trace.h"
#include "zcl/zcl.h"
#include "esl_cluster.h"

/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private defines -----------------------------------------------------------*/
#define ESL_SERVER_MAX_ATTRIBUTES 2
/* USER CODE REMOVED: Command arrays not used in this stack version */
// #define ESL_SERVER_MAX_COMMANDS_RX 1
// #define ESL_SERVER_MAX_COMMANDS_TX 1

/* Private variables ---------------------------------------------------------*/
/* ESL Cluster Attribute Definitions */
static struct ZbZclAttrT esl_server_attr_list[ESL_SERVER_MAX_ATTRIBUTES];
static uint16_t esl_attr_periodic_update_interval = 300; // Default 5 minutes
static uint8_t esl_attr_last_update_status = ESL_UPDATE_STATUS_SUCCESS;

/* USER CODE REMOVED: Command definition arrays removed as ZbZclCommandDefT/Register not used */
// static const struct ZbZclCommandDefT esl_server_cmds_rx[] = {
//     { ZCL_ESL_CMD_DISPLAY_MESSAGE, ZCL_DIRECTION_TO_SERVER, ZCL_CMD_FLAG_CLUSTER_SPECIFIC_PLACEHOLDER, 0 },
// };
// static const struct ZbZclCommandDefT esl_server_cmds_tx[] = {
//     { ZCL_ESL_CMD_PERIODIC_STATUS_UPDATE, ZCL_DIRECTION_TO_CLIENT, ZCL_CMD_FLAG_CLUSTER_SPECIFIC_PLACEHOLDER, 0 },
// };

/* Private function prototypes -----------------------------------------------*/
static enum ZclStatusCodeT esl_cluster_handler(
    struct ZbZclClusterT *cluster,
    struct ZbZclHeaderT *zclHdr,
    struct ZbApsdeDataIndT *dataInd
);

/* Public functions ----------------------------------------------------------*/

/**
 * @brief Allocate and initialize the ESL Custom Cluster Server instance.
 * @param zb Zigbee stack instance.
 * @param endpoint Endpoint ID.
 * @param callbacks Pointer to the application callback structure.
 * @param arg Optional argument for callbacks.
 * @return Pointer to the allocated cluster instance, or NULL on failure.
 */
struct ZbZclClusterT* ZbZclEslServerAlloc(struct ZigBeeT *zb, uint8_t endpoint, struct ZbZclEslServerCallbacksT *callbacks, void *arg)
{
    struct ZbZclClusterT *cluster;
    enum ZclStatusCodeT status;
    unsigned int alloc_sz = sizeof(struct ZbZclClusterT); // Use sizeof as placeholder

    /* USER CODE MODIFIED: Using user-provided ZbZclClusterAlloc signature & corrected direction macro */
    cluster = ZbZclClusterAlloc(zb, alloc_sz, ZCL_CLUSTER_ESL_CUSTOM, endpoint, ZCL_DIRECTION_TO_SERVER);
    if (cluster == NULL) {
        LOG_ERROR_APP("Failed to allocate ESL Cluster");
        return NULL;
    }

    /* Set manufacturer-specific ID */
    cluster->mfrCode = ESL_MANUFACTURER_ID;

    /* Store callbacks in cluster app_cb_arg */
    cluster->app_cb_arg = callbacks;

    /* Initialize attribute list */
    memset(esl_server_attr_list, 0, sizeof(esl_server_attr_list));

    /* Setup PeriodicUpdateInterval attribute */
    esl_server_attr_list[0].attributeId = ZCL_ESL_ATTR_PERIODIC_UPDATE_INTERVAL;
    esl_server_attr_list[0].dataType = ZCL_DATATYPE_UNSIGNED_16BIT;
    esl_server_attr_list[0].flags = ZCL_ATTR_FLAG_WRITABLE | ZCL_ATTR_FLAG_REPORTABLE | ZCL_ATTR_FLAG_CB_READ | ZCL_ATTR_FLAG_CB_WRITE;
    esl_server_attr_list[0].customValSz = sizeof(uint16_t);
    esl_server_attr_list[0].range.min = 1;         /* Minimum 1 second */
    esl_server_attr_list[0].range.max = 3600;      /* Maximum 1 hour */
    esl_server_attr_list[0].reporting.interval_min = 1;    /* 1 second minimum */
    esl_server_attr_list[0].reporting.interval_max = 3600; /* 1 hour maximum */

    /* Setup LastUpdateStatus attribute */
    esl_server_attr_list[1].attributeId = ZCL_ESL_ATTR_LAST_UPDATE_STATUS; 
    esl_server_attr_list[1].dataType = ZCL_DATATYPE_ENUMERATION_8BIT;
    esl_server_attr_list[1].flags = ZCL_ATTR_FLAG_REPORTABLE | ZCL_ATTR_FLAG_CB_READ;
    esl_server_attr_list[1].customValSz = sizeof(uint8_t);
    esl_server_attr_list[1].range.min = 0;         /* Update status min value */
    esl_server_attr_list[1].range.max = 5;         /* Update status max value - adjust based on your status enum */
    esl_server_attr_list[1].reporting.interval_min = 0;    /* Report immediately on change */
    esl_server_attr_list[1].reporting.interval_max = 300;  /* Report at least every 5 minutes */

    /* Register attributes */
    /* Register attributes */
    status = ZbZclAttrAppendList(cluster, esl_server_attr_list, ESL_SERVER_MAX_ATTRIBUTES);
    if (status != ZCL_STATUS_SUCCESS) {
        LOG_ERROR_APP("Failed to register ESL attributes: %d", status);
        ZbZclClusterFree(cluster);
        return NULL;
    }

    /* Set the cluster's callback argument */
    ZbZclClusterSetCallbackArg(cluster, callbacks);

    /* Register the cluster's command handler */
    status = ZbZclClusterEndpointRegister(cluster);
    if (status != ZCL_STATUS_SUCCESS) {
        LOG_ERROR_APP("Failed to register ESL command handler: %d", status);
        ZbZclClusterFree(cluster);
        return NULL;
    }

    LOG_INFO_APP("ESL Custom Cluster Server allocated successfully on endpoint %d", endpoint);
    return cluster;
}

/**
 * @brief Send the Periodic Status Update command.
 * @param cluster Pointer to the ESL cluster instance.
 * @param dst Destination address information.
 * @param req Pointer to the command payload structure.
 * @param callback Optional transaction callback.
 * @param arg Optional argument for the transaction callback.
 * @return ZCL status code.
 */
enum ZclStatusCodeT ZbZclEslServerSendPeriodicStatusUpdate(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst, struct ZbZclEslPeriodicStatusUpdateReqT *req, void* callback, void *arg)
{
    struct ZbZclHeaderT zclHdr;
    enum ZclStatusCodeT status;

    /* Build the ZCL header */
    ZbZclHeaderBuild(cluster, &zclHdr, ZCL_ESL_CMD_PERIODIC_STATUS_UPDATE,
                     ZCL_DIRECTION_TO_CLIENT, false, ESL_MANUFACTURER_ID);

    /* Send the request */
    status = ZbZclServerReq(cluster, dst, &zclHdr,
                           req, sizeof(struct ZbZclEslPeriodicStatusUpdateReqT),
                           callback, arg);

    return status;
}

/* Private functions ---------------------------------------------------------*/

/**
 * @brief Internal handler for received ESL cluster-specific commands.
 * @param cluster Pointer to the ESL cluster instance.
 * @param zclHdr Pointer to the received ZCL header.
 * @param dataInd Pointer to the APSDE data indication structure containing payload and source info.
 * @return ZCL status code.
 */
static enum ZclStatusCodeT esl_cluster_handler(
    struct ZbZclClusterT *cluster,
    struct ZbZclHeaderT *zclHdr,
    struct ZbApsdeDataIndT *dataInd)
{
    struct ZbZclEslServerCallbacksT *callbacks = (struct ZbZclEslServerCallbacksT *)cluster->app_cb_arg;
    enum ZclStatusCodeT status = ZCL_STATUS_SUCCESS;
    uint8_t *payload = dataInd->asdu;
    unsigned int payloadLen = dataInd->asduLength;

    /* For manufacturer-specific command, verify manufacturer code */
    if (zclHdr->frameCtrl.manufacturer) {
        if (zclHdr->manufacturerCode != ESL_MANUFACTURER_ID) {
            return ZCL_STATUS_UNSUPP_COMMAND;
        }
    } else {
        /* Non-manufacturer commands not supported in this cluster */
        return ZCL_STATUS_UNSUPP_COMMAND;
    }

    /* Handle commands based on ID */
    switch (zclHdr->cmdId) {
        case ZCL_ESL_CMD_DISPLAY_MESSAGE:
            if (callbacks != NULL && callbacks->display_message != NULL) {
                struct ZbZclEslDisplayMessageReqT req;
                uint8_t *ptr = payload;
                unsigned int expectedLen = 0;

                // Basic size check before parsing
                if (payloadLen < (1 + 1 + 1 + 1)) { // Minimum length for lengths only
                    status = ZCL_STATUS_MALFORMED_COMMAND;
                    break;
                }

                // Parse product_name
                req.product_name_len = *ptr++;
                if (req.product_name_len > sizeof(req.product_name) || (ptr + req.product_name_len > payload + payloadLen)) {
                    status = ZCL_STATUS_MALFORMED_COMMAND;
                    break;
                }
                memcpy(req.product_name, ptr, req.product_name_len);
                ptr += req.product_name_len;
                expectedLen += (1 + req.product_name_len);

                // Parse price
                if (ptr >= payload + payloadLen) { status = ZCL_STATUS_MALFORMED_COMMAND; break; }
                req.price_len = *ptr++;
                 if (req.price_len > sizeof(req.price) || (ptr + req.price_len > payload + payloadLen)) {
                    status = ZCL_STATUS_MALFORMED_COMMAND;
                    break;
                }
                memcpy(req.price, ptr, req.price_len);
                ptr += req.price_len;
                expectedLen += (1 + req.price_len);

                // Parse discount
                if (ptr >= payload + payloadLen) { status = ZCL_STATUS_MALFORMED_COMMAND; break; }
                req.discount_len = *ptr++;
                 if (req.discount_len > sizeof(req.discount) || (ptr + req.discount_len > payload + payloadLen)) {
                    status = ZCL_STATUS_MALFORMED_COMMAND;
                    break;
                }
                memcpy(req.discount, ptr, req.discount_len);
                ptr += req.discount_len;
                expectedLen += (1 + req.discount_len);

                // Parse producer
                if (ptr >= payload + payloadLen) { status = ZCL_STATUS_MALFORMED_COMMAND; break; }
                req.producer_len = *ptr++;
                 if (req.producer_len > sizeof(req.producer) || (ptr + req.producer_len > payload + payloadLen)) {
                    status = ZCL_STATUS_MALFORMED_COMMAND;
                    break;
                }
                memcpy(req.producer, ptr, req.producer_len);
                ptr += req.producer_len;
                expectedLen += (1 + req.producer_len);

                // Check if we consumed the exact payload length
                if (expectedLen != payloadLen) {
                     status = ZCL_STATUS_MALFORMED_COMMAND;
                     break;
                }

                // Null-terminate strings for safety
                if (req.product_name_len < sizeof(req.product_name)) req.product_name[req.product_name_len] = '\0'; else req.product_name[sizeof(req.product_name)-1] = '\0';
                if (req.price_len < sizeof(req.price)) req.price[req.price_len] = '\0'; else req.price[sizeof(req.price)-1] = '\0';
                if (req.discount_len < sizeof(req.discount)) req.discount[req.discount_len] = '\0'; else req.discount[sizeof(req.discount)-1] = '\0';
                if (req.producer_len < sizeof(req.producer)) req.producer[req.producer_len] = '\0'; else req.producer[sizeof(req.producer)-1] = '\0';

                /* Call the application callback */
                status = callbacks->display_message(cluster, cluster->app_cb_arg, &req, dataInd);
            } else {
                status = ZCL_STATUS_UNSUPP_COMMAND;
            }
            break;

        /* Add cases for other commands if needed */

        default:
            /* Command ID is not recognized by this cluster */
            status = ZCL_STATUS_UNSUPP_CLUSTER;
            break;
    }

    return status;
}

