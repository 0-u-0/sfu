#ifndef SRC_SENDER_H_
#define SRC_SENDER_H_

#include "media_types.h"

class Sender {
 public:
  Sender();
  const std::string id_;

  MediaType kind_;
};

#endif /* SRC_SENDER_H_ */
