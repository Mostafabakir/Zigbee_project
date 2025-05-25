#include "app_common.h"
#include "zcl/zcl.h"
#include "esl_cluster.h"

void ZbZclHeaderBuild(struct ZbZclClusterT *cluster, struct ZbZclHeaderT *zclHdr,
                     uint8_t cmdId, uint8_t direction, bool isGeneric, uint16_t manufacturerCode)
{
    memset(zclHdr, 0, sizeof(struct ZbZclHeaderT));
    zclHdr->frameCtrl.frameType = ZCL_FRAMETYPE_CLUSTER;
    zclHdr->frameCtrl.manufacturer = (manufacturerCode != 0) ? 1U : 0U;
    zclHdr->frameCtrl.direction = direction;
    zclHdr->frameCtrl.noDefaultResp = ZCL_NO_DEFAULT_RESPONSE_FALSE;
    zclHdr->manufacturerCode = manufacturerCode;
    zclHdr->seqNum = ZbZclGetNextSeqnum(cluster->zb);
    zclHdr->cmdId = cmdId;
}

enum ZclStatusCodeT ZbZclServerReq(struct ZbZclClusterT *cluster, const struct ZbApsAddrT *dst,
                                struct ZbZclHeaderT *zclHdr, void *payload, unsigned int length,
                                void (*callback)(struct ZbZclCommandRspT *rsp, void *arg), void *arg)
{
    struct ZbZclCommandReqT cmdReq;
    enum ZclStatusCodeT status;

    /* Configure the ZCL Command Request */
    memset(&cmdReq, 0, sizeof(cmdReq));
    cmdReq.dst = *dst;
    cmdReq.hdr = *zclHdr;
    cmdReq.payload = payload;
    cmdReq.length = length;

    /* Send the request using the core command function */
    status = ZbZclCommandReq(cluster->zb, &cmdReq, callback, arg);
    
    return status;
}
