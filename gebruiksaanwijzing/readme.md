# Gebruiksaanwijzing

### Opladen / vervangen batterijen
Batterijen kunnen op geladen worden in elke oplader voor 18650 batterijen.
Ik heb hiervoor een oude vipe gebruik maar je kunt ook de lader van bevoorbeeld een scheermachine gebruiken.

### Draadloze communicatie
#### verbinding maken
Zorg dat de microcontroller spanning krijgt
Ga gaan kijken naar in de bluetooth van je pc en zoek de naam die in het programma geven is (BrancoDD Auto).
Als het bluetooth appartaat verbonden is.
In appartaatbeheer kijken op welk COM poort de auto zit.
Dan selecteer je die COM poort in arduino IDE en dan kan je beginnen met de draadloze communicatie.

#### Commando's

debug<br>
help<br>
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
De robot rijdt stabiel met volgende parameters:<br> 
> Cycle: 15000<br>
> Power: 218<br>
> Diff: 0.80<br>
> Kp: 6.00<br>
> Ki: 0.10<br>
> Kd: 0.30<br>


### Start/stop button
Start/stop button, is niet aanwezig doordat er geen plaats is op de print was. 
