#ifndef SUBSYSTEM_H_
#define SUBSYSTEM_H_

#include <stddef.h>
#include <stdint.h>

class Subsystem {
public:
  virtual bool setup() = 0;
  virtual void run(uint16_t dt);
};

#endif // SUBSYSTEM_H_
