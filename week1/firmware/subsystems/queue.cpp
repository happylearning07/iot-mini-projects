#include "queue.h"
#include <string.h>

bool Queue::setup() {
  head = 0;
  tail = 0;
  count = 0;
  memset(buffer, 0, sizeof(buffer));
  return true;
}

void Queue::run(uint16_t dt) { (void)dt; }

bool Queue::push(const EncoderResult &result) {
  memcpy(&buffer[head], &result, sizeof(EncoderResult));
  head = (head + 1) % QUEUE_MAX_SIZE;

  if (count < QUEUE_MAX_SIZE) {
    count++;
  } else {
    tail = (tail + 1) % QUEUE_MAX_SIZE;
  }

  return true;
}

bool Queue::pop(EncoderResult &result) {
  if (isEmpty()) {
    return false;
  }

  memcpy(&result, &buffer[tail], sizeof(EncoderResult));
  tail = (tail + 1) % QUEUE_MAX_SIZE;
  count--;

  return true;
}

bool Queue::peek(EncoderResult &result) const {
  if (isEmpty()) {
    return false;
  }

  memcpy(&result, &buffer[tail], sizeof(EncoderResult));
  return true;
}

uint16_t Queue::size() const { return count; }

bool Queue::isEmpty() const { return count == 0; }

bool Queue::isFull() const { return count == QUEUE_MAX_SIZE; }

void Queue::clear() {
  head = 0;
  tail = 0;
  count = 0;
  memset(buffer, 0, sizeof(buffer));
}

uint16_t Queue::capacity() const { return QUEUE_MAX_SIZE; }
