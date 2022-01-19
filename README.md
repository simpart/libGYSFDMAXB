# libGYSFDMAXB
Arduino library for GYSFDMAXB module

You can easily get the current location information without understanding the module specifications.

# Simple Method

```c++
/**
 * get current position 
 * 
 * @param PosInfo * [out] position data
 * @return bool true: successful
 *              false: failed or not positioned yet
 */
bool GYSFDMAXB::getpos (PosInfo *pos);
```

# Sample

```c++
#include "libGYSFDMAXB.h"

GYSFDMAXB *gps;

void setup() {
  Serial.begin(9600);
  gps = new GYSFDMAXB();
}


void loop() {
  PosInfo pos;
  
  if (gps->getpos(&pos)) {
    float lati  = pos.latitude["value"];   // latitude value (ddmm.mmmm)
    float longi = pos.longitude["value"];  // longitude value (ddmm.mmmm)
    
    Serial.println(pos.dmmstr().c_str());  // it is possible to paste the result to google map
  }
  
  delay(1000);
}
```

# Attention
- supported only Hardware Serial
- only 9600 bps
- must be at least 1 second apart
