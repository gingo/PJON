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
  void begin();
  void set_default();
  void set_receiver(PJON_Receiver r);
  void set_error(PJON_Error e);
  void set_custom_pointer(void *pointer);
  uint16_t update();
  uint16_t receive();
  uint16_t receive(uint32_t duration);
  uint16_t send(
    uint8_t id,
    const char *string,
    uint16_t length,
    uint16_t header = PJON_FAIL,
    uint16_t p_id = 0,
    uint16_t requested_port = PJON_BROADCAST
  );
  uint16_t send(
    uint8_t id,
    const uint8_t *b_id,
    const char *string,
    uint16_t length,
    uint16_t header = PJON_FAIL,
    uint16_t p_id = 0,
    uint16_t requested_port = PJON_BROADCAST
  );

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
    _buses[i]->set_custom_pointer(this);
  }
}

// --- private methods ---
// gioblu: I wanna support important bus methods here like begin, recieve, update, ... Make sense?
// gioblu: Considering to inherit PjonRouter from PJON class to allow using set of buses the same way as one bus. For example to use Router in Router :-)
template<class Strategy>
void PjonRouter<Strategy>::begin() {
  for(uint8_t i = 0; i < _busCount; i++) {
    _buses[i]->begin();
  }
};

template<class Strategy>
void PjonRouter<Strategy>::set_default() {
  // set_error(PJON_dummy_error_handler);
  set_receiver(PJON_dummy_receiver_handler);
};

template<class Strategy>
void PjonRouter<Strategy>::set_receiver(PJON_Receiver r) {
  _receiver = r;
};


template<class Strategy>
void PjonRouter<Strategy>::set_error(PJON_Error e) {
  // TODO waiting for https://github.com/gioblu/PJON/issues/122
};

template<class Strategy>
void PjonRouter<Strategy>::set_custom_pointer(void *pointer) {
  _custom_pointer = pointer;
};

template<class Strategy>
uint16_t PjonRouter<Strategy>::update() {
  uint16_t total = 0;
  for(uint8_t i = 0; i < _busCount; i++) {
    total += _buses[i]->update();
  }

  return total;
}

template<class Strategy>
uint16_t PjonRouter<Strategy>::receive() {
  uint16_t total = 0;
  for(uint8_t i = 0; i < _busCount; i++) {
    total += _buses[i]->receive();
  }

  return total;
}


template<class Strategy>
uint16_t PjonRouter<Strategy>::receive(uint32_t duration) {
  // gioblu: I'm not sure how to implement this method correctly.
  uint32_t time = PJON_MICROS();
  while((uint32_t)(PJON_MICROS() - time) <= duration) {
    receive();
  }
  return PJON_FAIL;
};

template<class Strategy>
uint16_t PjonRouter<Strategy>::send(
  uint8_t id,
  const char *string,
  uint16_t length,
  uint16_t header,
  uint16_t p_id,
  uint16_t requested_port
) {
  // TODO implement
  return 0;
};

template<class Strategy>
uint16_t PjonRouter<Strategy>::send(
  uint8_t id,
  const uint8_t *b_id,
  const char *string,
  uint16_t length,
  uint16_t header,
  uint16_t p_id,
  uint16_t requested_port
) {
  // TODO implement
  return 0;
};

template<class Strategy>
void PjonRouter<Strategy>::_on_new_packet_received(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {

  bool handled = strategy.handle_packet_before_this_device(_buses, _busCount, payload, length, packet_info);

  handled = _handle_packet_for_this_device(payload, length, packet_info);

  if(!handled) {
    strategy.handle_packet_after_this_device(_buses, _busCount, payload, length, packet_info);
  }

}

template<class Strategy>
bool PjonRouter<Strategy>::_handle_packet_for_this_device(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {

  for(uint8_t i = 0; i < _busCount; i++) {
    if(PJON<Any>::bus_id_equality(_buses[i]->bus_id, packet_info.receiver_bus_id) &&
        _buses[i]->device_id() == packet_info.receiver_id) {

      PJON_Packet_Info packet_info_changed = packet_info;
      packet_info_changed.custom_pointer = _custom_pointer;

      _receiver(
        payload,
        length,
        packet_info_changed
      );

      return true;
    }

  }

  return false;
}

#endif
