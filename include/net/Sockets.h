#pragma once

namespace Sockets {

  void init();
  void loop();

  // Tells every connected controller that the session is over so the page
  // can redirect itself back to the games hub.
  void closeAll();

}
