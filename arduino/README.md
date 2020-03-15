This Arduino sketch will receive the matrix data over USB serial and output it onto the connected display.  
The LEDs are controlled using the library [FastLED](https://github.com/FastLED/FastLED) this library supports all major LED chipsets.

## Troubleshooting
- __Missing library__  
Ensure you have downloaded both required Arduino libraries:  
Arduino WebSockets: https://github.com/Links2004/arduinoWebSockets
FastLED: https://github.com/FastLED/FastLED
- __No LEDs light up__  
Double check wiring.  
Double check FastLED setting. [read more](https://github.com/FastLED/FastLED/wiki/Overview)
Check status pixel, see below
Confirm web interface is connected and sending data.
- __Something else__  
If you are having any problems please log an issue.

## Status pixel
The first pixel on your display chain (Pixel Zero) is used as the status pixel when no data is being cast.
 - Black:   Wiring/power issue
 - Green:   Ready