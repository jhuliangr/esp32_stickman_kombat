#pragma once

namespace Audio {

  void init();

  void tick(unsigned long now);

  void playHit();     
  void playBlock();   
  void playKO();      
  void playStart();   

}
