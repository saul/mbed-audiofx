/*
 *	HAPR Project 2014
 *	Group 6 - Tom Bryant (TB) & Saul Rennison (SR)
 *
 *	File created by:	TB
 *	File modified by:	TB
 *	File debugged by:	TB
 *
 * waves.c - Wave functions
 *
 * Defines functions for obtaining various wave values.
 */


#ifndef _WAVES_H_
#define _WAVES_H_

uint8_t get_square(uint8_t frequency);
float get_sawtooth(uint8_t frequency);
float get_inverse_sawtooth(uint8_t frequency);
float get_triangle(uint8_t frequency);

#endif