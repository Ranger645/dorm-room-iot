#include "Arduino.h"
#include "Led_Grid.h"


void LedGrid::start_grid() {
	begin();
}

void LedGrid::update() {
	switch (strip_mode) {
		case 0:
			turn_off();
			break;
		case 1:
			solid_color();
			break;
		case 2:
			breathing_color();
			break;
		case 3:
			rainbow();
			break;
		case 4:
			rainbow_wave();
			break;
		case 5:
			sound_visualizer(vReal, fft_value_number);
			break;
		case 6:
			party_mode();
			break;
		case 7:
			text();
			break;
		case 8:
			time();
			break;
	}
	show();
}

void LedGrid::set_mode(int mode) {
	strip_mode = mode;
}

void LedGrid::turn_off() {
	// Assigning the rgb values to the led strip
	for (int i = 0; i < pixel_count; i++) {
		setPixelColor(i, 0, 0, 0);
	}
	strip_mode = 0;
	r = g = b = dr = dg = db = dbrightness = 0;
}

void LedGrid::setup_solid_color(int red, int green, int blue, int brightness) {
	r = red;
	g = green;
	b = blue;
	brightness_percentage = brightness;
	
	int i = 0;
	for (; i < pixel_count; i++) {
		setPixelColor(i, r * (brightness_percentage / 100.0), 
						 g * (brightness_percentage / 100.0), 
						 b * (brightness_percentage / 100.0));
	}
	show();
	set_mode(1);
}

void LedGrid::solid_color() {
	delay(50);
}

void LedGrid::setup_breathing_color(int red, int green, int blue, int speed) {
	setup_solid_color(red, green, blue, 100);
	animation_speed = speed;
	set_mode(2);
}

void LedGrid::breathing_color() {
	// Breathing...
	if (brightness_percentage >= 100) {
		brightness_percentage = 100;
		animation_speed *= -1;
	} else if (brightness_percentage <= 0) {
		brightness_percentage = 0;
		animation_speed *= -1;
	} else
		brightness_percentage += animation_speed;
	
	// Setting LED Colors.
	for (int i = 0; i < pixel_count; i++) {
		setPixelColor(i, r * (brightness_percentage / 100.0), 
						 g * (brightness_percentage / 100.0), 
						 b * (brightness_percentage / 100.0));
	}
	show();
}

void LedGrid::setup_rainbow(int brightness, int speed) {
	turn_off();
	brightness_percentage = brightness;
	animation_speed = speed;
	r = 255;
	g = 0;
	b = 0;
	dr = -animation_speed;
	dg = animation_speed;
	db = 0;
	set_mode(3);
}

void LedGrid::rainbow() {
	increment_rgb();
	for (int i = 0; i < pixel_count; i++) {
		setPixelColor(i, r * (brightness_percentage / 100.0), 
						 g * (brightness_percentage / 100.0), 
						 b * (brightness_percentage / 100.0));
	}
}

void LedGrid::setup_rainbow_wave(int brightness, int speed) {
	setup_rainbow(brightness, speed);
	set_mode(4);
}

void LedGrid::rainbow_wave() {
	increment_rgb();
	for (int x = WIDTH; x > 0; x--)
		for (int y = 0; y < HEIGHT; y++)
			set_grid_pixel_color(x, y, get_grid_pixel_color(x - 1, y));
	for (int y = 0; y < HEIGHT; y++)
		set_grid_pixel_color(0, y, r * (brightness_percentage / 100.0), 
								   g * (brightness_percentage / 100.0), 
								   b * (brightness_percentage / 100.0));
}

void LedGrid::setup_sound_visualizer(int brightness) {
	set_mode(5);
	brightness_percentage = brightness;
}

