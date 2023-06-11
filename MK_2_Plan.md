# Mark 2 Plans
## Hardware Changes
* Would like to create PCB for ESP32C3 microcontroller to be mounted on, integrate multiple drv 8833 modules into schematic (to add support for all 5 fingers), and integrate 5v power into the schematic (this may not be needed since motors rely on 3v anyways.)
* Update case design and make assembly easier. 
  * Design a better way to secure the Microcontroller and battery without requiring the user to have small fingers.
* Secure connections between buzzer, wire, and control unit.
  * Current running idea is to solder the buzzer to a perf board, then solder larger gauge wire to the perf board.
  * Going to continue idea of adding knot inside control unit box to stop tugging from damaging internal electronics.
## Software Changes
* Wake on detect serial connection and do not go to sleep. Do not buzz any motors unless user does "turn on" action or the board was on before being plugged in.
  * The microcontroller cannot be programmed while sleeping. This new feature should allow programming of the device without the user needing to power it on.
