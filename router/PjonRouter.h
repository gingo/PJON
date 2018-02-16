#pragma once

#if defined(PJON_INCLUDE_ROUTER)

#include <strategies/Any/Any.h>

template<class Strategy>
class PjonRouter {
public:
  Strategy strategy;

  PjonRouter();
  PjonRouter(PJON<Any> ** buses, uint8_t busCount);

  void set_buses(PJON<Any> ** buses, uint8_t busCount);

  // gioblu: I wanna support important bus methods here like begin, recieve, update, ... Make sense?
  // gioblu: Considering to inherit PjonRouter from PJON class to allow using set of buses the same way as one bus. For example to use Router in Router :-)
  void begin() {
    for(uint8_t i = 0; i < _busCount; i++) {
      _buses[i]->begin();
    }
  };

  void set_default() {
    // set_error(PJON_dummy_error_handler);
    set_receiver(PJON_dummy_receiver_handler);
  };

  void set_receiver(PJON_Receiver r) {
    _receiver = r;
  };

  void set_custom_pointer(void *pointer) {
    _custom_pointer = pointer;
  };

  // TODO implement more PJON methods

  void _on_new_packet_received(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info);

private:
  PJON<Any> **_buses = NULL;
  uint8_t _busCount = -1;
  PJON_Receiver _receiver;
  void *_custom_pointer = NULL;
  bool _handle_packet_for_this_device(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info);
};

template<class Strategy>
void pjon_router_receiver_handler(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  ((PjonRouter<Strategy>*)packet_info.custom_pointer)->_on_new_packet_received(payload, length, packet_info);
};

// --- public methods ---
template<class Strategy>
PjonRouter<Strategy>::PjonRouter() {
  set_default();
};

template<class Strategy>
PjonRouter<Strategy>::PjonRouter(PJON<Any> ** buses, uint8_t busCount) {
  set_buses(buses, busCount);
  set_default();
};

template<class Strategy>
void PjonRouter<Strategy>::set_buses(PJON<Any> ** buses, uint8_t busCount) {
  _buses = buses;
  _busCount = busCount;

  for(uint8_t i = 0; i < _busCount; i++) {
    _buses[i]->set_router(true);
    _buses[i]->set_receiver(pjon_router_receiver_handler<Strategy>);
  }
}

// --- private methods ---
template<class Strategy>
void PjonRouter<Strategy>::_on_new_packet_received(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {

  bool handled = strategy.handle_packet_before_this_device(payload, length, packet_info);

  handled = _handle_packet_for_this_device(payload, length, packet_info);

  if(!handled) {
    strategy.handle_packet(payload, length, packet_info);
  }

}

template<class Strategy>
bool PjonRouter<Strategy>::_handle_packet_for_this_device(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {

  for(uint8_t i = 0; i < _busCount; i++) {
    if(PJON<Any>::bus_id_equality(_buses[i]->bus_id, packet_info.receiver_bus_id) &&
      _buses[i]->device_id() == packet_info.receiver_id) {
        _receiver(
          payload,
          length,
          packet_info
        );
      return true;
    }

  }

  return false;
}

#endif