void LedGrid::sound_visualizer(double* bins, int bin_number) {
	turn_off();
	set_mode(5);
	
	// Getting the number of frequency bins per bar:
	int bin_bar_width = 1;
	if (bin_number > WIDTH)
		bin_bar_width = bin_number / WIDTH;
	
	// Combining the bins to fit the size of the sound visualizer.
	double new_bins[WIDTH];
	for (int i = 0; i < WIDTH; i++) {
		double bin_sum = 0;
		for (int n = 0; n < bin_bar_width; n++) {
			bin_sum += bins[i * bin_bar_width + n];
		}
		new_bins[i] = bin_sum / bin_bar_width;
		
		// Finding the maximum bin.
		if (new_bins[i] > max_bin)
			max_bin = (new_bins[i] + max_bin) / 2;
	}
	
	// Displaying bins:
	for (int x = 0; x < WIDTH; x++) {
		int column = (int) ((HEIGHT - 1) * (new_bins[x] / max_bin));
		alter_rgb_to_percent(100.0 * x / 20.0);
		
		for (int y = HEIGHT - 2; y >= HEIGHT - column; y--) {
			set_grid_pixel_color(x, y, r * (brightness_percentage / 100.0), 
									   g * (brightness_percentage / 100.0), 
									   b * (brightness_percentage / 100.0));
		}
		set_grid_pixel_color(x, HEIGHT - 1, r * (brightness_percentage / 100.0), 
											g * (brightness_percentage / 100.0), 
											b * (brightness_percentage / 100.0));
	}
	
	if (max_bin > 250)
		max_bin -= 10;
	
}

void LedGrid::setup_party_mode(int brightness, int speed) {
	set_mode(6);
	brightness_percentage = brightness;
	animation_speed = speed;
}

void LedGrid::party_mode() {
	int decider = 0;
	if (decider == 0) {
		r = random(0, 255);
		g = random(0, 255 - r);
		b = random(255 - r - g);
	}
	for (int i = 0; i < pixel_count; i++)
		setPixelColor(i, r * brightness_percentage / 100, g * brightness_percentage / 100, b * brightness_percentage / 100);
	delay(animation_speed);
}

void LedGrid::setup_text(String text_str, int red, int green, int blue, int brightness, int speed, bool rainbow) {
	set_mode(7);
	text_str += " ";
	display_string = text_str;
	brightness_percentage = brightness;
	animation_speed = speed;
	r = red;
	g = green;
	b = blue;
	display_string_index = 0;
	current_letter_column = 0;
}

void LedGrid::text() {
	shift_left();
	if (letter_space) {
		for (int i = 0; i < letter_pixel_height; i++) {
			set_grid_pixel_color(WIDTH - 1, i, 0, 0, 0);
		}
		letter_space = false;
	} else {
		for (int i = 0; i < letter_pixel_height; i++) {
			int bit_index = (((int) display_string[display_string_index]) - 32) * 55 + current_letter_column * 11 + i;
			int bit_number = bit_index % 8;
			int byte_index = bit_index / 8;
			if ((alphabet[byte_index] & (0x80 >> bit_number)) != 0)
				set_grid_pixel_color(WIDTH - 1, i, r * brightness_percentage / 100.0, g * brightness_percentage / 100.0, b * brightness_percentage / 100.0);
			else
				set_grid_pixel_color(WIDTH - 1, i, 0, 0, 0);
		}
		current_letter_column++;
		if (current_letter_column == letter_pixel_width) {
			current_letter_column = 0;
			display_string_index++;
			letter_space = true;
			if (display_string_index == display_string.length())
				display_string_index = 0;
		}
	}
}

void LedGrid::setup_time(int red, int green, int blue, int brightness, int speed, bool rainbow) {
	set_mode(8);
	brightness_percentage = brightness;
	animation_speed = speed;
	r = red;
	g = green;
	b = blue;
}

void LedGrid::time() {
	draw_time_number((display_hours / 10) % 10, 0);
	draw_time_number(display_hours % 10, 1);
	
	set_grid_pixel_color(9, 3);
	set_grid_pixel_color(9, 4);
	set_grid_pixel_color(10, 3);
	set_grid_pixel_color(10, 4);
	
	set_grid_pixel_color(9, 6);
	set_grid_pixel_color(9, 7);
	set_grid_pixel_color(10, 6);
	set_grid_pixel_color(10, 7);
	
	draw_time_number((display_minutes / 10) % 10, 2);
	draw_time_number(display_minutes % 10, 3);
}

