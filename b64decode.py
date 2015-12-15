import sys,codecs
if len(sys.argv)!=3:
    sys.stderr.write('usasge: {} infile.b64 outfile.bin\n'.format(sys.argv[0]))
    sys.exit(1)
infile=open(sys.argv[1])
outfile=open(sys.argv[2],'wb')
for l in infile.readlines():
    outfile.write(codecs.decode(bytes(l,'ascii'),'base64'))
