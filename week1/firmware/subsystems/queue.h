#ifndef QUEUE_H_
#define QUEUE_H_

#include "encoder.h"
#include "subsystem.h"

#ifndef QUEUE_MAX_SIZE
#define QUEUE_MAX_SIZE 16
#endif

class Queue : public Subsystem {
private:
  EncoderResult buffer[QUEUE_MAX_SIZE];
  uint16_t head;
  uint16_t tail;
  uint16_t count;

public:
  bool setup();
  void run(uint16_t dt);

  bool push(const EncoderResult &result);
  bool pop(EncoderResult &result);
  bool peek(EncoderResult &result) const;

  uint16_t size() const;
  bool isEmpty() const;
  bool isFull() const;
  void clear();
  uint16_t capacity() const;
};

#endif // QUEUE_H_
