#if ARDUINO >= 100
 #include "Arduino.h"
 #include "SoftwareSerial.h"
#else
 #include "WProgram.h"
 #include "WConstants.h"
 #include "NewSoftSerial.h"
#endif

#include "Thermal.h"
#include "ThermalGrapher.h"

#define PRINTER_RX_PIN 2
#define PRINTER_TX_PIN 3

Thermal printer(PRINTER_RX_PIN, PRINTER_TX_PIN);
ThermalGrapher grapher(&printer, 0, 100);

void setup() {
  // Initialize the printer
  Serial.begin(19200);
  printer.begin();

  printer.println("BEGIN");

  // Minimum value.
  grapher.printValue(0);

  // 10 random points.
  int i;
  for (i = 0; i < 10; i++) {
    grapher.printValue(random(0, 100));
    delay(1000);
  }

  // Maximum value.
  grapher.printValue(100);
  
  printer.println("END");
  
  // Reset the printer
  printer.sleep();
  printer.wake();
  printer.setDefault(); 
}

void loop() {
}
