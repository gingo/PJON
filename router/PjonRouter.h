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

  void _route_packet(
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
  PJON<Any> **_buses = NULL;
  uint8_t _busCount = -1;
  PJON_Receiver _receiver;
  PJON_Error _error;
  uint8_t _currently_receiving_bus = PJON_NOT_ASSIGNED;
  void *_custom_pointer = NULL;
  bool _handle_packet_for_this_device(
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
};

template<class Strategy>
void pjon_router_receiver_handler(uint8_t * payload, uint16_t length, const PJON_Packet_Info &packet_info) {
  ((PjonRouter<Strategy>*)packet_info.custom_pointer)->_route_packet(
    packet_info.id,
    packet_info.sender_id,
    packet_info.sender_bus_id,
    packet_info.receiver_id,
    packet_info.receiver_bus_id,
    payload,
    length,
    packet_info.header,
    packet_info.port
  );
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
    _buses[i]->set_error(_error);
    _buses[i]->set_custom_pointer(this);
  }
}

// --- private methods ---
template<class Strategy>
void PjonRouter<Strategy>::begin() {
  for(uint8_t i = 0; i < _busCount; i++) {
    _buses[i]->begin();
  }
};

template<class Strategy>
void PjonRouter<Strategy>::set_default() {
  set_error(PJON_dummy_error_handler);
  set_receiver(PJON_dummy_receiver_handler);
};

template<class Strategy>
void PjonRouter<Strategy>::set_receiver(PJON_Receiver r) {
  _receiver = r;
};


template<class Strategy>
void PjonRouter<Strategy>::set_error(PJON_Error e) {
  _error = e;
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
    _currently_receiving_bus = i;
    total += _buses[i]->receive();
  }

  _currently_receiving_bus = PJON_NOT_ASSIGNED;

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
  return send(id, _buses[0]->bus_id, string, length, header, p_id, requested_port);
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
  _route_packet(p_id, _buses[0]->device_id(), _buses[0]->bus_id, id, b_id, (uint8_t*)string, length, header, requested_port);
  return 0;
};

template<class Strategy>
void PjonRouter<Strategy>::_route_packet(
    uint16_t packet_id,
    uint8_t sender_id,
    const uint8_t * sender_bus_id,
    uint8_t receiver_id,
    const uint8_t * receiver_bus_id,
    uint8_t * payload,
    uint16_t length,
    uint16_t header,
    uint16_t port) {

  PJON<Any> * from_bus;
  if(_currently_receiving_bus != PJON_NOT_ASSIGNED) {
    from_bus = _buses[_currently_receiving_bus];
  } else {
    from_bus = NULL;
  }

  bool handled = strategy.handle_packet_before_this_device(_buses, _busCount, from_bus, packet_id, sender_id, sender_bus_id, receiver_id, receiver_bus_id, payload, length, header, port);

  if(!handled) {
    handled = _handle_packet_for_this_device(from_bus, packet_id, sender_id, sender_bus_id, receiver_id, receiver_bus_id, payload, length, header, port);
  }

  if(!handled) {
    handled = strategy.handle_packet_after_this_device(_buses, _busCount, from_bus, packet_id, sender_id, sender_bus_id, receiver_id, receiver_bus_id, payload, length, header, port);
  }

  if(handled && from_bus != NULL) {
    from_bus->send_synchronous_acknowledge();
  }

}

template<class Strategy>
bool PjonRouter<Strategy>::_handle_packet_for_this_device(
  PJON<Any> * from_bus,
  uint16_t packet_id,
  uint8_t sender_id,
  const uint8_t * sender_bus_id,
  uint8_t receiver_id,
  const uint8_t * receiver_bus_id,
  uint8_t * payload,
  uint16_t length,
  uint16_t header,
  uint16_t port) {

  for(uint8_t i = 0; i < _busCount; i++) {
    if(PJON<Any>::bus_id_equality(_buses[i]->bus_id, receiver_bus_id) &&
        _buses[i]->device_id() == receiver_id) {

      PJON_Packet_Info packet_info;
      packet_info.id = packet_id;
      packet_info.receiver_id = receiver_id;
      PJON<Any>::copy_bus_id(packet_info.receiver_bus_id, receiver_bus_id);
      packet_info.sender_id = sender_id;
      PJON<Any>::copy_bus_id(packet_info.sender_bus_id, sender_bus_id);
      packet_info.header = header;
      packet_info.port = port;
      packet_info.custom_pointer = _custom_pointer;

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
