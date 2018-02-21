/* Hub strategy contributed by Vladislav Gingo Skoumal

  This is stupid routing strategy based on repeating everything to all available
  buses. It behaves like old good ethernet hub
  (https://en.wikipedia.org/wiki/Ethernet_hub)

  Hub strategy is great for testing and environments with low traffic. Avoid
  using it when your network contains cyrcles, otherwise your packets will be
  floating in cyrcle forever. There is no setup needed, just plug and play!

*/

#pragma once

#if defined(PJON_INCLUDE_ROUTER)

class Hub {
public:
  bool handle_packet_before_this_device(
    PJON<Any> **buses,
    uint8_t busCount,
    PJON<Any> * from_bus,
    uint16_t packet_id,
    uint8_t sender_id,
    const uint8_t * sender_bus_id,
    uint8_t receiver_id,
    const uint8_t * receiver_bus_id,
    uint8_t * payload,
    uint16_t length,
    uint16_t header,
    uint16_t port
  );
  bool handle_packet_after_this_device(
    PJON<Any> **buses,
    uint8_t busCount,
    PJON<Any> * from_bus,
    uint16_t packet_id,
    uint8_t sender_id,
    const uint8_t * sender_bus_id,
    uint8_t receiver_id,
    const uint8_t * receiver_bus_id,
    uint8_t * payload,
    uint16_t length,
    uint16_t header,
    uint16_t port
  );

private:

};

bool Hub::handle_packet_before_this_device(
  PJON<Any> **buses,
  uint8_t busCount,
  PJON<Any> * from_bus,
  uint16_t packet_id,
  uint8_t sender_id,
  const uint8_t * sender_bus_id,
  uint8_t receiver_id,
  const uint8_t * receiver_bus_id,
  uint8_t * payload,
  uint16_t length,
  uint16_t header,
  uint16_t port
) {
  // nothing here, we care only for packets for other devices
  return false;
};

bool Hub::handle_packet_after_this_device(
  PJON<Any> **buses,
  uint8_t busCount,
  PJON<Any> * from_bus,
  uint16_t packet_id,
  uint8_t sender_id,
  const uint8_t * sender_bus_id,
  uint8_t receiver_id,
  const uint8_t * receiver_bus_id,
  uint8_t * payload,
  uint16_t length,
  uint16_t header,
  uint16_t port
) {
  for(uint8_t i = 0; i < busCount; i++) {
    if(buses[i] != from_bus) {
      buses[i]->send_from_id(
        sender_id,
        sender_bus_id,
        receiver_id,
        receiver_bus_id,
        (char *)payload,
        length,
        header,
        packet_id,
        port
      );
    }
  }

  return true;
};


#endif
