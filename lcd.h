#pragma once

#include "gpio.h"
#include "spi.h"

#define PIN_BL  0
#define PIN_DC  5
#define PIN_RST  6
#define PIN_MOSI 10
#define PIN_SCK 11
#define PIN_CS  26

typedef unsigned char byte;


class Color {
public:
	double r, g, b, a;
	Color() : r(0), g(0), b(0), a(0) {}
	Color(double r, double g, double b, double a = 1) : r(r), g(g), b(b), a(a) {}

  // Converts the color into a 16 bit RGB565 integer
	int to565() {
		int r0 = floor(r * 0b11111);
		int g0 = floor(g * 0b111111);
		int b0 = floor(b * 0b11111);

		return r0 << 11 | g0 << 5 | b0;
	}
};

// Represents a frame, or an array of Colors (pixels)
class Framebuffer {
public:
	Color* pixels;
	int width, height;

	Framebuffer(int width, int height) : width(width), height(height) {
		init();
	}

	void init() {
		pixels = new Color[width * height];
	}

	Color getPixel(int x, int y) {
		return pixels[y * width + x];
	}

  // Sets a pixel to a certain color. If applyAlpha = true, takes the old color of the pixel
  // into account, based on color.a (alpha). If false, it just overwrites it.
	void setPixel(int x, int y, Color color, bool applyAlpha = true) {
		if (applyAlpha) {
			Color old = pixels[y * width + x];
			double r = old.r + (color.r - old.r) * color.a;
			double g = old.g + (color.g - old.g) * color.a;
			double b = old.b + (color.b - old.b) * color.a;

			// assert(r >= 0);

			pixels[y * width + x] = Color(r, g, b);
		} else {
			pixels[y * width + x] = color;
		}
	}
};

class LCD_1_8 {
public:
	int width = 160;
	int height = 128;

	spi_t* spi;
	gpio_t* dc;
	gpio_t* rst;
	gpio_t* mosi;
	gpio_t* sck;
	gpio_t* cs;

	LCD_1_8() {
		spi = spi_new();
		dc = gpio_new();
		rst = gpio_new();
		mosi = gpio_new();
		sck = gpio_new();
		cs = gpio_new();

		if (spi == NULL)
			printf("spi is null\n");
		if (dc == NULL)
			printf("dc is null\n");


		if (spi_open(spi, "/dev/spidev0.0", 0, 10000000) < 0)
			printf("spi_open(): Error!");

		gpio_open(dc, "/dev/gpiochip0", PIN_DC, GPIO_DIR_OUT);
		gpio_open(rst, "/dev/gpiochip0", PIN_RST, GPIO_DIR_OUT);
		//gpio_open(mosi, "/dev/gpiochip0", PIN_MOSI, GPIO_DIR_OUT);
		//gpio_open(sck, "/dev/gpiochip0", PIN_SCK, GPIO_DIR_OUT);
		if (gpio_open(cs, "/dev/gpiochip0", PIN_CS, GPIO_DIR_OUT) < 0) {
			printf(gpio_errmsg(cs));
			exit(1);
		}

		gpio_write(cs, 1);
		gpio_write(dc, 1);

		initDisplay();
	}

	void spiTransferByte(byte cmd) {
		byte* data = (byte*) malloc(sizeof(byte));
		data[0] = cmd;

		if (spi_transfer(spi, data, NULL, 1) < 0)
			printf("spi_tranfer(): Error!");
	}

	void writeCmd(byte cmd) {
		if (gpio_write(cs, 1) < 0)
			printf("gpio_write: Error");

		gpio_write(dc, 0);
		gpio_write(cs, 0);

		spiTransferByte(cmd);

		gpio_write(cs, 1);
	}

	void writeData(byte data) {
		gpio_write(cs, 1);
		gpio_write(dc, 1);
		gpio_write(cs, 0);

		spiTransferByte(data);

		gpio_write(cs, 1);
	}

