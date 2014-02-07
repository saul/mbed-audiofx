from ui import sercom

def main():
	stream = sercom.SerialStream()

	# Send a few probe packets
	print('Sending U2B_RESET...')
	sercom.ResetPacket(stream).send()

if __name__ == '__main__':
	main()
