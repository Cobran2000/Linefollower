# Gebruiksaanwijzing

### Opladen / vervangen batterijen
uitleg over het opladen of vervangen van de batterijen

### Draadloze communicatie
#### verbinding maken
uitleg over het verbinden van de robot met laptop / smartphone

#### Commando's

debug
start  
stop  
set cycle [µs]  
set power [0..255]  
set diff [0..1]  
set kp [0..]  
set ki [0..]  
set kd [0..]  
calibrate  

### Kalibratie
Zet de maximale tijd voor de sensor uit te lezen op 2000 micros.
Begint te meten hoe lang het duurt voor dat de condensator van de sensor geen spanning meer geeft.
Dit word dan gezien als wit en voor zwart word er dan 160 micros bij gezet bij de hoogste waarde die voor wit werd gemeten.
Dan is de kalibratie klaar.

### Settings
De robot rijdt stabiel met volgende parameters:  

### Start/stop button
Start/stop button, is niet aanwezig doordat er geen plaats is op de print was. 
