# Waveshare-LCD-1.8-CPP
C++ Code for sending data to a Waveshare 1.8 inch LCD. Mostly just rewritten from the official Python-Pico implementation you can find under "Demo Codes" [here](https://www.waveshare.com/wiki/Pico-LCD-1.8).

# Setup
This library uses [C-Periphery](https://github.com/vsergeev/c-periphery) for GPIO and SPI functionality. Make sure you clone it and include it into the compilation correctly.
You should also change the PIN declarations at the top of the header file. Setting a pin to 15 for example, means that the signal will be outputted to GPIO-15.

# Usage
```C++
LCD_1_8* lcd = new LCD_1_8();
Framebuffer* fb = new Framebuffer();

// Set pixel at 50/50 to red (1.0, 0.0, 0.0, alpha=1.0)
fb.setPixel(50, 50, Color(1, 0, 0));

// Send data
lcd.show(fb);
```

# Getting rid of Color and Framebuffer
Both the Color and the Framebuffer class were added by me to make the usage of this library simpler. You can just remove them if you want, but then you also have to change the code that actually writes the pixels to the screen. You can do this by changing the `spiWriteBuffer()` and `spiTransferColor()` functions. `spiTransferColor()` takes in a Color object and converts it into a 16-bit integer. See the official Waveshare documentation for more information on this.

# ToDo at some point
- show() updates all of the 20480 pixels, maybe make sure something has changed before actually sending the new buffer