	void initDisplay() {
		gpio_write(rst, 1);
		gpio_write(rst, 0);
		gpio_write(rst, 1);

		writeCmd(0x36);
		writeData(0x70);

		writeCmd(0x3A);
		writeData(0x05);

		// ST7735 Frame Rate
		writeCmd(0xB1);
		writeData(0x01);
		writeData(0x2C);
		writeData(0x2D);

		writeCmd(0xB2);
		writeData(0x01);
		writeData(0x2C);
		writeData(0x2D);

		writeCmd(0xB3);
		writeData(0x01);
		writeData(0x2C);
		writeData(0x2D);
		writeData(0x01);
		writeData(0x2C);
		writeData(0x2D);

		writeCmd(0xB4); // Column inversion
		writeData(0x07);

		// ST7735R Power Sequence
		writeCmd(0xC0);
		writeData(0xA2);
		writeData(0x02);
		writeData(0x84);
		writeCmd(0xC1);
		writeData(0xC5);

		writeCmd(0xC2);
		writeData(0x0A);
		writeData(0x00);

		writeCmd(0xC3);
		writeData(0x8A);
		writeData(0x2A);
		writeCmd(0xC4);
		writeData(0x8A);
		writeData(0xEE);

		writeCmd(0xC5); // VCOM
		writeData(0x0E);

		// ST7735R Gamma Sequence
		writeCmd(0xe0);
		writeData(0x0f);
		writeData(0x1a);
		writeData(0x0f);
		writeData(0x18);
		writeData(0x2f);
		writeData(0x28);
		writeData(0x20);
		writeData(0x22);
		writeData(0x1f);
		writeData(0x1b);
		writeData(0x23);
		writeData(0x37);
		writeData(0x00);
		writeData(0x07);
		writeData(0x02);
		writeData(0x10);

		writeCmd(0xe1);
		writeData(0x0f);
		writeData(0x1b);
		writeData(0x0f);
		writeData(0x17);
		writeData(0x33);
		writeData(0x2c);
		writeData(0x29);
		writeData(0x2e);
		writeData(0x30);
		writeData(0x30);
		writeData(0x39);
		writeData(0x3f);
		writeData(0x00);
		writeData(0x07);
		writeData(0x03);
		writeData(0x10);

		writeCmd(0xF0); // Enable test command
		writeData(0x01);

		writeCmd(0xF6); // Disable ram power save mode
		writeData(0x00);

		// sleep out
		writeCmd(0x11);
		writeCmd(0x29);
	}

	void spiTransferColor(Color color) {
		//printf("%d, %d, %d\n", color.r, color.g, color.b);
		int rgb565 = color.to565();
		//printf(" %d\n", rgb565);
		byte left = (rgb565 >> 8) & 0xFF;
		byte right = rgb565 & 0xFF;
		spiTransferByte(left);
		spiTransferByte(right);

		//spiTransferByte(0b00000111);
		//spiTransferByte(0b11100000);
	}

	void spiWriteBuffer(Framebuffer fb) {
		for (int i = 0; i < 160 * 128; i++) {
			spiTransferColor(fb.pixels[(160 * 128 - 1) - i]);
		}
	}

	void show(Framebuffer fb) {
		writeCmd(0x2A);
		writeData(0x00);
		writeData(0x01);
		writeData(0x00);
		writeData(0xA0);

		writeCmd(0x2B);
		writeData(0x00);
		writeData(0x02);
		writeData(0x00);
		writeData(0x81);

		writeCmd(0x2C);

		gpio_write(cs, 1);
		gpio_write(dc, 1);
		gpio_write(cs, 0);

		spiWriteBuffer(fb);
		gpio_write(cs, 1);
	}

	void close() {
		spi_close(spi);
		spi_free(spi);

		gpio_close(dc);
		gpio_close(rst);
		gpio_close(mosi);
		gpio_close(sck);
		gpio_close(cs);

		gpio_free(dc);
		gpio_free(rst);
		gpio_free(mosi);
		gpio_free(sck);
		gpio_free(cs);
	}
};
