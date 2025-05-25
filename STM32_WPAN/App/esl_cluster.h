#ifndef ESL_CLUSTER_H
#define ESL_CLUSTER_H

#include "app_conf.h"
#include "zcl/zcl.h"
#include "zigbee.h"

/* Cluster ID */
#define ZCL_CLUSTER_ESL_CUSTOM              0xFC01 // Example Manufacturer Specific Cluster ID

/* Manufacturer ID */
#define ESL_MANUFACTURER_ID                 0x1001 // Example STMicroelectronics ID (Replace if needed)

/* Attribute IDs */
#define ZCL_ESL_ATTR_PERIODIC_UPDATE_INTERVAL 0x0000
#define ZCL_ESL_ATTR_LAST_UPDATE_STATUS     0x0001
#define ZCL_WRITE_MODE_NORMAL 0x00

/* Command IDs */
#define ZCL_ESL_CMD_DISPLAY_MESSAGE         0x00 // Server Received 
#define ZCL_ESL_CMD_PERIODIC_STATUS_UPDATE  0x01 // Server Generated

/* Attribute Values */
enum ZbZclEslUpdateStatusT {
    ESL_UPDATE_STATUS_SUCCESS = 0x00,
    ESL_UPDATE_STATUS_DATA_INVALID = 0x01,
    ESL_UPDATE_STATUS_OTHER_ERROR = 0xFF
};

/* Command Payloads */

/* ZCL_ESL_CMD_DISPLAY_MESSAGE (Client -> Server) */
struct ZbZclEslDisplayMessageReqT {
    uint8_t product_name_len;
    char product_name[32]; // Max 32 chars
    uint8_t price_len;
    char price[16];        // Max 16 chars
    uint8_t discount_len;
    char discount[16];     // Max 16 chars
    uint8_t producer_len;
    char producer[24];     // Max 24 chars
};

/* ZCL_ESL_CMD_PERIODIC_STATUS_UPDATE (Server -> Client) */
struct ZbZclEslPeriodicStatusUpdateReqT {
    uint64_t label_id;      // IEEE address of the ESL
    uint8_t update_status; // enum ZbZclEslUpdateStatusT
};

/* Server Callback Structure */
struct ZbZclEslServerCallbacksT {
    enum ZclStatusCodeT (*display_message)(struct ZbZclClusterT *cluster, void *arg, struct ZbZclEslDisplayMessageReqT *req, struct ZbApsdeDataIndT *dataInd);
};

/* Public Functions */
struct ZbZclClusterT* ZbZclEslServerAlloc(struct ZigBeeT *zb, uint8_t endpoint, struct ZbZclEslServerCallbacksT *callbacks, void *arg);
enum ZclStatusCodeT ZbZclEslServerSendPeriodicStatusUpdate(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst, struct ZbZclEslPeriodicStatusUpdateReqT *req, void* callback, void *arg);

/* Helper Functions */
void ZbZclHeaderBuild(struct ZbZclClusterT *cluster, struct ZbZclHeaderT *zclHdr,
                      uint8_t cmdId, uint8_t direction, bool isGeneric, uint16_t manufacturerCode);
enum ZclStatusCodeT ZbZclServerReq(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst,
                                  struct ZbZclHeaderT *zclHdr, void *payload, unsigned int length,
                                  void (*callback)(struct ZbZclCommandRspT *rsp, void *arg), void *arg);

#endif /* ESL_CLUSTER_H */