void LedGrid::draw_time_number(int number, int slot) {
	int bit_index = number * 21;
	int bit_number = bit_index % 8;
	int byte_index = bit_index / 8;
	for (int x = 0; x < 3; x++) {
		for (int y = 0; y < 7; y++) {
			boolean draw = (small_numbers[byte_index] & (0x80 >> bit_number)) != 0;
			int addend = 1;
			if (slot > 1)
				addend += 3;
			int led_x = x + slot * 4 + addend;
			int led_y = y + 2;
			if (draw)
				set_grid_pixel_color(led_x, led_y);
			else
				set_grid_pixel_color(led_x, led_y, 0, 0, 0);
			bit_index++;
			bit_number = bit_index % 8;
			byte_index = bit_index / 8;
		}
	}
}

void LedGrid::shift_left() {
	for (int x = 0; x < WIDTH - 1; x++) 
		for (int y = 0; y < HEIGHT; y++)
			set_grid_pixel_color(x, y, get_grid_pixel_color(x + 1, y));
}

void LedGrid::set_time_to_display(int hours, int minutes, boolean AM) {
	display_hours = hours;
	display_minutes = minutes;
	display_am = AM;
}

void LedGrid::set_fft_raw_data(double *values, int value_number) {
	vReal = values;
	fft_value_number = value_number;
}

void LedGrid::set_grid_pixel_color(int x, int y, int red, int green, int blue) {
	if (y % 2 == 0) {
		x = WIDTH - x - 1;
	}
	y = HEIGHT - y - 1;
	int stripIndex = y * WIDTH + x;
	setPixelColor(stripIndex, red, green, blue);
}

void LedGrid::set_grid_pixel_color(int x, int y, int rgb) {
	if (y % 2 == 0) {
		x = WIDTH - x - 1;
	}
	y = HEIGHT - y - 1;
	int stripIndex = y * WIDTH + x;
	setPixelColor(stripIndex, rgb);
}

void LedGrid::set_grid_pixel_color(int x, int y) {
	set_grid_pixel_color(x, y, r * brightness_percentage / 100.0, g * brightness_percentage / 100.0, b * brightness_percentage / 100.0);
}

int LedGrid::get_grid_pixel_color(int x, int y) {
	if (y % 2 == 0) {
		x = WIDTH - x - 1;
	}
	y = HEIGHT - y - 1;
	int stripIndex = y * WIDTH + x;
	return getPixelColor(stripIndex);
}

void LedGrid::increment_rgb() {
	r += dr;
	g += dg;
	b += db;

	if (r >= 255) {
		dr = -animation_speed;
		dg = animation_speed;
		db = 0;
	}

	if (g >= 255) {
		dr = 0;
		dg = -animation_speed;
		db = animation_speed;
	}

	if (b >= 255) {
		dr = animation_speed;
		dg = 0;
		db = -animation_speed;
	}

	if (r < 0)
		r = 0;
	if (g < 0)
		g = 0;
	if (b < 0)
		b = 0;

	if (r > 255)
		r = 255;
	if (g > 255)
		g = 255;
	if (b > 255)
		b = 255;
}

void LedGrid::alter_rgb_to_percent(float percent) {
	if (percent >= 0 && percent <= 33) {
		g = (int) ((percent / 33.0) * 255.0);
		r = 255 - g;
	} else if (percent >= 33 && percent <= 66) {
		b = (int) (((percent - 33.0) / 33.0) * 255.0);
		g = 255 - b;
	} else if (percent >= 66 && percent <= 99) {
		r = (int) (((percent - 66.0) / 33.0) * 255.0);
		b = 255 - r;
	} else {
		r = 255;
		g = 0;
		b = 0;
	}
}
