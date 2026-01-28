#ifndef CADENCE_H_
#define CADENCE_H_

#include "subsystem.h"

#ifndef DEFAULT_SENSOR_INTERVAL_MS
#define DEFAULT_SENSOR_INTERVAL_MS 1000
#endif

#ifndef DEFAULT_TRANSMISSION_INTERVAL_MS
#define DEFAULT_TRANSMISSION_INTERVAL_MS 10000
#endif

class Cadence : public Subsystem {
private:
  uint16_t sensor_interval;
  uint16_t transmission_interval;
  uint16_t sensor_accumulator;
  uint16_t transmission_accumulator;

public:
  bool setup();
  void run(uint16_t dt);

  void setSensorInterval(uint16_t ms);
  void setTransmissionInterval(uint16_t ms);

  bool shouldUpdateSensor();
  bool shouldTransmit();

  void reset();
};

#endif // CADENCE_H_
