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
  bool handle_packet_before_this_device(PJON<Any> **buses, uint8_t busCount, uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info);
  bool handle_packet_after_this_device(PJON<Any> **buses, uint8_t busCount, uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info);

private:

};

bool Hub::handle_packet_before_this_device(PJON<Any> **buses, uint8_t busCount, uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  // nothing here, we care only for packets for other devices
  return false;
};

bool Hub::handle_packet_after_this_device(PJON<Any> **buses, uint8_t busCount, uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  for(uint8_t i = 0; i < busCount; i++) {
    buses[i]->send_from_id(
      packet_info.sender_id,
      packet_info.sender_bus_id,
      packet_info.receiver_id,
      packet_info.receiver_bus_id,
      (char *)payload,
      length,
      PJON_FAIL, //gioblu: why packet_info.header does not work here? Why it works here? https://github.com/gioblu/PJON/blob/master/examples/ARDUINO/Network/SoftwareBitBang/RecursiveAcknowledge/Router/Router.ino#L26
      packet_info.id,
      packet_info.port
    );
  }

  return true;
};


#endif
