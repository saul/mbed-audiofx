#!/usr/bin/env python
import serial
import struct
import time
import glob
import os
import sys


__all__ = ['ProbePacket', 'ResetPacket', 'PrintPacket', 'FilterListPacket', 'SerialStream', 'PacketTypes', 'PACKET_MAP']

# little-endian "MBED"
PACKET_IDENT = ord('D') << 24 | ord('E') << 16 | ord('B') << 8 | ord('M')


def read_ascii_string(s, offset):
	"""Reads an ASCII string from `s` at offset `offset`.
	Returns a tuple of (str, new_offset), where `new_offset` is the index of
	character after the NULL byte.
	"""
	s = s[offset:]
	end = s.index('\x00')
	return s[:end].decode('ascii'), offset + end + 1


def determine_port():
	# On Linux, connect to /dev/ttyACM0 if it exists
	if sys.platform.startswith('linux'):
		if os.path.exists('/dev/ttyACM0'):
			return '/dev/ttyACM0'

	# If OSX, connect to the first usbmodem
	elif sys.platform == 'darwin':
		modems = glob.glob('/dev/tty.usbmodem*')

		if modems:
			assert len(modems) == 1
			return modems[0]

	# HACK: connect to COM3 on Windows, actual port may vary!
	elif sys.platform == 'win32':
		return 'COM3'

	# No idea... what platform is this?!
	else:
		raise RuntimeError('unknown platform')

	raise RuntimeError('unable to determine serial port')


def get_packet_for_type(type_):
	clss = filter(lambda cls: cls.type_ == type_, PACKET_MAP)
	assert len(clss) <= 1, 'Handler for packet type %s multiply defined' % type_
	return clss[0]


class PacketTypes(object):
	A2A_PROBE = 0
	U2B_RESET = 1
	B2U_PRINT = 2
	B2U_FILTER_LIST = 3
	U2B_FILTER_CREATE = 4
	U2B_FILTER_DELETE = 5
	U2B_FILTER_FLAG = 6
	U2B_FILTER_MOD = 7


class Packet(object):
	def __init__(self, stream):
		self.stream = stream

	def send(self, *args, **kwargs):
		"""Send this packet down the serial stream."""
		data = self.construct(*args, **kwargs)
		assert data is None or isinstance(data, bytes)
		self.stream.send_packet(self.type_, data)

	def construct(self):
		"""Construct the packet data."""
		raise NotImplementedError

	def receive(self, data):
		"""Called when a packet has been received."""
		raise NotImplementedError


class ProbePacket(Packet):
	type_ = PacketTypes.A2A_PROBE

	def receive(self, data):
		# When we receive a probe packet, send one back
		# This tells the board to proceed with startup
		self.send()

	def construct(self):
		return None


class ResetPacket(Packet):
	type_ = PacketTypes.U2B_RESET

	def construct(self):
		return None


class PrintPacket(Packet):
	type_ = PacketTypes.B2U_PRINT

	def receive(self, data):
		chars = struct.unpack('<%dc' % len(data), data)
		self.msg = ''.join(chars).decode('ascii')
		print self.msg,


class FilterListPacket(Packet):
	type_ = PacketTypes.B2U_FILTER_LIST

	def receive(self, data):
		HEADER_FORMAT = '<B'
		offset = 0
		num_filters = struct.unpack_from(HEADER_FORMAT, data, offset)[0]
		offset += struct.calcsize(HEADER_FORMAT)

		self.filters = []

		for i in range(num_filters):
			name, offset = read_ascii_string(data, offset)
			param_format, offset = read_ascii_string(data, offset)

			print 'Filter: %s, %s' % (name, param_format)
			self.filters.append({
				'name': name,
				'format': param_format,
			})


PACKET_MAP = [
	ProbePacket, # B2U_PROBE
	ResetPacket, # U2B_RESET
	PrintPacket, # B2U_PRINT
	FilterListPacket, # B2U_FILTER_LIST
	#FilterChainPacket, # U2B_FILTER_CHAIN
	#FilterModPacket, # U2B_FILTER_MOD
]


class SerialStream:
	def __init__(self, port=None, baudrate=9600):
		if port is None:
			port = determine_port()

		self.serial = serial.Serial(port, baudrate)
		self._read_buf = ''

		# Flush input
		print 'Waiting to flush serial buffer...'
		time.sleep(1)
		self.serial.flushInput()

		# Send probe packet so board will reboot/resume startup
		#ProbePacket(self).send()

	def read_packet(self):
		"""Reads a packet from the serial port, doesn't return until a packet
		has been completely read.
		"""
		ident = self.serial.read(4)

		if ident != 'MBED':
			print 'read_packet: invalid packet ident (%s)' % ident
			return

		# Read packet type
		pack_type = struct.unpack('<B', self.serial.read(1))[0]

		packet_cls = filter(lambda cls: cls.type_ == pack_type, PACKET_MAP)
		assert len(packet_cls) <= 1, 'multiply defined packet handler'

		if len(packet_cls) == 0:
			print 'read_packet: got unknown packet type (%d)' % pack_type
			return

		# Read packet data
		size = struct.unpack('<B', self.serial.read(1))[0]

		if size == 0:
			data = None
		else:
			data = self.serial.read(size)

		packet = packet_cls[0](self)

		# Debug packet receipt
		if packet.__class__ is not PrintPacket:
			print 'read_packet: received packet %s (size=%d,data=%r)' % (packet.__class__.__name__, size, data)

		try:
			packet.receive(data)
		except:
			print 'read_packet: exception occurred when parsing packet data:\n%r' % data
			raise

		return packet

	def send_packet(self, type_, data=None):
		cls = get_packet_for_type(type_)
		print 'send_packet:  sending %s...' % cls.__name__

		# Calculate payload size
		size = len(data) if data is not None else 0

		# Write header
		header = struct.pack('<LBB', PACKET_IDENT, type_, size)
		self.serial.write(header)

		# Write packet data to serial
		if data is not None:
			self.serial.write(data)


if __name__ == '__main__':
	serstr = SerialStream()

	# Just read packets indefinitely
	while True:
		serstr.read_packet()
