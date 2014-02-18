# Setup path
import sys
import os
sys.path.append(os.path.join(os.path.abspath('.'), 'ui', 'Resources', 'py'))

import sercom

def main():
	stream = sercom.SerialStream()

	# Send a few probe packets
	print('Sending U2B_RESET...')
	sercom.ResetPacket(stream).send()

if __name__ == '__main__':
	main()
