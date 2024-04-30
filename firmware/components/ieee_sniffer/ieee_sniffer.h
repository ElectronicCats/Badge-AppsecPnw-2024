#pragma once

#include <stdint.h>
#ifndef IEEE_SNIFFER_H
  #define IEEE_SNIFFER_H
  #define TAG_IEEE_SNIFFER "ieee_sniffer"

  #define LIMIT_PACKETS          1000
  #define FRAME_VERSION_STD_2003 0
  #define FRAME_VERSION_STD_2006 1
  #define FRAME_VERSION_STD_2015 2

  #define FRAME_TYPE_BEACON       (0)
  #define FRAME_TYPE_DATA         (1)
  #define FRAME_TYPE_ACK          (2)
  #define FRAME_TYPE_MAC_COMMAND  (3)
  #define FRAME_TYPE_RESERVED     (4)
  #define FRAME_TYPE_MULTIPURPOSE (5)
  #define FRAME_TYPE_FRAGMENT     (6)
  #define FRAME_TYPE_EXTENDED     (7)

  #define ADDR_MODE_NONE     (0)  // PAN ID and address fields are not present
  #define ADDR_MODE_RESERVED (1)  // Reseved
  #define ADDR_MODE_SHORT    (2)  // Short address (16-bit)
  #define ADDR_MODE_LONG     (3)  // Extended address (64-bit)

  #define FRAME_TYPE_BEACON  (0)
  #define FRAME_TYPE_DATA    (1)
  #define FRAME_TYPE_ACK     (2)
  #define FRAME_TYPE_MAC_CMD (3)

typedef struct mac_fcs {
  uint8_t frameType : 3;
  uint8_t secure : 1;
  uint8_t framePending : 1;
  uint8_t ackReqd : 1;
  uint8_t panIdCompressed : 1;
  uint8_t rfu1 : 1;
  uint8_t sequenceNumberSuppression : 1;
  uint8_t informationElementsPresent : 1;
  uint8_t destAddrType : 2;
  uint8_t frameVer : 2;
  uint8_t srcAddrType : 2;
} mac_fcs_t;

/**
 * @brief Callback to handle the IEEE sniffer
 *
 * @param record The tracker profile record
 */
typedef void (*ieee_sniffer_cb_t)(int packets_count);

/**
 * @brief Register the callback for the IEEE sniffer
 *
 * @param callback The callback to handle the IEEE sniffer
 */
void ieee_sniffer_register_cb(ieee_sniffer_cb_t callback);

/**
 * @brief Begin the IEEE sniffer
 */
void ieee_sniffer_begin(void);

/**
 * @brief Stop the IEEE sniffer
 */
void ieee_snifffer_stop(void);
#endif  // IEEE_SNIFFER_H
